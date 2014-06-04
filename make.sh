
rm *.o

g++ -I. -pipe -g -O2 -MT hdr2yuv.o -MD -MP -MF main.Tpo -c -o hdr2yuv.o hdr2yuv.cpp

g++ -I. -I/usr/local/include/OpenEXR -pipe -g -O2 -MT exr.o -MD -MP -MF exr.Tpo -c -o exr.o exr.cpp

g++ -pipe -g -O2 -MT tiff.o -MD -MP -MF tiff.Tpo -c -ltiff -o tiff.o tiff.cpp

g++ -pipe -g -O2 -MT common.o -MD -MP -MF common.Tpo -c -o common.o common.cpp

g++ -pipe -g -O2 -MT cv.o -MD -MP -MF cv.Tpo -c -o cv.o cv.cpp

g++ -pipe -g -O2 -MT convert.o -MD -MP -MF convert.Tpo -c -o convert.o convert.cpp

g++ -I.  -I/usr/local/include/OpenEXR -pipe -g -O2 -MT dpx.o -MD -MP -MF dpx.Tpo -c -o dpx.o dpx.cpp

/bin/sh libtool --tag=CXX --mode=link g++ -pipe -g -O2   -o hdr2yuv hdr2yuv.o exr.o tiff.o convert.o common.o cv.o dpx.o -L/usr/local/lib -lImath -lHalf -lIex -lIexMath -lIlmThread -lIlmImf -lz -ltiff

#  ./hdr2yuv --src_filename Balloon_1920x1080p_25_hf_709_00000_RGB-PQ_for_BT2020-YCbCr.tiff --dst_matrix_coeffs 1 --dst_bit_depth 10 --dst_video_full_range_flag 0 --chroma_resampler_type 1 --dst_filename YDzDx.yuv

