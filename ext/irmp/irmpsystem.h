/*------------------------------------------------------------------------------
 * irmpsystem.h - system specific includes and defines
 *
 * Copyright (c) 2009-2015 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *------------------------------------------------------------------------------
 */

#ifndef _IRMPSYSTEM_H_
#define _IRMPSYSTEM_H_

typedef struct __attribute__ ((__packed__)) {
	uint8_t		protocol;
	uint16_t	address;
	uint16_t	command;
	uint8_t		flags;
} IRMP_DATA;

#endif /* _IRMPSYSTEM_H_ */
