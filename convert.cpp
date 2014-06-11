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

// non-linear to linear
float PQ10000_f( float V)
{
    float L;
    // Lw, Lb not used since absolute Luma used for PQ
    // formula outputs normalized Luma from 0-1
    L = pow(fmax(pow(V, 1.0/78.84375) - 0.8359375 ,0.0)/(18.8515625 - 18.6875 * pow(V, 1.0/78.84375)),1.0/0.1593017578);
    
    return L;
}

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



float bt1886_f( float V, float gamma, float Lw, float Lb)
{
    // The reference EOTF specified in Rec. ITU-R BT.1886
    // L = a(max[(V+b),0])^g
    float a = pow( pow( Lw, 1./gamma) - pow( Lb, 1./gamma), gamma);
    float b = pow( Lb, 1./gamma) / ( pow( Lw, 1./gamma) - pow( Lb, 1./gamma));
    float L = a * pow( fmax( V + b, 0.), gamma);
    return L;
}

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




int convert( pic_t *out_pic, hdr_t *h, pic_t *in_pic )
{
    user_args_t *ua = &(h->user_args);
    
    int arraySizeX = in_pic->width; // eg 3840
    int arraySizeY = in_pic->height;

    clip_limits_t *clip = &(out_pic->clip);
    
//    t_pic *in_pic = &(h->in_pic);
//    t_pic *out_pic = &(h->out_pic);
    
    
    int arraySizeXH = arraySizeX/2; // eg 1920 cols
    int arraySizeYH = arraySizeY/2; // eg 1080 rows

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
        {   // FIR
            Subsample444to420_FIR( scaled_linear_Y_plane, linear_Y_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
            Subsample444to420_FIR( scaled_linear_Z_plane, linear_Z_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
            Subsample444to420_FIR( scaled_linear_X_plane, linear_X_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
            Subsample444to420_FIR( scaled_gamma_Y_tmp_plane, gamma_Y_tmp_plane, arraySizeX, arraySizeY,clip->minCV,clip->maxCV);
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
        

    }else if( out_pic->chroma_format_idc == CHROMA_420 )
    {
        
        
        if( ua->chroma_resampler_type == 0 )
        {
            Subsample444to420_box( out_pic->buf[1], in_pic->buf[1],  arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
            Subsample444to420_box( out_pic->buf[2], in_pic->buf[2],  arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
        }
        else
        {
            // Subsample =1 means use FIR filter
            Subsample444to420_FIR( out_pic->buf[1], in_pic->buf[1], arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
            Subsample444to420_FIR( out_pic->buf[2], in_pic->buf[2], arraySizeX, arraySizeY, clip->minCV, clip->maxCV );
        }
        
        out_pic->chroma_format_idc = CHROMA_420;
        
    }
    else if( out_pic->chroma_format_idc == CHROMA_444 )
    {
        // TODO: use pointer swaps, copies from a universal frame buffer pool
        //  in future, rather than needlessly copy pictures
        int size = arraySizeY * arraySizeX * sizeof( unsigned short);
        
        memcpy(  out_pic->buf[1], in_pic->buf[1], size );
        memcpy(  out_pic->buf[2], in_pic->buf[2], size );
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


// convert a 4:4:4 planar picture to 4:4:4 planar picture with a color difference matrix

int matrix_convert( pic_t *out_pic, hdr_t *h, pic_t *in_pic )
{
    // Read 16 bit tiff values into unsigned short into unsigned int for headroom
    // assumption is that RGB = X'Y'Z' from ODT output into 16 bit tiff (as 12 bit clamped values)
    // other tests will clamp ODT at 14 bits and maybe 10 bits
    // intermediate values for color differencing
    unsigned int Y;
    long Cb, Cr,RC,BC;
    
    clip_limits_t *clip = &(out_pic->clip);
    
 //   out_pic->width = in_pic->width;
 //   out_pic->width = in_pic->width;
    
    user_args_t *ua = &(h->user_args);
    
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
    
    if( in_pic->transfer_characteristics != out_pic->transfer_characteristics )
    {
        // will convert transfer
        for(int cc=0;cc<MAX_CC;cc++)
        {
            range[cc] = in_pic->stats.estimated_ceiling[cc] - in_pic->stats.estimated_floor[cc];
            offset[cc] = in_pic->stats.estimated_floor[cc];
            
            printf("matrix_convert(cc=%d) in_pic: avg(%d) range(%f) offset(%f)\n", cc,
                   in_pic->stats.i_avg[cc], range[cc], offset[cc] );
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
            
            
            if( in_pic->pic_buffer_type == PIC_TYPE_FLOAT )
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
            
            
            int src_transfer = h->in_pic.transfer_characteristics;
            
            
            // step 1: convert
            if( src_transfer != out_pic->transfer_characteristics )
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
                        printf("WARNING: src_transfer_characteristics (%d) not supported (yet)\n", h->in_pic.transfer_characteristics );
                    
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
                G = G*range[0] + offset[0];
                B = B*range[1] + offset[1];
                R = R*range[2] + offset[2];

            
            }
            
               
            if( in_pic->pic_buffer_type == PIC_TYPE_U16 )
            {
                if( out_pic->matrix_coeffs == MATRIX_GBR ||
                   out_pic->matrix_coeffs == MATRIX_UNSPECIFIED
                   || out_pic->matrix_coeffs == MATRIX_RESERVED  )
                                          
                {  // RGB
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
            else if( in_pic->pic_buffer_type == PIC_TYPE_FLOAT)
            {
                // TODO: just swap pointers in a general frame buffer memory pool
                int dst_addr =  y*width + x;
                out_pic->fbuf[0][ dst_addr ]= G;
                out_pic->fbuf[1][ dst_addr ]= B;
                out_pic->fbuf[2][ dst_addr ]= R;

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

//#endif


