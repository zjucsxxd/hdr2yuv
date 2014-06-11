// openCV routines

#ifdef OPENCV_ENABLED

#include <iostream>
#include <cstring>

// TODO: make this a branch off stanadar include path

#include "/usr/local/include/opencv2/imgproc/imgproc.hpp" // cvCvtcolor function
#include "/usr/local/include/opencv2/highgui/highgui.hpp" // display

#include "/usr/local/include/opencv2/imgproc/types_c.h"
#include "/usr/local/include/opencv2/highgui/highgui_c.h"

#if 0
static void help(std::string errorMessage)
{
    std::cout<<"Program init error : "<<errorMessage<<std::endl;
    std::cout<<"\nProgram call procedure : ….."<<std::endl;
}
#endif

#include "hdr.h"

//int main(int argc, char* argv[]) {


int openCV_resize_picture(  pic_t *dst_pic, pic_t *src_pic  )
{
    int src_width[3], src_height[3], dst_width[3], dst_height[3];
    int src_chroma_width_scale = 0;
    int src_chroma_height_scale = 0;
    int dst_chroma_width_scale = 0;
    int dst_chroma_height_scale = 0;
    
    src_width[0] = src_pic->width;
    src_height[0] = src_pic->height;

    dst_width[0] = dst_pic->width;
    dst_height[0] = dst_pic->height;
    
    
    int src_chroma_format_idc = CHROMA_444;
    int dst_chroma_format_idc = CHROMA_420;
    
    
    src_chroma_width_scale = src_chroma_format_idc == CHROMA_444 ? 0 : 1;
    src_chroma_height_scale = src_chroma_format_idc == CHROMA_420 ? 1 : 0;

    dst_chroma_width_scale = dst_chroma_format_idc == CHROMA_444 ? 0 : 1;
    dst_chroma_height_scale = dst_chroma_format_idc == CHROMA_420 ? 1 : 0;
    
    src_width[1] = src_width[2] = src_width[0] >> src_chroma_width_scale;
    src_height[1] = src_height[2] = src_height[0] >> src_chroma_height_scale;

    dst_width[1] = dst_width[2] = dst_width[0] >> dst_chroma_width_scale;
    dst_height[1] = dst_height[2] = dst_height[0] >> dst_chroma_height_scale;
    
    cv::Mat srcMat0( src_height[0], src_width[0], CV_32F, cv::Scalar::all(0) );
    cv::Mat srcMat1( src_height[1], src_width[1], CV_32F, cv::Scalar::all(0) );
    cv::Mat srcMat2( src_height[2], src_width[2], CV_32F, cv::Scalar::all(0) );
    
    
  //  cv::Mat lapImage;
  //  cv::Mat sharpImage;

    {
    int cc = 0;
    
    for(int y = 0; y < src_height[cc] ; y++ )
    {
        int addr = y* src_width[cc];
        
        for(int x = 0; x < src_width[cc] ; x++ )
        {
            if( src_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                srcMat0.at<float>( y, x ) = src_pic->fbuf[cc][ addr ];
            }else if( src_pic->pic_buffer_type == PIC_TYPE_U16 ){
                srcMat0.at<float>( y, x ) = (float) src_pic->buf[cc][ addr ];
            }
            
            addr++;
        }
    }

    cc = 1;
    
    for(int y = 0; y < src_height[cc] ; y++ )
    {
        int addr = y* src_width[cc];
        
        for(int x = 0; x < src_width[cc] ; x++ )
        {
            if( src_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                srcMat1.at<float>( y, x ) = src_pic->fbuf[cc][ addr ];
            }else if( src_pic->pic_buffer_type == PIC_TYPE_U16 ){
                srcMat1.at<float>( y, x ) = (float) src_pic->buf[cc][ addr ];
            }
            
            addr++;
        }
    }

    cc = 2;
    
    for(int y = 0; y < src_height[cc] ; y++ )
    {
        int addr = y* src_width[cc];
        
        for(int x = 0; x < src_width[cc] ; x++ )
        {
            if( src_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                srcMat2.at<float>( y, x ) = src_pic->fbuf[cc][ addr ];
            }else if( src_pic->pic_buffer_type == PIC_TYPE_U16 ){
                srcMat2.at<float>( y, x ) = (float) src_pic->buf[cc][ addr ];
            }
            
            addr++;
        }
    }
    }


//    srcMat = cv::imread("lena.jpg");
 //   cv::Mat sumimg(rows,cols,CV_32F, cv::Scalar::all(0) );
    
    // step 1: copy universal frame buffer type (pic_t) into a CV matrix structure
    
    
    // step 2: scale the image.
    
    
    //   inputImage = cv::imread(inputImageName, CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYDEPTH);
 //   std::cout<<"=> image size (h,w) = "<<inputImage.size().height<<", "<<inputImage.size().width<<std::endl;
#if 0
    if (!srcMat.total())
    {
        help("could not load image, program end");
        return -1;
        
    }
#endif
    
    cv::Mat dstMat0( dst_height[0], dst_width[0], CV_32F, cv::Scalar::all(0) );
    cv::Mat dstMat1( dst_height[1], dst_width[1], CV_32F, cv::Scalar::all(0) );
    cv::Mat dstMat2( dst_height[2], dst_width[2], CV_32F, cv::Scalar::all(0) );
    
    // Insert scaling code
//    cv::resize(srcMat0, dstMat0, cv::Size(0,0), 2,2, CV_INTER_LANCZOS4);
//    cv::resize(srcMat1, dstMat1, cv::Size(0,0), 2,2, CV_INTER_LANCZOS4);
//    cv::resize(srcMat2, dstMat2, cv::Size(0,0), 2,2, CV_INTER_LANCZOS4);

    float fx[3], fy[3];
    
    for(int cc=0;cc<3;cc++){
        fx[cc] = (float) dst_width[cc] / (float )src_width[cc];
        fy[cc] = (float) dst_height[cc] / (float )src_height[cc];
    }
    
    //Size(im.cols/2,im.rows/2);
    
//    cv::resize( srcMat0, dstMat0, cv::Size(0,0), fx[0], fy[0], CV_INTER_LANCZOS4);
//    cv::resize( srcMat1, dstMat1, cv::Size(0,0), fx[1], fy[1], CV_INTER_LANCZOS4);
//    cv::resize( srcMat2, dstMat2, cv::Size(0,0), fx[2], fy[2], CV_INTER_LANCZOS4);

    cv::resize( srcMat0, dstMat0, cv::Size(fx[0], fx[1]), 0, 0, CV_INTER_LANCZOS4);
    cv::resize( srcMat1, dstMat1, cv::Size(fy[1], fy[1]), 0, 0, CV_INTER_LANCZOS4);
    cv::resize( srcMat2, dstMat2, cv::Size(fy[2], fy[2]), 0, 0, CV_INTER_LANCZOS4);
    
    {
        int sw[3], sh[3], dw[3], dh[3];
    
        sw[0] =  srcMat0.size().width;
        sh[0] = srcMat0.size().height;
        sw[1] =  srcMat1.size().width;
        sh[1] = srcMat1.size().height;
        sw[2] =  srcMat2.size().width;
        sh[2] = srcMat2.size().height;
        
        dw[0] =  dstMat0.size().width;
        dh[0] = dstMat0.size().height;
        dw[1] =  dstMat1.size().width;
        dh[1] = dstMat1.size().height;
        dw[2] =  dstMat2.size().width;
        dh[2] = dstMat2.size().height;
    
        for(int cc=0;cc<3;cc++){
            printf(" cc%d: w(%d) X h(%d) --> w(%d) X h(%d) ",cc, sw[cc], sh[cc], dw[cc], dh[cc]   );
        }
    }
    
    
    // Insert sharpening code
    // I might prefer to change to yuv and only operate on y …. but for now doing whole image
    
//    cv::Laplacian(bigImage, lapImage, CV_32F, 1, 1, 0);
//    cv::addWeighted(bigImage, 1.0, lapImage, 0.0, 0, sharpImage);
    
    // write linear EXR back out
 //   cv::imwrite(outputImageName,sharpImage);
    
#if 0
    for(int y = 0; y < src_pic->height ; y++ )
    {
        int addr = y* src_pic->width;
        
        for(int x = 0; x < src_pic->width ; x++ )
        {
            if( dst_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                dst_pic->fbuf[0][ addr ] = srcMat0.at<float>( y, x );
                dst_pic->fbuf[1][ addr ] = srcMat1.at<float>( y, x );
                dst_pic->fbuf[2][ addr ] = srcMat2.at<float>( y, x );
            }else if( dst_pic->pic_buffer_type == PIC_TYPE_U16 ){
                dst_pic->buf[0][ addr ] = (int) srcMat0.at<float>( y, x );
                dst_pic->buf[1][ addr ] = (int) srcMat1.at<float>( y, x );
                dst_pic->buf[2][ addr ] = (int) srcMat2.at<float>( y, x );
            }
            
            addr++;
        }
    }
#endif
    
    {
        int cc = 0;
        
        for(int y = 0; y < dst_height[cc] ; y++ )
        {
            int addr = y* dst_width[cc];
            
            for(int x = 0; x < dst_width[cc] ; x++ )
            {
                if( dst_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                    dst_pic->fbuf[cc][ addr ] = dstMat0.at<float>( y, x );
                }else if( dst_pic->pic_buffer_type == PIC_TYPE_U16 ){
                    dst_pic->buf[cc][ addr ] = (int) dstMat0.at<float>( y, x );
                }
                
                addr++;
            }
        }
        
        cc = 1;
        for(int y = 0; y < dst_height[cc] ; y++ )
        {
            int addr = y* dst_width[cc];
            
            for(int x = 0; x < dst_width[cc] ; x++ )
            {
                if( dst_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                    dst_pic->fbuf[cc][ addr ] = dstMat1.at<float>( y, x );
                }else if( dst_pic->pic_buffer_type == PIC_TYPE_U16 ){
                    dst_pic->buf[cc][ addr ] = (int) dstMat1.at<float>( y, x );
                }
                
                addr++;
            }
        }
        
        
        
        
        cc = 2;
        for(int y = 0; y < dst_height[cc] ; y++ )
        {
            int addr = y* dst_width[cc];
            
            for(int x = 0; x < dst_width[cc] ; x++ )
            {
                if( dst_pic->pic_buffer_type == PIC_TYPE_FLOAT ){
                    dst_pic->fbuf[cc][ addr ] = dstMat2.at<float>( y, x );
                }else if( dst_pic->pic_buffer_type == PIC_TYPE_U16 ){
                    dst_pic->buf[cc][ addr ] = (int) dstMat2.at<float>( y, x );
                }
                
                addr++;
            }
        }
        
    
    }

    // TODO: delete matrix

    return(0);
}
#endif
