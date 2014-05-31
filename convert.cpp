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




int convert( t_hdr *h )
{
    t_user_args *ua = &(h->user_args);
    
    int arraySizeX = ua->dst_pic_width; // eg 3840
    int arraySizeY = ua->dst_pic_height;

    t_pic *in_pic = &(h->in_pic);
    t_pic *out_pic = &(h->out_pic);
    
    
    int arraySizeXH = arraySizeX/2; // eg 1920 cols
    int arraySizeYH = arraySizeY/2; // eg 1080 rows

    
    
    printf("DYUVPRIME2: %d\n", ua->dst_matrix_coeffs == MATRIX_YUVPRIME2 );
    
    if( ua->dst_matrix_coeffs == MATRIX_YUVPRIME2 )
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
        {   // FIR
            Subsample444to420_FIR( scaled_linear_Y_plane, linear_Y_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
            Subsample444to420_FIR( scaled_linear_Z_plane, linear_Z_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
            Subsample444to420_FIR( scaled_linear_X_plane, linear_X_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
            Subsample444to420_FIR( scaled_gamma_Y_tmp_plane, gamma_Y_tmp_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
        }
        else if( ua->chroma_resampler_type == 0 )
        {
            Subsample444to420_box( scaled_linear_Y_plane, linear_Y_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
            Subsample444to420_box( scaled_linear_Z_plane, linear_Z_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
            Subsample444to420_box( scaled_linear_X_plane, linear_X_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
            Subsample444to420_box( scaled_gamma_Y_tmp_plane, gamma_Y_tmp_plane, arraySizeX, arraySizeY,h->minCV,h->maxCV);
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
  
                    h->out_pic.buf[1][ addr+i ] = clipped_u;
                    h->out_pic.buf[2][ addr+i ] = clipped_v;
                
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
        

    }else
    {
        
        
        if( ua->chroma_resampler_type == 0 )
        {
            Subsample444to420_box( h->out_pic.buf[1], h->in_pic.buf[1],  arraySizeX, arraySizeY, h->minCV, h->maxCV );
            Subsample444to420_box( h->out_pic.buf[2], h->in_pic.buf[2],  arraySizeX, arraySizeY, h->minCV, h->maxCV );
        }
        else
        {
            // Subsample =1 means use FIR filter
            Subsample444to420_FIR( h->out_pic.buf[1], h->in_pic.buf[1], arraySizeX, arraySizeY, h->minCV, h->maxCV );
            Subsample444to420_FIR( h->out_pic.buf[2], h->in_pic.buf[2], arraySizeX, arraySizeY, h->minCV, h->maxCV );
        }
        
    }
    
    
    // didn't do anything to Y in this convert() routine, so copy it to dest.
    for( int j = 0; j< arraySizeY; j++ )
    {
        for( int i = 0; i< arraySizeX; i++ )
        {
            int addr = j*arraySizeX + i;
            
            h->out_pic.buf[0][ addr ] = h->in_pic.buf[0][ addr ];
        }
        
    }

    
    
    h->out_pic.width = arraySizeX;
    h->out_pic.height = arraySizeY;
    h->out_pic.bit_depth = ua->dst_bit_depth;
    h->out_pic.matrix_coeffs = ua->dst_matrix_coeffs;
    h->out_pic.video_full_range_flag = ua->dst_video_full_range_flag;
    h->out_pic.chroma_format_idc = CHROMA_420;
    h->out_pic.transfer_characteristics = ua->dst_transfer_characteristics;
    h->out_pic.colour_primaries = ua->dst_colour_primaries;
    h->out_pic.chroma_sample_loc_type = 0;
    
    return 0;
}


// convert a 4:4:4 planar picture to 4:4:4 planar picture with a color difference matrix

int matrix_convert( t_pic *out_pic, t_hdr *h, t_pic *in_pic )
{
    // Read 16 bit tiff values into unsigned short into unsigned int for headroom
    // assumption is that RGB = X'Y'Z' from ODT output into 16 bit tiff (as 12 bit clamped values)
    // other tests will clamp ODT at 14 bits and maybe 10 bits
    // intermediate values for color differencing
    unsigned int Y;
    long Dz, Dx,RC,BC;
    
    
    t_user_args *ua = &(h->user_args);
    
    int width = in_pic->width;
    int height = in_pic->height;
    
    // max pixel value (e.g 12 bits is 4095)
    
    // set up alternate differencing equations for Y'DzwDxw
    float tmpF = 0.0;
    float P,Q,RR,S;
    if ( ua->dst_matrix_coeffs == MATRIX_YDzDx_Y100) {
        printf("Setting PQRS for 100nit PQ color difference point\n");
        P = -0.5;
        Q = 0.491722;
        RR = 0.5;
        S = -0.49495;
    } else if( ua->dst_matrix_coeffs == MATRIX_YDzDx_Y500) {
        printf("Setting PQRS for 500nit PQ color difference point\n");
        P = -0.5;
        Q = 0.493393;
        RR = 0.5;
        S = -0.49602;
    }
    
    
    // read lines to cover arraySizeY number of lines (e.g. 2160)
    for (int y=0;  y < height; y++)
    {
        int addr = y * width;
        
        // strip is equal to the scanline from 0 to 2159
        
        
        
        // Read one scan line of packed samples into strip (line) buffer
        //       TIFFReadRawStrip(tif, strip,   buf1, bc[0]);
        
        //printf("\rReading Line: %d-%d, Bytes: %d",strip+1,strip+4, bc[0]);
        
        for (int x = 0; x < width; x++)
        {
            // step line in blocks of numChan pixels:
            // that numChan2*ib is byte location of pixel start
            //     int ib = 0;
            
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
            
            unsigned int G = in_pic->buf[0][addr+x];
            unsigned int B = in_pic->buf[1][addr+x];
            unsigned int R = in_pic->buf[2][addr+x];
            
            
            if( ua->dst_matrix_coeffs == MATRIX_YDzDx ) {
                
                Y = G;
                Dz = (int)(-((float)G)/2.0 +((float)B)/2.0 +0.5); //-(int)((G+1)>>1) + (int)((B+1)>>1);
                Dx =  (int)(-((float)G)/2.0 +((float)R)/2.0 +0.5); //(int)((R+1)>>1) - (int)((G+1)>>1);
                //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
                
            } else if( ua->dst_matrix_coeffs == MATRIX_BT2020nc ){
                tmpF = (0.2627*(float)R + 0.6780*(float)G + 0.0593*(float)B) +0.5;
                Y = (unsigned int)(tmpF);
                Dz = (int)((((float)B) - tmpF)/1.8814 + 0.5);
                Dx = (int)((((float)R) - tmpF)/1.4746 + 0.5);
            } else if( ua->dst_matrix_coeffs == MATRIX_BT709 ) {
                tmpF = (0.2126*(float)R + 0.7152*(float)G + 0.0722*(float)B) +0.5;
                Y = (unsigned int)(tmpF);
                Dz = (int)((((float)B) - tmpF)/1.8556 + 0.5);
                Dx = (int)((((float)R) - tmpF)/1.5748 + 0.5);
            } else if(ua->dst_matrix_coeffs == MATRIX_YDzDx_Y100
                      || ua->dst_matrix_coeffs == MATRIX_YDzDx_Y500) {
                Y = (unsigned int)(G);
                Dz = (int)( P *((float)G) + Q *((float)B) + 0.5);
                Dx = (int)( RR *((float)R) + S *((float)G) + 0.5);
            } else if(ua->dst_matrix_coeffs == MATRIX_YUVPRIME2) {
                Y = (unsigned int) (G);
                Dz = (unsigned int) (B);
                Dx = (unsigned int) (R);
            } else {
                printf("Can't determine color difference to use?\n\n");
                exit(0);
            }
            
            Dz = Dz + h->Half - 1;
            Dx = Dx + h->Half - 1;
            
            //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
            // clamp to full range
            if( Y > h->maxCV) Y=h->maxCV;
            if( Dz > h->maxCV) Dz=h->maxCV;
            if( Dz < h->minCV) Dz=h->minCV;
            if( Dx > h->maxCV) Dx=h->maxCV;
            if( Dx < h->minCV) Dx=h->minCV;
            //printf("Y: %d, Dz: %d, Dz: %d\n",Y,Dz,Dx);
            
            int dst_addr =  y*width + x;
            
            h->in_pic.buf[0][ dst_addr ]= Y;
            h->in_pic.buf[1][ dst_addr ]= Dz;
            h->in_pic.buf[2][ dst_addr ]= Dx;
            
            // Fill Source 444 Chroma Arrays Cb444 and Cr444
        }   // x
    } // y
    
    return 0;
}

//#endif


