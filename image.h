/*
 *  Image.h
 *  fastdownscale
 *
 *  Created by Alexander Strange on 3/5/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "fastdownscale.h"

//memory use:
//input, temporary, output
//input: 1 row for png, 1 row for gif, 16 for jpeg, 2 for lowres jpeg
//temp: full size for interlaced input, otherwise enough to downscale into one output row
//output: full size rgb or subsampled yuv
//
//

struct image
{
	ssize_t image_w, image_h;
	
	//window_w = size of a line for all current files
	//window_h = above row counts
	ssize_t window_w, window_h;
	size_t channels;
	bool planar, subsampled, interlaced, YUV;
	
	uint8_t **data;
	
	bool valid;
	size_t last_row;
	
	image() : valid(true) {}
	image(FILE *from, size_t thumbsize);
	image(image *attributes, size_t width, size_t height, bool subsample_width = false, bool subsample_height = false);
	virtual ~image();
	
	void allocate(bool subsample_width = false, bool subsample_height = false);
	size_t planes() {return planar ? channels : 1;}
	virtual size_t refill() {return 0;}
	void to_raw(const char *fn, int plane);
	void to_jpeg(const char *fn, int quality);
};

#endif