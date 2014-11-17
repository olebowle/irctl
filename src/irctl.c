#include <endian.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "capabilities.h"
#include "stm32.h"

/* no need to initialize, since global variables are initialized to zero */
struct global_args args;

static void
usage (char *name)
{
	fprintf(stderr,
		"Usage:	%s -d driver -g|-r  [-a|-i slot|-m slot|-w slot] device\n"
		"	%s -d driver -s ARG [-a|-i slot|-m slot|-w slot] device\n"
		"	%s -d driver [-c|-e IR|-f firmware] device\n\n"
		"	-c, --caps		get capabilities of controller\n"
		"	-d, --driver		driver to use to communicate to controller\n"
		"	-e, --emit=IR		IR-code to emit once\n"
		"	-f, --flash=FW		flash firmware FW to controller\n"
		"	-g, --get		get property\n"
		"	-s, --set=ARG		set property to ARG\n"
		"	-r, --reset		reset property\n", name, name, name);
	fprintf(stderr,
		"Properties:\n"
		"	-a, --alarm		number of seconds to wakeup host from now\n"
		"	-i, --ir=I_SLOT		ir-code in I_SLOT of macro M_SLOT\n"
		"	-m, --macro=M_SLOT	select macro M_SLOT\n"
		"	-w, --wakeup=W_SLOT	wakeup ir-code of W_SLOT\n\n");
	fprintf(stderr,
		"Examples (using driver stm32 on /dev/hidraw0):\n"
		"	1. Get wakeup time in seconds from now\n"
		"		%s -dstm32 -a -g /dev/hidraw0\n"
		"	2. Set first wakeup ir-code to 0x112233445566\n"
		"		%s -dstm32 -w0 -s0x112233445566 /dev/hidraw0\n"
		"	3. Set third macro to be triggered by 0x112233445566,\n"
		"	   which will when send out two commands (1: 0x778899AABBCC, 2: 0xDDEEFF001122)\n"
		"		%s -dstm32 -m2 -i0 -s0x112233445566 /dev/hidraw0\n"
		"		%s -dstm32 -m2 -i1 -s0x778899AABBCC /dev/hidraw0\n"
		"		%s -dstm32 -m2 -i2 -s0xDDEEFF001122 /dev/hidraw0\n", name, name, name, name, name);
	exit(EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
	int tmp;
	ssize_t ret = 0;
	uint8_t buf[256];
	struct rc_driver drv;
	extern char *optarg;
	struct option long_options[] = {
		{"alarm",	no_argument,	 	NULL,	'a'},
		{"caps",	no_argument, 		NULL,	'c'},
		{"driver",	required_argument,	NULL,	'd'},
		{"emit",	required_argument, 	NULL,	'e'},
		{"flash",	required_argument,	NULL,	'f'},
		{"get",		no_argument, 		NULL,	'g'},
		{"ir",		required_argument, 	NULL,	'i'},
		{"macro",	required_argument, 	NULL,	'm'},
		{"reset",	no_argument,	 	NULL,	'r'},
		{"set",		required_argument, 	NULL,	's'},
		{"wakeup",	required_argument, 	NULL,	'w'},
		{0, 0, 0, 0}
	};

	while ((tmp = getopt_long (argc, argv, "acd:e:f:gi:m:rs:w:", long_options, NULL)) != -1) {
		switch (tmp) {
		case 'a':
			args.cmd = CMD_ALARM;
			args.sub_cmd++;
			break;
		case 'c':
			args.cmd = CMD_CAPS;
			args.main_cmd++;
			break;
		case 'd':
			args.drv_name = optarg;
			break;
		case 'e':
			args.emit = optarg;
			args.cmd = CMD_EMIT;
			args.main_cmd++;
			break;
		case 'f':
			args.fw = optarg;
			args.cmd = CMD_FW;
			args.main_cmd++;
			break;
		case 'g':
			args.acc = ACC_GET;
			args.main_cmd++;
			break;
		case 'i':
			args.ir = optarg;
			break;
		case 'm':
			args.sub_arg = optarg;
			args.cmd = CMD_MACRO;
			args.sub_cmd++;
			break;
		case 'r':
			args.acc = ACC_RESET;
			args.main_cmd++;
			break;
		case 's':
			args.set = optarg;
			args.acc = ACC_SET;
			args.main_cmd++;
			break;
		case 'w':
			args.sub_arg = optarg;
			args.cmd = CMD_WAKE;
			args.sub_cmd++;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if (args.main_cmd != 1 || args.sub_cmd > 1 || (args.sub_cmd && !args.main_cmd) || (args.cmd == CMD_MACRO && !args.ir) || optind + 1 != argc || !args.drv_name)
		usage(argv[0]);

	args.path = argv[optind];

	memset(&drv, 0, sizeof(drv));
	if (!strcmp(args.drv_name, "stm32")) {
		drv.init = stm32_init;
		drv.open = stm32_open;
		drv.close = stm32_close;
		drv.prepare_buf = stm32_prepare_buf;
		drv.parse_buf = stm32_parse_buf;
		drv.read = stm32_read;
		drv.write = stm32_write;
	} else {
		fprintf(stderr, "unkown driver\n");
		exit(EXIT_FAILURE);
	}

	if (drv.init)
		drv.init(&drv.dev);

	if (!(*drv.dev.com_mat)[args.acc][args.cmd].rx) {
		fprintf(stderr, "operation not supported\n");
		exit(EXIT_FAILURE);
	}

	if (drv.prepare_buf) {
		ret = drv.prepare_buf(&drv.dev, buf, sizeof(buf));
		if (ret == -1)
			exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	printf("%lu bytes prepared in buffer\n", ret);
	printf("0x");
	for (tmp = 0; tmp < ret; tmp++)
		printf("%02x", buf[tmp]);
	printf("\n");
#endif /* DEBUG */

	if (drv.open)
		ret = drv.open(&drv.dev, args.path, O_RDWR);
		if (ret == -1)
			exit(EXIT_FAILURE);

	if (drv.write)
		ret = drv.write(&drv.dev, buf, sizeof(buf));
		if (ret == -1)
			exit(EXIT_FAILURE);

	if (drv.read)
		ret = drv.read(&drv.dev, buf, sizeof(buf));
		if (ret == -1)
			exit(EXIT_FAILURE);

#ifdef DEBUG
	printf("%lu bytes read\n", ret);
	printf("0x");
	for (tmp = 0; tmp < ret; tmp++)
		printf("%02x", buf[tmp]);
	printf("\n");
#endif /* DEBUG */

	if (drv.parse_buf)
		ret = drv.parse_buf(&drv.dev, buf, ret);
		if (ret != EXIT_SUCCESS)
			exit(ret);

	if (drv.close)
		ret = drv.close(&drv.dev);
		if (ret == -1)
			exit(EXIT_FAILURE);

	return EXIT_SUCCESS;
}
