//

#include "/usr/local/include/tiffio.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hdr.h"

using namespace std;

typedef struct 
{
        int SamplesPerPixel;
        int ImageWidth;
        int ImageLength;
        int BitsPerSample;
        int PlanarConfig;
        int MinSampleValue;
        int MaxSampleValue;
        int RowsPerStrip;
        int NumStrips;
} t_tiff_info;



void get_tiff_info( TIFF *tif, t_tiff_info *in ) 
{
  memset ( in, 0, sizeof( t_tiff_info));

  TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &(in->ImageWidth));
  TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &(in->ImageLength));
  TIFFGetField( tif, TIFFTAG_PLANARCONFIG, &(in->PlanarConfig));
  TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &(in->BitsPerSample ));
  TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &(in->SamplesPerPixel));
  TIFFGetField( tif, TIFFTAG_MINSAMPLEVALUE, &(in->MinSampleValue ));
  TIFFGetField( tif, TIFFTAG_MAXSAMPLEVALUE, &(in->MaxSampleValue ));
  TIFFGetField( tif, TIFFTAG_ROWSPERSTRIP, &(in->RowsPerStrip ));

  printf("width(%d) height(%d) PlanarConfig(%d) BitsPerSample(%d) SamplesPerPixel(%d) MinSampleValue(%d) MaxSampleValue(%d)\n", 
    in->ImageWidth, in->ImageLength, in->PlanarConfig, in->BitsPerSample, 
    in->SamplesPerPixel, in->MinSampleValue, in->MaxSampleValue );

  in->NumStrips = TIFFNumberOfStrips(tif);
  printf("NumStrips(%d)\n", in->NumStrips );
  printf("rowsPerStrip(%d)\n", in->RowsPerStrip );

}


