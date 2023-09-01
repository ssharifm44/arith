/*
 * change this. Are we make a new file for this? 
 *
 */
#include "modules.h"

typedef struct Pnm_rgb* RGB;
typedef A2Methods_UArray2* A2U2;

static const unsigned DECOMPRESSED_DENOMINATOR = 255;
static const unsigned QUANT_MULTIPLIER = 50;

/*

╭━━━╮
┃╭━╮┃
┃┃╱╰╋━━┳╮╭┳━━┳━┳━━┳━━┳━━┳┳━━┳━╮
┃┃╱╭┫╭╮┃╰╯┃╭╮┃╭┫┃━┫━━┫━━╋┫╭╮┃╭╮╮
┃╰━╯┃╰╯┃┃┃┃╰╯┃┃┃┃━╋━━┣━━┃┃╰╯┃┃┃┃
╰━━━┻━━┻┻┻┫╭━┻╯╰━━┻━━┻━━┻┻━━┻╯╰╯
╱╱╱╱╱╱╱╱╱╱┃┃
╱╱╱╱╱╱╱╱╱╱╰╯ 
*/

/*
* name:     initialize_image
* purpose:  Initializes an image by copying its pixels to a new, even-sized 
*           UArray2 and setting the image's pixels, width, and height to those
*           of the new UArray2.
* returns:  void
* inputs:   A Pnm_ppm image
* output:   The image is modified in place
*/
void initialize_image(Pnm_ppm image) {
        unsigned new_width = image->width;
        unsigned new_height = image->height;
        if (image->width % 2 != 0) {
                new_width--;
        }
        if (image->height % 2 != 0) {
                new_height--;
        }

        UArray2_T pixels_unsigned = UArray2_new(new_width, new_height, sizeof(struct Pnm_rgb));

        for (unsigned i = 0; i < new_width; i++) {
                for (unsigned j = 0; j < new_height; j++) {
                        Pnm_rgb new_pixel = malloc(sizeof(struct Pnm_rgb));
                        assert(new_pixel != NULL);

                        Pnm_rgb old = UArray2_at(image->pixels, i, j);
                        Pnm_rgb new = UArray2_at(pixels_unsigned, i, j);

                        new_pixel->red = old->red;
                        new_pixel->green = old->green;
                        new_pixel->blue = old->blue;

                        *new = *new_pixel;
                        free(new_pixel);
                }
        }

        UArray2_T p = image->pixels;
        UArray2_free(&p);

        image->pixels = pixels_unsigned;
        image->width = new_width;
        image->height = new_height;
}

/*
* name:     rgb_unsigned_to_float
* purpose:  Maps over an unsigned RGB UArray2 and applies rgb_int_to_float_appl
*           to each pixel, storing the resulting RGB_vals_float in a 
*           corresponding UArray2.
* returns:  void
* inputs:   An unsigned RGB UArray2, a float RGB UArray2, and the denominator
*           used for conversion
* output:   The resulting float RGB UArray2 is stored in the second argument
*/
void rgb_unsigned_to_float(UArray2_T rgb_unsigned, UArray2_T rgb_float, unsigned denominator) {
        struct closure_u_to_f c;
        c.rgb_float_vals = rgb_float;
        c.denominator = denominator;
        UArray2_map_row_major(rgb_unsigned, rgb_int_to_float_apply, &c);
}

/*
* name:     rgb_int_to_float_apply
* purpose:  Applies the formula to convert an unsigned RGB value to a float RGB 
*           value to a single pixel in a UArray2.
* returns:  void
* inputs:   The pixel's location, the UArray2 containing the pixel, and a 
*           struct closure_u_to_f containing the float RGB UArray2 and the 
*           denominator used for conversion
* output:   The resulting RGB_vals_float is stored in the float RGB UArray2
*/
void rgb_int_to_float_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct RGB_vals_float *output = malloc(sizeof(struct RGB_vals_float));
        assert(output != NULL);
       
        Pnm_rgb pixel = UArray2_at(array2, i, j);
        struct closure_u_to_f *rgb_str = cl;

        output->red = ((float)(pixel)->red) / (float)rgb_str->denominator;
        output->green = ((float)(pixel)->green) / (float)rgb_str->denominator;
        output->blue = ((float)(pixel)->blue) / (float)rgb_str->denominator;

        struct RGB_vals_float *spot = UArray2_at(rgb_str->rgb_float_vals, i, j);
        *spot = *output;
        free(output);
}

