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
        else if( !strcmp( argv[i],  "--src_half_float_flag") && (i<argc) )
        {
            h->in_pic.half_float_flag = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--dst_bit_depth") && (i<argc) )
        {
            h->out_pic.bit_depth = atoi( argv[i+1] );
            i++;
        }
        else if( !strcmp( argv[i],  "--dst_half_float_flag") && (i<argc) )
        {
            h->out_pic.half_float_flag = atoi( argv[i+1] );
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
    
    if( par->input_file_type == INPUT_FILE_TYPE_YUV ||  par->input_file_type == INPUT_FILE_TYPE_TIFF || par->input_file_type == INPUT_FILE_TYPE_RGB || par->input_file_type == INPUT_FILE_TYPE_Y4M  )
    {
        h->in_pic.pic_buffer_type = PIC_TYPE_U16;
        
        if( h->in_pic.bit_depth < 10 || h->in_pic.bit_depth > 16 )
        {
            printf("WARNING: src bit_depth(%d) outside range [10,16] for integer input file type(%s)\n",
                   h->in_pic.bit_depth, input_file_types[par->input_file_type].name );
        }
    
    }
    else if ( par->input_file_type == INPUT_FILE_TYPE_EXR )
    {
        h->in_pic.pic_buffer_type = PIC_TYPE_F32;

        if( h->in_pic.bit_depth != 16 && h->in_pic.bit_depth != 32 )
        {
            printf("WARNING: src bit_depth(%d) must be 16 or 32 bits for float input file type(%s)\n",
                   h->in_pic.bit_depth, input_file_types[par->input_file_type].name );

            if( h->in_pic.half_float_flag )
                h->in_pic.bit_depth = 32;
            else
                h->in_pic.bit_depth = 16;
        }
        
    }
    else if ( par->input_file_type == INPUT_FILE_TYPE_DPX )
    {
        h->in_pic.pic_buffer_type = PIC_TYPE_F32;
        if(  h->in_pic.bit_depth != 32 )
        {
            printf("WARNING: src bit_depth(%d) must be 16 or 32 bits for float input file type(%s)\n",
                   h->in_pic.bit_depth, input_file_types[par->input_file_type].name );
            
            h->in_pic.bit_depth = 32;
        }
        
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

    if( par->output_file_type == OUTPUT_FILE_TYPE_YUV ||  par->output_file_type == OUTPUT_FILE_TYPE_TIFF || par->output_file_type == OUTPUT_FILE_TYPE_RGB || par->output_file_type == OUTPUT_FILE_TYPE_Y4M  )
    {
        h->out_pic.pic_buffer_type = PIC_TYPE_U16;
        
        if( h->out_pic.bit_depth < 10 || h->out_pic.bit_depth > 16 )
        {
            printf("WARNING: dst bit_depth(%d) outside range [10,16] for integer input file type(%s)\n",
                   h->out_pic.bit_depth, output_file_types[par->output_file_type].name );
        }
        
    }
    else if ( par->output_file_type == OUTPUT_FILE_TYPE_EXR )
    {
        h->out_pic.pic_buffer_type = PIC_TYPE_F32;
        
        if( h->out_pic.bit_depth != 16 && h->out_pic.bit_depth != 32 )
        {
            printf("WARNING: dst bit_depth(%d) must be 16 or 32 bits for float input file type(%s)\n",
                   h->out_pic.bit_depth, output_file_types[par->output_file_type].name );
            
            if( h->out_pic.half_float_flag )
                h->out_pic.bit_depth = 32;
            else
                h->out_pic.bit_depth = 16;
        }
        
    }
    else if ( par->output_file_type == OUTPUT_FILE_TYPE_DPX )
    {
        h->out_pic.pic_buffer_type = PIC_TYPE_F32;
 
        if(  h->out_pic.bit_depth != 32 )
        {
            printf("WARNING: dst bit_depth(%d) must be 16 or 32 bits for float input file type(%s)\n",
                   h->out_pic.bit_depth, output_file_types[par->output_file_type].name );
            
            h->out_pic.bit_depth = 32;
        }
        
    }
    
    // beginning of sanity checks..
    if( par->src_start_frame != 0 &&
       (par->input_file_type != INPUT_FILE_TYPE_YUV && par->input_file_type != INPUT_FILE_TYPE_RGB
        && par->input_file_type != INPUT_FILE_TYPE_Y4M )){
           
           printf("WARNING: start_frame(%d) only makes sense when file type is .yuv, .rgb, or .y4m\n", par->src_start_frame );
       }
    
    
    //printf(")
    printf("src_filename: %s (type: %s) %s\n",    par->src_filename, input_file_types[ par->input_file_type].name, input_file_types[ par->input_file_type].is_supported?  "(SUPPORTED)" : "(NOT SUPPORTED)"  );
    printf("src_pic_width: %d\n",       h->in_pic.width );
    printf("src_pic_height: %d\n",      h->in_pic.height );
    printf("src_chroma_format_idc: %d\n",   h->in_pic.chroma_format_idc);
    printf("src_bit_depth: %d\n",       h->in_pic.bit_depth );
    printf("src_half_float_flag: %d\n",       h->in_pic.half_float_flag );
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
    printf("dst_half_float_flag: %d\n",       h->out_pic.half_float_flag );
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
    
    if( h->in_pic.bit_depth < 8 || h->in_pic.bit_depth > 32 ){
        printf("WARNING: src bit_depth(%d) outside range [8,32]\n", h->in_pic.bit_depth );
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
    
    if( h->out_pic.bit_depth < 8 || h->out_pic.bit_depth > 32 ){
        printf("WARNING: dst bit_depth(%d) outside range [32]\n", h->out_pic.bit_depth );
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


// TODO: fseek
void read_planar_integer_file( pic_t *dst, char *filename, int file_type, int start_frame, int verbose_level )
{
    int error_cnt = 0;
    
    int y_size = dst->width * dst->height;
    int c_width = dst->chroma_format_idc  == CHROMA_444 ? dst->width : (dst->width /2);
    int c_height = dst->chroma_format_idc == CHROMA_420 ? (dst->height/2) : dst->height;
    int c_size = c_width * c_height;

    if( dst->pic_buffer_type != PIC_TYPE_U16) {
        printf("read_planar_integer_file(), WARNING: bit_depth(%d) outside supported range [10,16]\n", dst->bit_depth );
        error_cnt++;
    }

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
    
    int size = y_size + 2 * c_size;
    
    
    fseek( in_file_ptr, size * start_frame * sizeof( unsigned short), SEEK_SET );
    
    if( file_type == INPUT_FILE_TYPE_RGB || file_type == INPUT_FILE_TYPE_YUV )
    {
        int b = 0;
        
        // order is R,G,B in the JCT adhoc's  ... not G,B,R like our internal format here
        
        // assume each input sample is contained (LSB justified) in one 16-bit short.
        b += fread( dst->buf[2], sizeof( unsigned short), y_size, in_file_ptr );  // R
        b += fread( dst->buf[0], sizeof( unsigned short), y_size, in_file_ptr );  // G
        b += fread( dst->buf[1], sizeof( unsigned short), y_size, in_file_ptr );  // B
        
        if( b!= size )
        {
            printf("ERROR, read_planar_integer_file(): only %d shorts read from %s. Expecting %d shorts\n", b, filename, size );
            error_cnt++;
        } else if( verbose_level > 2 ) {
            printf("read %d unsigned shorts from %s from start frame (%d)\n", b, filename, start_frame );
        }
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
        if( ua->input_file_type == INPUT_FILE_TYPE_RGB || ua->input_file_type == INPUT_FILE_TYPE_YUV)
        {

  //          if( in_pic->matrix_coeffs != MATRIX_GBR )
    //            printf("reading .rgb file (%s):  setting input picture matrix_coef=%d (MATRIX_GBR)", ua->src_filename,
            //MATRIX_GBR  );

            if( in_pic->chroma_format_idc != CHROMA_444 )
                printf("reading .rgb or .yuv file (%s):  setting input picture chroma_format_idc=%d (CHROMA_444)", ua->src_filename, CHROMA_444  );
            
            if( ua->input_file_type == INPUT_FILE_TYPE_RGB ){
                if( in_pic->matrix_coeffs != MATRIX_GBR ){
                    printf("WARNING: RGB src matrix_coefs(%d) being overriden to MATRIX_GBR (%d)\n", in_pic->matrix_coeffs, MATRIX_GBR );
                    in_pic->matrix_coeffs = MATRIX_GBR ;
                }
                if( in_pic->chroma_format_idc != CHROMA_444 ){
                    printf("WARNING: RGB src chroma_format_idc(%d) being overriden to CHROMA_444 (%d)\n", in_pic->chroma_format_idc, CHROMA_444 );
                    in_pic->chroma_format_idc = CHROMA_444 ;
                }
            }
            
            init_pic( in_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, in_pic->bit_depth, in_pic->video_full_range_flag,
                     in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     PIC_TYPE_U16, 0, ua->verbose_level, "in_pic" );

            
            // TODO: derive attributes (size, chroma_format, bitdepth) from filename
            //   check x264 convention matches vooya.de
            
            // add .yuv next
            read_planar_integer_file( in_pic, ua->src_filename, INPUT_FILE_TYPE_RGB, ua->src_start_frame, ua->verbose_level );
        
            // TODO: check that range indicated by full_range_flag is being observed
            
            pic_stats( in_pic, &(in_pic->stats), 1 );
            

            init_pic( tmp_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, in_pic->bit_depth, in_pic->video_full_range_flag,
                     out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     out_pic->pic_buffer_type, 0, ua->verbose_level, ".rgb --> mati" );
            
         
            matrix_convert(  tmp_pic, &h, in_pic );
            
            pic_stats( tmp_pic, &(tmp_pic->stats), 1 );
            
            
        }
        else if( ua->input_file_type == INPUT_FILE_TYPE_DPX )
        {
            float *tmp_dpx_pic;
            short dpx_width, dpx_height;
            short cineon = 0;
   //         short half_flag = 0;

            if( in_pic->matrix_coeffs != MATRIX_GBR )
                printf("reading .dpx file (%s):  setting input picture matrix_coef=%d (MATRIX_GBR)", ua->src_filename, MATRIX_GBR  );
            
            if( in_pic->chroma_format_idc != CHROMA_444 )
                printf("reading .dpx file (%s):  setting input picture chroma_format_idc=%d (CHROMA_444)", ua->src_filename, CHROMA_444  );
 
            if( in_pic->video_full_range_flag != 1 )
                printf("reading .dpx file (%s):  setting input picture video_full_range_flag to 1", ua->src_filename );
            
            // dpx_read() initlaizes tmp_dpx_pic
            dpx_read ( ua->src_filename, &tmp_dpx_pic, &dpx_width, &dpx_height, cineon, in_pic->half_float_flag );

            
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
                     PIC_TYPE_F32, 0, ua->verbose_level, "in_pic from dpx"  );

            
            muxed_dpx_to_planar_float_buf( in_pic, dpx_width, dpx_height, tmp_dpx_pic );
            free( tmp_dpx_pic );

            pic_stats( in_pic, &(in_pic->stats), 1 );

            if( ua->verbose_level > 0 )
                printf("muxed_dpx_to_planar_float_buf\n");

  //          init_pic( tmp_pic, dpx_width, dpx_height, CHROMA_444, 32,  in_pic->video_full_range_flag,
    //                 in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
      //               PIC_TYPE_F32, 0, ua->verbose_level, "tmp_pic from dpx"  );

            init_pic( tmp_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, out_pic->bit_depth, out_pic->video_full_range_flag,
                     out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                     out_pic->pic_buffer_type, 0, ua->verbose_level, ".dpx --> mati" );
            
       //     if( ua->output_file_type != OUTPUT_FILE_TYPE_EXR )
            {
                if( ua->verbose_level > 0 )
                    printf("matrix_convert of DPX input\n");
                
                matrix_convert( tmp_pic, &h, in_pic );
            }
            
            pic_stats( tmp_pic, &(tmp_pic->stats), 1 );
          
           
       }
       else if( ua->input_file_type == INPUT_FILE_TYPE_EXR )
       {
   //        int half_flag = 0;  // internal processing always 32-bits
       
           // read_exr() initlaizes in_pic
            read_exr( in_pic, ua->src_filename );
 
           if( in_pic->video_full_range_flag != 1 )
               printf("reading .exr file (%s):  setting input picture video_full_range_flag to 1", ua->src_filename );
           
           
           pic_stats( in_pic, &(in_pic->stats), 1 );

           
 //          init_pic( tmp_pic, in_pic->width, in_pic->height, CHROMA_444, 32,  in_pic->video_full_range_flag,
   //                 in_pic->colour_primaries, in_pic->transfer_characteristics, in_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
     //               PIC_TYPE_F32, 0, ua->verbose_level, "in_pic from exr"  );

           init_pic( tmp_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, out_pic->bit_depth, out_pic->video_full_range_flag,
                    out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, in_pic->chroma_sample_loc_type,
                    out_pic->pic_buffer_type, 0, ua->verbose_level, ".exr --> mati" );
           
           matrix_convert( tmp_pic, &h, in_pic );
           
           pic_stats( tmp_pic, &(tmp_pic->stats), 1 );

 
        } else if( ua->input_file_type == INPUT_FILE_TYPE_TIFF ){

            // read_tiff() initlaizes in_pic
            
            read_tiff( in_pic, ua->src_filename, &h );
            pic_stats( in_pic, &(in_pic->stats), 1 );
            
            init_pic( tmp_pic, in_pic->width, in_pic->height, in_pic->chroma_format_idc, in_pic->bit_depth,  out_pic->video_full_range_flag,
                     out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, out_pic->chroma_sample_loc_type,
                     out_pic->pic_buffer_type, 0, ua->verbose_level, "tmp_pic from tiff"  );

            
            matrix_convert( tmp_pic, &h,  in_pic );
            
            pic_stats( tmp_pic, &(tmp_pic->stats), 1 );

        }
 
        
        if( ua->dst_filename != NULL ){
            
            int dst_pic_buf_type;
            
            if( ua->output_file_type == OUTPUT_FILE_TYPE_DPX || ua->output_file_type == OUTPUT_FILE_TYPE_EXR )
            {
                dst_pic_buf_type = PIC_TYPE_F32;
                
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
   //         int out_half_flag = 0;
            
            init_pic( out_pic, out_pic->width, out_pic->height, out_pic->chroma_format_idc, out_pic->bit_depth,  out_pic->video_full_range_flag, out_pic->colour_primaries, out_pic->transfer_characteristics, out_pic->matrix_coeffs, out_pic->chroma_sample_loc_type, dst_pic_buf_type, out_pic->half_float_flag, ua->verbose_level, "out_pic"  );

        
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
                openCV_resize_picture(  out_pic, tmp_pic );
#else
                convert( out_pic, &h, tmp_pic );
#endif
            }
            else if( tmp_pic->pic_buffer_type == PIC_TYPE_F32 && out_pic->pic_buffer_type == PIC_TYPE_F32
                    && tmp_pic->chroma_format_idc  == CHROMA_444 &&  tmp_pic->chroma_format_idc  == CHROMA_444)
            {
       //         dst_type = PIC_TYPE_FLOAT;
         //       dst_chroma = CHROMA_444;
        
                int size = out_pic->width * out_pic->height * sizeof(  float );

                printf("copying %d bytes for each plane in-->out pic float-buf\n" , size );
                
                memcpy(  out_pic->fbuf[0], tmp_pic->fbuf[0], size );
                memcpy(  out_pic->fbuf[1], tmp_pic->fbuf[1], size );
                memcpy(  out_pic->fbuf[2], tmp_pic->fbuf[2], size );
            }
            else if(  out_pic->chroma_format_idc  == CHROMA_444 &&  tmp_pic->chroma_format_idc  == CHROMA_444 )
            {
                int size = out_pic->width * out_pic->height * sizeof(  unsigned short );
           
                printf("copying %d bytes for each plane in in-->out pic u16-buf\n" , size);
                
                memcpy(  out_pic->buf[0], tmp_pic->buf[0], size );
                memcpy(  out_pic->buf[1], tmp_pic->buf[1], size );
                memcpy(  out_pic->buf[2], tmp_pic->buf[2], size );
    
            }
            
            pic_stats( out_pic, &(out_pic->stats), 1 );

            
            if( ua->output_file_type == OUTPUT_FILE_TYPE_YUV && out_pic->pic_buffer_type == PIC_TYPE_U16 ){
                write_yuv( ua->dst_filename, &h,  out_pic, tmp_pic->bit_depth );
                pic_stats( out_pic, &(out_pic->stats), 1 );
            }
            else if( ua->output_file_type == OUTPUT_FILE_TYPE_DPX && out_pic->pic_buffer_type == PIC_TYPE_F32 )
            {
                // TODO: convert integer to float if picture buffer type is U16
                float *tmp_dpx_pic = (float *) malloc( 3 * out_pic->width * out_pic->height * sizeof( float ) );


                planar_float_to_muxed_dpx_buf( tmp_dpx_pic, out_pic->width, out_pic->height, out_pic );
        
                //#if 0
                dpx_write_float( ua->dst_filename, tmp_dpx_pic, out_pic->width,  out_pic->height);
                free( tmp_dpx_pic );
            }
            else if ( ua->output_file_type == OUTPUT_FILE_TYPE_EXR && out_pic->pic_buffer_type == PIC_TYPE_F32 )
            {
          //      if( ua->input_file_type == OUTPUT_FILE_TYPE_EXR ||
            //       ua->input_file_type == INPUT_FILE_TYPE_DPX )
                {
              //      int half_float_flag = 1;
                    
                    write_exr_file(
                                   ua->dst_filename,
                                   out_pic->width,
                                   out_pic->height,
                                   out_pic->half_float_flag,
                                   out_pic );

                }
            }
            else
                printf("WARNING: don't know what file type to write to.\n");
        }
    }
    catch (const std::exception &exc)
    {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}

