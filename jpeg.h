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

#ifndef __JPEG_H__
#define __JPEG_H__
#include "image.h"

extern "C" {
#include <stdint.h>
#include <jpeglib.h>
}

struct jpeg : public image
{
	jpeg_decompress_struct jd;
	jpeg_error_mgr jerr;
	JSAMPIMAGE jpeg_rows;
    FILE *in_file;
    bool lowres; // if doing 8x reduction
                 // TODO: also support 4x/2x reduction

	jpeg(const char *file_path, int thumb_size);
	~jpeg();
	int refill();
};
#endif