/*
* name:     rgb_float_to_cvcs
* purpose:  Maps over a float RGB UArray2 and applies rgb_float_to_cvcs_apply to 
*           each pixel, storing the resulting CVCS_vals_float in a corresponding
*           UArray2.
* returns:  void
* inputs:   A float RGB UArray2 and a CVCS UArray2
* output:   The resulting CVCS UArray2 is stored in the second argument
*/
void rgb_float_to_cvcs(UArray2_T rgb_float, UArray2_T cvcs) {
        UArray2_map_row_major(rgb_float, rgb_float_to_cvcs_apply, cvcs);
}

/*
* name:     rgb_float_to_cvcs_apply
* purpose:  Applies the formula to convert a float RGB value to a CVCS value
*           to a single pixel in a UArray2.
* returns:  void
* inputs:   The pixel's location, the UArray2 containing the pixel, and a 
*           pointer to the CVCS UArray2
* output:   The resulting CVCS_vals_float is stored in the CVCS UArray2
*/
void rgb_float_to_cvcs_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct RGB_vals_float *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;
        struct CVCS_vals_float *output = malloc(sizeof(struct CVCS_vals_float));
        assert(output != NULL);

        float r = pixel->red;
        float g = pixel->green;
        float b = pixel->blue;

        float y = 0.299 * r + 0.587 * g + 0.114 * b;
        float pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
        float pr =  0.5 * r - 0.418688 * g - 0.081312 * b;

        output->y = y;
        output->pb = pb;
        output->pr = pr;

        struct CVCS_vals_float *spot = UArray2_at(final, i, j);
        *spot = *output;
        free(output);
}

/*
* name:     cvcs_to_DCT
* purpose:  Transform each 2x2 block of the given CVCS image into a DCT block
* returns:  void
* inputs:   UArray2_T cvcs: 2D array of CVCS_vals_float representing the 
*           input image
*           UArray2_T dct:  2D array of DCT_vals_float to store the output
*           DCT blocks
* output:   N/A
*/
void cvcs_to_DCT(UArray2_T cvcs, UArray2_T dct) {
        UArray2_map_row_major(cvcs, cvcs_to_DCT_apply, dct);
}

/*
* name:     cvcs_to_DCT_apply
* purpose:  Apply the cvcs_to_DCT transformation to a single 2x2 block of 
*           the image
* returns:  void
* inputs:   int i: row index of the current block
*           int j: column index of the current block
*           UArray2_T array2: 2D array of CVCS_vals_float representing the 
*               input image
*           void *elem: current element of the input array (not used)
*           void *cl: 2D array of DCT_vals_float to store the output DCT blocks
* output:   N/A
*/
void cvcs_to_DCT_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        /* Only run on even rows and columns */
        if ((i % 2 != 0) || (j % 2 != 0)) {
                return;
        }
        (void) elem;

        UArray2_T final = cl;
        struct DCT_vals_float *output = malloc(sizeof(struct DCT_vals_float));
        assert(output != NULL);

        /* 1 2 */
        /* 3 4 */
        struct CVCS_vals_float *pixel_1 = UArray2_at(array2, i, j);
        struct CVCS_vals_float *pixel_2 = UArray2_at(array2, i + 1, j);
        struct CVCS_vals_float *pixel_3 = UArray2_at(array2, i, j + 1);
        struct CVCS_vals_float *pixel_4 = UArray2_at(array2, i + 1, j + 1);

        output->a = (pixel_1->y + pixel_2->y + pixel_3->y + pixel_4->y) / 4.0;
        output->b = (pixel_4->y + pixel_3->y - pixel_2->y - pixel_1->y) / 4.0;
        output->c = (pixel_4->y - pixel_3->y + pixel_2->y - pixel_1->y) / 4.0;
        output->d = (pixel_4->y - pixel_3->y - pixel_2->y + pixel_1->y) / 4.0;
        output->avg_pb = (pixel_1->pb + pixel_2->pb + pixel_3->pb + pixel_4->pb) / 4.0;
        output->avg_pr = (pixel_1->pr + pixel_2->pr + pixel_3->pr + pixel_4->pr) / 4.0;

        struct DCT_vals_float *spot = UArray2_at(final, i / 2, j / 2);
        *spot = *output;
        free(output);
}

