
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

// read file header to determine picture size.
// malloc frame
// copy exr header info into data struct
// copy pixels
void
exr_test( char *filename )
{
    int w = 1920;
    int h = 1080;

    Array2D<Rgba> p (h, w);

    cout << "reading rgba file" << endl;
    readRgba1 ( filename, p, w, h);


    cout << "writing entire image" << endl;
    writeRgba1 ("rgba1.exr", &p[0][0], w, h);

    readHeader ("rgba1.exr");
}

