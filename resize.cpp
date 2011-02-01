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

#include "resize.h"
#include "jpeg.h"
#include <math.h>

//box-filter downscale has no support
#define support .5f

static uint8_t clip8(int c)
{
	//c = max(c, 0);
	c = min(c, 255);
	return c;
}

template <unsigned channels>
static inline void average_img_pass(const uint8_t *src, uint8_t *dest, int src_w, int dest_w, int src_stride, int dest_stride, int h, int src_rb, int dest_rb, float factor)
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < dest_w; x++) {
			float center = (x + .5f) * factor,
                    left = center - (factor * support),
                   right = center + (factor * support);
            
			unsigned src_l = max((int)floorf(left), 0),
                     src_r = min((int)ceilf(right), src_w - 1),
             sample_length = max((src_r - src_l) + 1, 1);
            
            // FIXME: the above bits quite clearly seem to be nonsense
            
			int sums[channels] = {0};
			
			for (int sx = src_l; sx <= src_r; sx++) {
				for (int c = 0; c < channels; c++)
                    sums[c] += src[y*src_rb + sx*src_stride*channels + c];
			}

			for (int c = 0; c < channels; c++)
                dest[y*dest_rb + x*dest_stride*channels + c] = clip8((sums[c] + (sample_length>>1))/sample_length);
		}
	}
}

static int nearest_even(float f)
{
	int a = floor(f), b = ceil(f);
	
	return (b & 1) ? a : b;
}

image *resize_to(const char *fn, FILE *f, int max_size)
{
	image *src = new jpeg(f, max_size);
	float factor = max(src->image_w,src->image_h) / (float)max_size;
	int thumb_w = nearest_even(src->image_w/factor), thumb_h = nearest_even(src->image_h/factor);
	int reduce_chroma = !src->subsampled;
	image temp(src, thumb_w, src->image_h, reduce_chroma);
	
	if (!src->planar || src->interlaced) return NULL;
	
	int row = 0;

	do {
		int s = src->refill(); // XXX handle errors here
		
		for (int c = 0; c < src->channels; c++) {
			int subsample = c > 0 && src->subsampled;
			int chroma_subsample = c > 0 && reduce_chroma;
			int src_w = src->window_w >> subsample,
               dest_w = temp.window_w >> (subsample+chroma_subsample),
                    r = row >> subsample,
                    h = src->window_h>>subsample;

			average_img_pass<1>(src->data[c], temp.data[c] + r*dest_w, src_w, dest_w, 
								1, 1, h, src_w, dest_w, factor * (chroma_subsample+1));
		}
		row += src->window_h;
	} while (row < src->image_h);

	delete src;
	image *final = new image(&temp, thumb_w, thumb_h, reduce_chroma, reduce_chroma);
	
	for (int c = 0; c < final->channels; c++) {
		int subsample = (temp.subsampled && c > 0) ? 1 : 0;
		int chroma_subsample = (reduce_chroma && c > 0) ? 1 : 0;
		int dest_h = final->image_h>>(subsample+chroma_subsample),
             src_h = temp.image_h>>subsample,
                 w = final->image_w>>(subsample+chroma_subsample);

		average_img_pass<1>(temp.data[c], final->data[c], src_h, dest_h, w, w, w,
                            1, 1, factor * (chroma_subsample+1));
	}
	
	return final;
}