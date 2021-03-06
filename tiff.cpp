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
} tiff_info_t;



void get_tiff_info( TIFF *tif, tiff_info_t *in )
{
  memset ( in, 0, sizeof( tiff_info_t));

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


int read_tiff( pic_t *tif_pic, char* filename, hdr_t *h )
{
 //   t_user_args *ua = &(h->user_args);
    
    int pic_width = tif_pic->width;
    int pic_height = tif_pic->height;
    int bit_depth = tif_pic->bit_depth;
    int matrix_coeffs = tif_pic->matrix_coeffs;
    int video_full_range_flag = tif_pic->video_full_range_flag;
    int chroma_format_idc = tif_pic->chroma_format_idc;
    int transfer_characteristics = tif_pic->transfer_characteristics;
    int colour_primaries = tif_pic->colour_primaries;
    int chroma_sample_loc_type = tif_pic->chroma_sample_loc_type;
    
    clip_limits_t *clip = &(tif_pic->clip);
    
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
    
    tiff_info_t ti;
    get_tiff_info( tif, &ti );
    
    tdata_t buf1;
    tstrip_t strip;
    uint32* bc;
    uint32 stripsize;
    uint32 pixel;
    uint32 packed_sample_start_idx = 0;
    
    
    if( ti.BitsPerSample != bit_depth ){
        printf("WARNING, read_tiff(): overriding used-specified bit_depth(%d) to tiff header value(%d)\n", bit_depth, ti.BitsPerSample );
        bit_depth = ti.BitsPerSample;
    }
    
    
    if( ti.ImageWidth != pic_width ){
        printf("WARNING, read_tiff(): overriding used-specifiedpic_width(%d) to tiff header value(%d)\n", pic_width, ti.ImageWidth );
        pic_width = ti.ImageWidth;
    }
    
    if( ti.ImageLength != pic_height ){
        printf("WARNING, read_tiff(): overriding used-specified pic_height(%d) to tiff header value(%d)\n", pic_height, ti.ImageLength );
        pic_height = ti.ImageWidth;
    }
    
    if( ti.SamplesPerPixel == 3 && chroma_format_idc != CHROMA_444){
        printf("WARNING, read_tiff(): overriding used-specified chroma_format_idc(%d) to tiff header value(%d)\n", chroma_format_idc, CHROMA_444 );
        chroma_format_idc = CHROMA_444;
    } else if ( ti.SamplesPerPixel != 3 ){
        printf("ERROR, read_tiff(): SamplesPerPixel(%d) not equal to 3\n", ti.SamplesPerPixel );
    }


 
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
    
    
    printf("full_range: %d\n", tif_pic->video_full_range_flag );
//    printf("alpha_channel: %d\n", ua->alpha_channel );
//    printf("chroma_resampler_type: %d\n", ua->chroma_resampler_type );
    
#if 0
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
#endif
    
    if( h->user_args.cutout_hd)printf("Cutting out HD1920x1080\n");
    if( h->user_args.cutout_qhd)printf("Cutting out HD960x540\n");
    if( h->user_args.alpha_channel)printf("Assuming TIF has ALPHA channel\n");
    
    
    
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
    if(h->user_args.cutout_hd) stripStart = (numStrips - 1080)/2;
    if(h->user_args.cutout_qhd)stripStart = (numStrips - 540)/2;
    
    
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
        if(h->user_args.cutout_hd)
            packed_sample_start_idx = (stripsize - 1920*numChan2)/2;
        
        // if qHD cutout adjust:
        if(h->user_args.cutout_qhd)
            packed_sample_start_idx = (stripsize - 960*numChan2)/2;
        
        
        // stripsize now to mean strip end pixel byte
        // scanline is pixelStart,stripsize,pixelstart e.g. = 8*4196
        stripsize = stripsize - packed_sample_start_idx;
        printf("    pixelStart: %d  stripsize: %d  (pixels)\n",packed_sample_start_idx/numChan2, (stripsize - packed_sample_start_idx)/numChan2);
        printf("    pixelStart: %d  stripsize: %d  (bytes)\n",packed_sample_start_idx, stripsize - packed_sample_start_idx);
    }
    
    
    // intermediate 16 bit values for final Y' and subsamples color difference
    
    int arraySizeX = stripsize/numChan2 - packed_sample_start_idx/numChan2; // eg 3840
    int arraySizeY = numStrips-2*stripStart;
    
    printf("Frame size: %d x %d\n",arraySizeX, arraySizeY);
    
    
    // now, from opening the .tiff file, we know how big the picture is and can initialize its bufffer..
  //   chroma_format_idc = CHROMA_444;
//    int bit_depth = 16;
//    int video_full_range_flag = ua->src_video_full_range_flag; // TODO: check this against stats
//    int colour_primaries = h->in_pic.colour_primaries;
//    int transfer_characteristics = h->in_pic.transfer_characteristics;
//    int matrix_coeffs;
//    int chroma_sample_loc_type;
//    int half_float_flagl
    if( bit_depth != 16 ){
        printf("WARNING, read_tiff(): bit_depth(%d) != 16-bit precision assumed for tiff input samples\n", bit_depth);
        bit_depth = 16;
    }
    if( chroma_format_idc != CHROMA_444 ){
        printf("WARNING, read_tiff(): chroma_format_idc(%d) != CHROMA_444 assumed for tiff input\n", chroma_format_idc );
        chroma_format_idc = CHROMA_444;
    }
    if( matrix_coeffs != MATRIX_GBR ){
        printf("WARNING, read_tiff(): matrix_coefs(%d) != MATRIX_GBR assumed for tiff input\n",  matrix_coeffs);
        matrix_coeffs = MATRIX_GBR;
    }
    
    init_pic( tif_pic, arraySizeX, arraySizeY, chroma_format_idc,   bit_depth,   video_full_range_flag,   colour_primaries,   transfer_characteristics,   matrix_coeffs,   chroma_sample_loc_type, PIC_TYPE_U16, 0, 0, "tiff_pic" );

//    tif_pic->buf[0] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
//    tif_pic->buf[1] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
//    tif_pic->buf[2] = (unsigned short*) malloc(arraySizeX*arraySizeY*sizeof(unsigned short));
    
    
    
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
            
            
            if(h->user_args.alpha_channel)
                A = static_cast<unsigned short *>(buf1)[pixel/2+3];
            
#if 0
            // TODO: this should be a scaling by 220/256 or 225/256
            if(( ua->src_video_full_range_flag == 1)
               && (ua->dst_video_full_range_flag == 0))
            {
                
                
            }
            
#endif
            
            // Insure RGB are clipped to video range if required:
            if(tif_pic->video_full_range_flag == 0) {
                R = (R< clip->minVR) ? clip->minVR : R;
                G = (G< clip->minVR) ? clip->minVR : G;
                B = (B< clip->minVR) ? clip->minVR : B;
                
                R = (R> clip->maxVR) ? clip->maxVR : R;
                G = (G> clip->maxVR) ? clip->maxVR : G;
                B = (B> clip->maxVR) ? clip->maxVR : B;
                
                
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
    
    tif_pic->pic_buffer_type = PIC_TYPE_U16;
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

// uses the tiff library to write raw (.yuv) file
// that is why write_yuv() is here.
// only tiff.cpp is supposed to link to the tiff library, not other .cpp files.

int write_yuv( char *filename, hdr_t *h, pic_t *pic, int src_bit_depth )
{
  //      t_user_args *ua = &(h->user_args);
    
   // t_pic *pic = &(h->out_pic);
    ofstream yuvOut;
  //  int arraySizeX = pic->width; // eg 3840
  //  int arraySizeY = pic->height;
    int bit_depth = pic->bit_depth;
    int matrix_coeffs = pic->matrix_coeffs;
    int video_full_range_flag = pic->video_full_range_flag;
    
    clip_limits_t *clip = &(pic->clip);

  //  int arraySizeXC = arraySizeX/2; // eg 1920 cols
  ///  int arraySizeYC = arraySizeY/2; // eg 1080 rows
    
    // Write YUV Frame
    // arrar to store line of output for writing process
    unsigned short  *Line;
    user_args_t *ua = &(h->user_args);
    // set output line array to unsigned short
    Line =  (unsigned short*) malloc(pic->width*sizeof(unsigned short));
    
    
    // TODO: make this pic->bit_depth
    int down_shift = ( src_bit_depth - pic->bit_depth);
    
    if( down_shift < 0 )
        
    {
        printf("ERROR: dst bitdepth(%d) > src bitdepth(%d)\n", pic->bit_depth, src_bit_depth );
        exit(0);
    }
    else
        printf("write_yuv(): shifting samples down by %d bits (from %d to %d)\n", down_shift, src_bit_depth, pic->bit_depth );
    
    {
        static char tbuf[256];
#if 0
        static char prefix[256];
        
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
          //     sprintf( tbuf, "YDzDx.yuv" );
        sprintf( tbuf, "%s", filename );
        yuvOut.open( tbuf, ios::ate | ios::app | ios::out | ios::binary);
        
        printf("Opened %s to file end and appending:\n", tbuf );
    }

    // write planer output:
    
    printf("writing %d x %d  pixels at %d bit_depth (from %d bits)\n", pic->width, pic->height, pic->bit_depth, h->out_pic.bit_depth);
    
    
    // TODO: move the clipping operation outside of writetiff
    
    // limit video range at 16 bits
    // shift data down to desired bit depth
    
//    pic->plane[0].width;
    
    int ph = pic->plane[0].height;
    int pw = pic->plane[0].width;
    
    for(int r = 0; r <ph ;r++)
    {
        unsigned short *Yptr = &(pic->buf[0][ r*pw ]);

        for(int c = 0; c < pw; c++)
        {
            // Fill line array
            // (10 bits) Line[c] = (((YP[c][r]+2)>>2));
            // limit to video range (for 16 bits then shift later)
            unsigned short Y =  Yptr[c];
            Y = Y >> down_shift;

            if( video_full_range_flag==0) {
                Y = (Y < clip->minVR) ? clip->minVR : Y;
                Y = (Y > clip->maxVR) ? clip->maxVR : Y;
            } else {
//                Y = (Y < clip->minCV) ? clip->minCV : Y;  // redundant
                Y = (Y > clip->maxCV) ? clip->maxCV : Y;
            }
            
            Yptr[c] = Y;
            Line[c] = Y;
            //printf(" Line[%d] = %d,  YP[%d][%d] = %d  ",c,Line[c],c,r/Users/chadfogg/Documents/Src/Traffic_2560x1600_420p_10bits.yuv,YP[c][r]);
        }
        //    write line arrary yuvOut.write(
        yuvOut.write((char *)Line,2*pw);
    }
    
    ph = pic->plane[1].height;
    pw = pic->plane[1].width;
    
    // Dz of 420
    for(int r = 0; r < ph; r++)
    {
        
        unsigned short *CbPtr = &(pic->buf[1][ r * pw ]);

        for(int c = 0; c < pw; c++ )
        {
            // Fill line array
            // (10 bits) Line[c] = (((DzP[c][r]+2)>>2));
       
            unsigned short Cb = CbPtr[c];
            Cb = Cb >> down_shift;

            if(video_full_range_flag==0) {
                Cb = (Cb < clip->minVRC) ? clip->minVRC : Cb;
                Cb = (Cb > clip->maxVRC) ? clip->maxVRC : Cb;
            } else {
   //             Cb = (Cb < clip->minCV) ? clip->minCV : Cb; // redundant
                Cb = (Cb > clip->maxCV) ? clip->maxCV : Cb;
            }

            CbPtr[c] = Cb;
            Line[c] = Cb;
        }
        //    write line arrary yuvOut.write(
        yuvOut.write((char *)Line,2*pw);
    }

    ph = pic->plane[2].height;
    pw = pic->plane[2].width;

    // Dx of 420
    for(int r = 0; r < ph;r++)
    {
        unsigned short *CrPtr = &(pic->buf[2][ r*pw ]);

        for( int  c = 0; c < pw;c++)
        {
            // Fill line array
            // (10 bits) Line[c] = (((DxP[c][r]+2)>>2));
            
            unsigned short Cr = CrPtr[c];
            Cr = Cr >> down_shift;

            if(video_full_range_flag==0) {
            
                Cr = (Cr < clip->minVRC) ? clip->minVRC : Cr;
                Cr = (Cr > clip->maxVRC) ? clip->maxVRC : Cr;
            } else {
                //             Cr = (Cr < clip->minCV) ? clip->minCV : Cr; // redundant
                Cr = (Cr > clip->maxCV) ? clip->maxCV : Cr;
            }
            
            CrPtr[c] = Cr;
            Line[c] = Cr;
        }
        //    write line arrary yuvOut.write(
        yuvOut.write((char *)Line,2*pw);
    }
    yuvOut.close(); // close file
    
    
    return 0;
    
}


int write_tiff(  char* filename, hdr_t *h, pic_t *pic, int src_bit_depth )
{
    short ALPHA = 0; // no alpha channel is default
    short numChan ; // only 3 channels rgb with no alpha
    
    short SR = pic->bit_depth - src_bit_depth;
    
    float avg_R = 0.0;
    float avg_G = 0.0;
    float avg_B = 0.0;
    
    TIFF* tif = TIFFOpen(filename, "w");
    
    if( tif == NULL )
    {
        printf("unable to open %s.  Exiting\n", filename );
        exit(0);
    }
    
    if(ALPHA){
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
        numChan = 4;
    } else {
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        numChan = 3;
    }
    
		
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, pic->width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, pic->height );
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
 
    // set output line array to unsigned short
    unsigned short *Line =  (unsigned short *)malloc( numChan * pic->width *sizeof(unsigned short));
    
    int n= 0;
    for (int line = 0;line < pic->height;line++)
    {
        int addr = line * pic->width;
            
        for (unsigned int pixel = 0; pixel < pic->width; pixel++ )
        {
                
            int G = pic->buf[0][ addr + pixel ];
            
            int B = pic->buf[1][ addr + pixel ];
            int R = pic->buf[2][ addr + pixel ];
            
            
            R = R << SR;
            G = G << SR;
            B = B << SR;
            
            avg_R += R;
            avg_G += G;
            avg_B += B;
            
            n++;
            
                // R = X = 2*Dx + Y
            Line[pixel*numChan + 0] = (unsigned short) R;     // R = X
				
				// G = Y
            Line[pixel*numChan + 1] = (unsigned short) G;  //G = Y or inverse 2020/709 equation
				
				// B = X = 2*Dx + Y
            Line[pixel*numChan + 2] = (unsigned short) B;   // B = Z
				
            // A
            if(ALPHA)
                Line[pixel*numChan+3] = 65535;  // A
                
                //printf("Rp=%d   Gp=%d   Bp=%d | ",Line[pixel],Line[pixel+1],Line[pixel+2]);
                
			}
			
            
  			//printf("Writing strip %d with width %d bytes %d pixels\n",line,4*arraySizeX*2,arraySizeX);
			TIFFWriteRawStrip(tif, (tstrip_t)line, (tdata_t)Line, 2 * numChan * pic->width );
            
		}
		

    printf( "write_tiff(): avg R(%f) G(%f) B(%f)\n", avg_R / n, avg_G / n, avg_B / n );
    
    TIFFClose(tif);

    free( Line );

    return(0);
}








