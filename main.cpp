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

int main (int argc, char * const argv[]) {
	if (argc < 3) {
		printf("fastdownscale [input JPEG] [output JPEG] [largest dimension]\n");
		return 1;
	}
	
    FILE *f = fopen(argv[1], "rb");
    
    if (f) {
        image *thumb = resize_to(argv[1], f, atoi(argv[3]));
        
        if (thumb) {
            thumb->to_jpeg(argv[2], 95);
            delete thumb;
            
            return 0;
        }
    }
    return 1;
}
