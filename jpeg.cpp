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

#include <limits.h>
#include <math.h>
#include <algorithm>

#include "jpeg.h"
#include "utils.h"

jpeg::jpeg(const char *file_path, int thumb_size)
{
	bool complicated;
	int factor;

    in_file = fopen(file_path, "rb");
    check_fatal(in_file, "couldn't open input");

	jd.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jd);
	jpeg_stdio_src(&jd, in_file);

    check_fatal(jpeg_read_header(&jd, true) == JPEG_HEADER_OK, "opening JPEG failed");
	check_fatal(jd.image_width < INT_MAX && jd.image_height < INT_MAX, "image too large");

	window_w = image_w = jd.image_width;
	image_h  = jd.image_height;
	window_h = 1;

	channels = 3;
    interlaced = false;

	// check if we can use a YUV->YUV fast path
    complicated = jd.num_components != 3 || jd.max_v_samp_factor > 2
               || jd.max_h_samp_factor != jd.max_v_samp_factor;
	YUV = !complicated && jd.jpeg_color_space == JCS_YCbCr;

	if (YUV) {
		jd.raw_data_out = true;
		jd.out_color_space = JCS_YCbCr;
		window_h = jd.max_v_samp_factor*8;
		planar = true;
		subsampled = jd.max_v_samp_factor == 2;
	} else {
        planar = false;
        subsampled = false;
    }

    subsampled_h = subsampled_w = subsampled;

	jd.dct_method = JDCT_FASTEST;

	factor = floor(std::max(image_w,image_h) / (float)thumb_size);
	lowres = YUV && (factor >= 8);

	if (lowres) {
        jd.scale_num   = 1;
		jd.scale_denom = 8;
		jd.buffered_image     = true;
		jd.do_block_smoothing = false;
		window_h = jd.max_v_samp_factor * (jd.min_DCT_h_scaled_size/8);
		image_h /= 8;
        image_w /= 8;
		window_w = image_w;

        assert(window_h == 1);
	}

	jpeg_start_decompress(&jd);
	allocate();

	if (YUV) {
		jpeg_rows = new JSAMPARRAY[3];
		for (int c = 0; c < 3; c++) {
			int plane_subsample = c > 0 && subsampled,
                 plane_window_h = window_h >> plane_subsample,
                 plane_window_w = window_w >> plane_subsample;
			JSAMPARRAY rows = new JSAMPROW[plane_window_h];

            assert(plane_window_h);

			if (lowres && plane_subsample) {
                assert(jd.comp_info[c].downsampled_width < jd.comp_info[0].downsampled_width);
			}

			for (int j = 0; j < plane_window_h; j++)
                rows[j] = &data[c][j * plane_window_w];
			jpeg_rows[c] = rows;
		}
	}

	if (lowres) jpeg_start_output(&jd, 1);
}

jpeg::~jpeg()
{
    fclose(in_file);
	jpeg_destroy_decompress(&jd);
	if (YUV) {
		for (int i = 0; i < 3; i++) {
			delete[] jpeg_rows[i];
		}
		delete[] jpeg_rows;
	}
}

int jpeg::refill()
{
	if (YUV) {
		int s = jpeg_read_raw_data(&jd, jpeg_rows, window_h);
        return s;
	}
    return 0;
}

void image::to_jpeg(const char *fn, float quality)
{
	FILE *f = fopen(fn, "wb");
	jpeg_error_mgr je;
	jpeg_compress_struct jc;

	jc.err = jpeg_std_error(&je);
	jpeg_create_compress(&jc);

	jpeg_stdio_dest(&jc, f);
	jc.image_width  = window_w;
	jc.image_height = window_h;
	jc.input_components = channels;
	jc.in_color_space = YUV ? JCS_YCbCr : JCS_RGB;

	jpeg_set_defaults(&jc);

	if (YUV) {
		jc.raw_data_in = true;
        jc.do_fancy_downsampling = false; // bug in libjpeg 8?
    }
	jc.optimize_coding = true;


	jpeg_set_quality(&jc, quality, true);
	jpeg_start_compress(&jc, true);

    assert(planar && YUV);
    assert(subsampled_h == subsampled_w);
    if (YUV && !subsampled) assert(0);

    int lines_per_imcu = jc.max_v_samp_factor * DCTSIZE;
    JSAMPARRAY image_rows[3];

    for (int c = 0; c < 3; c++) {
        int plane_subsample = c > 0 && subsampled;
        image_rows[c] = new JSAMPROW[lines_per_imcu >> plane_subsample];
    }

    for (int row = 0; row < window_h; ) {
        for (int c = 0; c < 3; c++) {
            int plane_subsample = c > 0 && subsampled,
                plane_remaining = (window_h - row) >> plane_subsample,
                plane_imcu_height = lines_per_imcu >> plane_subsample,
                plane_real_height = std::min(plane_remaining, plane_imcu_height),
                plane_width  = window_w >> plane_subsample,
                plane_row    = row >> plane_subsample,
                i = 0;
            JSAMPARRAY plane_rows = image_rows[c];

            if (!row) check_nonfatal(plane_width == round_up(plane_width, 8), "insufficient dct padding");

            for (int i = 0; i < plane_real_height; i++) {
                plane_rows[i] = &data[c][(i+plane_row)*plane_width];
            }
            for (i = plane_real_height; i < plane_imcu_height; i++) {
                plane_rows[i] = plane_rows[plane_real_height-1];
            }
        }

        int res = jpeg_write_raw_data(&jc, image_rows, lines_per_imcu);
        check_fatal(res > 0, "jpeg_write_raw_data error");

        row += res;
    }

	jpeg_finish_compress(&jc);
	jpeg_destroy_compress(&jc);

	fclose(f);
}