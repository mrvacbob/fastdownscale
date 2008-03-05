/*
 *  fastdownscale.h
 *  fastdownscale
 *
 *  Created by Alexander Strange on 3/5/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __FASTDOWNSCALE_H__
#define __FASTDOWNSCALE_H__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>

#define max(a, b) ((a) > (b)) ? a : b
#define min(a, b) ((a) < (b)) ? a : b

static unsigned round_up(unsigned a, unsigned b)
{
	if (a % b == 0) return a;
	return ((a / b) + 1) * b;
}

#define round_up_16(a) round_up(a, 16)
#define round_up_8(a) round_up(a, 8)
#endif