int read_tiff( t_pic *tif_pic, t_hdr *h, char* filename )
{
    t_user_args *ua = &(h->user_args);
    
    int pic_width = ua->src_pic_width;
    int pic_height = ua->src_pic_height;
    int bit_depth = ua->src_bit_depth;
    int matrix_coeffs = ua->src_matrix_coeffs;
    int video_full_range_flag =ua->src_video_full_range_flag;
    int chroma_format_idc = ua->src_chroma_format_idc;
    int transfer_characteristics = ua->src_transfer_characteristics;
    int colour_primaries = ua->src_colour_primaries;
    int chroma_sample_loc_type = ua->src_chroma_sample_loc_type;
    
    TIFF* tif = TIFFOpen( filename,  "r");
    
    
    if( tif  )
    {
        printf("opened %s\n", filename );
    }
    else
    {
        printf("ERROR: unable top open %s\n", filename );
        return(1);
    }
    
    t_tiff_info ti;
    get_tiff_info( tif, &ti );
    
    tdata_t buf1;
    tstrip_t strip;
    uint32* bc;
    uint32 stripsize;
    uint32 pixel;
    uint32 packed_sample_start_idx = 0;
    
    
    
    // Defaults to process as 16 bit
 
    short numChan2 = 3*2; // only 3 channels rgb with no alpha
    short numChan = 3;
    //       unsigned short minVR, maxVR, minVRC,maxVRC;
    // video range is
    // Luma and R,G,B:  CV = Floor(876*D*N+64*D+0.5)
    // Chroma:  CV = Floor(896*D*N+64*D+0.5)
    // Process will be assume range of input is correct and clip/ignore out of range on input
    // convert to YUV working in integer or floating point
    // subsample at 16 bit
    // range limit subsampled output to 16 bit video legal range
    // shift down to 10,12,14 bits and write out into YUV planar file
    
    
    printf("full_range: %d\n", ua->dst_video_full_range_flag );
    printf("alpha_channel: %d\n", ua->alpha_channel );
    printf("chroma_resampler_type: %d\n", ua->chroma_resampler_type );
    
    
    if(  ua->dst_matrix_coeffs == MATRIX_BT709 )
        printf("processesing for Rec. 709\n");
    else if(  ua->dst_matrix_coeffs == MATRIX_BT2020nc )
        printf("processesing for BT.2020 non-constant luminance\n");
    else if(  ua->dst_matrix_coeffs == MATRIX_BT2020c )
        printf("processesing for BT.2020 constant luminance\n");
    else if(  ua->dst_matrix_coeffs == MATRIX_YDzDx_Y100 )
        printf("processesing for YDzDx 100 nits average\n");
    else if(  ua->dst_matrix_coeffs == MATRIX_YDzDx_Y500 )
        printf("processesing for YDzDx 500 nits average\n");
    else if(  ua->dst_matrix_coeffs == MATRIX_YDzDx )
        printf("processesing for YDzDx\n");
    
    
    if(ua->cutout_hd)printf("Cutting out HD1920x1080\n");
    if(ua->cutout_qhd)printf("Cutting out HD960x540\n");
    if(ua->alpha_channel)printf("Assuming TIF has ALPHA channel\n");
    
    
    
    printf("Stripsize: %ld\n",TIFFStripSize(tif));
    printf("NumStrips: %d\n",TIFFNumberOfStrips(tif));
    int numStrips = TIFFNumberOfStrips(tif);
#if 0
    // this causes program to crash
    printf("TIFFTAG_ROWSPERSTRIP: %d\n",TIFFGetField(tif,TIFFTAG_ROWSPERSTRIP));
    printf("SamplePerPixel: %d\n",TIFFGetField(tif,TIFFTAG_SAMPLESPERPIXEL));
    printf("TIFFTAG_IMAGELENGTH = %d\n",TIFFGetField(tif,TIFFTAG_IMAGELENGTH));
#endif
    
    
    
    
    //get count of bytes in each strip of image as vector of rows/lines
    TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &bc);
    stripsize = bc[0];
    printf("Original: Stripsize pixels: %d, Stripsize bytes: %d\n",stripsize/numChan2, stripsize);
    buf1 = _TIFFmalloc(stripsize);
    
    // Adjust numStrips to 2160
    if(numStrips != 2160 && numStrips != 1080)
    {
        printf("Error numstrips not 2160 or 1080\n");
        exit(0);
    }
    
    int stripStart = 0;
    
    
    
    
    // Adjust stripstart to cut out if needed
    if(ua->cutout_hd) stripStart = (numStrips - 1080)/2;
    if(ua->cutout_qhd)stripStart = (numStrips - 540)/2;
    
    
    // Adjust stripsize to 3840x4x2bytes 3840xnumChanx2
    // pixelstart will be byte location of first pixel
    // stripsize will be byte location just after last pixel
    if(stripsize>(960*numChan2))
    {
        packed_sample_start_idx = (stripsize - 3840*numChan2)/2;
        
        // bug fix
        if( packed_sample_start_idx >= stripsize )
            packed_sample_start_idx = 0;
        
        // if HD cutout adjust:
        if(ua->cutout_hd)
            packed_sample_start_idx = (stripsize - 1920*numChan2)/2;
        
        // if qHD cutout adjust:
        if(ua->cutout_qhd)
            packed_sample_start_idx = (stripsize - 960*numChan2)/2;
        
        
        // stripsize now to mean strip end pixel byte
        // scanline is pixelStart,stripsize,pixelstart e.g. = 8*4196
        stripsize = stripsize - packed_sample_start_idx;
        printf("    pixelStart: %d  stripsize: %d  (pixels)\n",packed_sample_start_idx/numChan2, (stripsize - packed_sample_start_idx)/numChan2);
        printf("    pixelStart: %d  stripsize: %d  (bytes)\n",packed_sample_start_idx, stripsize - packed_sample_start_idx);
    }
    
    
    // intermediate 16 bit values for final Y' and subsamples color difference
    
    unsigned short** YP;
    int arraySizeX = stripsize/numChan2 - packed_sample_start_idx/numChan2; // eg 3840
    int arraySizeY = numStrips-2*stripStart;
    
    printf("Frame size: %d x %d\n",arraySizeX, arraySizeY);
    
    int arraySizeXH = arraySizeX/2; // eg 1920 cols
    int arraySizeYH = arraySizeY/2; // eg 1080 rows
    
    
    
    tif_pic->buf[0] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    tif_pic->buf[1] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    tif_pic->buf[2] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    
    
    h->in_pic.buf[0] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    h->in_pic.buf[1] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    h->in_pic.buf[2] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    
    h->out_pic.buf[0] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    h->out_pic.buf[1] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    h->out_pic.buf[2] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    
    
    // Reduce numStrips to what is needed to center cut:
    numStrips = numStrips - stripStart;
    
    // read lines to cover arraySizeY number of lines (e.g. 2160)
    for (int y=0, strip = stripStart; strip < numStrips; strip++, y++)
    {
        // Read one scan line of packed samples into strip (line) buffer
        TIFFReadRawStrip(tif, strip,   buf1, bc[0]);
        
        int addr = y * arraySizeX;
        
        for (int x = 0, pixel = packed_sample_start_idx; pixel < stripsize; pixel+=(numChan2), x++)
        {
            uint32 A;
            
            uint32 R = static_cast<unsigned short *>(buf1)[pixel/2];
            uint32 G = static_cast<unsigned short *>(buf1)[pixel/2+1];
            uint32 B = static_cast<unsigned short *>(buf1)[pixel/2+2];
            
            
            if(ua->alpha_channel)
                A = static_cast<unsigned short *>(buf1)[pixel/2+3];
            
#if 0
            // TODO: this should be a scaling by 220/256 or 225/256
            if(( ua->src_video_full_range_flag == 1)
               && (ua->dst_video_full_range_flag == 0))
            {
                
                
            }
            
#endif
            
            // Insure RGB are clipped to video range if required:
            if(ua->dst_video_full_range_flag == 0) {
                R = (R< h->minVR) ? h->minVR : R;
                G = (G< h->minVR) ? h->minVR : G;
                B = (B< h->minVR) ? h->minVR : B;
                
                R = (R> h->maxVR) ? h->maxVR : R;
                G = (G> h->maxVR) ? h->maxVR : G;
                B = (B> h->maxVR) ? h->maxVR : B;
                
                
                // TODO: add a check to see whether or not input was full range against ua->src_video_full_range_flag
            }
            
            tif_pic->buf[0][addr+x] = G;
            tif_pic->buf[1][addr+x] = B;
            tif_pic->buf[2][addr+x] = R;
            
            // don't bother storing alpha (if present)
        }
    }
    
    _TIFFfree(buf1);
    
    
    TIFFClose(tif);
    printf("closed tiff\n");
    
    int warning_count = 0;
    
    if( arraySizeX != pic_width  ){
        printf("overriding default pic_width(%d) to TIFF header ImageWidth(%d) != 3\n", pic_width, ti.ImageWidth );
    }
    pic_width =  arraySizeX; // eg 3840
    
    if( arraySizeY != pic_height  ){
        printf("overriding default pic_height(%d) to TIFF header ImageWidth(%d) != 3\n", pic_width, ti.ImageLength );
    }
    pic_height = arraySizeY;
    
    if( ti.BitsPerSample != bit_depth ){
        printf("overriding default bitdepth(%d) to TIFF header BitsPerSample(%d) != 3\n", bit_depth, ti.BitsPerSample);
        // TODO: in future, force the input bitdepth to be what the user specifies.
        // This will be the internal bitdepth of the processing stages prior to output (dst_bit_depth)
    }
    bit_depth = ti.BitsPerSample;
    
    if( ti.SamplesPerPixel == 3 ){
        chroma_format_idc = CHROMA_444;
    }else{
        printf("WARNING: tiff info SamplesPerPixel(%d) != 3\n", ti.SamplesPerPixel );
        warning_count++;
    }
    
    
    tif_pic->width = pic_width;
    tif_pic->height = pic_height;
    tif_pic->bit_depth = bit_depth;
    tif_pic->matrix_coeffs = matrix_coeffs;
    tif_pic->video_full_range_flag = video_full_range_flag;
    tif_pic->chroma_format_idc = chroma_format_idc;
    tif_pic->transfer_characteristics = transfer_characteristics;
    tif_pic->colour_primaries = colour_primaries;
    tif_pic->chroma_sample_loc_type = chroma_sample_loc_type;
    
    
    return 0;
}


