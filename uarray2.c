/*
 * uarray2.c
 * Nick Dresens 
 * Sam Miller
 * 02/23/2023
 *
 * COMP40 HW4 - arith 
 * 
 * Purpose: The implementation of our uarray2 Program. This program was
 *          was provided to us. 
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "assert.h"
#include "mem.h"
#include "uarray.h"
#include "uarray2.h"

#define T UArray2_T

/* 
 * Element (i, j) in the world of ideas maps to
 * rows[j][i] where the square brackets stand for access
 * to a Hanson UArray_T
 */
struct T {
        int width, height;
        int size;
        UArray_T rows; /* UArray_T of 'height' UArray_Ts,
                          each of length 'width' and size 'size' */
};

static inline UArray_T row(T a, int j)
{
        UArray_T *prow = UArray_at(a->rows, j);   /* Ramsey idiom */
        return *prow;
}

static int is_ok(T a)
{
        return a && UArray_length(a->rows) == a->height &&
               UArray_size(a->rows) == sizeof(UArray_T) &&
               (a->height == 0 || (UArray_length(row(a, 0)) == a->width
                                   && UArray_size  (row(a, 0)) == a->size));
}

T UArray2_new(int width, int height, int size)
{
        int i;  /* interates over row number */
        T array;
        NEW(array);
        array->width  = width;
        array->height = height;
        array->size   = size;
        array->rows   = UArray_new(height, sizeof(UArray_T));
        for (i = 0; i < height; i++) {
                UArray_T *rowp = UArray_at(array->rows, i);
                *rowp = UArray_new(width, size);
        }
        assert(is_ok(array));
        return array;
}

void UArray2_free(T *array2)
{
        int i;
        assert(array2 != NULL && *array2 != NULL);
        for (i = 0; i < (*array2)->height; i++) {
                UArray_T p = row(*array2, i);
                UArray_free(&p);
        }
        UArray_free(&(*array2)->rows);
        FREE(*array2);
}

void *UArray2_at(T array2, int i, int j)
{
        assert(array2 != NULL);
        return UArray_at(row(array2, j), i);
}

int UArray2_height(T array2)
{
        assert(array2 != NULL);
        return array2->height;
}

int UArray2_width(T array2)
{
        assert(array2 != NULL);
        return array2->width;
}

int UArray2_size(T array2)
{
        assert(array2 != NULL);
        return array2->size;
}

void UArray2_map_row_major(T array2, 
                           void apply(int i, int j, T array2, 
                                      void *elem, void *cl), 
                           void *cl)
{
        assert(array2!= NULL);
        int h = array2->height;  /* keeping height and width in registers */
        int w = array2->width;   /* avoids extra memory traffic           */
        for (int j = 0; j < h; j++) {
                /* don't want row/UArray_at in inner loop */
                UArray_T thisrow = row(array2, j); 
                for (int i = 0; i < w; i++) 
                        apply(i, j, array2, UArray_at(thisrow, i), cl);
        }
}

void UArray2_map_col_major(T array2, 
                           void apply(int i, int j, T array2, 
                                      void *elem, void *cl), 
                           void *cl)
{
        assert(array2 != NULL);
        int h = array2->height;  /* keeping height and width in registers */
        int w = array2->width;   /* avoids extra memory traffic           */
        for (int i = 0; i < w; i++)
                for (int j = 0; j < h; j++)
                        apply(i, j, array2, UArray_at(row(array2, j), i), cl);
}