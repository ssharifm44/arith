#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "assert.h"
#include "compress40.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "uarray2.h"
#include "arith40.h"
#include "bitpack.h"

struct closure_u_to_f {
        UArray2_T rgb_float_vals;
        unsigned denominator;
};

struct RGB_vals_float {
        float red;
        float green;
        float blue;
};

struct CVCS_vals_float {
        float y;
        float pb;
        float pr;
};

struct DCT_vals_float {
        float a;
        float b;
        float c;
        float d;
        float avg_pb;
        float avg_pr;
};

struct DCT_vals_quantized {
        unsigned a;
        int b;
        int c;
        int d;
        unsigned avg_pb;
        unsigned avg_pr;
};

/* Compression Functions */
// step 1
extern void initialize_image(Pnm_ppm image);

// step 2
extern void rgb_unsigned_to_float(UArray2_T rgb_unsigned, UArray2_T rgb_float, unsigned denominator);
extern void rgb_int_to_float_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 3
extern void rgb_float_to_cvcs(UArray2_T rgb_float, UArray2_T cvcs);
extern void rgb_float_to_cvcs_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 4
extern void cvcs_to_DCT(UArray2_T cvcs, UArray2_T dct);
extern void cvcs_to_DCT_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 5
extern void dct_to_quantized(UArray2_T dct, UArray2_T cvcs);
extern void dct_to_quantized_apply(int i, int j, UArray2_T array2, void *elem, void *cl);
extern int  quantizer(float val);

// step 6
extern void quantize_to_bitpack(UArray2_T quantized, UArray2_T bitpack);
extern void quantize_to_bitpack_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 7
extern void bitpack_to_file(UArray2_T bitpack);
extern void bitpack_to_file_apply(int i, int j, UArray2_T array2, void *elem, void *cl);


/* Decompression Functions */
// step 1
extern UArray2_T file_to_bitpack(FILE *in);
extern void file_to_bitpack_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 2
extern void bitpack_to_quantize(UArray2_T bitpack, UArray2_T quantize);
extern void bitpack_to_quantize_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 3
extern void quantized_to_dct(UArray2_T quantized, UArray2_T dct);
extern void quantized_to_dct_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 4
extern void dct_to_cvcs(UArray2_T dct, UArray2_T cvcs);
extern void dct_to_cvcs_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 5
extern void cvcs_float_to_rgb(UArray2_T cvcs, UArray2_T rgb_float);
extern void cvcs_to_rgb_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 6
extern void rgb_float_to_unsigned(UArray2_T rgb_float, UArray2_T rgb_unsigned);
extern void rgb_float_to_int_apply(int i, int j, UArray2_T array2, void *elem, void *cl);

// step 7
extern void printDecompressed(UArray2_T final, A2Methods_T methods);