// common routines 
// (doesn't require linking to library that is not supplied by C or C++ )

#include <iostream>

#include "hdr.h"

// convert multiplexed float to planar float

// DPX is in R,G,B multiplexed sample order,  not G,B,R   or planar

int muxed_dpx_to_planar_float_buf( t_pic *dst, int width, int height, float *src )
{
    int pic_size = height*width;

    memcpy( dst->fbuf[2], &(src[ pic_size*0 ]), pic_size * sizeof( float)  ); // R
    memcpy( dst->fbuf[0], &(src[ pic_size*1 ]), pic_size * sizeof( float)  ); // G
    memcpy( dst->fbuf[1], &(src[ pic_size*2 ]), pic_size * sizeof( float)  ); // B
    

  return 0;
}



int planar_float_to_muxed_dpx_buf( float *dst, int width, int height, t_pic *src )
{
    int pic_size = height*width;

    memcpy( &(dst[ pic_size*0 ]), src->fbuf[2], pic_size * sizeof( float)  ); // R
    memcpy( &(dst[ pic_size*1 ]), src->fbuf[0], pic_size * sizeof( float)  ); // G
    memcpy( &(dst[ pic_size*2 ]), src->fbuf[1], pic_size * sizeof( float)  ); // B
    
    return 0;
}



#if 0
int planar_float_to_muxed_dpx_buf( float *dst, int width, int height, t_pic *src )
{
    for( int y = 0; y < height; y++ )
    {
        float *dst_line = &(dst[ y * width * 3 ]);
        float *srcG =  &(src->fbuf[0][ y * width]);
        float *srcR =  &(src->fbuf[1][ y * width]);
        float *srcB =  &(src->fbuf[2][ y * width]);
        
        
        for( int x = 0; x < width; x++  )
        {
            dst_line[ x*3 + 0 ] = srcR[x];
            dst_line[ x*3 + 1 ] = srcG[x];
            dst_line[ x*3 + 2 ] = srcB[x];
        }
    }
    
    return 0;
}
#endif


