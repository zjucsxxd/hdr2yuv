// main() hdr2yuv.cpp
//
// at this level, there are no tiff or exr specific data structs or other 
// libraries specific to these formats since they tend to conflict with 
// each other.  Only the separate .o files contain library specific references.

// list of subroutine proto-types only 
#include "hdr.h"

#include <iostream>
#include <stdexcept>

#include <math.h>

static inline char *get_filename_extension( char *filename )
{
    char *ext = filename + strlen( filename );
    while( *ext != '.' && ext > filename )
        ext--;
    ext += *ext == '.';
    return ext;
}

void print_help( void )
{
    int n = sizeof( transfer_types_table ) / sizeof( transfer_types_table[0]);
    
    printf("Transfer chracteristics options:\n");
    
    for(int idx=0; idx<n; idx++ )
    {
        printf("%d: %s %s\n" ,idx, transfer_types_table[idx].name,
            transfer_types_table[idx].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)");
    }

    n = sizeof( input_file_types ) / sizeof( input_file_types[0]);
            
    printf("input file types:\n");
    
    for(int idx=0; idx<n; idx++ )
    {
        printf("%d: %s %s\n", idx, input_file_types[idx].name,
            input_file_types[ idx].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)");
    }
    
    printf("output file types:\n");
               
    for(int idx=0; idx<n; idx++ )
    {
        printf("%d: %s %s\n" ,idx, output_file_types[idx].name,
            output_file_types[ idx].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)");
    }
    
}

