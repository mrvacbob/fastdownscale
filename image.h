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
	int image_w, image_h;   // size of input image
	int window_w, window_h; // size of image we need to store at once

	int channels;

    // image representation flags
	bool planar, interlaced, YUV, subsampled;

    bool subsampled_w, subsampled_h;

	uint8_t **data;

	bool valid;

	image() : valid(false) {}
	image(const char *file_path, int thumb_size);
	image(image *attributes, int width, int height, bool subsample_w = false, bool subsample_h = false);
	virtual ~image();

	void allocate();
	int num_planes() {return planar ? channels : 1;}
	virtual int refill() {return 0;}

	void  to_raw(const char *fn, int plane);
	void to_jpeg(const char *fn, float quality); // in jpeg.cpp
};

#endif