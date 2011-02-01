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

#include "Image.h"

image::image(image *attributes, size_t width, size_t height, bool subsample_width, bool subsample_height)
{
	window_w = image_w = width;
	window_h = image_h = height;
	channels = attributes->channels;
	planar = attributes->planar;
	subsampled = attributes->subsampled || (subsample_width && subsample_height);
	YUV = attributes->YUV;
	allocate(subsample_width,subsample_height);
}

image::~image()
{
	for (int i = 0; i < planes(); i++) free(data[i]);
	free(data);
}

void image::allocate(bool subsample_width, bool subsample_height)
{
	data = (uint8_t**)malloc(planes()*sizeof(uint8_t*));
	for (int i = 0; i < planes(); i++) {
		size_t s = round_up_16(window_w) * round_up_16(window_h);
		if (i > 0 && (subsampled || subsample_width)) s /= 2;
		if (i > 0 && (subsampled || subsample_height)) s /= 2;
		
		data[i] = (uint8_t*)malloc(s);
	}
}

void image::to_raw(const char *fn, int plane)
{
	FILE *f = fopen(fn, "wb");
	size_t subsample = (subsampled && plane > 0) ? 2 : 1;
	printf("%d x %d -> %s\n",(window_w/subsample),(window_h/subsample),fn);
	fwrite(data[plane], (window_w/subsample)*(window_h/subsample)*(planar?1:channels), 1, f);
	fclose(f);
}