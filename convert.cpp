// convert.cpp:   color conversion and chroma resampling

#include <iostream>
//#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hdr.h"


float RHO_GAMMA_f( float V)
{
    const float rho = 25.0;
    const float gamma = 2.4;
    float L;
    
    // 2-May-2014 email from rene.van.der.vleuten@philips.com does not have
    // a separate function for < 0.018 as was documented in Phlip's earlier
    // ITU-R submission (R12-WP6C-C-0302). Instead, the curve now has an offset
    // that encompasses the 1.8% dark range
    
    // formula outputs normalized Luma from 0-1
    L = pow( (pow( rho, V) - 1.0)  / (rho - 1.0), gamma );
    
    return L;
}

// encode (V^gamma ish calculate L from V)
float RHO_GAMMA_r( float L)
{
    const float rho = 25.0;
    const float gamma = 2.4;
    float V;
    
    V = log( 1.0 + (rho - 1.0) * pow( L , 1.0/gamma ) ) / log( rho);
    
    return V;
}

// SMPTE FCD ST 2084
// non-linear to linear
float PQ10000_f( float V)
{
    float L;
    // Lw, Lb not used since absolute Luma used for PQ
    // formula outputs normalized Luma from 0-1
    L = pow(fmax(pow(V, 1.0/78.84375) - 0.8359375 ,0.0)/(18.8515625 - 18.6875 * pow(V, 1.0/78.84375)),1.0/0.1593017578);
    
    return L;
}

// SMPTE FCD ST 2084
// encode (V^gamma ish calculate L from V)
//  encode   V = ((c1+c2*Y**n))/(1+c3*Y**n))**m
float PQ10000_r( float L)
{
    float V;
    // Lw, Lb not used since absolute Luma used for PQ
    // input assumes normalized luma 0-1
    V = pow((0.8359375+ 18.8515625*pow((L),0.1593017578))/(1+18.6875*pow((L),0.1593017578)),78.84375);
    return V;
}


// BT.2020 , BT.601, BT.709 EOTF:  non-linear (gamma) --> linear
float bt1886_f( float V, float gamma, float Lw, float Lb)
{
    // The reference EOTF specified in Rec. ITU-R BT.1886
    // L = a(max[(V+b),0])^g
    float a = pow( pow( Lw, 1./gamma) - pow( Lb, 1./gamma), gamma);
    float b = pow( Lb, 1./gamma) / ( pow( Lw, 1./gamma) - pow( Lb, 1./gamma));
    float L = a * pow( fmax( V + b, 0.), gamma);
    return L;
}


// BT.2020 , BT.601, BT.709 OETF:  linear --> non-linear (gamma)
float bt1886_r( float L, float gamma, float Lw, float Lb)
{
    // The reference EOTF specified in Rec. ITU-R BT.1886
    // L = a(max[(V+b),0])^g
    float a = pow( pow( Lw, 1./gamma) - pow( Lb, 1./gamma), gamma);
    float b = pow( Lb, 1./gamma) / ( pow( Lw, 1./gamma) - pow( Lb, 1./gamma));
    float V = pow( fmax( L / a, 0.), 1./gamma) - b;
    return V;
}



void Subsample444to420_box(unsigned short *dst_plane, unsigned short *src_plane, short width, short height, unsigned long minCV, unsigned long maxCV)
{
   // algorithmn 0 is box

    unsigned short C444[4][4];
    unsigned short C420[2][2];
    
    printf("doing new Box\n");
    
    for (int strip = 0; strip < height; strip+=4)
    {
        
        for (int pixel = 0; pixel < width; pixel+=4) {
            
            
            //C444 is row by column
            // src and dst are x by y e.g. column by row (enjoy the madness!)
            //C444[0][0]  = src[pixel][strip];
            
            int src_addr0 = (strip+0)*width + pixel;
            int src_addr1 = (strip+1)*width + pixel;
            int src_addr2 = (strip+2)*width + pixel;
            int src_addr3 = (strip+3)*width + pixel;
            
            C444[0][0]  = src_plane[ src_addr0 ];
            C444[0][1]  = src_plane[ src_addr0 + 1 ];
            C444[0][2]  = src_plane[ src_addr0 + 2 ];
            C444[0][3]  = src_plane[ src_addr0 + 3 ];
            
            
            
            C444[1][0]  = src_plane[ src_addr1 ];
            C444[1][1]  = src_plane[ src_addr1 + 1 ];
            C444[1][2]  = src_plane[ src_addr1 + 2 ];
            C444[1][3]  = src_plane[ src_addr1 + 3 ];
            
            
            
            C444[2][0]  = src_plane[ src_addr2 ];
            C444[2][1]  = src_plane[ src_addr2 + 1 ];
            C444[2][2]  = src_plane[ src_addr2 + 2 ];
            C444[2][3]  = src_plane[ src_addr2 + 3 ];
            
            
            
            
            C444[3][0]  = src_plane[ src_addr3 ];
            C444[3][1]  = src_plane[ src_addr3 + 1 ];
            C444[3][2]  = src_plane[ src_addr3 + 2 ];
            C444[3][3]  = src_plane[ src_addr3 + 3 ];
            
                
            // step line in blocks of 4 pixels:
            // ib is the pixel counter from 1-4 such
            // that 8*ib is byte location of pixel start
            for (int ib = 0;ib<4;ib+=1) {
                    
                    
                // Perform box averaging
                // Create 420 blocks of 2x2 from 4x4
                // rows = scanlines columns are pixels across
                // should i do higher precision math here?
                //  0 1 2 3
                //  1
                //  2
                //  3
                C420[0][0] = ((unsigned long)C444[0][0] + (unsigned long)C444[0][1] + (unsigned long)C444[1][0] + (int)C444[1][1])/4;
                C420[0][1] = ((unsigned long)C444[0][2] + (unsigned long)C444[0][3] + (unsigned long)C444[1][2] + (int)C444[1][3])/4;
                C420[1][0] = ((unsigned long)C444[2][0] + (unsigned long)C444[2][1] + (unsigned long)C444[3][0] + (int)C444[3][1])/4;
                C420[1][1] = ((unsigned long)C444[2][2] + (unsigned long)C444[2][3] + (unsigned long)C444[3][2] + (int)C444[3][3])/4;
                    
                // write 2x2 Dz elements
                dst_plane[(pixel/2)+ (strip/2)*(width/2)] = C420[0][0];
                dst_plane[ 1+(pixel/2) + (strip/2) * (width/2) ] = C420[0][1];
                dst_plane[(pixel/2)  + (1+(strip/2))*(width/2)] = C420[1][0];
                dst_plane[1+(pixel/2)+ (1+(strip/2))*(width/2)] = C420[1][1];
                
            }
        }
    }
        
}


void Subsample444to420_box_float(float  *dst_plane, float *src_plane, short width, short height, unsigned long minCV, unsigned long maxCV)
{
    // algorithmn 0 is box
    
    float C444[4][4];
    float C420[2][2];
    
    printf("doing new Box\n");
    
    for (int strip = 0; strip < height; strip+=4)
    {
        
        for (int pixel = 0; pixel < width; pixel+=4) {
            
            
            //C444 is row by column
            // src and dst are x by y e.g. column by row (enjoy the madness!)
            //C444[0][0]  = src[pixel][strip];
            
            int src_addr0 = (strip+0)*width + pixel;
            int src_addr1 = (strip+1)*width + pixel;
            int src_addr2 = (strip+2)*width + pixel;
            int src_addr3 = (strip+3)*width + pixel;
            
            C444[0][0]  = src_plane[ src_addr0 ];
            C444[0][1]  = src_plane[ src_addr0 + 1 ];
            C444[0][2]  = src_plane[ src_addr0 + 2 ];
            C444[0][3]  = src_plane[ src_addr0 + 3 ];
            
            
            
            C444[1][0]  = src_plane[ src_addr1 ];
            C444[1][1]  = src_plane[ src_addr1 + 1 ];
            C444[1][2]  = src_plane[ src_addr1 + 2 ];
            C444[1][3]  = src_plane[ src_addr1 + 3 ];
            
            
            
            C444[2][0]  = src_plane[ src_addr2 ];
            C444[2][1]  = src_plane[ src_addr2 + 1 ];
            C444[2][2]  = src_plane[ src_addr2 + 2 ];
            C444[2][3]  = src_plane[ src_addr2 + 3 ];
            
            
            
            
            C444[3][0]  = src_plane[ src_addr3 ];
            C444[3][1]  = src_plane[ src_addr3 + 1 ];
            C444[3][2]  = src_plane[ src_addr3 + 2 ];
            C444[3][3]  = src_plane[ src_addr3 + 3 ];
            
            
            // step line in blocks of 4 pixels:
            // ib is the pixel counter from 1-4 such
            // that 8*ib is byte location of pixel start
            for (int ib = 0;ib<4;ib+=1) {
                
                
                // Perform box averaging
                // Create 420 blocks of 2x2 from 4x4
                // rows = scanlines columns are pixels across
                // should i do higher precision math here?
                //  0 1 2 3
                //  1
                //  2
                //  3
                C420[0][0] = (C444[0][0] + C444[0][1] + C444[1][0] + C444[1][1])/4;
                C420[0][1] = (C444[0][2] + C444[0][3] + C444[1][2] + C444[1][3])/4;
                C420[1][0] = (C444[2][0] + C444[2][1] + C444[3][0] + C444[3][1])/4;
                C420[1][1] = (C444[2][2] + C444[2][3] + C444[3][2] + C444[3][3])/4;
                
                // write 2x2 Dz elements
                dst_plane[(pixel/2)+ (strip/2)*(width/2)] = C420[0][0];
                dst_plane[ 1+(pixel/2) + (strip/2) * (width/2) ] = C420[0][1];
                dst_plane[(pixel/2)  + (1+(strip/2))*(width/2)] = C420[1][0];
                dst_plane[1+(pixel/2)+ (1+(strip/2))*(width/2)] = C420[1][1];
                
            }
        }
    }
    
}




