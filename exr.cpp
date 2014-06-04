
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>

#include <iostream>

#include "namespaceAlias.h"

#include "hdr.h"

using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


void
writeRgba1 (const char fileName[],
	    const Rgba *pixels,
	    int width,
	    int height)
{
    //
    // Write an RGBA image using class RgbaOutputFile.
    //
    //	- open the file
    //	- describe the memory layout of the pixels
    //	- store the pixels in the file
    //


    RgbaOutputFile file (fileName, width, height, WRITE_RGBA);
    file.setFrameBuffer (pixels, 1, width);
    file.writePixels (height);
}


void
readRgba1 (const char fileName[],
	   Array2D<Rgba> &pixels,
	   int &width,
	   int &height)
{
    //
    // Read an RGBA image using class RgbaInputFile:
    //
    //	- open the file
    //	- allocate memory for the pixels
    //	- describe the memory layout of the pixels
    //	- read the pixels from the file
    //

    RgbaInputFile file (fileName);
    Box2i dw = file.dataWindow();

    width  = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;
    pixels.resizeErase (height, width);

    file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels (dw.min.y, dw.max.y);
}


void
readHeader (const char fileName[])
{
    //
    // Read an image's header from a file, and if the header
    // contains comments and camera transformation attributes,
    // print the values of those attributes.
    //
    //	- open the file
    //	- get the file header
    //	- look for the attributes
    //

    RgbaInputFile file (fileName);

    const StringAttribute *comments =
	file.header().findTypedAttribute <StringAttribute> ("comments");

    const M44fAttribute *cameraTransform = 
	file.header().findTypedAttribute <M44fAttribute> ("cameraTransform");

    if (comments)
	cout << "comments\n   " << comments->value() << endl;

    if (cameraTransform)
	cout << "cameraTransform\n" << cameraTransform->value() << flush;
}


//write_exr_file( (char *)"test.exr", half_float_pixels, pic_width, pic_height );
// TODO: make half float flag
//    add stats

int write_exr_file(
    char *filename,
    int pic_width,
    int pic_height,
    int half_float_flag,
    t_pic *src_pic )
{
    Array2D<Rgba> half_float_pixels;

    half_float_pixels.resizeErase( pic_height, pic_width );

    for(int j=0;  j< pic_height; j++)
    {
        int addr = j * pic_width;
        
        for(int i=0; i< pic_width; i++)
        {
            float g = src_pic->fbuf[0][addr + i];
            float b = src_pic->fbuf[1][addr + i];
            float r = src_pic->fbuf[2][addr + i];
            
            half_float_pixels[j][i].g = g;
            half_float_pixels[j][i].b = b;
            half_float_pixels[j][i].r = r;
            
            // ignore alpha (if present)
        }
    }

    writeRgba1 ( filename , &half_float_pixels[0][0], pic_width, pic_height);
    
    // verify
    readHeader ( filename );
    return(0);
}


// read exr and convert to universal hdr2yuv picture buffer

void read_exr( t_pic *dst_pic, char *filename )
{
    int measure_exr_stats = 1;
    
    
    Array2D<Rgba> half_float_pixels;

    // open file first, see how big it is, then allocate memory
    RgbaInputFile EXRfile (filename);

    // extract picture size attributes (read in all pixels)
    Box2i dataw = EXRfile.dataWindow();
//    Box2i dispw = EXRfile.dispWindow();
    
    int pic_width = dataw.max.x - dataw.min.x + 1;
    int pic_height = dataw.max.y - dataw.min.y + 1;
    
    dst_pic->width = pic_width;
    dst_pic->height = pic_height;
    dst_pic->chroma_format_idc = CHROMA_444;
    dst_pic->bit_depth = 32;
    dst_pic->chroma_sample_loc_type = 0;
    dst_pic->transfer_characteristics = TRANSFER_LINEAR;
    dst_pic->colour_primaries = COLOR_PRIMARY_BT709;
    dst_pic->video_full_range_flag = 1;
    dst_pic->matrix_coeffs = MATRIX_GBR;
    
    
    
    // allocate memory now that active picure size is read from file header
    dst_pic->fbuf[0] = (float *) malloc( pic_width * pic_height * sizeof( float) );
    dst_pic->fbuf[1] = (float *) malloc( pic_width * pic_height * sizeof( float) );
    dst_pic->fbuf[2] = (float *) malloc( pic_width * pic_height * sizeof( float) );
    
    half_float_pixels.resizeErase( pic_height, pic_width );
    
    EXRfile.setFrameBuffer (&half_float_pixels[0][0] - dataw.min.x - dataw.min.y * pic_width, 1, pic_width);
    
    EXRfile.readPixels (dataw.min.y, dataw.max.y);
   
#include <limits>
    
    float max_r = numeric_limits<float>::min();
    float min_r = numeric_limits<float>::max();

    float max_g = numeric_limits<float>::min();
    float min_g = numeric_limits<float>::min();
    
    float max_b = numeric_limits<float>::min();
    float min_b = numeric_limits<float>::max();
    
    float sum_r = 0.0;
    float sum_g = 0.0;
    float sum_b = 0.0;
    
    for(int j=0;  j< pic_height; j++)
    {
        int addr = j * pic_width;
        
        for(int i=0; i< pic_width; i++)
        {
            float g = half_float_pixels[j][i].g;
            float b = half_float_pixels[j][i].b;
            float r = half_float_pixels[j][i].r;
            
            if( measure_exr_stats )
            {
                min_g = g < min_g ? g: min_g;
                max_g = g > max_g ? g: max_g;
                min_b = b < min_b ? b: min_b;
                max_b = b > max_b ? b: max_b;
                min_r = r < min_r ? r: min_r;
                max_r = r > max_r ? r: max_r;
                
                sum_r += r;
                sum_g += g;
                sum_b += b;
            }

            dst_pic->fbuf[0][addr + i] = g;
            dst_pic->fbuf[1][addr + i] = b;
            dst_pic->fbuf[2][addr + i] = r;
                           
            // ignore alpha (if present)
                           
        }
    }
    
    
    if( measure_exr_stats )
    {
        float size = (float) (pic_height * pic_width);
        
        printf("EXR input file stats: Min(R:%f,G:%f,B:%f) Max(R:%f,G:%f,B:%f) Avg(R:%f,G:%f,B:%f)\n",
               min_r, min_g, min_b, max_r, max_g, max_b, sum_r/size, sum_g/size, sum_b/size );
               
    }
    
    // TODO: de-alloc half_float_pixels
    
  //  cout << "reading rgba file" << endl;
  //  readRgba1 ( filename, half_float_pixels, w, h);

//    cout << "writing entire image" << endl;
    
    //write_exr_file( (char *)"test.exr", half_float_pixels, pic_width, pic_height );
    
}



