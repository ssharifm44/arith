/*
 * ppmdiff.c
 * Nick Dresens 
 * Sam Miller
 * 03/09/2023
 *
 * COMP40 HW4 - Arith 
 * 
 * Purpose: This code reads two PPM images from files specified by command line
 *          arguments, checks if their dimensions differ by more than 1, 
 *          calculates the root mean squared difference (RMSD) between the two 
 *          images, and prints the result to the console. Essentially, this 
 *          program calculates the percent difference between two images.
 *
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"

typedef struct Pnm_rgb* RGB;
typedef A2Methods_UArray2* A2U2;

int main(int argc, char *argv[])
{
        (void) argc;
        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ppm image1;
        Pnm_ppm image2;
   
        FILE *fp1 = fopen(argv[1], "rb");
        FILE *fp2 = fopen(argv[2], "rb");
        if (fp1 == NULL || fp2 == NULL) {
                fprintf(stderr,
                        "Could not open one or both images for reading\n");
                exit(EXIT_FAILURE);
        }

        image1 = Pnm_ppmread(fp1, methods);
        image2 = Pnm_ppmread(fp2, methods);

        if (image1->height - image2->height > 1 ||
            (int)image1->height - (int)image2->height < -1 ||
            image1->width - image2->width > 1 ||
            (int)image1->width - (int)image2->width < -1) {
                fprintf(stderr, "width/height differs by more than 1\n");
                printf("1.0\n");
                exit(EXIT_FAILURE);
        }

        unsigned int width;
        if (image1->width > image2->width) {
                width = image2->width;
        } else {
                width = image1->width;
        }

        unsigned int height;
        if (image1->height > image2->height) {
                height = image2->height;
        } else {
                height = image1->height;
        }

        float sum = 0;

        float red1;
        float red2;
        float redDiff;

        float green1;
        float green2;
        float greenDiff;

        float blue1;
        float blue2;
        float blueDiff;

        float maxVal1 = (float)image1->denominator;
        float maxVal2 = (float)image2->denominator;

        for (unsigned int i = 0; i < width; i++) {
                for (unsigned int j = 0; j < height; j++) {
                        red1 = ((float)((RGB)methods->at((A2U2)image1->pixels, i, j))->red) / maxVal1;
                        red2 = (float)((RGB)methods->at((A2U2)image2->pixels, i, j))->red / maxVal2;
                        redDiff = red1 - red2;

                        green1 = (float)((RGB)methods->at((A2U2)image1->pixels, i, j))->green / maxVal1;
                        green2 = (float)((RGB)methods->at((A2U2)image2->pixels, i, j))->green /maxVal2;
                        greenDiff = green1 - green2;
                        
                        blue1 = (float)((RGB)methods->at((A2U2)image1->pixels, i, j))->blue / maxVal1;
                        blue2 = (float)((RGB)methods->at((A2U2)image2->pixels, i, j))->blue / maxVal2;
                        blueDiff = blue1 - blue2; 

                        sum += (redDiff * redDiff) + (greenDiff * greenDiff) + (blueDiff * blueDiff);
                }
        }

        float final = (float) sqrt(sum / (3 * (float)width * (float)height));
        printf("\nDifference = %f\n\n", final);
        
        fclose(fp1);
        fclose(fp2);

        Pnm_ppmfree(&image1);
        Pnm_ppmfree(&image2);
}