/*
* name:     dct_to_quantized
* purpose:  Quantize each DCT block in the given array
* returns:  void
* inputs:   UArray2_T dct:  2D array of DCT_vals_float representing the input
*           DCT blocks
*           UArray2_T quantized: 2D array of DCT_vals_quantized to store the 
*           output quantized blocks
* output:   N/A
*/
void dct_to_quantized(UArray2_T dct, UArray2_T quantized) {
        UArray2_map_row_major(dct, dct_to_quantized_apply, quantized);
}

/*
* name:     dct_to_quantized_apply
* purpose:  Apply the dct_to_quantized transformation to a single DCT block
* returns:  void
* inputs:   int i: row index of the current DCT block
*           int j: column index of the current DCT block
*           UArray2_T array2: 2D array of DCT_vals_float representing the 
*               input DCT blocks
*           void *elem: current element of the input array (not used)
*           void *cl: 2D array of DCT_vals_quantized to store the output 
8               quantized blocks
* output:   N/A
*/
void dct_to_quantized_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct DCT_vals_float *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;
        struct DCT_vals_quantized *output = malloc(sizeof(struct DCT_vals_quantized));
        assert(output != NULL);

        unsigned a = pixel->a * 511;
        float decimal = pixel->a - (float)a;
        if (decimal >= 0.5) {
                a++;
        }

        int b = quantizer(pixel->b);
        int c = quantizer(pixel->c);
        int d = quantizer(pixel->d);

        unsigned pb = Arith40_index_of_chroma(pixel->avg_pb);
        unsigned pr = Arith40_index_of_chroma(pixel->avg_pr);

        output->a = a;
        output->b = b;
        output->c = c;
        output->d = d;
        output->avg_pb = pb;
        output->avg_pr = pr;

        struct DCT_vals_quantized *spot = UArray2_at(final, i, j);
        *spot = *output;
        free(output);
}

/*
* name:     quantizer
* purpose:  Quantizes a single DCT coefficient using the default 
*           compression scheme
* returns:  int: the quantized coefficient
* inputs:   float val: the DCT coefficient to be quantized
* output:   int: the quantized coefficient
*/
int quantizer(float val) {
        if (val > 0.3) {
                return 15;
        }
        if (val < -0.3) {
                return -15;
        }

        int final = val * QUANT_MULTIPLIER;
        float final_with_decimal = val * (float)QUANT_MULTIPLIER;
        float decimal = final_with_decimal - (float)final;

        if ((val >= 0) && (decimal >= 0.5)) {
                final++;
        }
        if ((val < 0) && (decimal <= -0.5)) {
                final--;
        }
        return final;
}

//////////////

/*
* name:     quantize_to_bitpack
* purpose:  Pack each quantized DCT block into a 64-bit word and store in 
*           the given array
* returns:  void
* inputs:   UArray2_T quantized: 2D array of DCT_vals_quantized representing
*           the input quantized blocks
*           UArray2_T bitpack:  2D array of 64-bit words to store the packed
*           quantized blocks
* output:   N/A
*/
void quantize_to_bitpack(UArray2_T quantized, UArray2_T bitpack) {
        UArray2_map_row_major(quantized, quantize_to_bitpack_apply, bitpack);
}

