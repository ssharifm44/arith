/* 
 * 40image.c
 * Nick Dresens * Sam Miller
 * 03/05/2023
 *
 * COMP40 HW04 Arith *
 * 
 * Purpose: The purpose of this program is to provide a command line interface
 *          for users to compress and decompress images. The program reads 
 *          command line arguments and uses them to determine whether to 
 *          compress or decompress an image file. It then calls the appropriate
 *          function from the compress40.h and decompress40.h modules to 
 *          perform the desired operation.
 *
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "pnm.h"
#include "assert.h"
#include "compress40.h"
#include "bitpack.h"

/* Functions */
static void (*compress_or_decompress)(FILE *input) = compress40;
void checkForTrim(Pnm_ppm image);

int main(int argc, char *argv[])
{
        int i;
        A2Methods_T methods = uarray2_methods_plain;
        (void) methods;
        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0) {
                        compress_or_decompress = compress40;
                } else if (strcmp(argv[i], "-d") == 0) {
                        compress_or_decompress = decompress40;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n",
                                argv[0], argv[i]);
                        exit(1);
                } else if (argc - i > 2) {
                        fprintf(stderr, "Usage: %s -d [filename]\n"
                                "       %s -c [filename]\n",
                                argv[0], argv[0]);
                        exit(1);
                } else {
                        break;
                }
        }
        assert(argc - i <= 1); /* at most one file on command line */
        if (i < argc) {
                FILE *fp = fopen(argv[i], "r");
                assert(fp != NULL);
                compress_or_decompress(fp);
                fclose(fp);
        } else {
                compress_or_decompress(stdin);
        }

        return EXIT_SUCCESS; 
}