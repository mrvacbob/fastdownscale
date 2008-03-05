/*
 *  jpeg.h
 *  fastdownscale
 *
 *  Created by Alexander Strange on 3/5/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __JPEG_H__
#define __JPEG_H__
#include "image.h"
extern "C" {
#include <jpeglib.h>
}

struct jpeg : public image
{
	jpeg_decompress_struct jd;
	jpeg_error_mgr jerr;
	bool lowres;
	JSAMPIMAGE jpeg_rows;
	
	jpeg(FILE *, size_t);
	~jpeg();
	size_t refill();
};
#endif