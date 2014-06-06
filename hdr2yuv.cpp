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

int parse_options( t_user_args *par, int argc,  char *argv[] )
{
//    t_user_args *par = &(h->args);
    int arg_errors =0;

    // default values overriden first by parse_options() then read_tiff() or exr
    par->src_pic_width = 1920;
    par->src_pic_height = 1080;
    par->src_chroma_format_idc = CHROMA_444;
    par->src_bit_depth = 16;
    par->src_video_full_range_flag =0;
    par->src_colour_primaries = COLOR_PRIMARY_XYZ;
    par->src_transfer_characteristics = TRANSFER_PQ;
    par->src_matrix_coeffs = MATRIX_GBR;
    par->src_chroma_sample_loc_type = 0;
    
    par->dst_pic_width = 1920;
    par->dst_pic_height = 1080;
    par->dst_chroma_format_idc = CHROMA_420;
    par->dst_bit_depth = 10;
    par->dst_video_full_range_flag =0;
    par->dst_colour_primaries = COLOR_PRIMARY_BT2020_10bit;
    par->dst_transfer_characteristics = TRANSFER_BT2020_10bit;
    par->dst_matrix_coeffs = MATRIX_BT2020nc;
    par->dst_chroma_sample_loc_type = 0;
    
    par->cutout_hd = 0;
    par->cutout_qhd = 0;
    par->alpha_channel = 0;
    
    par->src_filename = NULL;
    par->dst_filename = NULL;
    
    par->input_file_type = INPUT_FILE_TYPE_UNDEFINED;
    par->output_file_type = OUTPUT_FILE_TYPE_UNDEFINED;
    // TODO: use C++ map 
    for( int i=1; i < argc; i++)
    {
        if( !strcmp( argv[i], "--src_filename") && (i<argc) )
        {
            par->src_filename = argv[i+1];
            
            i++;
        }
        else if( !strcmp( argv[i], "--help") && (i<argc) )
        {
            print_help();
        }
        else if( !strcmp( argv[i], "--dst_filename") && (i<argc) )
        {
            par->dst_filename = argv[i+1];
            i++;
        }
        else if( !strcmp( argv[i], "--src_pic_width") && (i<argc) )
        {
            par->src_pic_width = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_pic_height") && (i<argc) )
        {
            par->src_pic_height = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_pic_width") && (i<argc) )
        {
            par->dst_pic_width = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_pic_height") && (i<argc) )
        {
            par->dst_pic_height = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--src_bit_depth") && (i<argc) )
        {
            par->src_bit_depth = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--dst_bit_depth") && (i<argc) )
        {
            par->dst_bit_depth = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--dst_chroma_format_idc") && (i<argc) )
        {
            par->dst_chroma_format_idc = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--src_chroma_format_idc") && (i<argc) )
        {
            par->src_chroma_format_idc = atoi( argv[i+1] );
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
            par->src_colour_primaries = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_colour_primaries") && (i < argc) )
        {
            par->dst_colour_primaries = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_matrix_coeffs") && (i < argc) )
        {
            par->src_matrix_coeffs = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--dst_matrix_coeffs") && (i < argc) )
        {
            par->dst_matrix_coeffs = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_transfer_characteristics") && (i < argc) )
        {
            char *val = argv[i+1];
            int val_int;
            
            if( sscanf(val, "%d", &val_int) != 0 )
                par->src_transfer_characteristics = val_int;
            {
                int n = sizeof( transfer_types_table ) / sizeof( transfer_types_table[0]);
                
                // wish there was content-addressible tables in C
                for(int idx=0; idx<n; idx++ )
                {
                    if( !strcasecmp( val, transfer_types_table[i].name ))
                        par->src_transfer_characteristics = transfer_types_table[i].idx;
                }
            }
            
            i++;
        }
        else if( !strcmp( argv[i], "--dst_transfer_characteristics") && (i < argc) )
        {
            char *val = argv[i+1];
            int val_int;
            
            if( sscanf(val, "%d", &val_int) != 0 )
                par->dst_transfer_characteristics = val_int;
            {
                int n = sizeof( transfer_types_table ) / sizeof( transfer_types_table[0]);
                
                // wish there was content-addressible tables in C
                for(int idx=0; idx<n; idx++ )
                {
                    if( !strcasecmp( val, transfer_types_table[i].name ))
                        par->dst_transfer_characteristics = transfer_types_table[i].idx;
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
            par->dst_video_full_range_flag = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i], "--src_video_full_range_flag") && (i < argc) )
        {
            par->src_video_full_range_flag = atoi( argv[i+1] );
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
    printf("src_pic_width: %d\n",       par->src_pic_width );
    printf("src_pic_height: %d\n",      par->src_pic_height );
    printf("src_chroma_format_idc: %d\n",   par->src_chroma_format_idc);
    printf("src_bit_depth: %d\n",       par->src_bit_depth );
    printf("src_full_range_video_flag: %d\n",   par->src_video_full_range_flag );
    printf("src_colour_primaries: %d\n",        par->src_colour_primaries );

    printf("src_transfer_characteristics: %d (type: %s) %s\n",
           par->src_transfer_characteristics,
           transfer_types_table[ par->src_transfer_characteristics].name, transfer_types_table[ par->src_transfer_characteristics].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)"  );
    printf("src_matrix_coeffs: %d\n",       par->src_matrix_coeffs );
    printf("src_chroma_sample_loc_type: %d\n",       par->src_chroma_sample_loc_type );

    
    printf("dst_filename: %s (type: %s) %s\n",    par->dst_filename, output_file_types[ par->output_file_type].name, output_file_types[ par->output_file_type].is_supported ?  "(SUPPORTED)" :  "(NOT SUPPORTED)"   );
    printf("dst_pic_width: %d\n",       par->dst_pic_width );
    printf("dst_pic_height: %d\n",      par->dst_pic_height );
    printf("dst_chroma_format_idc: %d\n",   par->dst_chroma_format_idc );
    printf("dst_bit_depth: %d\n",       par->dst_bit_depth );
    printf("dst_video_full_range_flag: %d\n",   par->dst_video_full_range_flag );
    printf("dst_colour_primaries: %d\n",        par->dst_colour_primaries );
    printf("dst_transfer_characteristics: %d (type: %s) %s\n",
           par->dst_transfer_characteristics,
           transfer_types_table[ par->dst_transfer_characteristics].name, transfer_types_table[ par->dst_transfer_characteristics].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)"  );
    printf("dst_matrix_coeffs: %d\n",       par->dst_matrix_coeffs );
    printf("dst_chroma_sample_loc_type: %d\n",       par->dst_chroma_sample_loc_type );

    
    
    printf("verbose_level: %d\n",   par->verbose_level );
    printf("src_start_frame: %d\n", par->src_start_frame);
    printf("n_frames: %d\n",        par->n_frames);
    
    
    if( par->src_pic_width < 2 || par->src_pic_width > 10000 ){
        printf("WARNING: pic_width(%d) outside range [0,10000]\n", par->src_pic_width );
        arg_errors++;
    }
    
    if( par->src_pic_height < 2 || par->src_pic_height > 10000 ){
        printf("WARNING: pic_height(%d) outside range [0,10000]\n", par->src_pic_height );
        arg_errors++;
    }
    
    if( par->src_bit_depth < 8 || par->src_bit_depth > 16 ){
        printf("WARNING: bit_depth(%d) outside range [8,16]\n", par->src_bit_depth );
        arg_errors++;
    }
    
    if( par->src_chroma_format_idc != CHROMA_444 ){
        printf("WARNING: chroma_format_idc(%d) not %d, Only 4:4:4 input supported at this moment..\n",
               CHROMA_444, par->src_chroma_format_idc );
        arg_errors++;
    }
    

    if( par->dst_pic_width < 2 || par->dst_pic_width > 10000 ){
        printf("WARNING: pic_width(%d) outside range [0,10000]\n", par->dst_pic_width );
        arg_errors++;
    }
    
    if( par->dst_pic_height < 2 || par->dst_pic_height > 10000 ){
        printf("WARNING: pic_height(%d) outside range [0,10000]\n", par->dst_pic_height );
        arg_errors++;
    }
    
    if( par->dst_bit_depth < 8 || par->dst_bit_depth > 16 ){
        printf("WARNING: bit_depth(%d) outside range [8,16]\n", par->dst_bit_depth );
        arg_errors++;
    }
    
    if( par->dst_chroma_format_idc != 1 ){
        printf("WARNING: chroma_format_idc(%d) outside range [1,1]. Only 4:2:0 output supported at this moment..\n",
               par->dst_chroma_format_idc );
        arg_errors++;
    }
    
    if( arg_errors )
    {
  //      print_help();
        printf("TOO MANY ARGUMENT ERRORS. ABORTING PROGRAM. --help to show options\n\n");
        exit(0);
    }

    
    
    return(0);
    
}

// TODO: add all the t_pic attributes to the command line input
void copy_pic_vars( t_pic *dst, t_pic *src )
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
int init_pic( t_pic *pic, int width, int height, int chroma_format_idc, int bit_depth, int sample_type, int half_float_flag, int verbose_level, const char *name )
{
    pic->width = width;
    pic->height = height;
    pic->pic_buffer_type = sample_type;
    pic->chroma_format_idc = chroma_format_idc;
    pic->bit_depth = bit_depth;
    pic->name = name;

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

int deinit_pic( t_pic *pic )
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
void read_planar_integer_file( t_pic *dst, char *filename, int file_type, int start_frame )
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
    t_hdr h;
    t_user_args *ua = &(h.user_args);
 //   char fn[256];
    
    parse_options( ua,  argc, argv );
    
    h.minCV = 0; //12 bits
    h.maxCV = 65535;
    h.Half = 32768; // e.g half value 12 bits would have been 2048
 
    h.in_pic.init = 0;
    h.out_pic.init = 0;
    
    
    // Set up for video range if needed
    if( ua->dst_video_full_range_flag == 0) {
        printf("Processing for Video Range\n");
        unsigned short D = 64;
        h.minVR = 64*D;
        h.maxVR = 876*D+h.minVR;
        h.minVRC = h.minVR;
        h.maxVRC = 896*D+h.minVRC;
        //achromatic point for chroma will be "Half"(e.g. 512 for 10 bits, 2048 for 12 bits etc..)
        
    }
    
    try
    {
        t_pic exr_pic;
        t_pic tif_pic;
        
        exr_pic.init = 0;
        tif_pic.init = 0;
        
        int pic_width, pic_height;

        exr_pic.pic_buffer_type = PIC_TYPE_FLOAT;
        tif_pic.pic_buffer_type = PIC_TYPE_U16;
        
       // exr_test((char *)"Market3_1920x1080p_50_hf_709_00000.exr");
        //       exr_test( &exr_pic, (char *)"Seine_1920x1080p_25_hf_709_00000_rrtd_tmp.exr" );
        if( ua->input_file_type == INPUT_FILE_TYPE_RGB )
        {
            t_pic raw_pic;
            raw_pic.init = 0;

            
            init_pic( &raw_pic, ua->src_pic_width, ua->src_pic_height, ua->src_chroma_format_idc, ua->src_bit_depth, PIC_TYPE_U16, 0, ua->verbose_level, "raw_pic from rgb" );

            // TODO: derive attributes (size, chroma_format, bitdepth from filename )
            read_planar_integer_file( &raw_pic, ua->src_filename, ua->input_file_type, 0);

            init_pic( &(h.in_pic), raw_pic.width, raw_pic.height, raw_pic.chroma_format_idc, raw_pic.bit_depth, raw_pic.pic_buffer_type, 0, ua->verbose_level, "RGB in_pic" );
            
            matrix_convert(  &(h.in_pic), &h, &raw_pic );
            
            pic_width = raw_pic.width;
            pic_height = raw_pic.height;
            
        }
        else if( ua->input_file_type == INPUT_FILE_TYPE_DPX )
        {
            float *tmp_dpx_pic;
            short dpx_width, dpx_height;
            short cineon = 0;
            short half_flag = 0;


            dpx_read ( ua->src_filename, &tmp_dpx_pic, &dpx_width, &dpx_height, cineon, half_flag);
            
#if 0
            dst_pic->bit_depth = 32;
            dst_pic->chroma_sample_loc_type = 0;
            dst_pic->transfer_characteristics = TRANSFER_LINEAR;
            dst_pic->colour_primaries = COLOR_PRIMARY_BT709;
            dst_pic->video_full_range_flag = 1;
            dst_pic->matrix_coeffs = MATRIX_GBR;
#endif
            
            pic_width = dpx_width;
            pic_height = dpx_height;

            printf("read dpx file: width(%d) height(%d)\n", pic_width, pic_height );
            init_pic( &exr_pic, pic_width, pic_height, CHROMA_444, 32, PIC_TYPE_FLOAT, half_flag, ua->verbose_level, "exr_pic from dpx" );

    
            muxed_dpx_to_planar_float_buf( &exr_pic, pic_width, pic_height, tmp_dpx_pic );

            if( ua->verbose_level > 0 )
                printf("muxed_dpx_to_planar_float_buf\n");

            init_pic( &(h.in_pic), exr_pic.width, exr_pic.height, CHROMA_444, 32, PIC_TYPE_FLOAT, half_flag, ua->verbose_level, "in_pic from dpx" );
       
       //     if( ua->output_file_type != OUTPUT_FILE_TYPE_EXR )
            {
                if( ua->verbose_level > 0 )
                    printf("matrix_convert of DPX input\n");
                
                matrix_convert( &(h.in_pic), &h, &exr_pic );
            }
            
           free( tmp_dpx_pic );
           
       }
       else if( ua->input_file_type == INPUT_FILE_TYPE_EXR )
       {
           int half_flag = 0;  // internal processing always 32-bits
           
            read_exr( &exr_pic, ua->src_filename );

           pic_width = exr_pic.width;
           pic_height = exr_pic.height;
           
            init_pic( &(h.in_pic), exr_pic.width, exr_pic.height, CHROMA_444, 32, PIC_TYPE_FLOAT, half_flag, ua->verbose_level, "in_pic" );
           
       
           matrix_convert( &(h.in_pic), &h, &exr_pic );
 
        } else if( ua->input_file_type == INPUT_FILE_TYPE_TIFF ){
            read_tiff( &tif_pic, &h, ua->src_filename );
            
            pic_width = tif_pic.width;
            pic_height = tif_pic.height;

            init_pic( &(h.in_pic), tif_pic.width, tif_pic.height,  CHROMA_444, 16, PIC_TYPE_U16, 0, ua->verbose_level, "in_pic" );
            matrix_convert( &(h.in_pic), &h, &tif_pic );
        }
        
        if( ua->dst_filename != NULL ){
            
            int dst_pic_buf_type, dst_chroma_format_idc;
   
            if( ua->output_file_type == OUTPUT_FILE_TYPE_DPX || ua->output_file_type == OUTPUT_FILE_TYPE_EXR )
            {
                dst_pic_buf_type = PIC_TYPE_FLOAT;
                dst_chroma_format_idc = CHROMA_444;
            }
            else
            {
                dst_pic_buf_type = PIC_TYPE_U16;
                dst_chroma_format_idc = CHROMA_420;
            }
            
            init_pic( &(h.out_pic), pic_width, pic_height, dst_chroma_format_idc, ua->dst_bit_depth, dst_pic_buf_type, 0, ua->verbose_level, "out_pic" );
        
            int should_convert = 0;
            
            if( h.out_pic.chroma_format_idc !=  h.in_pic.chroma_format_idc )
            {
                printf("convert() because chroma format dst(%d) != src(%d)\n", h.out_pic.chroma_format_idc, h.in_pic.chroma_format_idc);
                should_convert++;
            }
            
            if( h.out_pic.width !=  h.in_pic.width )
            {
                printf("convert() because width dst(%d) != src(%d)\n",  h.out_pic.width ,  h.in_pic.width );
                should_convert++;
            }
            
            if( h.out_pic.height !=  h.in_pic.height )
            {
                printf("convert() because height dst(%d) != src(%d)\n",  h.out_pic.height , h.in_pic.height );
                should_convert++;
            }
 
            printf("converting: %d\n", should_convert );

            if( should_convert != 0 )
                convert( &(h.out_pic), &h, &(h.in_pic) );
            else if( h.out_pic.pic_buffer_type == PIC_TYPE_FLOAT && h.in_pic.pic_buffer_type == PIC_TYPE_FLOAT
                    && h.out_pic.chroma_format_idc  == CHROMA_444 &&  h.in_pic.chroma_format_idc  == CHROMA_444)
            {
       //         dst_type = PIC_TYPE_FLOAT;
         //       dst_chroma = CHROMA_444;
        
                int size = pic_width * pic_height * sizeof(  float );

                printf("copying in-->out pic float-buf\n" );
                
                memcpy(  h.out_pic.fbuf[0], h.in_pic.fbuf[0], size );
                memcpy(  h.out_pic.fbuf[1], h.in_pic.fbuf[1], size );
                memcpy(  h.out_pic.fbuf[2], h.in_pic.fbuf[2], size );
            }
            else if(  h.out_pic.chroma_format_idc  == CHROMA_444 &&  h.in_pic.chroma_format_idc  == CHROMA_444 )
            {
                int size = pic_width * pic_height * sizeof( unsigned short);
                
                printf("copying in-->out pic u16-buf\n" );
                
                memcpy(  h.out_pic.buf[0], h.in_pic.buf[0], size );
                memcpy(  h.out_pic.buf[1], h.in_pic.buf[1], size );
                memcpy(  h.out_pic.buf[2], h.in_pic.buf[2], size );
    
            }
            
            
            if( ua->output_file_type == OUTPUT_FILE_TYPE_YUV && h.out_pic.pic_buffer_type == PIC_TYPE_U16 )
                write_yuv( ua->dst_filename, &h,  &(h.out_pic) );
            else if( ua->output_file_type == OUTPUT_FILE_TYPE_DPX && h.out_pic.pic_buffer_type == PIC_TYPE_FLOAT )
            {
                float *tmp_dpx_pic = (float *) malloc( 3 * exr_pic.width * exr_pic.height * sizeof( float ) );


                planar_float_to_muxed_dpx_buf( tmp_dpx_pic, exr_pic.width, exr_pic.height, &(h.in_pic) );
        
                //#if 0
                dpx_write_float( ua->dst_filename, tmp_dpx_pic, exr_pic.width,  exr_pic.height);
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
                                   exr_pic.width,
                                   exr_pic.height,
                                   half_float_flag,
                                   &exr_pic );

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

