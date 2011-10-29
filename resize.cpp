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

#include <algorithm>
#include <math.h>

#include "resize.h"
#include "jpeg.h"
#include "utils.h"

// XXX: switchable filter here (blackman, gaussian, box)

#define support .5f

static uint8_t clip8(int c)
{
	c = std::max(c, 0);
	c = std::min(c, 255);
	return c;
}

template <unsigned channels>
static inline void resize_pass(const uint8_t *src,
                               uint8_t *dest,
                               int src_w,
                               int dest_w,
                               int src_xstride,
                               int dest_xstride,
                               int height, // src height = dest height
                               int src_ystride,
                               int dest_ystride,
                               float factor)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < dest_w; x++) {
			float center = (x + .5f) * factor,
                  left   = floorf(center - (factor * support)),
                  right  =  ceilf(center + (factor * support)),
                  length = right - left;

			int src_l = std::max((int)left, 0),
                src_r = std::min((int)right, src_w);

			int sums[channels] = {0};

			for (int sx = src_l; sx < src_r; sx++) {
				for (int c = 0; c < channels; c++)
                    sums[c] += src[y*src_ystride + sx*src_xstride + c];
			}

			for (int c = 0; c < channels; c++) {
                int sum = sums[c];
                float avg = sum / length;
                int yoff = y*dest_ystride;
                int xoff = x*dest_xstride;

                dest[yoff + xoff + c] = clip8(avg + .5f);
            }
		}
	}
}

void resize(const char *in_file, const char *out_file, int max_dimension, float quality)
{
    max_dimension = round_down(max_dimension, 2);
	image *src = new jpeg(in_file, max_dimension);
	float factor = std::max(src->image_w, src->image_h) / (float)max_dimension;

    check_fatal(factor > 1);

	int thumb_w = round_up(src->image_w/factor, 2), thumb_h = round_up(src->image_h/factor, 2);
    int reduce_chroma = src->YUV && src->planar; // dest chroma planes are subsampled, whether or not src are
	image *temp = new image(src, thumb_w, src->image_h, reduce_chroma, false);

    assert(src->planar && !src->interlaced);

	int row = 0;

    // horizontal scale

	do {
		int s = src->refill();
        assert(s == src->window_h);

		for (int c = 0; c < src->channels; c++) {
			bool src_subsampled  = c > 0 && src->subsampled,
                 dest_subsampled = c > 0 && reduce_chroma;
			int src_w  = src->window_w >> src_subsampled,
                dest_w = temp->window_w >> dest_subsampled,
                src_height = src->window_h >> src_subsampled,
                dest_row = row >> src_subsampled,
                extra_chroma_factor = 1 + (dest_subsampled && !src_subsampled);

            assert(src_height);
            assert(dest_w);

			resize_pass<1>(src->data[c],
                           temp->data[c] + dest_row*dest_w,
                            src_w,
                           dest_w,
								1,
                                1,
                       src_height,
                            src_w,
                           dest_w,
                           factor * extra_chroma_factor);
		}
		row += src->window_h;
	} while (row < src->image_h);

	delete src;

    image final(temp, thumb_w, thumb_h, reduce_chroma, reduce_chroma);

    for (int c = 0; c < final.channels; c++) {
        bool src_subsampled  = c > 0 && temp->subsampled_h,
             dest_subsampled = c > 0 && reduce_chroma;
        int src_h  = temp->window_h >> src_subsampled,
            dest_h = final.window_h >> dest_subsampled,
            src_w  = temp->window_w >> dest_subsampled, // note dest not src
            dest_w = final.window_w >> dest_subsampled,
            extra_chroma_factor = 1 + (dest_subsampled && !src_subsampled);

        resize_pass<1>(temp->data[c],
                       final.data[c],
                       src_h,
                       dest_h,
                       src_w,
                       dest_w,
                       src_w,
                       1,
                       1,
                       factor * extra_chroma_factor);
    }

    delete temp;
    final.to_jpeg(out_file, quality);
}