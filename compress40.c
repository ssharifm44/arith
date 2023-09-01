/*
 * compress40.c
 * Nick Dresens 
 * Sam Miller
 * 03/09/2023
 *
 * COMP40 HW4 - Arith 
 * 
 * Purpose: This is the implementation for our image compression and
 *          decompression program. It reads in a PPM image file, performs
 *          several steps to compress the image, and then outputs a compressed
 *          bit-packed representation of the image. The decompress40() function
 *          reads in a compressed image file, performs several steps to 
 *          decompress the image, and then outputs the original PPM image.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "assert.h"
#include "compress40.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "modules.h"
#include "uarray2.h"



/* reads PPM, writes compressed image */
void compress40(FILE *input) {
        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ppm image = Pnm_ppmread(input, methods);

        /* Step 1 */
        initialize_image(image);

        /* Step 2 */
        UArray2_T rgb_float = UArray2_new(image->width, image->height, sizeof(struct RGB_vals_float));
        rgb_unsigned_to_float(image->pixels, rgb_float, image->denominator);

        /* Step 3 */
        UArray2_T cvcs = UArray2_new(image->width, image->height, sizeof(struct CVCS_vals_float));
        rgb_float_to_cvcs(rgb_float, cvcs);

        /* Step 4 */
        UArray2_T dct_float = UArray2_new(image->width / 2, image->height / 2, sizeof(struct DCT_vals_float));
        cvcs_to_DCT(cvcs, dct_float);

        /* Step 5 */
        UArray2_T dct_quantized = UArray2_new(image->width / 2, image->height / 2, sizeof(struct DCT_vals_quantized));
        dct_to_quantized(dct_float, dct_quantized);

        /* Step 6 */
        UArray2_T bitpack = UArray2_new(image->width / 2, image->height / 2, sizeof(uint64_t));
        quantize_to_bitpack(dct_quantized, bitpack);

        /* Step 7 */
        // bitpack_to_file(bitpack);

        // DECOMPRESSION
        // undo step 7
        FILE *fp;
        fp = fopen("words", "r");
        UArray2_T bitpack_decompress;
        bitpack_decompress = file_to_bitpack(fp);
        fclose(fp);

        // undo step 6
        UArray2_T test5 = UArray2_new(image->width / 2, image->height / 2, sizeof(struct DCT_vals_quantized));
        bitpack_to_quantize(bitpack_decompress, test5);

        // undo step 5
        UArray2_T test4 = UArray2_new(image->width / 2, image->height / 2, sizeof(struct DCT_vals_float));
        quantized_to_dct(test5, test4);

        // undo step 4
        UArray2_T test3 = UArray2_new(image->width, image->height, sizeof(struct CVCS_vals_float));
        dct_to_cvcs(test4, test3);

        // undo step 3
        UArray2_T test2 = UArray2_new(image->width, image->height, sizeof(struct RGB_vals_float));
        cvcs_float_to_rgb(test3, test2);

        // undo step 2
        UArray2_T test = UArray2_new(image->width, image->height, sizeof(struct Pnm_rgb));
        rgb_float_to_unsigned(test2, test);

        // undo step 1
        printDecompressed(test, methods);

        /* cleanup */
        Pnm_ppmfree(&image);
        UArray2_free(&rgb_float);
        UArray2_free(&cvcs);
        UArray2_free(&dct_float);
        UArray2_free(&dct_quantized);
        UArray2_free(&bitpack);

        // testing cleanup
        UArray2_free(&test);
        UArray2_free(&test2);
        UArray2_free(&test3);
        UArray2_free(&test4);
        UArray2_free(&test5);
        UArray2_free(&bitpack_decompress);
}

/* reads compressed image, writes PPM */
void decompress40(FILE *input) {
        (void) input;
}