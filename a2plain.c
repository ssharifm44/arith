/*
 * a2plain.c
 * Nick Dresens 
 * Sam Miller
 * 02/23/2023
 *
 * COMP40 HW4 - Arith 
 * 
 * Purpose: The implementation of our uarray2 Program. The purpose of this 
 *          program is to implement the A2Methods interface for a 
 *          two-dimensional array. The A2Methods interface is used by other 
 *          programs to manipulate two-dimensional arrays in a generic way, 
 *          without knowing the underlying implementation of the array. This 
 *          program provides a plain implementation of the A2Methods interface
 *          using the UArray2 module.
 *
 */


#include <string.h>
#include <assert.h>
#include <a2plain.h>
#include "uarray2.h"


typedef A2Methods_UArray2 A2;

static A2Methods_UArray2 new(int width, int height, int size)
{ 
        return UArray2_new(width, height, size);
}

static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

static void a2free(A2 * array2p)
{
        assert(array2p);
        UArray2_free((UArray2_T *)array2p);
}

static int width(A2 array2)
{
        assert(array2);
        return UArray2_width(array2);
}
static int height(A2 array2)
{
        assert(array2);
        return UArray2_height(array2);
}
static int size(A2 array2)
{
        assert(array2);
        return UArray2_size(array2);
}
static int blocksize(A2 array2)
{
        assert(array2);
        return 1;
}

static A2Methods_Object *at(A2 array2, int i, int j)
{
        assert(array2);
        assert(i >= 0 && j >= 0);
        return UArray2_at(array2, i, j);
}


static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply;
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        (void) i;
        (void) j;
        (void) uarray2;
        struct small_closure *cl = vcl;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,
        map_col_major,
        NULL,
        map_row_major,
        small_map_row_major,
        small_map_col_major,
        NULL,
        small_map_row_major,
};

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
