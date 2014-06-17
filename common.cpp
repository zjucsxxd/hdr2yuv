// common routines 
// (doesn't require linking to library that is not supplied by C or C++ )

#include <iostream>

#include "hdr.h"

// convert multiplexed float to planar float

// DPX is in R,G,B multiplexed sample order,  not G,B,R   or planar

int muxed_dpx_to_planar_float_buf( pic_t *dst, int width, int height, float *src )
{
    int pic_size = height*width;

    if( src == NULL || dst->init == 0 )
    {
        printf("ERROR, muxed_dpx_to_planar_float_buf(): unitilized picture buffers\n");
        exit(0);
    }
    
    memcpy( dst->fbuf[2], &(src[ pic_size*0 ]), pic_size * sizeof( float)  ); // R
    memcpy( dst->fbuf[0], &(src[ pic_size*1 ]), pic_size * sizeof( float)  ); // G
    memcpy( dst->fbuf[1], &(src[ pic_size*2 ]), pic_size * sizeof( float)  ); // B
    

  return 0;
}



int planar_float_to_muxed_dpx_buf( float *dst, int width, int height, pic_t *src )
{
    int pic_size = height*width;
    
    if( dst == NULL || src->init == 0 )
    {
        printf("ERROR, planar_float_to_muxed_dpx_buf(): unitilized picture buffers\n");
        exit(0);
    }
    
    memcpy( &(dst[ pic_size*0 ]), src->fbuf[2], pic_size * sizeof( float)  ); // R
    memcpy( &(dst[ pic_size*1 ]), src->fbuf[0], pic_size * sizeof( float)  ); // G
    memcpy( &(dst[ pic_size*2 ]), src->fbuf[1], pic_size * sizeof( float)  ); // B
    
    return 0;
}




// TODO: add all the t_pic attributes to the command line input
void copy_pic_vars( pic_t *dst, pic_t *src )
{
    dst->width = src->width;
    dst->height = src->height;
    dst->pic_buffer_type = src->pic_buffer_type;
    dst->chroma_format_idc = src->chroma_format_idc;
    dst->bit_depth = src->bit_depth;
    dst->name = src->name;
    dst->half_float_flag = src->half_float_flag;
    dst->pic_buffer_type = src->pic_buffer_type;
    
}

void pic_stats( pic_t *pic, pic_stats_t *stats, int print_stats )
{
    
    
    for( int cc = 0; cc< pic->n_components; cc++ )
    {
        float sum = 0.0;
        
        if( pic->pic_buffer_type == PIC_TYPE_U16 )
        {
            stats->i_min[cc] = std::numeric_limits<unsigned short>::max( );
            stats->i_max[cc] = std::numeric_limits<unsigned short>::min( );
            int size = pic->plane[cc].width  * pic->plane[cc].height;
            
            for( int i = 0; i < size; i++ )
            {
                unsigned short s = pic->buf[cc][ i];
                
                sum += (float) s;
                
                stats->i_min[cc] = s < stats->i_min[cc] ? s: stats->i_min[cc];
                stats->i_max[cc] = s > stats->i_max[cc] ? s: stats->i_max[cc];
            }
            
            float avg = sum / (float) size;
            
            
            
            int D = (1<<( pic->bit_depth - 8 ));
            int SMin = D*16;
            int YMax = 219*D + SMin;
            int CMax = 224*D + SMin;
            
            stats->estimated_floor[cc] = stats->i_min[cc];
            stats->estimated_ceiling[cc] = stats->i_max[cc];
            
            if( stats->estimated_ceiling[cc] < YMax &&  stats->estimated_ceiling[cc] > ((YMax*3)/4) )
                stats->estimated_ceiling[cc] = YMax;
            
            if( stats->estimated_ceiling[cc] < CMax &&  stats->estimated_ceiling[cc] > ((CMax*3)/4) )
                stats->estimated_ceiling[cc] = CMax;
            
            stats->i_avg[cc] = (int) avg;
            
            //     if( print_stats )
            {
                printf("pic_stats(%s)[c=%d]: min(%d) max(%d) avg(%f)\n",
                       pic->name, cc, stats->i_min[cc], stats->i_max[cc], avg );
            }
        }
        else if( pic->pic_buffer_type == PIC_TYPE_F32)
        {
            stats->f_min[cc] = std::numeric_limits<float>::max( );
            stats->f_max[cc] = std::numeric_limits<float>::min( );
            
            int size = pic->plane[cc].width  * pic->plane[cc].height;
            
            for( int i = 0; i < size; i++ )
            {
                float s = pic->fbuf[cc][ i];
                
                sum += s;
                
                stats->f_min[cc] = s < stats->f_min[cc] ? s: stats->f_min[cc];
                stats->f_max[cc] = s > stats->f_max[cc] ? s: stats->f_max[cc];
            }
            
            float avg = sum / (float) size;
            
            stats->estimated_floor[cc] = (int) stats->f_min[cc];
            stats->estimated_ceiling[cc] = (int) stats->f_max[cc];
            
            stats->f_avg[cc] = avg;
            
            // have to ratchet for floating point
#if 0
            // more sophisticated statistics
            for(int i=MIN_BIT_DEPTH;i<MAX_BIT_DEPTH;i++ )
            {
                int D = (1<<( i - 8 )) * 16;
                
                if( stats->i_min[cc] == D )
                    stats->estimated_floor[cc] = D;
                {
                    
                }
            }
#endif
            
  //          if( print_stats )
            {
                
                printf("pic_stats(%s)[c=%d]: min(%f) max(%f) avg(%f)\n",
                       pic->name, cc, stats->f_min[cc], stats->f_max[cc], avg );
            }
            
            
        }
    }
    
    stats->init = 1;
    
}



