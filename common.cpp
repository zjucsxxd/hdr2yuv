// common routines 
// (doesn't require linking to library that is not supplied by C or C++ )

#include "hdr.h"

// convert multiplexed float to planar float

// DPX is in R,G,B multiplexed sample order,  not G,B,R   or planar

int muxed_dpx_to_planar_float_buf( t_pic *dst, int width, int height, float *src )
{
    for( int y = 0; y < height; y++ )
    {
        float *src_line = &(src[ y * width * 3 ]);
        
//        float *dstG =  &(;
//        float *dstR =  &(dst->fbuf[1][ y * width]);
//        float *dstB =  &(dst->fbuf[2][ y * width]);
        
        
        for( int x = 0; x < width; x++  )
        {
            float r = src_line[ x*3 + 0 ];
            float g = src_line[ x*3 + 1 ];
            float b = src_line[ x*3 + 2 ];
#if 0
            dst->fbuf[0][ y * width + x] = g;
            dst->fbuf[1][ y * width + x] = b;
            dst->fbuf[2][ y * width + x] = r;
#endif
        }
    }

  return 0;
}


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