void quantize_to_bitpack_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct DCT_vals_quantized *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;
        uint64_t *word = malloc(sizeof(uint64_t));
        assert(word != NULL);

        unsigned a = pixel->a;
        int b = pixel->b;
        int c = pixel->c;
        int d = pixel->d;
        unsigned pb = pixel->avg_pb;
        unsigned pr = pixel->avg_pr;

        *word = 0;
        *word = Bitpack_newu(*word, 4, 0, pr);
        *word = Bitpack_newu(*word, 4, 4, pb);
        *word = Bitpack_news(*word, 5, 8, d);
        *word = Bitpack_news(*word, 5, 13, c);
        *word = Bitpack_news(*word, 5, 18, b);
        *word = Bitpack_newu(*word, 9, 23, a);

        uint64_t *spot = UArray2_at(final, i, j);
        *spot = *word;
        free(word);
}

/*
* name:     bitpack_to_file
* purpose:  Writes compressed image data to stdout in compressed image
*           format 2 (PPM format)
* returns:  void
* inputs:   a UArray2_T bitpack
* output:   Writes compressed image data to stdout
*/
void bitpack_to_file(UArray2_T bitpack) {
        unsigned width = 2 * UArray2_width(bitpack);
        unsigned height = 2 * UArray2_height(bitpack);
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
        UArray2_map_row_major(bitpack, bitpack_to_file_apply, NULL);
}

/*
* name: bitpack_to_file_apply
* purpose: apply function used by bitpack_to_file to write each pixel
*          to stdout in compressed image format 2
* returns: void
* inputs: 
*         - i: row index of the pixel in the UArray2_T
*         - j: column index of the pixel in the UArray2_T
*         - array2: the UArray2_T holding the pixel data
*         - elem: unused
*         - cl: unused
* output: writes a compressed pixel to stdout
*/
void bitpack_to_file_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        (void) cl;
        uint64_t *pixel = UArray2_at(array2, i, j);
        putchar(Bitpack_getu(*pixel, 8, 23));
        putchar(Bitpack_getu(*pixel, 8, 15));
        putchar(Bitpack_getu(*pixel, 8, 7));
        putchar(Bitpack_getu(*pixel, 8, 0));
}


/*
  ╭╮
  ┃┃
╭━╯┣━━┳━━┳━━┳╮╭┳━━┳━┳━━┳━━┳━━┳┳━━┳━╮
┃╭╮┃┃━┫╭━┫╭╮┃╰╯┃╭╮┃╭┫┃━┫━━┫━━╋┫╭╮┃╭╮╮
┃╰╯┃┃━┫╰━┫╰╯┃┃┃┃╰╯┃┃┃┃━╋━━┣━━┃┃╰╯┃┃┃┃
╰━━┻━━┻━━┻━━┻┻┻┫╭━┻╯╰━━┻━━┻━━┻┻━━┻╯╰╯
               ┃┃
               ╰╯
*/

/*
* name:     printDecompressed
* purpose:  prints the decompressed image data to stdout in PPM format
* returns:  void
* inputs: 
*           - final: a UArray2_T holding the decompressed pixel data
*           - methods: an A2Methods_T struct used to access the necessary
*             functions to convert between RGB and CVCS pixel data
* output:   prints the decompressed image data to stdout in PPM format
*/
void printDecompressed(UArray2_T final, A2Methods_T methods) {
        Pnm_ppm image;
        struct Pnm_ppm pp;
        image = &pp;
        image->pixels = final;
        image->height = UArray2_height(final);
        image->width = UArray2_width(final);
        image->denominator = DECOMPRESSED_DENOMINATOR;
        image->methods = methods;
        Pnm_ppmwrite(stdout, image);
}

/*
* name:     rgb_float_to_unsigned
* purpose:  converts a UArray2_T of RGB pixel data in float form to 
*           a UArray2_T of RGB pixel data in unsigned form
* returns:  void
* inputs: 
*           - rgb_float: a UArray2_T holding the RGB pixel data in float form
*           - rgb_unsigned: a UArray2_T holding the RGB pixel data in 
*                          unsigned form
* output:   None
*/
void rgb_float_to_unsigned(UArray2_T rgb_float, UArray2_T rgb_unsigned) {
        UArray2_map_row_major(rgb_float, rgb_float_to_int_apply, rgb_unsigned);
}

