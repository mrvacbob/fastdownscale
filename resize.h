/*
 *  resize.h
 *  fastdownscale
 *
 *  Created by Alexander Strange on 3/5/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __RESIZE_H__
#define __RESIZE_H__
#include "image.h"

extern image *resize_to(const char *fn, FILE *f, size_t max_size);

#endif