int write_tiff( char *filename, t_hdr *h )
{
  //      t_user_args *ua = &(h->user_args);
    
    t_pic *pic = &(h->out_pic);
    
    ofstream yuvOut;
    int arraySizeX = pic->width; // eg 3840
    int arraySizeY = pic->height;
    int bit_depth = pic->bit_depth;
    int matrix_coeffs = pic->matrix_coeffs;
    int video_full_range_flag = pic->video_full_range_flag;
    
    int arraySizeXH = arraySizeX/2; // eg 1920 cols
    int arraySizeYH = arraySizeY/2; // eg 1080 rows
    
    // Write YUV Frame
    // arrar to store line of output for writing process
    unsigned short  *Line;
    
    // set output line array to unsigned short
    Line =  (unsigned short*) malloc(arraySizeX*sizeof(unsigned short));
    
    
    {
        static char tbuf[256];
        static char prefix[256];
        
#if 0
        if( matrix_coeffs == MATRIX_YDzDx )
            strcat( prefix, "YDzDx");
        else if( matrix_coeffs == MATRIX_YDzDx_Y100 )
            strcat( prefix, "Y100DzDx");
        else if( matrix_coeffs == MATRIX_YDzDx_Y500 )
            strcat( prefix, "Y500DzDx");
        else if( matrix_coeffs == MATRIX_BT2020nc )
            strcat( prefix, "YCbCr2020");
        else if( matrix_coeffs == MATRIX_BT709 )
            strcat( prefix, "YCbCr709");
        else if( matrix_coeffs == MATRIX_YUVPRIME2 )
            strcat( prefix, "Yuvprime2");
        else{
            printf("WARNING: don't know what .yuv type prefix to assign\n");
            exit(1);
        }
        
        
        //           if( DXYZ ){
        //                yuvOut.open("YDzDx.yuv", ios::ate | ios::app | ios::out | ios::binary);
        //                printf("Opened YDzDx.yuv to file end and appending:\n");
        //          }
        //        else // Y'u''v''
        
        // format for vooya
        sprintf( tbuf, "%s_%s_%dx%d_420p_%dbits.yuv", filename, prefix, arraySizeX, arraySizeY, bit_depth );
        yuvOut.open( tbuf, ios::ate | ios::app | ios::out | ios::binary);
#endif
        sprintf( tbuf, "YDzDx.yuv" );
        yuvOut.open( tbuf, ios::ate | ios::app | ios::out | ios::binary);
        
        printf("Opened %s to file end and appending:\n", tbuf );
    }

    // write planer output:
    
    printf("writing line data %d bits\n", bit_depth);
    
    
    // TODO: move the clipping operation outside of writetiff
    
    // limit video range at 16 bits
    // shift data down to desired bit depth
    for(unsigned short r = 0; r < arraySizeY;r++)
    {
        unsigned short *Yptr = &(pic->buf[0][ r*arraySizeX ]);

        for(unsigned short c = 0; c < arraySizeX;c++)
        {
            // Fill line array
            // (10 bits) Line[c] = (((YP[c][r]+2)>>2));
            // limit to video range (for 16 bits then shift later)
            if( video_full_range_flag==0) {
                unsigned short Y =  Yptr[c];
                Y = (Y < h->minVR) ? h->minVR : Y;
                Y = (Y > h->maxVR) ? h->maxVR : Y;
                Yptr[c] = Y;
            }
            Line[c] = Yptr[c]>>(16-bit_depth);
            //printf(" Line[%d] = %d,  YP[%d][%d] = %d  ",c,Line[c],c,r,YP[c][r]);
        }
        //    write line arrary yuvOut.write(
        yuvOut.write((char *)Line,2*arraySizeX);
    }
    // Dz of 420
    for(unsigned short r = 0; r < arraySizeY/2;r++)
    {
        unsigned short *CbPtr = &(pic->buf[1][ r*arraySizeX/2 ]);

        for(unsigned short c = 0; c < arraySizeX/2;c++)
        {
            // Fill line array
            // (10 bits) Line[c] = (((DzP[c][r]+2)>>2));
            
            if(video_full_range_flag==0) {
                unsigned short Cb = CbPtr[c];
                
                Cb = (Cb < h->minVRC) ? h->minVRC : Cb;
                Cb = (Cb > h->maxVRC) ? h->maxVRC : Cb;
                CbPtr[c] = Cb;
            }
            Line[c] = CbPtr[c] >>(16-bit_depth);
        }
        //    write line arrary yuvOut.write(
        yuvOut.write((char *)Line,2*arraySizeXH);
    }
    
    // Dx of 420
    for(unsigned short r = 0; r < arraySizeY/2;r++)
    {
        unsigned short *CrPtr = &(pic->buf[2][ r*arraySizeX/2 ]);

        for(unsigned short c = 0; c < arraySizeX/2;c++)
        {
            // Fill line array
            // (10 bits) Line[c] = (((DxP[c][r]+2)>>2));
            
            if(video_full_range_flag==0) {
                unsigned short Cr = CrPtr[c];
            
                Cr = (Cr < h->minVRC) ? h->minVRC : Cr;
                Cr = (Cr > h->maxVRC) ? h->maxVRC : Cr;
                CrPtr[c] = Cr;
            }
            Line[c] = CrPtr[c] >>(16-bit_depth);
        }
        //    write line arrary yuvOut.write(
        yuvOut.write((char *)Line,2*arraySizeXH);
    }
    yuvOut.close(); // close file
    
    
    return 0;
    
}

#if 0
// Open Binary PlanarYUV file for writing:


#endif