/*
* name:     rgb_float_to_int_apply
* purpose:  apply function used by rgb_float_to_unsigned to convert each
*           RGB pixel in float form to an RGB pixel in unsigned form
* returns:  void
* inputs: 
*           - i: row index of the pixel in the UArray2_T
*           - j: column index of the pixel in the UArray2_T
*           - array2: the UArray2_T holding the RGB pixel data in float form
*           - elem: unused
*           - cl: a UArray2_T holding the RGB pixel data in unsigned form
* output:   None
*/
void rgb_float_to_int_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct Pnm_rgb *vals = malloc(sizeof(struct Pnm_rgb));
        assert(vals != NULL);
        
        struct RGB_vals_float *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;
        
        float r = pixel->red * (float)DECOMPRESSED_DENOMINATOR;
        float g = pixel->green * (float)DECOMPRESSED_DENOMINATOR;
        float b = pixel->blue * (float)DECOMPRESSED_DENOMINATOR;

        if (r < 0) {
                r = 0;
        }
        if (g < 0) {
                g = 0;
        }
        if (b < 0) {
                b = 0;
        }

        vals->red = (unsigned)r;
        vals->green = (unsigned)g;
        vals->blue = (unsigned)b;
        
        if (vals->red > DECOMPRESSED_DENOMINATOR) {
                vals->red = DECOMPRESSED_DENOMINATOR;
        }
        if (vals->green > DECOMPRESSED_DENOMINATOR) {
                vals->green = DECOMPRESSED_DENOMINATOR;
        }
        if (vals->blue > DECOMPRESSED_DENOMINATOR) {
                vals->blue = DECOMPRESSED_DENOMINATOR;
        }

        struct Pnm_rgb *index = UArray2_at(final, i, j);
        *index = *vals;
        free(vals);
}

////////////////////////////////

/*
* name:     cvcs_float_to_rgb
* purpose:  converts a UArray2_T of CVCS pixel data in float form to 
*           a UArray2_T of RGB pixel data in float form
* returns:  void
* inputs: 
*           - cvcs: a UArray2_T holding the CVCS pixel data in float form
*           - rgb_float: a UArray2_T holding the RGB pixel data in 
*                       float form
* output:   None
*/
void cvcs_float_to_rgb(UArray2_T cvcs, UArray2_T rgb_float) {
        UArray2_map_row_major(cvcs, cvcs_to_rgb_apply, rgb_float);
}

/*
* name: cvcs_to_rgb_apply
* purpose: converts a CVCS pixel value to an RGB pixel value and writes the 
*          result to the output 2D array
* returns: void
* inputs:
*          - int i: the row index of the pixel in the input 2D array
*          - int j: the column index of the pixel in the input 2D array
*          - UArray2_T array2: the input 2D array containing CVCS pixel values
*          - void *elem: a pointer to the CVCS pixel value at position (i, j) in array2
*          - void *cl: a pointer to the output 2D array where the RGB pixel value will be written
* outputs: none
*/
void cvcs_to_rgb_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct CVCS_vals_float *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;

        struct RGB_vals_float *output = malloc(sizeof(struct RGB_vals_float));
        assert(output != NULL);
        
        float y = pixel->y;
        float pb = pixel->pb;
        float pr = pixel->pr;
        
        float r = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float b = 1.0 * y + 1.772 * pb + 0.0 * pr;

        output->red = r;
        output->green = g;
        output->blue = b;
        
        struct RGB_vals_float *spot = UArray2_at(final, i, j);
        *spot = *output;
        free(output);
}

///////////////////////////////

// step 4
/*
* name:     dct_to_cvcs
* purpose:  converts DCT pixel values to CVCS pixel values
* returns:  void
* inputs:
*           - UArray2_T dct: the 2D array containing the DCT pixel values
*           - UArray2_T cvcs: the 2D array to which the CVCS pixel values 
*             will be written
* outputs:  none
*/
void dct_to_cvcs(UArray2_T dct, UArray2_T cvcs) {
        UArray2_map_row_major(dct, dct_to_cvcs_apply, cvcs);
}