void Subsample444to420_FIR( unsigned short *dst_plane, unsigned short *src_plane, short width, short height,unsigned long minCV, unsigned long maxCV)
{
	   
        // perform 444 to 422 conversion horizontally
        // array is aligned as src & dst[width][height]
        printf("FIR Filter subsampling...\n");
        
        int w, jm6, jm5, jm4, jm3, jm2, jm1;
        int jp1, jp2, jp3, jp4, jp5, jp6;
        
        // Allocate dst422 storage
        short w422 = width>>1;
        
        //unsigned short** dst422 = (unsigned short**) malloc(w422*sizeof(unsigned short*)); // cols
        
        unsigned short*  dst422 = (unsigned short*) malloc( height * w422 * sizeof(unsigned short) );
        
        int im5, im4, im3, im2, im1, ip1, ip2, ip3, ip4, ip5, ip6;
        float scale = 512.0;
        float c21 = 21.0/scale;
        float c52  = 52.0/scale;
        float c159 =  159.0/scale;
        float c256 =  256.0/scale;
        
        float temp;
        
        
        // stage 1:  4:4:4 to 4:2:2 (horizontal resampling)
        
        for (int j=0; j<height; j++)
        {
            for (int i=0; i<width; i+=2)
            {
                // picture border logic
                im5 = (i<5) ? 0 : i-5;
                im3 = (i<3) ? 0 : i-3;
                im1 = (i<1) ? 0 : i-1;
                ip1 = (i<width-1) ? i+1 : width-1;
                ip3 = (i<width-3) ? i+3 : width-1;
                ip5 = (i<width-5) ? i+5 : width-1;
                
                unsigned short *s = &(src_plane[ j*width ]);
                
                // convolution
                temp = c21*((float)(s[im5])+
                            (float)(s[ip5]))
                -c52*((float)(s[im3])+
                      (float)(s[ip3]))
                +c159*((float)(s[im1])+
                       (float)(s[ip1]))
                +c256*((float)(s[i]))+0.5;

                // clipping
                if(temp>maxCV)temp = maxCV;
                if(temp<minCV)temp = minCV;
                
                dst422[ j * (width>>1) + (i>>1) ] = (unsigned short)temp;
                
                //printf("s:%d  d:%d  ",src[i][j],dst422[(i>>1)][j]);
            }
        }
        
        
        // stage 2: perform 422 to 420 conversion vertically
        float c228 = 228.0/scale;
        float c70  =  70.0/scale;
        float c37  =  37.0/scale;
        c21  =  21.0/scale;
        float c11  =  11.0/scale;
        float c5   =   5.0/scale;
        
        
		for (int i=0; i<w422; i++)
		{
			for (int j=0; j<height; j+=2)
			{
				jm5 = (j<5) ? 0 : j-5;
				jm4 = (j<4) ? 0 : j-4;
				jm3 = (j<3) ? 0 : j-3;
				jm2 = (j<2) ? 0 : j-2;
				jm1 = (j<1) ? 0 : j-1;
				jp1 = (j<height-1) ? j+1 : height-1;
				jp2 = (j<height-2) ? j+2 : height-1;
				jp3 = (j<height-3) ? j+3 : height-1;
				jp4 = (j<height-4) ? j+4 : height-1;
				jp5 = (j<height-5) ? j+5 : height-1;
				jp6 = (j<height-6) ? j+6 : height-1;
                
                int addr_m0 = j*w422 + i;
                int addr_m1 = jm1*w422 + i;
                int addr_m2 = jm2*w422 + i;
                int addr_m3 = jm3*w422 + i;
                int addr_m4 = jm4*w422 + i;
                int addr_m5 = jm5*w422 + i;
                
                int addr_p1 = jp1*w422 + i;
                int addr_p2 = jp2*w422 + i;
                int addr_p3 = jp3*w422 + i;
                int addr_p4 = jp4*w422 + i;
                int addr_p5 = jp5*w422 + i;
                int addr_p6 = jp6*w422 + i;
                
				
				/* FIR filter with 0.5 sample interval phase shift */
				temp = c228*((float)(dst422[addr_m0])+(float)(dst422[addr_p1]))
                +c70*((float)(dst422[addr_m1])+(float)(dst422[addr_p2]))
                -c37*((float)(dst422[addr_m2])+(float)(dst422[addr_p3]))
                -c21*((float)(dst422[addr_m3])+(float)(dst422[addr_p4]))
                +c11*((float)(dst422[addr_m4])+(float)(dst422[addr_p5]))
                + c5*((float)(dst422[addr_m5])+(float)(dst422[addr_p6]))+0.5;
                if(temp>maxCV)temp = maxCV;
                if(temp<minCV)temp = minCV;
                
                dst_plane[ (j>>1)*w422 + i ] = (unsigned short)temp;
			}
            
		}
        
    
    
    free( dst422 );
	
}



