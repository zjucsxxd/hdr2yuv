// set up for 12 bits
// ASSUMES 3840x2160 ONLY


#include "/usr/local/include/tiffio.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void Subsample420to444(unsigned short ** src, unsigned short ** dst, short width, short height,short algorithmn,unsigned short minCV, unsigned short maxCV);

using namespace std;

// Globals

uint32 R;
uint32 G;
uint32 B;
uint32 A;

uint32 Y;
uint32 YnegRMx=0, YnegBMx=0;
int Yav;
int YavSave;
int Ybar;
int Y1,Y2,Y3,Y4;
int Dz;
int Dx;
int Rp,Bp;
uint32 invalidPixels = 0;

// file pointer for tifstripsize = 3840*2*4;
TIFF* tif;

// hard code 3840 x 2160
unsigned short stripsize;
unsigned short pixelStart = 0;
unsigned short numStrips = 2160;
short  stripStart;
ifstream yuvIn;

// from  rho-gamma.ctl

// non-linear (luma: V) --> linear (luminance: L)
double RHO_GAMMA_f( double V)
{
    const double rho = 25.0;
    const double gamma = 2.4;
    double L;
    
    // formula outputs normalized Luma from 0-1
    L = pow( (pow( rho, V) - 1.0)  / (rho - 1.0), gamma );
    
    return L;
}

// linear (luminance: L) -> non-linear (luma: V)

double RHO_GAMMA_r( double L)
{
    const double rho = 25.0;
    const double gamma = 2.4;
    double V;
    
    V = log( 1.0 + (rho - 1.0) * pow( L , 1.0/gamma ) ) / log( rho);
    
    return V;
}