/*
* name:    dct_to_cvcs_apply
* purpose: Applies a discrete cosine transform (DCT) to color space (CVCS) 
*          conversion to a 2D array of DCT_vals_float structs at position (i,j).
* returns: void
* inputs:  int i - row index of the current element
*          int j - column index of the current element
*          UArray2_T array2 - 2D array of DCT_vals_float structs to be transformed
*          void *elem - unused element for mapping function
*          void *cl - 2D array of CVCS_vals_float structs to hold the 
*          transformed pixels
* output:  void
*/
void dct_to_cvcs_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct DCT_vals_float *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;

        struct CVCS_vals_float *output_1 = malloc(sizeof(struct CVCS_vals_float));
        struct CVCS_vals_float *output_2 = malloc(sizeof(struct CVCS_vals_float));
        struct CVCS_vals_float *output_3 = malloc(sizeof(struct CVCS_vals_float));
        struct CVCS_vals_float *output_4 = malloc(sizeof(struct CVCS_vals_float));

        assert(output_1 != NULL);
        assert(output_2 != NULL);
        assert(output_3 != NULL);
        assert(output_4 != NULL);
        
        /* 1 2 */
        /* 3 4 */
        output_1->y = pixel->a - pixel->b - pixel->c + pixel->d;
        output_2->y = pixel->a - pixel->b + pixel->c - pixel->d;
        output_3->y = pixel->a + pixel->b - pixel->c - pixel->d;
        output_4->y = pixel->a + pixel->b + pixel->c + pixel->d;
        
        output_1->pb = pixel->avg_pb;
        output_2->pb = pixel->avg_pb;
        output_3->pb = pixel->avg_pb;
        output_4->pb = pixel->avg_pb;

        output_1->pr = pixel->avg_pr;
        output_2->pr = pixel->avg_pr;
        output_3->pr = pixel->avg_pr;
        output_4->pr = pixel->avg_pr;
        
        struct CVCS_vals_float *spot_1 = UArray2_at(final, (i * 2), (j * 2));
        struct CVCS_vals_float *spot_2 = UArray2_at(final, (i * 2) + 1, (j * 2));
        struct CVCS_vals_float *spot_3 = UArray2_at(final, (i * 2), (j * 2) + 1);
        struct CVCS_vals_float *spot_4 = UArray2_at(final, (i * 2) + 1, (j * 2) + 1);

        *spot_1 = *output_1;
        *spot_2 = *output_2;
        *spot_3 = *output_3;
        *spot_4 = *output_4;

        free(output_1);
        free(output_2);
        free(output_3);
        free(output_4);
}

/////////////////////////////

/*
* name:     quantized_to_dct
* purpose:  Converts a 2D array of quantized values to a 2D array of DCT_vals_float structs.
* returns:  void
* inputs:   UArray2_T quantized - 2D array of quantized values to be converted
            UArray2_T dct - 2D array of DCT_vals_float structs to hold the converted values
* output:   void
*/
void quantized_to_dct(UArray2_T quantized, UArray2_T dct) {
        UArray2_map_row_major(quantized, quantized_to_dct_apply, dct);
}

/*
* name:     quantized_to_dct_apply
* purpose:  Applies a conversion from a quantized pixel to a DCT pixel to a 2D array at position (i,j).
* returns:  void
* inputs:   int i - row index of the current element
            int j - column index of the current element
            UArray2_T array2 - 2D array of quantized pixels to be converted
            void *elem - unused element for mapping function
            void *cl - 2D array of DCT_vals_float structs to hold the converted pixels
* output:   void
*/
void quantized_to_dct_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        struct DCT_vals_quantized *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;
        struct DCT_vals_float *output = malloc(sizeof(struct DCT_vals_float));
        assert(output != NULL);

        output->a = pixel->a / 511.0;
        output->b = pixel->b / (float)QUANT_MULTIPLIER;
        output->c = pixel->c / (float)QUANT_MULTIPLIER;
        output->d = pixel->d / (float)QUANT_MULTIPLIER;
        output->avg_pb = Arith40_chroma_of_index(pixel->avg_pb);
        output->avg_pr = Arith40_chroma_of_index(pixel->avg_pr);

        struct DCT_vals_float *spot = UArray2_at(final, i, j);
        *spot = *output;
        free(output);
}