int parse_options( hdr_t *h, int argc,  char *argv[] )
{
    user_args_t *par = &(h->user_args);
    int arg_errors =0;

    h->out_pic.chroma_format_idc = h->out_pic.chroma_sample_loc_type = h->out_pic.video_full_range_flag = -1;
    h->out_pic.transfer_characteristics = h->out_pic.matrix_coeffs = h->out_pic.colour_primaries = -1;
    
    int src_filename_parsed = 0;
    int dst_filename_parsed = 0;
    
    // default values overriden first by parse_options() then read_tiff() or exr
 
    // TODO: use C++ map
    for( int i=1; i < argc; i++)
    {
        if( !strcmp( argv[i], "--src_filename") && (i<argc) )
        {
            par->src_filename = argv[i+1];
            src_filename_parsed++;
            i++;
        }
        else if( !strcmp( argv[i], "--help") && (i<argc) )
        {
            print_help();
        }
        else if( !strcmp( argv[i], "--dst_filename") && (i<argc) )
        {
            par->dst_filename = argv[i+1];
            dst_filename_parsed++;
            i++;
        }
        else if( !strcmp( argv[i], "--src_pic_width") && (i<argc) )
        {
            h->in_pic.width = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_pic_height") && (i<argc) )
        {
            h->in_pic.height = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_pic_width") && (i<argc) )
        {
            h->out_pic.width = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_pic_height") && (i<argc) )
        {
            h->out_pic.height = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--src_bit_depth") && (i<argc) )
        {
            h->in_pic.bit_depth = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--dst_bit_depth") && (i<argc) )
        {
            h->out_pic.bit_depth = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--dst_chroma_format_idc") && (i<argc) )
        {
            h->out_pic.chroma_format_idc = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--src_chroma_format_idc") && (i<argc) )
        {
            h->in_pic.chroma_format_idc = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--src_start_frame") && (i<argc) )
        {
            par->src_start_frame = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--n_frames") && (i < argc) )
        {
            par->n_frames = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--verbose_level") && (i < argc) )
        {
            par->verbose_level = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_colour_primaries") && (i < argc) )
        {
            h->in_pic.colour_primaries = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_colour_primaries") && (i < argc) )
        {
            h->out_pic.colour_primaries = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_matrix_coeffs") && (i < argc) )
        {
            h->in_pic.matrix_coeffs = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_matrix_coeffs") && (i < argc) )
        {
            h->out_pic.matrix_coeffs = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_transfer_characteristics") && (i < argc) )
        {
            char *val = argv[i+1];
            int val_int;
            
            if( sscanf(val, "%d", &val_int) != 0 )
                h->in_pic.transfer_characteristics = val_int;
            {
                int n = sizeof( transfer_types_table ) / sizeof( transfer_types_table[0]);
                
                // wish there was content-addressible tables in C
                for(int idx=0; idx<n; idx++ )
                {
                    if( !strcasecmp( val, transfer_types_table[i].name ))
                        h->in_pic.transfer_characteristics = transfer_types_table[i].idx;
                }
            }
            
            i++;
        }
        else if( !strcmp( argv[i], "--dst_transfer_characteristics") && (i < argc) )
        {
            char *val = argv[i+1];
            int val_int;
            
            if( sscanf(val, "%d", &val_int) != 0 )
                h->out_pic.transfer_characteristics = val_int;
            {
                int n = sizeof( transfer_types_table ) / sizeof( transfer_types_table[0]);
                
                // wish there was content-addressible tables in C
                for(int idx=0; idx<n; idx++ )
                {
                    if( !strcasecmp( val, transfer_types_table[i].name ))
                        h->out_pic.transfer_characteristics = transfer_types_table[i].idx;
                }
            }
            
            i++;
        }
        else if( !strcmp( argv[i], "--alpha_channel") && (i < argc) )
        {
            par->alpha_channel = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_video_full_range_flag") && (i < argc) )
        {
            h->out_pic.video_full_range_flag = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_video_full_range_flag") && (i < argc) )
        {
            h->in_pic.video_full_range_flag = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--cutout_hd") && (i < argc) )
        {
            par->cutout_hd = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--cutout_qhd") && (i < argc) )
        {
            par->cutout_qhd = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--chroma_resampler_type") && (i < argc) )
        {
            par->chroma_resampler_type = atoi( argv[i+1] );
            i++;
        }
        else
            printf("WARNING: argument (%s) unrecongized\n", argv[i] );
        //    arg_errors++;
        //    i++;
        //}
        
    }
 
    if( h->out_pic.bit_depth == 0 ){
        h->out_pic.bit_depth = h->in_pic.bit_depth;
        if(par->verbose_level>1)
            printf("output picture bit_depth not specified.  using input picture bit_depth(%d)\n", h->in_pic.bit_depth );
    }

    if( h->out_pic.width == 0 ){
        h->out_pic.width = h->in_pic.width;
        if(par->verbose_level>1)
            printf("output picture width not specified.  using input picture width(%d)\n", h->in_pic.width );
    }
 
    if( h->out_pic.height == 0 ){
        h->out_pic.height = h->in_pic.height;
        if(par->verbose_level>1)
            printf("output picture width not specified.  using input picture height(%d)\n", h->in_pic.height );
    }

    if( h->out_pic.chroma_format_idc == -1 ){
        h->out_pic.chroma_format_idc = h->in_pic.chroma_format_idc;
        if(par->verbose_level>1)
            printf("output picture chroma_format_idc not specified.  using input picture chroma_format_idc(%d)\n", h->in_pic.chroma_format_idc );
    }
    
    
    if( h->out_pic.chroma_sample_loc_type == -1 ){
        h->out_pic.chroma_sample_loc_type = h->in_pic.chroma_sample_loc_type;
        if(par->verbose_level>1)
            printf("output picture chroma_sample_loc_type not specified.  using input picture chroma_sample_loc_type(%d)\n", h->in_pic.chroma_sample_loc_type );
    }
    
    if( h->out_pic.video_full_range_flag == -1 ){
        h->out_pic.video_full_range_flag = h->in_pic.video_full_range_flag;
        if(par->verbose_level>1)
            printf("output picture video_full_range_flag not specified.  using input picture video_full_range_flag(%d)\n", h->in_pic.video_full_range_flag );
    }
    
    if( h->out_pic.colour_primaries == -1 ){
        h->out_pic.colour_primaries = h->in_pic.colour_primaries;
        if(par->verbose_level>1)
            printf("output picture colour_primaries not specified.  using input picture colour_primaries(%d)\n", h->in_pic.colour_primaries );
    }

    if( h->out_pic.transfer_characteristics == -1 ){
        h->out_pic.transfer_characteristics = h->in_pic.transfer_characteristics;
        if(par->verbose_level>1)
            printf("output picture transfer_characteristics not specified.  using input picture transfer_characteristics(%d)\n", h->in_pic.transfer_characteristics );
    }

    if( h->out_pic.matrix_coeffs == -1 ){
        h->out_pic.matrix_coeffs = h->in_pic.matrix_coeffs;
        if(par->verbose_level>1)
            printf("output picture matrix_coeffs not specified.  using input picture matrix_coeffs(%d)\n", h->in_pic.matrix_coeffs );
    }
   
    // beginning of sanity checks..
    
 
    char *ext = get_filename_extension( par->src_filename );
    
    for(int i=0; i<INPUT_FILE_TYPE_LAST; i++ )
    {
        if( !strcasecmp( ext, input_file_types[i].name ))
            par->input_file_type = input_file_types[i].idx;
    }
    
    
    if( input_file_types[ par->input_file_type].is_supported != 1 )
    {
        printf("WARNING: input file (%s) type extension (%s) idx(%d) is either not recongized or not supported\n", par->src_filename, ext,  par->input_file_type );
        arg_errors++;
    }
    
    
    
    
    ext = get_filename_extension( par->dst_filename );
    
    for(int i=0; i<OUTPUT_FILE_TYPE_LAST; i++ )
    {
        if( !strcasecmp( ext, output_file_types[i].name ))
            par->output_file_type = output_file_types[i].idx;
    }
    
    
    if( output_file_types[ par->output_file_type].is_supported != 1 )
    {
        printf("WARNING: output file (%s) type extension (%s) idx(%d) is either not recongized or not supported\n", par->dst_filename, ext, par->output_file_type );
        arg_errors++;
    }
    
    
    //printf(")
    printf("src_filename: %s (type: %s) %s\n",    par->src_filename, input_file_types[ par->input_file_type].name, input_file_types[ par->input_file_type].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)"  );
    printf("src_pic_width: %d\n",       h->in_pic.width );
    printf("src_pic_height: %d\n",      h->in_pic.height );
    printf("src_chroma_format_idc: %d\n",   h->in_pic.chroma_format_idc);
    printf("src_bit_depth: %d\n",       h->in_pic.bit_depth );
    printf("src_full_range_video_flag: %d\n",   h->in_pic.video_full_range_flag );
    printf("src_colour_primaries: %d\n",        h->in_pic.colour_primaries );

    printf("src_transfer_characteristics: %d (type: %s) %s\n",
           h->in_pic.transfer_characteristics,
           transfer_types_table[ h->in_pic.transfer_characteristics].name, transfer_types_table[ h->in_pic.transfer_characteristics].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)"  );
    printf("src_matrix_coeffs: %d\n",       h->in_pic.matrix_coeffs );
    printf("src_chroma_sample_loc_type: %d\n",       h->in_pic.chroma_sample_loc_type );

    
    printf("dst_filename: %s (type: %s) %s\n",    par->dst_filename, output_file_types[ par->output_file_type].name, output_file_types[ par->output_file_type].is_supported ?  "(SUPPORTED)" :  "(NOT SUPPORTED)"   );
    printf("dst_pic_width: %d\n",       h->out_pic.width );
    printf("dst_pic_height: %d\n",      h->out_pic.height );
    printf("dst_chroma_format_idc: %d\n",   h->out_pic.chroma_format_idc );
    printf("dst_bit_depth: %d\n",       h->out_pic.bit_depth );
    printf("dst_video_full_range_flag: %d\n",   h->out_pic.video_full_range_flag );
    printf("dst_colour_primaries: %d\n",        h->out_pic.colour_primaries );
    printf("dst_transfer_characteristics: %d (type: %s) %s\n",
           h->out_pic.transfer_characteristics,
           transfer_types_table[ h->out_pic.transfer_characteristics].name, transfer_types_table[ h->out_pic.transfer_characteristics].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)"  );
    printf("dst_matrix_coeffs: %d\n",       h->out_pic.matrix_coeffs );
    printf("dst_chroma_sample_loc_type: %d\n",       h->out_pic.chroma_sample_loc_type );

    
    
    printf("verbose_level: %d\n",   par->verbose_level );
    printf("src_start_frame: %d\n", par->src_start_frame);
    printf("n_frames: %d\n",        par->n_frames);
    
    
    if( h->in_pic.width < 2 || h->in_pic.width > 10000 ){
        printf("WARNING: pic_width(%d) outside range [0,10000]\n", h->in_pic.width );
        arg_errors++;
    }
    
    if( h->in_pic.height < 2 || h->in_pic.height > 10000 ){
        printf("WARNING: pic_height(%d) outside range [0,10000]\n", h->in_pic.height );
        arg_errors++;
    }
    
    if( h->in_pic.bit_depth < 8 || h->in_pic.bit_depth > 16 ){
        printf("WARNING: bit_depth(%d) outside range [8,16]\n", h->in_pic.bit_depth );
        arg_errors++;
    }
    
    if( h->in_pic.chroma_format_idc != CHROMA_444 ){
        printf("WARNING: chroma_format_idc(%d) not %d, Only 4:4:4 input supported at this moment..\n",
               CHROMA_444, h->in_pic.chroma_format_idc );
        arg_errors++;
    }
    

    if( h->out_pic.width < 2 || h->out_pic.width > 10000 ){
        printf("WARNING: pic_width(%d) outside range [0,10000]\n", h->out_pic.width );
        arg_errors++;
    }
    
    if( h->out_pic.height < 2 || h->out_pic.height > 10000 ){
        printf("WARNING: pic_height(%d) outside range [0,10000]\n", h->out_pic.height );
        arg_errors++;
    }
    
    if( h->out_pic.bit_depth < 8 || h->out_pic.bit_depth > 16 ){
        printf("WARNING: bit_depth(%d) outside range [8,16]\n", h->out_pic.bit_depth );
        arg_errors++;
    }
    
#if 0
    if( h->out_pic.chroma_format_idc != CHROMA_420 &&  h->out_pic.chroma_format_idc !=  ){
        printf("WARNING: chroma_format_idc(%d) outside range [1,1]. Only 4:2:0 output supported at this moment..\n",
               h->out_pic.chroma_format_idc );
        arg_errors++;
    }
#endif
    
    if( arg_errors )
    {
  //      print_help();
        printf("TOO MANY ARGUMENT ERRORS. ABORTING PROGRAM. --help to show options\n\n");
        exit(0);
    }

    
    
    return(0);
    
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
            int size = pic->width * pic->height;
            
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
        else if( pic->pic_buffer_type == PIC_TYPE_FLOAT)
        {
            stats->f_min[cc] = std::numeric_limits<float>::max( );
            stats->f_max[cc] = std::numeric_limits<float>::min( );
            
            int size = pic->width * pic->height;
            
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
            
            if( print_stats )
            {
                
                printf("pic_stats(%s)[c=%d]: min(%f) max(%f) avg(%f)",
                       pic->name, cc, stats->f_min[cc], stats->f_max[cc], avg );
            }
        
        
        }
    }
    
    stats->init = 1;
    
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
               pic->name, pic->width, pic->height, pic->bit_depth, pic->pic_buffer_type, pic->pic_buffer_type == PIC_TYPE_FLOAT ? "FLOAT" : "U16", pic->chroma_format_idc, pic->bit_depth);
    
    if( width < 1 || width > 10000 || height < 1 || height > 10000 )
        printf("init_pic(%s) ERROR: picture dimensions( %d x %d) out of bounds\n ",name,  width, height );
    
    if( sample_type == PIC_TYPE_U16 )
    {
        int size = width * height * sizeof(unsigned short);
    
        pic->buf[0] = (unsigned short*) malloc(size);
        pic->buf[1] = (unsigned short*) malloc(size);
        pic->buf[2] = (unsigned short*) malloc(size);

        pic->init = 1;
        pic->pic_buffer_type = PIC_TYPE_U16;
    }
    else if( sample_type == PIC_TYPE_FLOAT )
    {
        int size = width * height * sizeof(float);
        printf("allocating %d bytes to fbuf\n", size);
        
        pic->fbuf[0] = (float*) malloc(size);
        pic->fbuf[1] = (float*) malloc(size);
        pic->fbuf[2] = (float*) malloc(size);
        
        pic->init = 1;
        pic->pic_buffer_type = PIC_TYPE_FLOAT;
        
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
        else if( pic->pic_buffer_type == PIC_TYPE_FLOAT )
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

// TODO: fseek
void read_planar_integer_file( pic_t *dst, char *filename, int file_type, int start_frame )
{
    int error_cnt = 0;
    
    int y_size = dst->width * dst->height;
    int c_width = dst->chroma_format_idc  == CHROMA_444 ? dst->width : (dst->width /2);
    int c_height = dst->chroma_format_idc == CHROMA_420 ? (dst->height/2) : dst->height;
    int c_size = c_width * c_height;
    
    if( dst->bit_depth < 10 || dst->bit_depth > 16 ) {
       printf("read_planar_integer_file(), WARNING: bit_depth(%d) outside supported range [10,16]\n", dst->bit_depth );
       error_cnt++;
    }

    if( file_type == INPUT_FILE_TYPE_RGB && dst->chroma_format_idc != CHROMA_444 )
    {
        printf("read_planar_integer_file(), WARNING: file type is .rgb but src_chroma_format_idc(%d) not equal to CHROMA_444 (%d)\n", dst->chroma_format_idc, CHROMA_444 );
        error_cnt++;
    }

    if( error_cnt ){
        printf("read_planar_integer_file(): Too many warnings (%d). Exiting program\n", error_cnt );
      exit(0);
    }

    FILE *in_file_ptr  = fopen( filename, "rb" );
    
    if( file_type == INPUT_FILE_TYPE_RGB  )
    {
        int b = 0;
        
        // order is R,G,B in the JCT adhoc's  ... not G,B,R like our internal format here
        
        // assume each input sample is contained (LSB justified) in one 16-bit short.
        b += fread( dst->buf[2], sizeof( unsigned short), y_size, in_file_ptr );  // R
        b += fread( dst->buf[0], sizeof( unsigned short), y_size, in_file_ptr );  // G
        b += fread( dst->buf[1], sizeof( unsigned short), y_size, in_file_ptr );  // B
        
        
        printf("read %d unsigned shorts from %s\n", b, filename );
    }
    
    fclose( in_file_ptr );
}

// TODO: y4m, hist, write openxr,  psnr, check video range, ...

// TODO: move init_pic() into common.   Make all read routines (read_exr, read_tiff, etc.) use it


int main (int argc, char *argv[])
{
    hdr_t h;
    user_args_t *ua = &(h.user_args);
 //   char fn[256];

    // clear all variables in the primary pictures
    memset( &(h.in_pic), 0, sizeof( pic_t) );
    memset( &(h.out_pic), 0, sizeof( pic_t) );
    
    parse_options( &h,  argc, argv );
    

  //  h.maxCV = 65535;
  //  h.Half = 32768; // e.g half value 12 bits would have been 2048
    
    
    
    try
    {
        pic_t matrix_converted_picture;
        pic_t *tmp_pic = &matrix_converted_picture;
        
        pic_t *in_pic = &(h.in_pic);
        pic_t *out_pic = &(h.out_pic);


        tmp_pic->matrix_coeffs = out_pic->matrix_coeffs;
        tmp_pic->transfer_characteristics = out_pic->transfer_characteristics;
        tmp_pic->colour_primaries = out_pic->colour_primaries;
        tmp_pic->chroma_sample_loc_type = out_pic->chroma_sample_loc_type;

        
       // exr_test((char *)"Market3_1920x1080p_50_hf_709_00000.exr");
        //       exr_test( &exr_pic, (char *)"Seine_1920x1080p_25_hf_709_00000_rrtd_tmp.exr" );
        if( ua->input_file_type == INPUT_FILE_TYPE_RGB )
        {

            if( in_pic->matrix_coeffs != MATRIX_GBR )
                printf("reading .rgb file (%s):  setting input picture matrix_coef=%d (MATRIX_GBR)", ua->src_filename, MATRIX_GBR  );

            if( in_pic->chroma_format_idc != CHROMA_444 )
                printf("reading .rgb file (%s):  setting input picture chroma_format_idc=%d (CHROMA_444)", ua->src_filename, CHROMA_444  );
            
            in_pic->matrix_coeffs = MATRIX_GBR;
            in_pic->chroma_format_idc = CHROMA_444;
            
            init_pic( in_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, in_pic->bit_depth, in_pic->video_full_range_flag,
                     in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     PIC_TYPE_U16, 0, ua->verbose_level, "in_pic" );

            
            // TODO: derive attributes (size, chroma_format, bitdepth from filename )
            // add .yuv next
            read_planar_integer_file( in_pic, ua->src_filename, INPUT_FILE_TYPE_RGB, 0 );
        
            // TODO: check that range indicated by full_range_flag is being observed
            
            pic_stats( in_pic, &(in_pic->stats), 1 );
            

            init_pic( tmp_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, in_pic->bit_depth, in_pic->video_full_range_flag,
                     out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     PIC_TYPE_U16, 0, ua->verbose_level, ".rgb --> mati" );
            
         
            matrix_convert(  tmp_pic, &h, in_pic );
            
            pic_stats( tmp_pic, &(h.in_pic.stats), 1 );
            
  //          pic_width = raw_pic.width;
    //        pic_height = raw_pic.height;
            
        }
        else if( ua->input_file_type == INPUT_FILE_TYPE_DPX )
        {
            float *tmp_dpx_pic;
            short dpx_width, dpx_height;
            short cineon = 0;
            short half_flag = 0;

            if( in_pic->matrix_coeffs != MATRIX_GBR )
                printf("reading .exr file (%s):  setting input picture matrix_coef=%d (MATRIX_GBR)", ua->src_filename, MATRIX_GBR  );
            
            if( in_pic->chroma_format_idc != CHROMA_444 )
                printf("reading .exr file (%s):  setting input picture chroma_format_idc=%d (CHROMA_444)", ua->src_filename, CHROMA_444  );
 
            // dpx_read() initlaizes tmp_dpx_pic
            dpx_read ( ua->src_filename, &tmp_dpx_pic, &dpx_width, &dpx_height, cineon, half_flag);
            
#if 0
            dst_pic->bit_depth = 32;
            dst_pic->chroma_sample_loc_type = 0;
            dst_pic->transfer_characteristics = TRANSFER_LINEAR;
            dst_pic->colour_primaries = COLOR_PRIMARY_BT709;
            dst_pic->video_full_range_flag = 1;
            dst_pic->matrix_coeffs = MATRIX_GBR;
#endif
            
      //      pic_width = dpx_width;
        //    pic_height = dpx_height;

    //        printf("read dpx file: width(%d) height(%d)\n", pic_width, pic_height );
            
            in_pic->matrix_coeffs = MATRIX_GBR;
            in_pic->chroma_format_idc = CHROMA_444;

            init_pic( in_pic, dpx_width, dpx_height, CHROMA_444, 32,  in_pic->video_full_range_flag,
                     in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     PIC_TYPE_FLOAT, half_flag, ua->verbose_level, "in_pic from dpx"  );

            
            muxed_dpx_to_planar_float_buf( in_pic, dpx_width, dpx_height, tmp_dpx_pic );
            free( tmp_dpx_pic );

            if( ua->verbose_level > 0 )
                printf("muxed_dpx_to_planar_float_buf\n");

            init_pic( tmp_pic, dpx_width, dpx_height, CHROMA_444, 32,  in_pic->video_full_range_flag,
                     in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     PIC_TYPE_FLOAT, half_flag, ua->verbose_level, "tmp_pic from dpx"  );

       //     if( ua->output_file_type != OUTPUT_FILE_TYPE_EXR )
            {
                if( ua->verbose_level > 0 )
                    printf("matrix_convert of DPX input\n");
                
                matrix_convert( tmp_pic, &h, in_pic );
            }
            
           
       }
       else if( ua->input_file_type == INPUT_FILE_TYPE_EXR )
       {
           int half_flag = 0;  // internal processing always 32-bits
       
           // read_exr() initlaizes in_pic
           
            read_exr( in_pic, ua->src_filename );

      //     pic_width = exr_pic.width;
        //   pic_height = exr_pic.height;
           
           init_pic( tmp_pic, in_pic->width, in_pic->height, CHROMA_444, 32,  in_pic->video_full_range_flag,
                    in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                    PIC_TYPE_FLOAT, half_flag, ua->verbose_level, "in_pic from exr"  );

           matrix_convert( tmp_pic, &h, in_pic );
 
        } else if( ua->input_file_type == INPUT_FILE_TYPE_TIFF ){

            // read_tiff() initlaizes in_pic
            
            read_tiff( in_pic, ua->src_filename, &h );
            
  
            //init_pic( tmp_pic, in_pic->width, in_pic->height,  CHROMA_444, 16, PIC_TYPE_U16, 0, ua->verbose_level, "in_pic" );
            
            
            init_pic( tmp_pic, in_pic->width, in_pic->height, CHROMA_444, 32,  in_pic->video_full_range_flag,
                     in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     PIC_TYPE_FLOAT, 0, ua->verbose_level, "in_pic from tiff"  );

            
            matrix_convert( tmp_pic, &h,  in_pic );
        }
 
        
        if( ua->dst_filename != NULL ){
            
            int dst_pic_buf_type;
            
            if( ua->output_file_type == OUTPUT_FILE_TYPE_DPX || ua->output_file_type == OUTPUT_FILE_TYPE_EXR )
            {
                dst_pic_buf_type = PIC_TYPE_FLOAT;
                
                if( out_pic->chroma_format_idc != CHROMA_444 )
                {
                    printf("WARNING: out is RGB pic: chroma_format_idc(%d) != CHROMA_444 (%d)", out_pic->chroma_format_idc, CHROMA_444 );
                    out_pic->chroma_format_idc = CHROMA_444;
                }
                if( out_pic->matrix_coeffs != MATRIX_GBR )
                {
                    printf("WARNING: out is RGB pic: matrix_coeffs(%d) != MATRIX_GBR (%d)", out_pic->matrix_coeffs, MATRIX_GBR );
                    out_pic->matrix_coeffs = MATRIX_GBR;
                }
            }
            else {
                dst_pic_buf_type = PIC_TYPE_U16;
            }
            
            // dimensions already established in parse_options()
     //       init_pic( &(h.out_pic), h.out_pic.width, h.out_pic.height, h.out_pic.chroma_format_idc, h.out_pic.bit_depth, dst_pic_buf_type, 0, ua->verbose_level, "out_pic" );
            
            // TODO: make this command line variable
            int out_half_flag = 0;
            
            init_pic( out_pic, out_pic->width, out_pic->height, out_pic->chroma_format_idc, out_pic->bit_depth,  out_pic->video_full_range_flag,
                     out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, out_pic->chroma_sample_loc_type,
                     dst_pic_buf_type, out_half_flag, ua->verbose_level, "out_pic"  );

        
            int should_convert = 0;
            
            if( out_pic->chroma_format_idc != in_pic->chroma_format_idc )
            {
                printf("convert() because chroma format dst(%d) != src(%d)\n", out_pic->chroma_format_idc, in_pic->chroma_format_idc);
                should_convert++;
            }
            
            if( out_pic->width != in_pic->width )
            {
                printf("convert() because width dst(%d) != src(%d)\n",  out_pic->width ,  in_pic->width );
                should_convert++;
            }
            
            if( out_pic->height != in_pic->height )
            {
                printf("convert() because height dst(%d) != src(%d)\n",  out_pic->height , in_pic->height );
                should_convert++;
            }
 
            printf("converting: %d\n", should_convert );

            if( should_convert != 0 ){
                
#ifdef OPENCV_ENABLED
                openCV_resize_picture(  &(h.out_pic), tmp_pic );
#else
                convert( &(h.out_pic), &h, tmp_pic );
#endif
            }
            else if( tmp_pic->pic_buffer_type == PIC_TYPE_FLOAT && out_pic->pic_buffer_type == PIC_TYPE_FLOAT
                    && tmp_pic->chroma_format_idc  == CHROMA_444 &&  tmp_pic->chroma_format_idc  == CHROMA_444)
            {
       //         dst_type = PIC_TYPE_FLOAT;
         //       dst_chroma = CHROMA_444;
        
                int size = h.out_pic.width * h.out_pic.height * sizeof(  float );

                printf("copying in-->out pic float-buf\n" );
                
                memcpy(  out_pic->fbuf[0], tmp_pic->fbuf[0], size );
                memcpy(  out_pic->fbuf[1], tmp_pic->fbuf[1], size );
                memcpy(  out_pic->fbuf[2], tmp_pic->fbuf[2], size );
            }
            else if(  out_pic->chroma_format_idc  == CHROMA_444 &&  tmp_pic->chroma_format_idc  == CHROMA_444 )
            {
                int size = out_pic->width * out_pic->height * sizeof(  unsigned short );
           
                printf("copying in-->out pic u16-buf\n" );
                
                memcpy(  out_pic->buf[0], tmp_pic->buf[0], size );
                memcpy(  out_pic->buf[1], tmp_pic->buf[1], size );
                memcpy(  out_pic->buf[2], tmp_pic->buf[2], size );
    
            }
            
            pic_stats( out_pic, &(out_pic->stats), 1 );

            
            if( ua->output_file_type == OUTPUT_FILE_TYPE_YUV && h.out_pic.pic_buffer_type == PIC_TYPE_U16 )
                write_yuv( ua->dst_filename, &h,  out_pic );
            else if( ua->output_file_type == OUTPUT_FILE_TYPE_DPX && out_pic->pic_buffer_type == PIC_TYPE_FLOAT )
            {
                // TODO: convert integer to float if picture buffer type is U16
                float *tmp_dpx_pic = (float *) malloc( 3 * out_pic->width * out_pic->height * sizeof( float ) );


                planar_float_to_muxed_dpx_buf( tmp_dpx_pic, out_pic->width, out_pic->height, out_pic );
        
                //#if 0
                dpx_write_float( ua->dst_filename, tmp_dpx_pic, out_pic->width,  out_pic->height);
                free( tmp_dpx_pic );
            }
            else if ( ua->output_file_type == OUTPUT_FILE_TYPE_EXR && h.out_pic.pic_buffer_type == PIC_TYPE_FLOAT )
            {
                if( ua->input_file_type == OUTPUT_FILE_TYPE_EXR ||
                   ua->input_file_type == INPUT_FILE_TYPE_DPX )
                {
                    int half_float_flag = 1;
                    
                    write_exr_file(
                                   ua->dst_filename,
                                   out_pic->width,
                                   out_pic->height,
                                   half_float_flag,
                                   out_pic );

                }
            }
        }
    }
    catch (const std::exception &exc)
    {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}