int main(int argc, char* argv[])
{
    
    // set up to handle inverse color difference
    short D709 = 0;
    short D2020 = 0;
    short DYUVPRIME2 = 0;
    float tmpF = 0.0;
    short HD = 0; // ==1 if 1920x1080 cutout
    short qHD = 0; // ==1 if 960x540 cutout
    int frames = -999;
    short IPixF = 0;
    short Y500 =0;
    short Y100 =0;
    short DXYZ = 1;
    short BD = 12; // 10 bit mode
    short SR=4;
    unsigned short Half = 2048.0; // e.g half value 12 bits
    unsigned short Full = 4096.0; //
    short XX = 0;
    unsigned short minCV = 0; //12 bits
    unsigned short maxCV = 4095;
    short FIR = 1;
    short FULLRANGE = 0; // default is video range
    short ALPHA = 0; // no alpha channel is default
    short numChan2 = 3*2; // only 3 channels rgb with no alpha
    short numChan = 3;
    unsigned short D = 4; //D=4 for 12 bits =1 10bits = 16 for 14 bits
    unsigned short minVR, maxVR, minVRC,maxVRC;
    // video range is
    // Luma and R,G,B:  CV = Floor(876*D*N+64*D+0.5)
    // Chroma:  CV = Floor(896*D*N+64*D+0.5)
    
    //Process Args:
    short arg = 2;
    while(arg < argc) {
        
        
        
        if(strcmp(argv[arg],"-h")==0) {
            printf("\n ARGS:\n 709 (use Rec709 Color Dif)\n 2020 (use Rec2020 Color Dif)\n HD1920 (Format is 1920x1080 images)\n YUVPRIME (Y'u''v'')\nHD960 (Format is 960x540 images)\n (no args get Y'DzDx color difference and 3840x2160 cutout)\n\n\n");
            exit(0);
        }
        
        if(strcmp(argv[arg],"BOX")==0)FIR = 0;
        if(strcmp(argv[arg],"FULL")==0)FULLRANGE = 1;
        if(strcmp(argv[arg],"ALPHA")==0){
     		ALPHA = 1;
     		numChan2= 4*2;
     		numChan = 4;
        }
        
        if(strcmp(argv[arg],"709")==0)D709 = 1;
        if(strcmp(argv[arg],"2020")==0)D2020 = 1;
        if(strcmp(argv[arg],"YUVPRIME2")==0)DYUVPRIME2 = 1;
        
        if(strcmp(argv[arg],"HD1920")==0)HD = 1;
        if(strcmp(argv[arg],"HD960")==0)qHD = 1;
        
        
        if(strcmp(argv[arg],"Y100")==0)Y100 = 1;
        if(strcmp(argv[arg],"Y500")==0)Y500 = 1;
        
        
        // B12 is by default
        if(strcmp(argv[arg],"B10")==0) {
            BD = 10;
            SR=6;
            Half = 512.0;
            Full = 1024.0;
            minCV = 0;
            maxCV = 1023;
            D=1;
            printf("\nprocessing line data 10 bits\n");
        }
        
        if(strcmp(argv[arg],"B14")==0) {
            BD = 14;
            SR=2;
            Half = 8192;
            Full = 16384;
            minCV = 0;
            maxCV = 16383;
            D=16;
            printf("\nprocessing line data 14 bits\n");
        }
        
        if(D709 || D2020 || Y100 || Y500)DXYZ = 0;
        
        if(strcmp(argv[arg],"-I")==0)IPixF = 1;
        if(strcmp(argv[arg],"-X")==0)XX = 1;
        
        if(strcmp(argv[arg],"-f")==0) {
			arg++;
			if(arg < argc)frames=atoi(argv[arg]);
        }
        
        arg++;
    }
    if(D709)printf("Processing for Rec709\n");
    if(D2020)printf("Processing for Rec2020\n");
    if(DYUVPRIME2)printf("Processing for Y'u''v'' (Y=rho-gamma, u'',v'' = linear)\n");
    if(HD)printf("Processing for HD1920x1080\n");
    if(qHD)printf("Processing for HD960x540\n");
    
    // Set up for video range if needed
    if(!FULLRANGE) {
        printf("Processing for Video Range\n");
        minVR = 64*D;
        maxVR = 876*D+minVR;
        minVRC = minVR;
        maxVRC = 896*D+minVRC;
        //achromatic point fo r chroma will be "Half"(e.g. 512 for 10 bits, 2048 for 12 bits etc..)
        
    }
    
    stripsize = 3840*numChan2;
    stripStart =0;
    pixelStart = 0;
    if(HD) {
        stripsize = 1920*numChan2;
        numStrips = 1080;
    }
    if(qHD) {
        stripsize= 960*numChan2;
        numStrips = 540;
    }
    printf ("Stripsize (bytes): %d, %d (pixels), numStrips %d \n",stripsize, stripsize/8, numStrips);
    
    // Saving Invalid Pixel Data as InvalidPixel.txt:
    ofstream invPix;
    if(IPixF) invPix.open("InvalidPixel.txt");
    
    // set up alternate differencing equations for Y'DzwDxw
    float T,U,V,W;
    if (Y100) {
        printf("Setting TUVW for 100nit PQ color difference point\n");
        T = 0.98989899;
        U = 2.0;
        V = 1.016835017;
        W = 2.03367;
    } else if(Y500) {
        printf("Setting TUVW for 500nit PQ color difference point\n");
        T = 0.99203764;
        U = 2.0;
        V = 1.013391241;
        W = 2.026782;
    }
    
    // Readign array (stripsize/numChan2 = 3840 unsigned shorts)
    unsigned short *yuvLine;
    // Array to store line of output for writing process
    // will be allocated to line width with 4 unsigned shorts
    unsigned short *Line;
    
    // Allocate memory to read each image
    unsigned short** YP;
    int arraySizeX = stripsize/numChan2 - pixelStart/numChan2; // eg 3840
    int arraySizeY = numStrips;
    int arraySizeXH = arraySizeX/2; // eg 1920 cols
    int arraySizeYH = arraySizeY/2; // eg 1080 rows
    printf("Frame Size: %d x %d\n",arraySizeX,arraySizeY);
    YP = (unsigned short**) malloc(arraySizeX*sizeof(unsigned short*)); // cols
    for (int i = 0; i < arraySizeX; i++)
        YP[i] = (unsigned short*) malloc(arraySizeY*sizeof(unsigned short)); //rows
    
    // allocate for 444 chroma
    
    unsigned short** Cb444;
    Cb444 = (unsigned short**) malloc(arraySizeX*sizeof(unsigned short*)); // cols
    for (int i = 0; i < arraySizeX; i++)
        Cb444[i] = (unsigned short*) malloc(arraySizeY*sizeof(unsigned short)); //rows
    
    unsigned short** Cr444;
    Cr444 = (unsigned short**) malloc(arraySizeX*sizeof(unsigned short*)); // cols
    for (int i = 0; i < arraySizeX; i++)
        Cr444[i] = (unsigned short*) malloc(arraySizeY*sizeof(unsigned short)); //rows
    
    // allocate for 420 chroma
    unsigned short** DzP;
    DzP = (unsigned short**) malloc(arraySizeXH*sizeof(unsigned short*)); // cols
    for (int i = 0; i < arraySizeXH; i++)
        DzP[i] = (unsigned short*) malloc(arraySizeYH*sizeof(unsigned short)); // rows
    
    unsigned short** DxP;
    DxP = (unsigned short**) malloc(arraySizeXH*sizeof(unsigned short*)); // cols
    for (int i = 0; i < arraySizeXH; i++)
        DxP[i] = (unsigned short*) malloc(arraySizeYH*sizeof(unsigned short)); // rows
    
    // set output line array to unsigned short
    Line =  (unsigned short *)malloc(((numChan2/2)*arraySizeX*sizeof(unsigned short)));
    
    yuvLine = (unsigned short *) malloc(arraySizeX*sizeof(unsigned short));
    
    
    
	// Open yuv file
	// Open Binary PlanarYUV file for reading:
	yuvIn.open(argv[1], ios::in | ios::binary);
	printf("Opened YDzDx.yuv reading...:\n");
    
	// process yuv
	int tifNum = 0;
	char tifName[] = "tifXYZ/XpYpZp00000.tif";
	int line = 0;
	int pixel = 0;
	while(yuvIn)
	{
		YnegRMx=0;
		YnegBMx=0;
		// read Y' data
		for ( line = 0;line < arraySizeY;line++)
		{
			yuvIn.read((char *)yuvLine, arraySizeX*sizeof(unsigned short));
			for ( pixel = 0; pixel < arraySizeX;pixel++) {
				YP[pixel][line] = yuvLine[pixel];
				//printf(" YP[%d][%d]= %d ",pixel,line,YP[pixel][line]);
				if(!FULLRANGE) {
					YP[pixel][line] = (YP[pixel][line]<minVR) ? minVR : YP[pixel][line];
					YP[pixel][line] = (YP[pixel][line]>maxVR) ? maxVR : YP[pixel][line];
				}
			}
		}
		
		// read Dz data
		for ( line = 0;line < arraySizeY/2;line++)
		{
			yuvIn.read((char *)yuvLine, arraySizeX*sizeof(unsigned short)/2);
			for ( pixel = 0; pixel < arraySizeX/2;pixel++){
				DzP[pixel][line] = yuvLine[pixel];
				if(!FULLRANGE) {
					DzP[pixel][line] = (DzP[pixel][line]<minVRC) ? minVRC : DzP[pixel][line];
					DzP[pixel][line] = (DzP[pixel][line]>maxVRC) ? maxVRC : DzP[pixel][line];
				}
			}
		}
        
		// read Dx data
		for ( line = 0;line < arraySizeY/2;line++)
		{
			yuvIn.read((char *)yuvLine, arraySizeX*sizeof(unsigned short)/2);
			for ( pixel = 0; pixel < arraySizeX/2;pixel++) {
				DxP[pixel][line] = yuvLine[pixel];
				if(!FULLRANGE) {
					DxP[pixel][line] = (DxP[pixel][line]<minVRC) ? minVRC : DxP[pixel][line];
					DxP[pixel][line] = (DxP[pixel][line]>maxVRC) ? maxVRC : DxP[pixel][line];
				}
			}
		}
        
		printf("Writing tifXYZ/XpYpZp%05d.tif\n",tifNum);
		// Open TIF File
		sprintf(tifName, "tifXYZ/XpYpZp%05d.tif",tifNum);
		invalidPixels = 0;
		
		tif = TIFFOpen(tifName, "w");
		if(ALPHA) {
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
		} else {
            TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
		}
		
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, arraySizeX);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, arraySizeY);
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		
		Subsample420to444(DzP, Cb444, arraySizeX, arraySizeY, FIR, minCV,  maxCV);
		Subsample420to444(DxP, Cr444, arraySizeX, arraySizeY, FIR, minCV,  maxCV);
		
		for (int line = 0;line < arraySizeY;line++)
		{
			for (unsigned int pixel = 0; pixel < numChan*arraySizeX;pixel+=numChan)
			{
                
                // Y = G = Y
                // Dz = -0.5*G + 0.5*B  = 0.5*Z - 0.5*Y + 2048
                // Dx = 0.5*R -0.5*G  = 0.5*X - 0.5*Y  +2048
                
                // 4095 = 2047.5 * 2
                // recall 2047.5 came from adding 2047 to Dz, Dx
                //  and adding 8 during initial round off from 16bits to 12
                
                
                
                // Try calculating an average Y to use with subsampled Dz, Dx
                //Yav = (YP[pixel>>2][line>>1 + line>>1] + YP[1+ pixel>>2][line>>1 + line>>1] + YP[1+pixel>>2][line>>1 + line>>1] + YP[1 + pixel>>2][1 + line>>1 + line>>1])/4;
                Yav = (int)(YP[pixel/numChan][line]);
                YavSave = Yav; // keep copy of original Y'
                
                // Calculate Ybar
                if (XX) {
                    short p0 = (pixel/numChan) % 2;
                    short l0 = line % 2;
                    if(p0 == 0 && l0 == 0) {
                        Ybar = (int)(YP[pixel/numChan][line]) + (int)(YP[(pixel+numChan)/numChan][line]) + (int)(YP[pixel/numChan][line+1]) + (int)(YP[(pixel+numChan)/numChan][line+1]);
                        Ybar = Ybar/4;
                    }
                    if(p0 == 1 && l0 == 0) {
                        Ybar = (int)(YP[(pixel-numChan)/numChan][line]) + (int)(YP[pixel/numChan][line]) + (int)(YP[(pixel-numChan)/numChan][line+1]) + (int)(YP[pixel/numChan][line+1]);
                        Ybar = Ybar/4;
                    }
                    if(p0 == 0 && l0 == 1) {
                        Ybar = (int)(YP[pixel/numChan][line-1]) + (int)(YP[(pixel+numChan)/numChan][line-1]) + (int)(YP[pixel/numChan][line]) + (int)(YP[(pixel+numChan)/numChan][line]);
                        Ybar = Ybar/4;
                    }
                    if(p0 == 1 && l0 == 1) {
                        Ybar = (int)(YP[(pixel-numChan)/numChan][line-1]) + (int)(YP[pixel/numChan][line-1]) + (int)(YP[(pixel-numChan)/numChan][line]) + (int)(YP[pixel/numChan][line]);
                        Ybar = Ybar/4;
                    }
                    if(Ybar<1)Ybar = 1;
                    if(Ybar>(Full-1))Ybar=Full-1;
                }
                
                
                if(DXYZ) {
                    if(XX) {
                        //reconstruct with Ybar then scale
                        float RED,BLUE;
                        RED = (2.0*(float)(Cr444[pixel/numChan][line]) - (float)(Full-1.0) + (float)Ybar) * ((float)Yav)/((float)Ybar);
                        BLUE = (2.0*(float)(Cb444[pixel/numChan][line]) - (float)(Full-1.0) + (float)Ybar) * ((float)Yav)/((float)Ybar);
                        if(RED>(Full-1.5))RED=Full-1.0;
                        if(BLUE>(Full-1.5))BLUE=Full-1.0;
                        Rp = RED;
                        Bp = BLUE;
                        
                    } else {
                        Rp = ((int)2*(int)(Cr444[pixel/numChan][line]) - (int)(Full-1.0) + Yav);
                        Bp = ((int)2*(int)(Cb444[pixel/numChan][line]) - (int)(Full-1.0) + Yav);
                    }
                } else if(D2020){
                    tmpF = ((float)(Cb444[pixel/numChan][line])-(Half-0.5))*1.8814 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Bp = tmpF;
                    tmpF = ((float)(Cr444[pixel/numChan][line])-(Half-0.5))*1.4746 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Rp = tmpF;
                    tmpF = ((float)Yav - 0.0593*(float)Bp -0.2627*(float)Rp)/0.6780 +0.5 ; // green
                    if(tmpF > (Full-1.0)) tmpF = (Full-1.0);
                    Yav = tmpF; //green
                } else if(D709) {
                    tmpF = ((float)(Cb444[pixel/numChan][line])-(Half-0.5))*1.8556 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Bp = tmpF;
                    tmpF = ((float)(Cr444[pixel/numChan][line])-(Half-0.5))*1.5748 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Rp = tmpF;
                    tmpF = ((float)Yav - 0.07222*(float)Bp -0.2126*(float)Rp)/0.7152 +0.5 ; // green
                    if(tmpF > (Full-1.0)) tmpF = (Full-1.0);
                    Yav = tmpF;  //green
                } else if(Y100 || Y500) {
                    tmpF = ((float)(Cb444[pixel/numChan][line])-(Half-0.5))*W + ((float)Yav)*V;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Bp = tmpF;
                    tmpF = ((float)(Cr444[pixel/numChan][line])-(Half-0.5))*U + ((float)Yav)*T;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Rp = tmpF;
                } else if(DYUVPRIME2) {
                    
                    // Convert Y'u''v'' back to XYZ
                    const double e = 0.25;
                    const double ur = 0.1978;
                    const double vr = 0.4683;
                    double u1, v1;
                    
                    double Y1 = Yav;
                    unsigned short iu16_u2 = Cb444[pixel/numChan][line];
                    unsigned short iu16_v2 = Cr444[pixel/numChan][line];
                    
                    double u2 = ((double) iu16_u2)/ Full;
                    double v2 = ((double) iu16_v2)/ Full;
                    
                    if( Y1 > 0.0 ){
                        u1 = (((u2 - ur) * fmax(Y1,e)) / Y1) + ur;
                        v1 = (((v2 - vr) * fmax(Y1,e)) / Y1) + vr;
                    }
                    
                    double linearY = RHO_GAMMA_f( Y1 );
                    
                    double Z = linearY* ((3.0 - 0.75*u1)/v1 - 5.0 );
                    double X = (linearY*9*u1) / (4*v1);
                    
                    ///if(tmpF > (Full-1.0))
                       // tmpF = (Full-1.0);
                    
                    // convert from real-value (0.0 to 1.0) to integer-like range
                    Z *= Full;
                    X *= Full;
                    
                    if(Z > (Full-1.0))
                        Z = (Full-1.0);

                    if(X > (Full-1.0))
                        X = (Full-1.0);
                    
                    Bp = (int) Z;
                    Rp = (int) X;
                    
 //               } else if(DYUVPRIME1) {
                } else {
                    printf("Can't determine color difference to use?\n\n");
                    exit(0);
                }
                
                if(Yav < 0)
                {
                    Yav = 0;
                    invalidPixels++;
                    if(IPixF)invPix << "Pixel=" << pixel/8 << ", " << line << "  Y'=" << YavSave << "  Dz=" << Cb444[pixel/numChan][line] << "  Dx=" << Cr444[pixel/numChan][line] << "  G'=" << Yav << " !! Gneg\n";
                }
                
                if(Rp < 0 )
                {
                    
                    if(IPixF && YavSave>0)invPix << "Pixel=" << pixel/8 << ", " << line << "  Y'=" << YavSave << "  Dz=" << Cb444[pixel/numChan][line] << "  Dx=" << Cr444[pixel/numChan][line] << "  R'=" << Rp << " !! Rneg\n";
                    
                    if(Yav > YnegRMx && Yav > 0)
                    {
                        YnegRMx = Yav;
                        printf("Line: %d Pixel: %d, Y %d, Dz: %d, Dx %d, X' %d, YnegRMx %d\n", line,pixel/8, Yav, Cb444[pixel/numChan][line], Cr444[pixel/numChan][line], Rp,YnegRMx);
                    }
                    Rp = 0; // zero invalid pixels whether Yav == 0 or not
                    if(YavSave != 0)invalidPixels++;
                }
                
                if(Bp < 0 )
                {
                    
                    if(IPixF && YavSave>0)invPix << "Pixel=" << pixel/8 << ", " << line << "  Y'=" << YavSave << "  Dz=" << Cb444[pixel/numChan][line] << "  Dx=" << Cr444[pixel/numChan][line] << "  B'=" << Bp << " !! Bneg\n";
                    
                    if(Yav > YnegBMx && Yav > 0)
                    {
                        YnegBMx = Yav;
                        printf("Line: %d Pixel: %d, Y %d, Dz: %d, Dx %d, Z' %d,YnegBMx %d\n", line,pixel/8, Yav, Cb444[pixel/numChan][line], Cr444[pixel/numChan][line], Bp, YnegBMx);
                    }
                    Bp = 0; // zero invalid pixels whether Yav == 0 or not
                    if(YavSave != 0)invalidPixels++;
                }
                
                
                // Insure RGB are clipped to video range if required:
                if(!FULLRANGE) {
                    Rp = (Rp<minVR) ? minVR : Rp;
                    Yav = (Yav<minVR) ? minVR : Yav;
                    Bp = (Bp<minVR) ? minVR : Bp;
                    
                    Rp = (Rp>maxVR) ? maxVR : Rp;
                    Yav = (Yav>maxVR) ? maxVR : Yav;
                    Bp = (Bp>maxVR) ? maxVR : Bp;
                    
                }
                
                
                // Calculate Rp from Cr444
                
                
                // Calculate Bp from Cb444
                
                
                
                // R = X = 2*Dx + Y
                Line[pixel] = ((unsigned short)Rp) << SR;     // R = X
				
				// G = Y
				Line[pixel+1] = ((unsigned short)Yav) << SR;  //G = Y or inverse 2020/709 equation
				
				// B = X = 2*Dx + Y
				Line[pixel+2] = ((unsigned short)Bp) << SR;   // B = Z
				
				// A
				if(ALPHA) Line[pixel+3] = 65535;  // A
                
                //printf("Rp=%d   Gp=%d   Bp=%d | ",Line[pixel],Line[pixel+1],Line[pixel+2]);
                
			}
			
            
  			//printf("Writing strip %d with width %d bytes %d pixels\n",line,4*arraySizeX*2,arraySizeX);
			TIFFWriteRawStrip(tif, (tstrip_t)line, (tdata_t)Line, numChan2*arraySizeX);
            
		}
		
		TIFFClose(tif);
		
		tifNum++;
		
		printf("Max YnegR = %d, Max YnegB = %d\n",YnegRMx, YnegBMx);
		printf("Invalid Pixels:  %d\n",invalidPixels);
		if(frames > 0) {
			frames--;
			if(frames == 0)exit(0);
		}
        
	}
    
    
    if(IPixF) invPix.close();
    
}