/*
* name:     bitpack_to_quantize
* purpose:  Converts a 2D array of 64-bit integers in bitpacked format to a 2D array of DCT_vals_quantized structs.
* returns:  void
* inputs:   UArray2_T bitpack - 2D array of 64-bit integers in bitpacked format to be converted
            UArray2_T quantize - 2D array of DCT_vals_quantized structs to hold the converted values
* output:   void
*/
void bitpack_to_quantize(UArray2_T bitpack, UArray2_T quantize) {
        UArray2_map_row_major(bitpack, bitpack_to_quantize_apply, quantize);
}

/*
* name:     bitpack_to_quantize_apply
* purpose:  Applies a conversion from a bitpacked pixel to a quantized pixel to 
*           a 2D array at position (i,j).
* returns:  void
* inputs:   int i - row index of the current element
            int j - column index of the current element
            UArray2_T array2 - 2D array of bitpacked pixels to be converted
            void *elem - unused element for mapping function
            void *cl - 2D array of DCT_vals_quantized structs to hold the converted pixels
* output:   void
*/
void bitpack_to_quantize_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        uint64_t *pixel = UArray2_at(array2, i, j);
        UArray2_T final = cl;
        struct DCT_vals_quantized *output = malloc(sizeof(struct DCT_vals_quantized));
        assert(output != NULL);
        
        unsigned a = Bitpack_getu(*pixel, 9, 23);
        int b = Bitpack_gets(*pixel, 5, 18);
        int c = Bitpack_gets(*pixel, 5, 13);
        int d = Bitpack_gets(*pixel, 5, 8);
        unsigned pb = Bitpack_getu(*pixel, 4, 4);
        unsigned pr = Bitpack_getu(*pixel, 4, 0);
        
        output->a = a;
        output->b = b;
        output->c = c;
        output->d = d;
        output->avg_pb = pb;
        output->avg_pr = pr;

        struct DCT_vals_quantized *spot = UArray2_at(final, i, j);
        *spot = *output;
        free(output);
}

/*
* name:     file_to_bitpack
* purpose:  Read in a compressed image file and convert it to a 2D array 
*           of 64-bit words (bitpack)
* returns:  UArray2_T - a 2D array of 64-bit words (bitpack)
* inputs:   FILE *in - a file pointer to the compressed image file
* output:   N/A
*/
UArray2_T file_to_bitpack(FILE *in) {
        unsigned height, width;
        int read = fscanf(in, "COMP40 Compressed image format 2\n%u %u", &width, &height);
        assert(read == 2);
        int c = getc(in);
        assert(c == '\n');

        UArray2_T bitpack = UArray2_new(width, height, sizeof(uint64_t));
        UArray2_map_row_major(bitpack, file_to_bitpack_apply, in);

        return bitpack;
}

/*
* name:     file_to_bitpack_apply
* purpose:  Convert four 8-bit characters read from a compressed image file
*           into a 64-bit word (bitpack)
* returns:  void
* inputs:   int i - the row index of the element in the 2D array
*           int j - the column index of the element in the 2D array
*           UArray2_T array2 - a pointer to the 2D array
*           void *elem - a pointer to the element being processed
*           void *cl - a pointer to a file pointer to the compressed image file
* output:   N/A
*/
void file_to_bitpack_apply(int i, int j, UArray2_T array2, void *elem, void *cl) {
        (void) elem;
        uint64_t *pixel = UArray2_at(array2, i, j);
        FILE *in = cl;
        uint64_t word = 0;

        // char c1 = getc(in);
        // char c2 = getc(in);
        // char c3 = getc(in);
        // char c4 = getc(in);

        word = Bitpack_newu(word, 8, 23, getc(in));
        word = Bitpack_newu(word, 8, 15, getc(in));
        word = Bitpack_newu(word, 8, 7, getc(in));
        word = Bitpack_newu(word, 8, 0, getc(in));

        *pixel = word;
}