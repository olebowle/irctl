irctl
=====

irctl is a command-line utilty to query and set properties of common remote control receivers. At the moment only [IRMP_STM32](https://github.com/j1rie/IRMP_STM32) is supported.

## Compiling ##
```Shell
make
```

## Installing ##
```Shell
make install
```

This will install the binary irctl to the default location /usr/local.

You can override the location with the PREFIX variable. For packaging one might also want to use the DESTDIR variable.

```Shell
make DESTDIR="$pkgdir" PREFIX='/usr' install
```

## Commandline interface ##
```Shell
Usage:	irctl [-c] [-d driver] -g|-r  [-a|   -m M_SLOT -i I_SLOT|-w W_SLOT] device
	irctl [-c] [-d driver] -s ARG [-a|-e|-m M_SLOT -i I_SLOT|-w W_SLOT] device
	irctl [-c] [-d driver] -f firmware device

	-c, --caps		print capabilities of controller
	-d, --driver		driver to use to communicate to controller (default: stm32)
	-e, --emit		emit once
	-f, --flash=FW		flash firmware FW to controller
	-g, --get		get property
	-s, --set=ARG		set property to ARG
	-r, --reset		reset property
Properties:
	-a, --alarm		number of seconds to wakeup host from now
	-i, --ir=I_SLOT		ir-code in I_SLOT of macro M_SLOT
	-m, --macro=M_SLOT	select macro M_SLOT
	-w, --wakeup=W_SLOT	wakeup ir-code of W_SLOT

Examples (using driver stm32 on /dev/hidraw0):
	1. Get wakeup time in seconds from now
		./irctl -dstm32 -a -g /dev/hidraw0
	2. Set wakeup time to 1 hour from now
		./irctl -dstm32 -a -s3600 /dev/hidraw0
	3. Set first wakeup ir-code to 0x112233445566
		./irctl -dstm32 -w1 -s0x112233445566 /dev/hidraw0
	4. Send 0x112233445566 once
		./irctl -dstm32 -e -s0x112233445566 /dev/hidraw0
	5. Set third macro to be triggered by 0x112233445566,
	   which will when send out two commands (1: 0x778899AABBCC, 2: 0xDDEEFF001122)
		./irctl -dstm32 -m3 -i0 -s0x112233445566 /dev/hidraw0
		./irctl -dstm32 -m3 -i1 -s0x778899AABBCC /dev/hidraw0
		./irctl -dstm32 -m3 -i2 -s0xDDEEFF001122 /dev/hidraw0
		./irctl -dstm32 -m3 -i3 -r /dev/hidraw0

	These ir-codes can be retrieved from irmplircd and are based on the IRMP protocol.
```