void Subsample444to420_FIR_float( float *dst_plane, float *src_plane, short width, short height,unsigned long minCV, unsigned long maxCV)
{
    
    // perform 444 to 422 conversion horizontally
    // array is aligned as src & dst[width][height]
    printf("float 32-bit FIR Filter subsampling...\n");
    
    int w, jm6, jm5, jm4, jm3, jm2, jm1;
    int jp1, jp2, jp3, jp4, jp5, jp6;
    
    // Allocate dst422 storage
    short w422 = width>>1;
    
    //unsigned short** dst422 = (unsigned short**) malloc(w422*sizeof(unsigned short*)); // cols
    
    float *dst422 = (float*) malloc( height * w422 * sizeof(float) );
    
    int im5, im4, im3, im2, im1, ip1, ip2, ip3, ip4, ip5, ip6;
    float scale = 512.0;
    float c21 = 21.0/scale;
    float c52  = 52.0/scale;
    float c159 =  159.0/scale;
    float c256 =  256.0/scale;
    
    float temp;
    
    
    // stage 1:  4:4:4 to 4:2:2 (horizontal resampling)
    
    for (int j=0; j<height; j++)
    {
        for (int i=0; i<width; i+=2)
        {
            // picture border logic
            im5 = (i<5) ? 0 : i-5;
            im3 = (i<3) ? 0 : i-3;
            im1 = (i<1) ? 0 : i-1;
            ip1 = (i<width-1) ? i+1 : width-1;
            ip3 = (i<width-3) ? i+3 : width-1;
            ip5 = (i<width-5) ? i+5 : width-1;
            
            float *s = &(src_plane[ j*width ]);
            
            // convolution
            temp = c21*((float)(s[im5])+
                        (float)(s[ip5]))
            -c52*((float)(s[im3])+
                  (float)(s[ip3]))
            +c159*((float)(s[im1])+
                   (float)(s[ip1]))
            +c256*((float)(s[i]))+0.5;
            
            // clipping
            if(temp>maxCV)temp = maxCV;
            if(temp<minCV)temp = minCV;
            
            //          dst422[ j * (width>>1) + (i>>1) ] = (unsigned short)temp;
           dst422[ j * (width>>1) + (i>>1) ] = temp;
            
            //printf("s:%d  d:%d  ",src[i][j],dst422[(i>>1)][j]);
        }
    }
    
    
    // stage 2: perform 422 to 420 conversion vertically
    float c228 = 228.0/scale;
    float c70  =  70.0/scale;
    float c37  =  37.0/scale;
    c21  =  21.0/scale;
    float c11  =  11.0/scale;
    float c5   =   5.0/scale;
    
    
    for (int i=0; i<w422; i++)
    {
        for (int j=0; j<height; j+=2)
        {
            jm5 = (j<5) ? 0 : j-5;
            jm4 = (j<4) ? 0 : j-4;
            jm3 = (j<3) ? 0 : j-3;
            jm2 = (j<2) ? 0 : j-2;
            jm1 = (j<1) ? 0 : j-1;
            jp1 = (j<height-1) ? j+1 : height-1;
            jp2 = (j<height-2) ? j+2 : height-1;
            jp3 = (j<height-3) ? j+3 : height-1;
            jp4 = (j<height-4) ? j+4 : height-1;
            jp5 = (j<height-5) ? j+5 : height-1;
            jp6 = (j<height-6) ? j+6 : height-1;
            
            int addr_m0 = j*w422 + i;
            int addr_m1 = jm1*w422 + i;
            int addr_m2 = jm2*w422 + i;
            int addr_m3 = jm3*w422 + i;
            int addr_m4 = jm4*w422 + i;
            int addr_m5 = jm5*w422 + i;
            
            int addr_p1 = jp1*w422 + i;
            int addr_p2 = jp2*w422 + i;
            int addr_p3 = jp3*w422 + i;
            int addr_p4 = jp4*w422 + i;
            int addr_p5 = jp5*w422 + i;
            int addr_p6 = jp6*w422 + i;
            
            
            /* FIR filter with 0.5 sample interval phase shift */
            temp = c228*((float)(dst422[addr_m0])+(float)(dst422[addr_p1]))
            +c70*((float)(dst422[addr_m1])+(float)(dst422[addr_p2]))
            -c37*((float)(dst422[addr_m2])+(float)(dst422[addr_p3]))
            -c21*((float)(dst422[addr_m3])+(float)(dst422[addr_p4]))
            +c11*((float)(dst422[addr_m4])+(float)(dst422[addr_p5]))
            + c5*((float)(dst422[addr_m5])+(float)(dst422[addr_p6]))+0.5;
            if(temp>maxCV)temp = maxCV;
            if(temp<minCV)temp = minCV;
            
            dst_plane[ (j>>1)*w422 + i ] = temp;
        }
        
    }
    
    
    
    free( dst422 );
	
}

