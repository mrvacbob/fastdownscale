/*
 *  jpeg.cpp
 *  fastdownscale
 *
 *  Created by Alexander Strange on 3/5/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "jpeg.h"

jpeg::jpeg(FILE *f, size_t thumbsize)
{
	bool complicated = false;
	float factor;
	
	jd.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jd);
	jpeg_stdio_src(&jd, f);
	
	if (jpeg_read_header(&jd, true) != JPEG_HEADER_OK) goto err;
	
	window_w = image_w = jd.image_width;
	image_h = jd.image_height;
	window_h = 1;
	
	channels = 3;
	
	if (jd.num_components != 3 || jd.max_v_samp_factor > 2 || jd.max_h_samp_factor != jd.max_v_samp_factor) complicated = true;
	
	YUV = (jd.jpeg_color_space == JCS_YCbCr) && !complicated;
	if (YUV) {
		jd.raw_data_out = true;
		jd.out_color_space = JCS_YCbCr;
		window_h = jd.max_v_samp_factor*8;
		planar = true;
		subsampled = jd.max_v_samp_factor == 2;
	}
	
	jd.dct_method = JDCT_FASTEST;
	
	factor = (float)max(image_w,image_h) / (float)thumbsize;
	lowres = YUV && (factor > 8.);
		
	if (lowres) {
		jd.scale_denom = 8;
		jd.buffered_image = true;
		jd.do_block_smoothing = false;
		window_h = jd.max_v_samp_factor*(jd.min_DCT_h_scaled_size/8);
		image_h /= 8;
		window_w = image_w /= 8;
		factor = (float)max(image_w,image_h) / (float)thumbsize;
	}
	
	jpeg_start_decompress(&jd);
	allocate();
	
	if (YUV) {
		jpeg_rows = new JSAMPARRAY[3];
		for (int i = 0; i < 3; i++) {
			size_t subsample = (subsampled && i > 0) ? 2 : 1; 
			size_t rh = window_h / subsample;
			JSAMPARRAY a = new JSAMPROW[rh];
			
			if (lowres && subsample != 1) {
				jd.comp_info[i].downsampled_width /= 2;
				jd.comp_info[i].downsampled_height /= 2;
				jd.comp_info[i].DCT_h_scaled_size /= 2;
                jd.comp_info[i].DCT_v_scaled_size /= 2;
			}
			
			for (int j = 0; j < rh; j++) a[j] = &data[i][j * (window_w / subsample)];
			jpeg_rows[i] = a;
		}
	}
		
	if (lowres) jpeg_start_output(&jd, 1);
	return;
err:
	;
}

jpeg::~jpeg()
{
	jpeg_destroy_decompress(&jd);
	if (YUV) {
		for (int i = 0; i < 3; i++) {
			delete[] jpeg_rows[i];
		}
		delete[] jpeg_rows;
	}
}

size_t jpeg::refill()
{
	size_t s;
	if (YUV) {
		s = jpeg_read_raw_data(&jd, jpeg_rows, window_h);
		last_row += s;
	}
	return s;
}

void image::to_jpeg(const char *fn, int quality)
{
	FILE *f = fopen(fn, "wb");
	jpeg_error_mgr je;
	jpeg_compress_struct jc;
	
	jc.err = jpeg_std_error(&je);
	jpeg_create_compress(&jc);
	
	jpeg_stdio_dest(&jc, f);
	jc.image_width = window_w;
	jc.image_height = window_h;
	jc.input_components = channels;
	jc.in_color_space = YUV ? JCS_YCbCr : JCS_RGB;
	
	jpeg_set_defaults(&jc);
	
	if (YUV) {
		jc.raw_data_in = true;
		if (!subsampled) { // xxx get rid of this
			jc.comp_info[0].h_samp_factor /= 2;
			jc.comp_info[0].v_samp_factor /= 2;
		}
	}
	jc.optimize_coding = true;
	
	jpeg_set_quality(&jc, quality, true);
	jpeg_start_compress(&jc, true);
	
	if (planar)	{
		JSAMPARRAY image[3];
		
		for (int c = 0; c < 3; c++) {
			size_t subsample = (subsampled && c > 0) ? 2 : 1, height = round_up_16(window_h)/subsample, width = window_w/subsample; 
			image[c] = new JSAMPROW[height];
			
			for (int i = 0; i < height; i++) image[c][i] = &data[c][i*width];
		}
		
		int row=0;
		unsigned jpeg_height = subsampled ? round_up_16(window_h) : round_up_8(window_h);
		
		do {
			JSAMPARRAY rows[3];
			unsigned res;
			for (int c = 0; c < 3; c++) {
				size_t subsample = (subsampled && c > 0) ? 2 : 1;
				rows[c] = &image[c][row/subsample];
			}
			
			res = jpeg_write_raw_data(&jc, rows, jpeg_height - row);
			
			if (!res) break;
			row += res;
		} while (row < jpeg_height);
		
		for (int c = 0; c < 3; c++) delete[] image[c];
	}
	
	jpeg_finish_compress(&jc);
	jpeg_destroy_compress(&jc);
err:
	fclose(f);
}