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

#include <stdio.h>
#include <limits.h>

#include "image.h"
#include "utils.h"

image::image(image *attributes, int width, int height, bool subsample_w, bool subsample_h)
{
    window_w = image_w  = width;
	window_h = image_h  = height;
    check_fatal(round_up(window_h, 8) < INT_MAX && round_up(window_w, 8) < INT_MAX, "created image size too large");
	channels = attributes->channels;
	planar   = attributes->planar;
    subsampled_w = attributes->subsampled_w || subsample_w;
    subsampled_h = attributes->subsampled_h || subsample_h;
    subsampled = subsampled_w && subsampled_h;
	YUV      = attributes->YUV;
	allocate();
}

image::~image()
{
	for (int i = 0; i < num_planes(); i++)
        delete[] data[i];
	delete[] data;
}

void image::allocate()
{
    data = new uint8_t*[num_planes()];

	for (int i = 0; i < num_planes(); i++) {
        bool subsample_w = i > 0 && subsampled_w;
        bool subsample_h = i > 0 && subsampled_h;
		int s = (window_w >> subsample_w) * (window_h >> subsample_h);

		data[i] = new uint8_t[s + 16];
	}
}

void image::to_raw(const char *fn, int plane)
{
	FILE *f = fopen(fn, "wb");
    bool subsample_w = plane > 0 && subsampled_w;
    bool subsample_h = plane > 0 && subsampled_h;

	fwrite(data[plane], (window_w>>subsample_w)*(window_h>>subsample_h)*(planar?1:channels), 1, f);
	fclose(f);
}