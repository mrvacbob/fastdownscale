/*
 * Copyright (c) 2008 Alexander Strange <astrange@ithinksw.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__
#include "fastdownscale.h"

//memory use:
//input, temporary, output
//input: 1 row for png, 1 row for gif, 16 for jpeg, 2 for lowres jpeg
//temp: full size for interlaced input, otherwise enough to downscale into one output row
//output: full size rgb or subsampled yuv

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