int init_pic( pic_t *pic, int width, int height, int chroma_format_idc, int bit_depth, int video_full_range_flag, int colour_primaries, int transfer_characteristics, int matrix_coeffs, int chroma_sample_loc_type, int sample_type, int half_float_flag, int verbose_level, const char *name )
{
    pic->width = width;
    pic->height = height;
    pic->pic_buffer_type = sample_type;
    pic->chroma_format_idc = chroma_format_idc;
    pic->bit_depth = bit_depth;
    pic->name = name;
    
    pic->video_full_range_flag = video_full_range_flag;
    pic->matrix_coeffs = matrix_coeffs;
    pic->transfer_characteristics = transfer_characteristics;
    pic->colour_primaries = colour_primaries;
    pic->chroma_sample_loc_type = chroma_sample_loc_type;
    
    pic->n_components = 3;
    
    memset( &(pic->stats), 0, sizeof( pic_stats_t) );
    
    pic->plane[0].width = pic->width;
    pic->plane[0].height = pic->height;
    
    int chroma_width_scale = pic->chroma_format_idc == CHROMA_444 ? 0 : 1;
    int chroma_height_scale = pic->chroma_format_idc == CHROMA_420 ? 1 : 0;
    
    pic->plane[1].width = pic->plane[2].width = pic->plane[0].width >> chroma_width_scale;
    pic->plane[1].height = pic->plane[2].height = pic->plane[0].height >> chroma_height_scale;
    
    set_pic_clip( pic );
    
    
#if 0
    if( verbose_level > 2 )
    {
        printf("init_pic(%s): width(%d) height(%d) buf_type(%d) chroma_format(%d) bit_depth(%d)\n", pic->name, pic->width, pic->height, pic->pic_buffer_type, pic->chroma_format_idc, pic->bit_depth );
        
    }
#endif
    
    if( verbose_level > 2 )
        printf("intializing picture (%s):  w(%d) h(%d) depth(%d) type(%d: %s) chroma_format_idc(%d) depth(%d)\n",
               pic->name, pic->width, pic->height, pic->bit_depth, pic->pic_buffer_type, pic->pic_buffer_type == PIC_TYPE_F32 ? "F32" : "U16", pic->chroma_format_idc, pic->bit_depth);
    
    if( width < 1 || width > 10000 || height < 1 || height > 10000 )
        printf("init_pic(%s) ERROR: picture dimensions( %d x %d) out of bounds\n ",name,  width, height );
    
    if( sample_type == PIC_TYPE_U16 )
    {
        int size = width * height * sizeof(unsigned short);
        
        pic->buf[0] = (unsigned short*) malloc(size);
        
        // TODO: allocate for 4:2:0 etc.
        pic->buf[1] = (unsigned short*) malloc(size);
        pic->buf[2] = (unsigned short*) malloc(size);
        
        pic->init = 1;
        pic->pic_buffer_type = PIC_TYPE_U16;
    }
    else if( sample_type == PIC_TYPE_F32 )
    {
        int size = width * height * sizeof(float);
        printf("allocating %d bytes to fbuf\n", size);
        
        pic->fbuf[0] = (float*) malloc(size);
        pic->fbuf[1] = (float*) malloc(size);
        pic->fbuf[2] = (float*) malloc(size);
        
        pic->init = 1;
        pic->pic_buffer_type = PIC_TYPE_F32;
        
    }
    else
    {
        pic->init = 0;
        
        printf("ERROR: init_pic(): sample_type(%d) not recognized. Exiting program\n", sample_type );
        exit(0);
    }
    
    return(0);
}



int deinit_pic( pic_t *pic )
{
    int errors= 0;
    
    if( pic->init )
    {
        if( pic->pic_buffer_type == PIC_TYPE_U16 )
        {
            for( int cc = 0; cc < 3; cc++ )
            {
                if( pic->buf[cc] ){
                    free( pic->buf[cc] );
                }
                else
                {
                    printf("WARNING: deinit_pic(%s) buf[%d] active but NULL\n", pic->name, cc );
                    errors++;
                }
            }
        }
        else if( pic->pic_buffer_type == PIC_TYPE_F32 )
        {
            for( int cc = 0; cc < 3; cc++ )
            {
                if( pic->fbuf[cc] ){
                    free( pic->fbuf[cc] );
                }
            	else
                {
                    printf("WARNING: deinit_pic(%s) fbuf[%d] active but NULL\n", pic->name, cc );
                    errors++;
                }
            }
            
        }
        else
        {
            printf("WARNING: deinit_pic(%s) pic_buffer_type(%d) not recognized\n", pic->name, pic->pic_buffer_type );
            errors++;
        }
        
    }
    
    return( errors );
    
}




void set_pic_clip( pic_t *pic )
{
    clip_limits_t *clip = &(pic->clip);
    
    clip->minCV = 0; //12 bits
    clip->maxCV = (1<< pic->bit_depth)-1;
    clip->Half = 1<<(pic->bit_depth -1);
    
    // Set up for video range if needed
    if( pic->video_full_range_flag == 0 ) {
        printf("Processing (%s) for Video Range\n", pic->name );
        unsigned short D = 1<<(pic->bit_depth - 8);
        clip->minVR = 16*D;
        clip->maxVR = 219*D+clip->minVR;
        clip->minVRC = clip->minVR;
        clip->maxVRC = 224*D+clip->minVRC;
        //achromatic point for chroma will be "Half"(e.g. 512 for 10 bits, 2048 for 12 bits etc..)
    } else {
        printf("Processing (%s) for full range\n", pic->name );
        clip->minVR = 0;
        clip->maxVR = clip->maxCV;
        clip->minVRC = 0;
        clip->maxVRC = clip->maxCV;
    }
    
    
    
}