void Subsample420to444(unsigned short ** src, unsigned short ** dst, short width, short height,short algorithmn,unsigned short minCV, unsigned short maxCV)
{
	if(algorithmn == 0) {
		short widthH = width/2;
		short heightH = height/2;
		for (int line = 0;line < heightH;line++)
		{
			for (unsigned int pixel = 0; pixel < widthH;pixel++) {
				dst[2*pixel][2*line] = src[pixel][line];
				dst[2*pixel+1][2*line] = src[pixel][line];
				dst[2*pixel][2*line+1] = src[pixel][line];
				dst[2*pixel+1][2*line+1] = src[pixel][line];
            }
        }
    } else {
        // Implement FIR filter for 420 to 422 then 444
        
        int wH,i, j, j2;
        int jm6, jm5, jm4, jm3, jm2, jm1, jp1, jp2, jp3, jp4, jp5, jp6, jp7;
        
        // width and height come in as of src array
        
        
        // Allocate dst422 storage
        short w422 = width>>1;
        short h420 = height>>1;
        unsigned short** dst422 = (unsigned short**) malloc(w422*sizeof(unsigned short*)); // cols
        for (i = 0; i < w422; i++)
            dst422[i] = (unsigned short*) malloc(height*sizeof(unsigned short)); //rows
        
        float scale = 256.0;
        float c3    = 3.0/scale;
        float c16   = 16.0/scale;
        float c67  =  67.0/scale;
        float c227 =  227.0/scale;
        float c32  =   32.0/scale;
        float c7   =   7.0/scale;
        float temp;
        
        /* intra frame */
        for (i=0; i<w422; i++) // w here is half width (e.g. 422 width)
        {
            for (j=0; j<h420; j++)
            {
                j2 = j<<1;
                jm3 = (j<3) ? 0 : j-3;
                jm2 = (j<2) ? 0 : j-2;
                jm1 = (j<1) ? 0 : j-1;
                jp1 = (j<h420-1) ? j+1 : h420-1;
                jp2 = (j<h420-2) ? j+2 : h420-1;
                jp3 = (j<h420-3) ? j+3 : h420-1;
                
                /* FIR filter coefficients (*256): 5 -21 70 228 -37 11 */
                /* New FIR filter coefficients (*256): 3 -16 67 227 -32 7 */
                temp =     c3*((float)(src[i][jm3]))
                -c16*((float)(src[i][jm2]))
                +c67*((float)(src[i][jm1]))
                +c227*((float)(src[i][j]))
                -c32*((float)(src[i][jp1]))
                +c7*((float)(src[i][jp2]))+0.5;
                if(temp>maxCV)temp = maxCV;
                if(temp<minCV)temp = minCV;
                dst422[i][j2] = (unsigned short)temp;
                
                temp = c3*((float)(src[i][jp3]))
                -c16*((float)(src[i][jp2]))
                +c67*((float)(src[i][jp1]))
                +c227*((float)(src[i][j]))
                -c32*((float)(src[i][jm1]))
                +c7*((float)(src[i][jm2]))+0.5;
                if(temp>maxCV)temp = maxCV;
                if(temp<minCV)temp = minCV;
                dst422[i][j2+1] = (unsigned short)temp;
                
            }
        }
        
        // 422 to 444 now
        int im2, im1, ip1, ip2, ip3,i2;
        scale = 256.0;
        float c21 = 21.0/scale;
        float c52  = 52.0/scale;
        float c159 =  159.0/scale;
        float c256 =  256.0/scale;
        
        
        for (j=0; j<height; j++)
        {
            for (i=0; i<w422; i++) // half width (e.g. 422 width)
            {
                i2 = i<<1;
                im2 = (i<2) ? 0 : i-2;
                im1 = (i<1) ? 0 : i-1;
                ip1 = (i<w422-1) ? i+1 : w422-1;
                ip2 = (i<w422-2) ? i+2 : w422-1;
                ip3 = (i<w422-3) ? i+3 : w422-1;
                
                /* FIR filter coefficients (*256): 21 0 -52 0 159 256 159 0 -52 0 21 */
                /* even samples (0 0 256 0 0) */
                dst[i2][j] = dst422[i][j];
                
                
                /* odd samples (21 -52 159 159 -52 21) */
                temp = c21*(((float)(dst422[im2][j]))+((float)(dst422[ip3][j])))
                -c52*(((float)(dst422[im1][j]))+((float)(dst422[ip2][j]))) 
                +c159*(((float)(dst422[i][j]))+((float)(dst422[ip1][j])))+0.5;
                if(temp>maxCV)temp = maxCV;
                if(temp<minCV)temp = minCV;
                dst[i2+1][j] = (unsigned short)temp;
                
            }
            
        }    
        
        
    }
    
}
