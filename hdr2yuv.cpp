// main() hdr2yuv.cpp
//
// at this level, there are no tiff or exr specific data structs or other 
// libraries specific to these formats since they tend to conflict with 
// each other.  Only the separate .o files contain library specific references.

// list of subroutine proto-types only 
#include "hdr.h"

#include <iostream>
#include <stdexcept>


int parse_options( t_user_args *par, int argc,  char *argv[] )
{
//    t_user_args *par = &(h->args);
    
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
    
    for( int i=1; i < argc; i++)
    {
        if( !strcmp( argv[i], "--src_filename") && (i<argc) )
        {
            par->src_filename = argv[i+1];
            i++;
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
            printf("argument (%s) unrecongized\n", argv[i] );
        
    }
    
    
    
    printf("src_pic_width: %d\n",       par->src_pic_width );
    printf("src_pic_height: %d\n",      par->src_pic_height );
    printf("src_bit_depth: %d\n",       par->src_bit_depth );
    printf("src_chroma_format_idc: %d\n",   par->src_chroma_format_idc);
    printf("src_filename: %s\n",    par->src_filename);
    printf("dst_pic_width: %d\n",       par->dst_pic_width );
    printf("dst_pic_height: %d\n",      par->dst_pic_height );
    printf("dst_bit_depth: %d\n",       par->dst_bit_depth );
    printf("dst_chroma_format_idc: %d\n",   par->dst_chroma_format_idc );
    printf("dst_filename: %s\n",    par->dst_filename);
    printf("verbose_level: %d\n",   par->verbose_level );
    printf("src_start_frame: %d\n", par->src_start_frame);
    printf("n_frames: %d\n",        par->n_frames);
    
    int arg_errors =0;
    
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
    
    if( par->src_filename == NULL ){
        printf("WARNING: src_filename not specified\n" );
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
    
    if( par->dst_filename == NULL ){
        printf("WARNING: src_filename not specified\n" );
        arg_errors++;
    }

    
    if( arg_errors != 0 ){
        printf("TOO MANY ARGUMENT ERRORS. ABORTING PROGRAM");
    }
    
    return(0);
    
}



int main (int argc, char *argv[])
{
    t_hdr h;
    t_user_args *ua = &(h.user_args);
 //   char fn[256];
    
    parse_options( ua,  argc, argv );
    
    h.minCV = 0; //12 bits
    h.maxCV = 65535;
    h.Half = 32768; // e.g half value 12 bits would have been 2048
 
    
    
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
    

  //  strcpy( fn, "Balloon_1920x1080p_25_hf_709_00000_RGB-PQ_for_BT2020-YCbCr.tiff");
    
    try
    {
        exr_test((char *)"Market3_1920x1080p_50_hf_709_00000.exr");

        t_pic tif_pic;
        
        read_tiff( &tif_pic, &h, ua->src_filename );
        matrix_convert( &(h.in_pic), &h, &tif_pic );
        
        convert( &h );

        
        write_tiff( ua->dst_filename, &h );

    }
    catch (const std::exception &exc)
    {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}

