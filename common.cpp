// common routines 
// (doesn't require linking to library that is not supplied by C or C++ )

#include <iostream>

#include "hdr.h"

// convert multiplexed float to planar float

// DPX is in R,G,B multiplexed sample order,  not G,B,R   or planar

int muxed_dpx_to_planar_float_buf( pic_t *dst, int width, int height, float *src )
{
    int pic_size = height*width;

    memcpy( dst->fbuf[2], &(src[ pic_size*0 ]), pic_size * sizeof( float)  ); // R
    memcpy( dst->fbuf[0], &(src[ pic_size*1 ]), pic_size * sizeof( float)  ); // G
    memcpy( dst->fbuf[1], &(src[ pic_size*2 ]), pic_size * sizeof( float)  ); // B
    

  return 0;
}



int planar_float_to_muxed_dpx_buf( float *dst, int width, int height, pic_t *src )
{
    int pic_size = height*width;

    memcpy( &(dst[ pic_size*0 ]), src->fbuf[2], pic_size * sizeof( float)  ); // R
    memcpy( &(dst[ pic_size*1 ]), src->fbuf[0], pic_size * sizeof( float)  ); // G
    memcpy( &(dst[ pic_size*2 ]), src->fbuf[1], pic_size * sizeof( float)  ); // B
    
    return 0;
}




