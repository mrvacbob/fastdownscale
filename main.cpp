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