// chroma resample 
int convert( pic_t *out_pic, hdr_t *h, pic_t *in_pic )
{
   user_args_t *ua = &(h->user_args);
    
    int arraySizeX = in_pic->width; // eg 3840
    int arraySizeY = in_pic->height;

    clip_limits_t *clip = &(in_pic->clip);

//#if 0
    if( in_pic->pic_buffer_type != PIC_TYPE_U16 ||
       out_pic->pic_buffer_type != PIC_TYPE_U16 )
    {
        printf("WARNING: convert() only takes in_pic and out_pic with type U16\n");
        return(1);
    }
//#endif
    
    printf("DYUVPRIME2: %d\n", out_pic->matrix_coeffs == MATRIX_YUVPRIME2 );
    
    if( out_pic->matrix_coeffs == MATRIX_YUVPRIME2
       && out_pic->chroma_format_idc == CHROMA_420 )
    {
        // the samples loaded into the 2D arrays YP[][], Cb444[][] and Cr444[][] from the TIFF source are 16-bits
        // with a rho-gamma EOTF applied.
        //  *both* linear and non-linear (EOTF'd) forms of XYZ are needed to generate u'' v''
        // filter (low-pass or resample) Y' linear-X linear-Z, then derive u'' v''
        float u_r = 0.1978;
        float v_r = 0.4683;
        float e = 0.25;

        int size =arraySizeX*arraySizeY*sizeof(unsigned short);
        unsigned short *linear_Y_plane = (unsigned short*) malloc(size);
        unsigned short *linear_X_plane = (unsigned short*) malloc(size);
        unsigned short *linear_Z_plane = (unsigned short*) malloc(size);
        unsigned short *scaled_linear_Y_plane  = (unsigned short*) malloc(size);
        unsigned short *scaled_linear_Z_plane = (unsigned short*) malloc(size);
        unsigned short *scaled_linear_X_plane = (unsigned short*) malloc(size);

        unsigned short *gamma_Y_tmp_plane  = (unsigned short*) malloc(size);
        unsigned short *scaled_gamma_Y_tmp_plane = (unsigned short*) malloc(size);
        

        
        
        float sum_u_prime2 = 0.0;
        float sum_v_prime2 = 0.0;
        float sum_Y_prime = 0.0;
        
        // convert the non-linear Y .tiff source into linear XYZ
        for (int j = 0; j < arraySizeY; j++)
        {
            int addr = arraySizeX*j;
            
            unsigned short *Yptr = &(in_pic->buf[0][addr]);
            unsigned short *Uptr = &(in_pic->buf[1][addr]);
            unsigned short *Vptr = &(in_pic->buf[2][addr]);

            for (int i = 0; i < arraySizeX; i++)
            {
               unsigned short Yprime = Yptr[i];

                
#ifdef GAMMA_MAPPED_XZ_INPUT
                unsigned short Zprime = Cb444[i][j];
                unsigned short Xprime = Cr444[i][j];
                float gamma_Z = ((float) Zprime) / 65535.0;
                float gamma_X = ((float) Xprime) / 65535.0;
                float Z_f = RHO_GAMMA_f( gamma_Z );
                float X_f = RHO_GAMMA_f( gamma_X );
                linear_X[i][j] = (unsigned short) (Z_f * 65535.0);
                linear_Z[i][j] = (unsigned short) (X_f * 65535.0);
#endif
                //      sum_Y_prime += linear_Y[i][j];
                float gamma_Y = ((float) Yprime) / 65535.0;
                float Y_f = RHO_GAMMA_f( gamma_Y );
                gamma_Y_tmp_plane[addr+i] = Yprime;
   
                linear_Y_plane[ addr + i] = (unsigned short) (Y_f * 65535.0);
                linear_Z_plane[ addr + i] = Uptr[i];
                linear_X_plane[ addr + i] = Vptr[i];
                
                sum_Y_prime += linear_Y_plane[ addr + i];

            }
        }
        
        if( ua->chroma_resampler_type == 1 )
            
        {
            
#if 0
            if( in_pic->pic_buffer_type == PIC_TYPE_U16 && out_pic->pic_buffer_type == PIC_TYPE_U16)
            {
#endif
            // FIR
                Subsample444to420_FIR( scaled_linear_Y_plane, linear_Y_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                Subsample444to420_FIR( scaled_linear_Z_plane, linear_Z_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                Subsample444to420_FIR( scaled_linear_X_plane, linear_X_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                Subsample444to420_FIR( scaled_gamma_Y_tmp_plane, gamma_Y_tmp_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
#if 0
                // TODO: all in/out data types to be different..
            }else if(  in_pic->pic_buffer_type == PIC_TYPE_F32 && out_pic->pic_buffer_type == PIC_TYPE_F32 ){
                Subsample444to420_FIR_float( scaled_linear_Y_plane, linear_Y_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                Subsample444to420_FIR_float( scaled_linear_Z_plane, linear_Z_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                Subsample444to420_FIR_float( scaled_linear_X_plane, linear_X_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                Subsample444to420_FIR_float( scaled_gamma_Y_tmp_plane, gamma_Y_tmp_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
                
            }
#endif
        }
        else if( ua->chroma_resampler_type == 0 )
        {
            Subsample444to420_box( scaled_linear_Y_plane, linear_Y_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
            Subsample444to420_box( scaled_linear_Z_plane, linear_Z_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
            Subsample444to420_box( scaled_linear_X_plane, linear_X_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
            Subsample444to420_box( scaled_gamma_Y_tmp_plane, gamma_Y_tmp_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
        }
        
        
#ifdef SANITY_CHECKS
        int n_sum_zero = 0;
        int u_prime2_oob_gt1 = 0;
        int v_prime2_oob_gt1 = 0;
        int Y_prime_oob_gt1 = 0;
        int u_prime2_oob_lt0 = 0;
        int v_prime2_oob_lt0 = 0;
        int Y_prime_oob_lt0 = 0;
        
        double u_prime_oob_gt1_max = 0.0;
        double v_prime_oob_gt1_max = 0.0;
        double Y_prime_oob_gt1_max = 0.0;
        
        double u_prime_oob_gt1_e = 0.0;
        double v_prime_oob_gt1_e = 0.0;
        double Y_prime_oob_gt1_e = 0.0;
        
        double u_prime_oob_lt0_e = 0.0;
        double v_prime_oob_lt0_e = 0.0;
        double Y_prime_oob_lt0_e = 0.0;
        
        double u_prime_oob_lt0_min = 0.0;
        double v_prime_oob_lt0_min = 0.0;
        double Y_prime_oob_lt0_min = 0.0;
#endif
        
        printf("computing u''v''\n");
        
        for (int j = 0; j < arraySizeY/2; j++)
        {
            int addr = j * (arraySizeX/2);
            
                for (int i = 0; i < arraySizeX/2; i++)
                {
                    double X = ((double) scaled_linear_X_plane[addr+i]) / 65535.0;
                    double Z = ((double) scaled_linear_Z_plane[addr+i]) / 65535.0;
                    double Y = ((double) scaled_linear_Y_plane[ addr+ i ]) / 65535.0;
                    double Yprime = ((double)scaled_gamma_Y_tmp_plane[addr+i]) / 65535.0;
                double sum = (X + 15.0*Y + 3.0*Z);
                
                double u_prime, v_prime, u_prime2, v_prime2;
                
                if( sum > 0.0 ){
                    u_prime = 4.0 * X / sum;
                    v_prime = 9.0 * Y / sum;
                    
                }
                else
                {
                    u_prime = v_prime = 0.0;
#ifdef SANITY_CHECKS
                    n_sum_zero++;
#endif
                }
                
                u_prime2 = (u_prime - u_r) * Yprime / fmax( Yprime,e) + u_r;
                v_prime2 = (v_prime - v_r) * Yprime / fmax( Yprime,e) + v_r;
                
                
#ifdef SANITY_CHECKS
                
                if(u_prime2 > 1.0  ){
                    u_prime2_oob_gt1++;
                    
                    u_prime_oob_gt1_max = u_prime2 > u_prime_oob_gt1_max ? u_prime2 : u_prime_oob_gt1_max;
                    u_prime_oob_gt1_e +=  (u_prime2  - 1.0 );
                }
                
                if(v_prime2 > 1.0 ){
                    v_prime2_oob_gt1++;
                    v_prime_oob_gt1_max = v_prime2 > v_prime_oob_gt1_max ? v_prime2 : v_prime_oob_gt1_max;
                    v_prime_oob_gt1_e +=  (v_prime2  - 1.0 );
                }
                
                if(Yprime > 1.0  ){
                    Y_prime_oob_gt1++;
                    Y_prime_oob_gt1_max = Yprime > Y_prime_oob_gt1_max ? Yprime : Y_prime_oob_gt1_max;
                    Y_prime_oob_gt1_e +=  (Yprime  - 1.0 );
                }
                
                if(  u_prime2 < 0.0 ){
                    u_prime2_oob_lt0++;
                    u_prime_oob_lt0_min = u_prime2 < u_prime_oob_lt0_min ? u_prime2 : u_prime_oob_lt0_min;
                    u_prime_oob_lt0_e +=  u_prime2;
                }
                
                if( v_prime2 < 0.0 ){
                    v_prime2_oob_lt0++;
                    v_prime_oob_lt0_min = v_prime2 < v_prime_oob_lt0_min ? v_prime2 : v_prime_oob_lt0_min;
                    v_prime_oob_lt0_e +=  v_prime2;
                }
                
                if( Yprime < 0.0 ){
                    Y_prime_oob_lt0++;
                    Y_prime_oob_lt0_min = Yprime < Y_prime_oob_lt0_min ? Yprime : Y_prime_oob_lt0_min;
                    Y_prime_oob_lt0_e +=  Yprime;
                }
#endif
                
                // HACK: test coding with u' v' instead of u''v'' .  Do not distriute this hack!
                u_prime2 = u_prime;
                v_prime2 = v_prime;
                
                // clip
                u_prime2 = u_prime2 < 0.0 ? 0.0 : u_prime2;
                v_prime2 = v_prime2 < 0.0 ? 0.0 : v_prime2;
                u_prime2 = u_prime2 > 1.0 ? 1.0 : u_prime2;
                v_prime2 = v_prime2 > 1.0 ? 1.0 : v_prime2;
                
                // in-place replacement
                   unsigned short clipped_u = (unsigned short)(((double)u_prime2) * 65535.0);
                   unsigned short clipped_v = (unsigned short)(((double)v_prime2) * 65535.0);
  
                    out_pic->buf[1][ addr+i ] = clipped_u;
                    out_pic->buf[2][ addr+i ] = clipped_v;
                
                sum_u_prime2 += clipped_u;
                sum_v_prime2 += clipped_v;
            }
        }
        
#ifdef SANITY_CHECKS
        printf("Out of bounds count:  <0.0: 1.0 Y'(%d) u''(%d) v''(%d) \n",
               Y_prime_oob_lt0, u_prime2_oob_lt0, v_prime2_oob_lt0 );
        
        printf("Out of bounds < 0.0 min: Y'(%f) u''(%f) v''(%f)\n",
               Y_prime_oob_lt0_min, u_prime_oob_lt0_min, v_prime_oob_lt0_min );
        
        printf("Out of bounds < 1.0 energy: Y'(%f) u''(%f) v''(%f)\n",
               Y_prime_oob_lt0_e, u_prime_oob_lt0_e, v_prime_oob_lt0_e );
        
        
        printf("Out of bounds > 1.0 count: Y'(%d) u''(%d) v''(%d)\n",
               Y_prime_oob_gt1, u_prime2_oob_gt1, v_prime2_oob_gt1 );
        
        printf("Out of bounds > 1.0 max: Y'(%f) u''(%f) v''(%f)\n",
               Y_prime_oob_gt1_max, u_prime_oob_gt1_max, v_prime_oob_gt1_max );
        
        printf("Out of bounds > 1.0 energy: Y'(%f) u''(%f)v''(%f)\n",
               Y_prime_oob_gt1_e, u_prime_oob_gt1_e, v_prime_oob_gt1_e );
        
        
        printf("avg Y':%f u'': %f  v'':%f\n",
               sum_Y_prime / (arraySizeX * arraySizeY),
               sum_u_prime2 / (arraySizeX * arraySizeY)/4.0,
               sum_v_prime2 / (arraySizeX * arraySizeY)/4.0 );
        
        printf("count of (X+15*Y + 3*Z) = 0:   %d occurrences out of %d potential\n", n_sum_zero, (arraySizeX * arraySizeY)/4 );
#endif
        
        
        if( linear_Y_plane )
            free( linear_Y_plane );
        if( linear_X_plane )
            free( linear_X_plane );
        if( linear_Z_plane )
            free( linear_Z_plane );
        if( scaled_linear_Y_plane )
            free( scaled_linear_Y_plane );
        if( scaled_linear_Z_plane )
            free( scaled_linear_Z_plane );
        if( scaled_linear_X_plane )
            free( scaled_linear_X_plane );
        if( gamma_Y_tmp_plane )
            free( gamma_Y_tmp_plane );
        if( scaled_gamma_Y_tmp_plane )
            free( scaled_gamma_Y_tmp_plane );
        

    }else if( out_pic->chroma_format_idc == CHROMA_420 )
    {
        
        // TODO: replace with function pointers or C++ overloads
        
        if( ua->chroma_resampler_type == 0 )
        {
            if( in_pic->pic_buffer_type == PIC_TYPE_U16 && out_pic->pic_buffer_type == PIC_TYPE_U16)
            {
                Subsample444to420_box( out_pic->buf[1], in_pic->buf[1],  arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
                Subsample444to420_box( out_pic->buf[2], in_pic->buf[2],  arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
            }else if(  in_pic->pic_buffer_type == PIC_TYPE_F32 && out_pic->pic_buffer_type == PIC_TYPE_F32 ){
                Subsample444to420_box_float( out_pic->fbuf[1], in_pic->fbuf[1],  arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
                Subsample444to420_box_float( out_pic->fbuf[2], in_pic->fbuf[2],  arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
                
            }
        }
        else
        {
            if( in_pic->pic_buffer_type == PIC_TYPE_U16 && out_pic->pic_buffer_type == PIC_TYPE_U16)
            {
                Subsample444to420_FIR( out_pic->buf[1], in_pic->buf[1], arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
                Subsample444to420_FIR( out_pic->buf[2], in_pic->buf[2], arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
            }else if(  in_pic->pic_buffer_type == PIC_TYPE_F32 && out_pic->pic_buffer_type == PIC_TYPE_F32 ){
                Subsample444to420_FIR_float( out_pic->fbuf[1], in_pic->fbuf[1], arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
                Subsample444to420_FIR_float( out_pic->fbuf[2], in_pic->fbuf[2], arraySizeX, arraySizeY, clip->minCV, clip->maxCV );

            }else{
                printf("ERROR, convert():  input and output picture data types (F32 or U16) must be the same\n");
            }
            // Subsample =1 means use FIR filter
        }
        
        out_pic->chroma_format_idc = CHROMA_420;
        
    }
    else if( out_pic->chroma_format_idc == CHROMA_444 )
    {
        // TODO: use pointer swaps, copies from a universal frame buffer pool
        //  in future, rather than needlessly copy pictures
        int size = arraySizeY * arraySizeX * sizeof( unsigned short);
        
        if( in_pic->pic_buffer_type == PIC_TYPE_U16 && out_pic->pic_buffer_type == PIC_TYPE_U16)
        {
            memcpy(  out_pic->buf[1], in_pic->buf[1], size );
            memcpy(  out_pic->buf[2], in_pic->buf[2], size );
        }else if(  in_pic->pic_buffer_type == PIC_TYPE_F32 && out_pic->pic_buffer_type == PIC_TYPE_F32 ){
            memcpy(  out_pic->fbuf[1], in_pic->fbuf[1], size );
            memcpy(  out_pic->fbuf[2], in_pic->fbuf[2], size );
        }else{
            printf("ERROR, convert():  input and output picture data types (F32 or U16) must be the same\n");
        }
    }
    
    
    int size = arraySizeY * arraySizeX * sizeof( unsigned short);
                                              
    memcpy(  out_pic->buf[0], in_pic->buf[0], size );
    
    
    
//    h->out_pic.width = arraySizeX;
//    h->out_pic.height = arraySizeY;
//    h->out_pic.bit_depth = ua->dst_bit_depth;
//    h->out_pic.matrix_coeffs = ua->dst_matrix_coeffs;
//    h->out_pic.video_full_range_flag = ua->dst_video_full_range_flag;
//    h->out_pic.chroma_format_idc = CHROMA_420;
//    h->out_pic.transfer_characteristics = ua->dst_transfer_characteristics;
//    h->out_pic.colour_primaries = ua->dst_colour_primaries;
//    h->out_pic.chroma_sample_loc_type = 0;
    
    return 0;
}


// 4:4:4  RGB ---> YCbCr  and OETF

int matrix_convert( pic_t *out_pic, hdr_t *h, pic_t *in_pic )
{
    // Read 16 bit tiff values into unsigned short into unsigned int for headroom
    // assumption is that RGB = X'Y'Z' from ODT output into 16 bit tiff (as 12 bit clamped values)
    // other tests will clamp ODT at 14 bits and maybe 10 bits
    // intermediate values for color differencing
    
    if( in_pic->chroma_format_idc != CHROMA_444 || out_pic->chroma_format_idc != CHROMA_444 )
    {
        printf("ERROR, matrix_convert(): both input and ouput pictures to this subroutine must be 4:4:4 in order to peform color space conversion\n" );
        return(1);
    }
    
    clip_limits_t *clip = &(out_pic->clip);
    
    printf("matrix() -- clip:  Half(%d), MinCV(%d) MaxCV(%d) MinVR(%d) MaxVR(%d) MinVRC(%d) MaxVRC(%d)",
           clip->Half, clip->minCV, clip->maxCV, clip->minVR, clip->maxVR, clip->minVRC , clip->maxVRC );
    
    //   out_pic->width = in_pic->width;
    //   out_pic->width = in_pic->width;
    
 //   user_args_t *ua = &(h->user_args);
    
    int width = in_pic->width;
    int height = in_pic->height;
    
    // max pixel value (e.g 12 bits is 4095)
    
    if( h->user_args.verbose_level > 0 )
        printf("matrix conversion\n");
    
    // set up alternate differencing equations for Y'DzwDxw
    float tmpF = 0.0;
    float P,Q,RR,S;
    if ( out_pic->matrix_coeffs == MATRIX_YDzDx_Y100) {
        printf("Setting PQRS for 100nit PQ color difference point\n");
        P = -0.5;
        Q = 0.491722;
        RR = 0.5;
        S = -0.49495;
    } else if( out_pic->matrix_coeffs == MATRIX_YDzDx_Y500) {
        printf("Setting PQRS for 500nit PQ color difference point\n");
        P = -0.5;
        Q = 0.493393;
        RR = 0.5;
        S = -0.49602;
    }
    
    float range[MAX_CC];
    float offset[MAX_CC];
    
    int convert_transfer =  in_pic->transfer_characteristics != out_pic->transfer_characteristics;
    
    printf("matrix: convert_transfer(%d)\n", convert_transfer );
    
    if( convert_transfer)
    {
        // will convert transfer
        for(int cc=0;cc<MAX_CC;cc++)
        {
            range[cc] = in_pic->stats.estimated_ceiling[cc] - in_pic->stats.estimated_floor[cc];
            offset[cc] = in_pic->stats.estimated_floor[cc];
            
            if( in_pic->pic_buffer_type == PIC_TYPE_U16 )
                printf("matrix_convert(cc=%d) in_pic: avg(%d) range(%f) offset(%f)\n", cc,
                       in_pic->stats.i_avg[cc], range[cc], offset[cc] );
            else
                printf("matrix_convert(cc=%d) in_pic: avg(%f) range(%f) offset(%f)\n", cc,
                       in_pic->stats.f_avg[cc], range[cc], offset[cc] );
        }
    }
    
    
    // read lines to cover arraySizeY number of lines (e.g. 2160)
    for (int y=0;  y < height; y++)
    {
        int addr = y * width;
        
        for (int x = 0; x < width; x++)
        {
            // read rgba
            // 12 bit YDzDx formular from octave script:
            //  trans_new = [ 0 1 0; 0 -0.5 0.5; 0.5 -0.5 0 ]
            //  output_new = trans_new * source_vector + [0; 2048; 2048];
            //  2048 = 2**12 / 2
            // Y = G = Y
            // Dz = -0.5*G + 0.5*B  = 0.5*Z - 0.5*Y + 2048
            // Dx = 0.5*R -0.5*G  = 0.5*X - 0.5*Y  +2048
            
            // [consider changing RGBA aka XYZ to 12 bits here first]
            // pixel/2 is unsigned short width index to first R
            // in block of 4 pixels of 4 values RGBA
            // buf1-4 are the 4 scan lines reading
            float G, B, R;
            
            
            // first convert input to float if not already float
            
            
            if( in_pic->pic_buffer_type == PIC_TYPE_F32 )
            {
                G =  in_pic->fbuf[0][addr+x];
                B =  in_pic->fbuf[1][addr+x];
                R =  in_pic->fbuf[2][addr+x];
                
                // exr content from Technicolor appears to be ranged for 4k nits
                // we could use stats in read_exr to autoscale..
                
            }
            //            else if(  in_pic->pic_buffer_type == PIC_TYPE_U16 )
            else
            {
                G = (float) in_pic->buf[0][addr+x];
                B = (float) in_pic->buf[1][addr+x];
                R = (float) in_pic->buf[2][addr+x];
            }
            
            // scale the image to real-value 0.0 to 1.0 range
            
            // if your source video is not graded with human assist tools like
            // BaseLight or Davinci Resolve, then can use AMPAS' ctlrender
            // to run each input frame through a tone mapper like rrt.ctl.
            
            
            // for this program, we assume .exr files are RRT tone mapped and
            // in OCES RGB space.
            // this simulates what odt_PQ10k2020.ctl does here..
            
            
            int src_transfer = in_pic->transfer_characteristics;
            
            
            // step 1: convert
            if( convert_transfer )
            {
                // perform conversion from src to dst transfer
                
                // frist, scale to real-value range (0.0 to 1.0)
                G = (G - offset[0])/range[0];
                B = (B - offset[1])/range[1];
                R = (R - offset[2])/range[2];
                
                // should probably use function pointers for this next step
                
                // step 1: convert to linear (float)
                if( src_transfer != TRANSFER_LINEAR )
                {
                    // convert to non-linear
                    if( src_transfer == TRANSFER_PQ )
                    {
                        G = PQ10000_f( G );
                        B = PQ10000_f( B );
                        R = PQ10000_f( R );
                        
                        src_transfer = TRANSFER_LINEAR;
                    }
                    else if( src_transfer == TRANSFER_RHO_GAMMA )
                    {
                        G = RHO_GAMMA_f( G );
                        B = RHO_GAMMA_f( B );
                        R = RHO_GAMMA_f( R );
                        
                        src_transfer = TRANSFER_LINEAR;
                    }
                    // these transfers are not exactly the same but close enough
                    // for now..
                    else if( src_transfer == TRANSFER_BT709
                            || src_transfer == TRANSFER_BT2020_10bit
                            || src_transfer == TRANSFER_BT2020_12bit
                            || src_transfer == TRANSFER_BT601 )
                    {
                        
                        const float DISPGAMMA = 2.4;
                        const float L_W = 1.0;
                        const float L_B = 0.0;
                        
                        G = bt1886_f( G, DISPGAMMA, L_W, L_B);
                        B = bt1886_f( B, DISPGAMMA, L_W, L_B);
                        R = bt1886_f( R, DISPGAMMA, L_W, L_B);
                        
                        src_transfer =TRANSFER_LINEAR;
                    }
                    else
                        printf("WARNING: src_transfer_characteristics (%d) not supported (yet)\n", in_pic->transfer_characteristics );
                    
                }
                
                // step 2: convert to transfer desired for output
                
                if( src_transfer == TRANSFER_LINEAR &&
                   out_pic->transfer_characteristics != TRANSFER_LINEAR )
                {
                    // convert to non-linear
                    if( out_pic->transfer_characteristics == TRANSFER_PQ )
                    {
                        G = PQ10000_r( G );
                        B = PQ10000_r( B );
                        R = PQ10000_r( R );
                        
                        src_transfer = TRANSFER_PQ;
                    }
                    else if( out_pic->transfer_characteristics == TRANSFER_RHO_GAMMA )
                    {
                        G = RHO_GAMMA_r( G );
                        B = RHO_GAMMA_r( B );
                        R = RHO_GAMMA_r( R );
                        
                        src_transfer = TRANSFER_RHO_GAMMA;
                    }
                    // these transfers are not exactly the same but close enough
                    // for now..
                    else if( out_pic->transfer_characteristics == TRANSFER_BT709
                            || out_pic->transfer_characteristics == TRANSFER_BT2020_10bit
                            || out_pic->transfer_characteristics == TRANSFER_BT2020_12bit
                            || out_pic->transfer_characteristics == TRANSFER_BT601 )
                    {
                        
                        const float DISPGAMMA = 2.4;
                        const float L_W = 1.0;
                        const float L_B = 0.0;
                        
                        G = bt1886_r( G, DISPGAMMA, L_W, L_B);
                        B = bt1886_r( B, DISPGAMMA, L_W, L_B);
                        R = bt1886_r( R, DISPGAMMA, L_W, L_B);
                        
                        src_transfer = out_pic->transfer_characteristics;
                    }
                    else
                        printf("WARNING: dst_transfer_characteristics(%d) not supported (yet)\n",
                               out_pic->transfer_characteristics );
                }
                
                // scale back
                // this should be a variable and be checked against whether
                // the transfer function assumes a fixed map to brightness
                
                //    if( in_pic->matrix_coeffs == MATRIX_GBR || in_pic->matrix_coeffs == MATRIX_XYZ )
                if( out_pic->pic_buffer_type == PIC_TYPE_F32 )
                {
                    //        float offset = in_pic->video_full_range_flag ?
                    G = G*range[0] + offset[0];
                    B = B*range[1] + offset[1];
                    R = R*range[2] + offset[2];
                }
                else if( out_pic->pic_buffer_type == PIC_TYPE_U16 )
                {
                    // TODO: make clip a planar struct member (range determined by Y or C)
                    
                    if( out_pic->video_full_range_flag )
                    {
                        G = G*clip->maxCV;
                        B = B*clip->maxCV;
                        R = R*clip->maxCV;
                    }
                    else if(  out_pic->matrix_coeffs == MATRIX_GBR  )
                    {
                        G = G*clip->maxVR + clip->minVR;
                        B = B*clip->maxVR + clip->minVR;
                        R = R*clip->maxVR + clip->minVR;
                    }
                    else
                    {
                        G = G*clip->maxVR + clip->minVR;
                        B = B*clip->maxVRC + clip->minVRC;
                        R = R*clip->maxVRC + clip->minVRC;
                    }
                }
                
            }
            
            
            if( out_pic->pic_buffer_type == PIC_TYPE_U16 )
            {
                unsigned int Y;
                long Cb, Cr,RC,BC;
                
                //                if( out_pic->matrix_coeffs == MATRIX_GBR ||
                //                   out_pic->matrix_coeffs == MATRIX_UNSPECIFIED
                //                   || out_pic->matrix_coeffs == MATRIX_RESERVED  )
                
                if( out_pic->matrix_coeffs == in_pic->matrix_coeffs &&
                   out_pic->colour_primaries == in_pic->colour_primaries  )
                    
                {  // RGB to RGB  or YUV to YUV
                    Y = (unsigned int)G;
                    Cb = (unsigned int)B;
                    Cr = (unsigned int)R;
                }
                else  // color difference space
                {
                    if( out_pic->matrix_coeffs == MATRIX_YDzDx ) {
                        
                        Y = (unsigned int) G;
                        Cb = (int)(-G/2.0  + B/2.0 +0.5); // Dz: -(int)((G+1)>>1) + (int)((B+1)>>1);
                        Cr =  (int)(-G/2.0 +R/2.0 +0.5);  // Dx: (int)((R+1)>>1) - (int)((G+1)>>1);
                        //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                        
                    } else if( out_pic->matrix_coeffs == MATRIX_BT2020nc ){
                        tmpF = (0.2627*R + 0.6780*G + 0.0593*B) +0.5;
                        Y = (unsigned int)(tmpF);
                        Cb = (int)((B - tmpF)/1.8814 + 0.5);
                        Cr = (int)((R - tmpF)/1.4746 + 0.5);
                    } else if( out_pic->matrix_coeffs == MATRIX_BT709 ) {
                        tmpF = (0.2126*R + 0.7152*G + 0.0722*B) +0.5;
                        Y = (unsigned int)(tmpF);
                        Cb = (int)((B - tmpF)/1.8556 + 0.5);
                        Cr = (int)((R - tmpF)/1.5748 + 0.5);
                    } else if(out_pic->matrix_coeffs == MATRIX_YDzDx_Y100
                              || out_pic->matrix_coeffs == MATRIX_YDzDx_Y500) {
                        Y = (unsigned int)(G);
                        Cb = (int)( P *G + Q*B + 0.5);
                        Cr = (int)( RR*R + S*G + 0.5);
                    } else if(out_pic->matrix_coeffs == MATRIX_YUVPRIME2) {
                        Y = (unsigned int) (G);
                        Cb = (unsigned int) (B);
                        Cr = (unsigned int) (R);
                    } else {
                        printf("Can't determine color difference to use?\n\n");
                        exit(0);
                    }
                    
                    Cb = Cb + clip->Half - 1;
                    Cr = Cr + clip->Half - 1;
                }
                
                
                //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                // clamp to full range
                if( Y > clip->maxCV) Y=clip->maxCV;
                if( Y < clip->minCV) Y=clip->minCV;
                
                if( Cb > clip->maxCV) Cb=clip->maxCV;
                if( Cb < clip->minCV) Cb=clip->minCV;
                if( Cr > clip->maxCV) Cr=clip->maxCV;
                if( Cr < clip->minCV) Cr=clip->minCV;
                //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                
                int dst_addr =  y*width + x;
                
                out_pic->buf[0][ dst_addr ]= Y;
                out_pic->buf[1][ dst_addr ]= Cb;
                out_pic->buf[2][ dst_addr ]= Cr;
            }
            else if( out_pic->pic_buffer_type == PIC_TYPE_F32)
            {
                float Y;
                float Cb, Cr,RC,BC;
                
                // TODO: just swap pointers in a general frame buffer memory pool
#if 0
                int dst_addr =  y*width + x;
                out_pic->fbuf[0][ dst_addr ]= G;
                out_pic->fbuf[1][ dst_addr ]= B;
                out_pic->fbuf[2][ dst_addr ]= R;
#endif
                
                //               if( out_pic->matrix_coeffs == MATRIX_GBR ||
                //                  out_pic->matrix_coeffs == MATRIX_UNSPECIFIED
                //                  || out_pic->matrix_coeffs == MATRIX_RESERVED  )
                
                if( out_pic->matrix_coeffs == in_pic->matrix_coeffs &&
                   out_pic->colour_primaries == in_pic->colour_primaries  )
                    
                {  // RGB
                    Y = G;
                    Cb = B;
                    Cr = R;
                }
                else  // color difference space
                {
                    if( out_pic->matrix_coeffs == MATRIX_YDzDx ) {
                        
                        Y = G;
                        Cb = (-G/2.0  + B/2.0 +0.5); // Dz: -(int)((G+1)>>1) + (int)((B+1)>>1);
                        Cr = (-G/2.0 +R/2.0 +0.5);  // Dx: (int)((R+1)>>1) - (int)((G+1)>>1);
                        //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                        
                    } else if( out_pic->matrix_coeffs == MATRIX_BT2020nc ){
                        tmpF = (0.2627*R + 0.6780*G + 0.0593*B) +0.5;
                        Y = tmpF;
                        Cb = ((B - tmpF)/1.8814 + 0.5);
                        Cr = ((R - tmpF)/1.4746 + 0.5);
                    } else if( out_pic->matrix_coeffs == MATRIX_BT709 ) {
                        tmpF = (0.2126*R + 0.7152*G + 0.0722*B) +0.5;
                        Y = tmpF;
                        Cb = ((B - tmpF)/1.8556 + 0.5);
                        Cr = ((R - tmpF)/1.5748 + 0.5);
                    } else if(out_pic->matrix_coeffs == MATRIX_YDzDx_Y100
                              || out_pic->matrix_coeffs == MATRIX_YDzDx_Y500) {
                        Y = G;
                        Cb =( P *G + Q*B + 0.5);
                        Cr = ( RR*R + S*G + 0.5);
                    } else if(out_pic->matrix_coeffs == MATRIX_YUVPRIME2) {
                        Y =  G;
                        Cb = B;
                        Cr = R;
                    } else {
                        printf("Can't determine color difference to use?\n\n");
                        exit(0);
                    }
                    
                    Cb = Cb + clip->Half - 1;
                    Cr = Cr + clip->Half - 1;
                }
                
                
                //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                // clamp to full range (not legal range just yet..)
                if( Y > (float) clip->maxCV) Y= (float) clip->maxCV;
                if( Y < (float) clip->minCV) Y=( float) clip->minCV;
                
                if( Cb > (float) clip->maxCV) Cb=(float) clip->maxCV;
                if( Cb < (float) clip->minCV) Cb=(float) clip->minCV;
                if( Cr > (float) clip->maxCV) Cr=(float) clip->maxCV;
                if( Cr < (float) clip->minCV) Cr=(float) clip->minCV;
                //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                
                int dst_addr =  y*width + x;
                
                out_pic->fbuf[0][ dst_addr ]= Y;
                out_pic->fbuf[1][ dst_addr ]= Cb;
                out_pic->fbuf[2][ dst_addr ]= Cr;
                
                
                
            }
            else
            {
                printf("ERROR, matrix_convert(): in_pic pic_buffer_type (%d) not recognized", in_pic->pic_buffer_type );
                exit(0);
            }
            // Fill Source 444 Chroma Arrays Cb444 and Cr444
        }   // x
    } // y
    
    return 0;
}


// assumes any resizing such as 4:2:0 to 4:4:4 resampling has already taken place..

int matrix_inverse( pic_t *out_pic, hdr_t *h, pic_t *in_pic )
{
    
    // set up to handle inverse color difference
    short D709 = in_pic->matrix_coeffs == MATRIX_BT709;
    short D2020 = in_pic->matrix_coeffs == MATRIX_BT2020c;
 //   short DYUVPRIME2 = in_pic->matrix_coeffs == MA;
   // float tmpF = 0.0;
    short HD = h->user_args.cutout_hd; // ==1 if 1920x1080 cutout
    short qHD = h->user_args.cutout_qhd; // ==1 if 960x540 cutout
//    int frames = -999;
 //   short IPixF = 0;
    short Y500 =0;
    short Y100 =0;
    short DXYZ = 1;
//    short BD = 12; // 10 bit mode
  //  short SR=4;
    unsigned short Half = 2048.0; // e.g half value 12 bits
    unsigned short Full = 4096.0; //
  //  short XX = 0;
//    unsigned short minCV = 0; //12 bits
//    unsigned short maxCV = 4095;
//    short FIR = 1;
    short FULLRANGE = 0; // default is video range
 //   short ALPHA = 0; // no alpha channel is default
 //   short numChan2 = 3*2; // only 3 channels rgb with no alpha
   // short numChan = 3;
//    unsigned short D = 4; //D=4 for 12 bits =1 10bits = 16 for 14 bits
//   unsigned short minVR, maxVR, minVRC,maxVRC;
    // video range is
    // Luma and R,G,B:  CV = Floor(876*D*N+64*D+0.5)
    // Chroma:  CV = Floor(896*D*N+64*D+0.5)
    
    int invalidPixels = 0;
    
    clip_limits_t  *clip = &(in_pic->clip);
    
    //Process Args:
#if 0
    if( ALPHA ){
        	numChan2= 4*2;
     		numChan = 4;
    }
        
    // B12 is by default
    if( in_pic->bit_depth == 10 ) {
            BD = 10;
            SR=6;
            Half = 512.0;
            Full = 1024.0;
            minCV = 0;
            maxCV = 1023;
            D=1;
            printf("\nprocessing line data 10 bits\n");
        }
        
        if( in_pic->bit_depth == 14 ) {
            BD = 14;
            SR=2;
            Half = 8192;
            Full = 16384;
            minCV = 0;
            maxCV = 16383;
            D=16;
            printf("\nprocessing line data 14 bits\n");
        }
#endif
        if( in_pic->matrix_coeffs == D709 || in_pic->matrix_coeffs == D2020 || in_pic->matrix_coeffs == Y100 || in_pic->matrix_coeffs == Y500 )
            DXYZ = 0;
        
  //      if(strcmp(argv[arg],"-I")==0)IPixF = 1;
    //    if(strcmp(argv[arg],"-X")==0)XX = 1;
        
      //  if(strcmp(argv[arg],"-f")==0) {
		//	arg++;
		//	if(arg < argc)frames=atoi(argv[arg]);
      //  }
        
        //arg++;
    //}
    
    
    if(D709)printf("Processing for Rec709\n");
    if(D2020)printf("Processing for Rec2020\n");
//    if(DYUVPRIME2)printf("Processing for Y'u''v'' (Y=rho-gamma, u'',v'' = linear)\n");
    if(HD)printf("Processing for HD1920x1080\n");
    if(qHD)printf("Processing for HD960x540\n");
#if 0
    // Set up for video range if needed
    if(!FULLRANGE) {
        printf("Processing for Video Range\n");
        minVR = 64*D;
        maxVR = 876*D+minVR;
        minVRC = minVR;
        maxVRC = 896*D+minVRC;
        //achromatic point fo r chroma will be "Half"(e.g. 512 for 10 bits, 2048 for 12 bits etc..)
        
    }
#endif
    
#if 0
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
#endif
    
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

#if 0
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
#endif
		
		for (int line = 0;line < in_pic->height;line++)
		{
            int addr = line * in_pic->width;
            
			for (unsigned int pixel = 0; pixel < in_pic->width; pixel++ )
			{
                float tmpF, Yav, Cb, Cr;
                float  Rp, Bp;
                
                // Y = G = Y
                // Dz = -0.5*G + 0.5*B  = 0.5*Z - 0.5*Y + 2048
                // Dx = 0.5*R -0.5*G  = 0.5*X - 0.5*Y  +2048
                
                // 4095 = 2047.5 * 2
                // recall 2047.5 came from adding 2047 to Dz, Dx
                //  and adding 8 during initial round off from 16bits to 12
                
                
                
                // Try calculating an average Y to use with subsampled Dz, Dx
                //Yav = (YP[pixel>>2][line>>1 + line>>1] + YP[1+ pixel>>2][line>>1 + line>>1] + YP[1+pixel>>2][line>>1 + line>>1] + YP[1 + pixel>>2][1 + line>>1 + line>>1])/4;
            //    Yav = (int)(YP[pixel/numChan][line]);
                float YavSave = Yav; // keep copy of original Y'
                
                if( in_pic->pic_buffer_type == PIC_TYPE_F32 ){
                    Yav = in_pic->fbuf[0][ addr + pixel ];
                    Cb = in_pic->fbuf[1][ addr + pixel ];
                    Cr = in_pic->fbuf[2][ addr + pixel ];
                } else {
                    Yav = (float )in_pic->buf[0][ addr + pixel ];
                    Cb = (float )in_pic->buf[1][ addr + pixel ];
                    Cr = (float )in_pic->buf[2][ addr + pixel ];
                }
   
#if 0
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
#endif
                
                
                if(DXYZ) {
                    // TODO: re-integrate this...
                    
#if 0
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
#endif
      //                  Rp = ((int)2*(int)(Cr) - (int)(Full-1.0) + Yav);
        //                Bp = ((int)2*(int)(Cb) - (int)(Full-1.0) + Yav);
                        Rp = (2.0*Cr - (Full-1.0) + Yav);
                        Bp = (2.0*Cb - (Full-1.0) + Yav);
                    }
                else if(D2020){
                    tmpF = ((float)(Cb)-(Half-0.5))*1.8814 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Bp = tmpF;
                    tmpF = ((float)(Cr)-(Half-0.5))*1.4746 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Rp = tmpF;
                    tmpF = ((float)Yav - 0.0593*(float)Bp -0.2627*(float)Rp)/0.6780 +0.5 ; // green
                    if(tmpF > (Full-1.0)) tmpF = (Full-1.0);
                    Yav = tmpF; //green
                } else if(D709) {
                    tmpF = ((float)(Cb)-(Half-0.5))*1.8556 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Bp = tmpF;
                    tmpF = ((float)(Cr)-(Half-0.5))*1.5748 + Yav;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Rp = tmpF;
                    tmpF = ((float)Yav - 0.07222*(float)Bp -0.2126*(float)Rp)/0.7152 +0.5 ; // green
                    if(tmpF > (Full-1.0)) tmpF = (Full-1.0);
                    Yav = tmpF;  //green
                } else if(Y100 || Y500) {
                    tmpF = ((float)(Cb)-(Half-0.5))*W + ((float)Yav)*V;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Bp = tmpF;
                    tmpF = ((float)(Cr)-(Half-0.5))*U + ((float)Yav)*T;
                    if(tmpF > (Full-1.0))tmpF = (Full-1.0);
                    Rp = tmpF;
                    
#if 0
                }
                else if(DYUVPRIME2) {
                    
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
#endif
                } else {
                    printf("Can't determine color difference to use?\n\n");
                    exit(0);
                }
                
                int G = (int) Yav;
                int B = (int) Bp;
                int R = (int) Rp;
                    
                if(G < 0)
                {
                    G = 0;
                    invalidPixels++;
                  //  if(IPixF)invPix << "Pixel=" << pixel/8 << ", " << line << "  Y'=" << YavSave << "  Dz=" << Cb444[pixel/numChan][line] << "  Dx=" //<< Cr444[pixel/numChan][line] << "  G'=" << Yav << " !! Gneg\n";
                }
                
                if( R < 0 )
                {
                    invalidPixels++;
                    
                  //  if(IPixF && YavSave>0)invPix << "Pixel=" << pixel/8 << ", " << line << "  Y'=" << YavSave << "  Dz=" << Cb444[pixel/numChan][line] << "  Dx=" << Cr444[pixel/numChan][line] << "  R'=" << Rp << " !! Rneg\n";
                    
        //            if(Yav > YnegRMx && Yav > 0)
          //          {
      //                  YnegRMx = Yav;
                 //       printf("Line: %d Pixel: %d, Y %d, Dz: %d, Dx %d, X' %d, YnegRMx %d\n", line,pixel/8, Yav, Cb444[pixel/numChan][line], Cr444[pixel/numChan][line], Rp,YnegRMx);
            //        }
                    R = 0; // zero invalid pixels whether Yav == 0 or not
                    
    //                if(YavSave != 0)invalidPixels++;
                }
                
                if(B < 0 )
                {
                    invalidPixels++;
                    
                    //if(IPixF && YavSave>0)invPix << "Pixel=" << pixel/8 << ", " << line << "  Y'=" << YavSave << "  Dz=" << Cb444[pixel/numChan][line] << "  Dx=" << Cr444[pixel/numChan][line] << "  B'=" << Bp << " !! Bneg\n";
                    
              //      if(Yav > YnegBMx && Yav > 0)
                //    {
                  //      YnegBMx = Yav;
                      //  printf("Line: %d Pixel: %d, Y %d, Dz: %d, Dx %d, Z' %d,YnegBMx %d\n", line,pixel/8, Yav, Cb444[pixel/numChan][line], Cr444[pixel/numChan][line], Bp, YnegBMx);
                  //  }
                    B = 0; // zero invalid pixels whether Yav == 0 or not
             //       if(YavSave != 0)invalidPixels++;
                }
                
                
                // Insure RGB are clipped to video range if required:
                if(!FULLRANGE) {
                    R = (R< clip->minVR) ? clip->minVR : R;
                    G = (G<clip->minVR) ? clip->minVR : G;
                    B = (B<clip->minVR) ? clip->minVR : B;
                    
                    R = (R>clip->maxVR) ? clip->maxVR : R;
                    G = (G>clip->maxVR) ? clip->maxVR : G;
                    B = (B>clip->maxVR) ? clip->maxVR : B;
                    
                }
                
             
                    if( in_pic->bit_depth > out_pic->bit_depth )
                    {
                        int shift =in_pic->bit_depth - out_pic->bit_depth;
                        
                        R = R >> shift;
                        G = G >> shift;
                        B = B >> shift;
                    } else {
                        int shift = out_pic->bit_depth - in_pic->bit_depth;

                        R = R << shift;
                        G = G << shift;
                        B = B <<  shift;
                    }
                    
                
                    if( in_pic->pic_buffer_type == PIC_TYPE_F32 ){
                        out_pic->fbuf[0][ addr + pixel ] = (float )G;
                        out_pic->fbuf[1][ addr + pixel ] = (float )B;
                        out_pic->fbuf[2][ addr + pixel ] = (float) R;
                    } else {
                        out_pic->buf[0][ addr + pixel ] = G;
                        out_pic->buf[1][ addr + pixel ] = B;
                        out_pic->buf[2][ addr + pixel ] = R;
                    }
                
                    
                    
                    
#if 0
                // R = X = 2*Dx + Y
                Line[pixel] = ((unsigned short)Rp) << SR;     // R = X
				
				// G = Y
				Line[pixel+1] = ((unsigned short)Yav) << SR;  //G = Y or inverse 2020/709 equation
				
				// B = X = 2*Dx + Y
				Line[pixel+2] = ((unsigned short)Bp) << SR;   // B = Z
                    
				// A
				if(ALPHA) Line[pixel+3] = 65535;  // A
#endif
                
                //printf("Rp=%d   Gp=%d   Bp=%d | ",Line[pixel],Line[pixel+1],Line[pixel+2]);
                
			}
			
            
//  			//printf("Writing strip %d with width %d bytes %d pixels\n",line,4*arraySizeX*2,arraySizeX);
//			TIFFWriteRawStrip(tif, (tstrip_t)line, (tdata_t)Line, numChan2*arraySizeX);
            
		}

#if 0
		TIFFClose(tif);
		
		tifNum++;
		
		printf("Max YnegR = %d, Max YnegB = %d\n",YnegRMx, YnegBMx);
		printf("Invalid Pixels:  %d\n",invalidPixels);
		if(frames > 0) {
			frames--;
			if(frames == 0)exit(0);
		}
#endif
            
//	}
    
    
  //  if(IPixF) invPix.close();
        return(0);
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
        
        

// convert color difference (YCbCr, YDzDx) or itentity (GBR, YZX) to primaries 
void matrix_to_primaries( )
{
            
            
}

        
        
        
        

