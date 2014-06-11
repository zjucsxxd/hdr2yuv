CC=g++
CFLAGS=-c -O3 

clean:
	rm -rf *.o hdr2yuv

all:  hdr2yuv

objects = hdr2yuv.o exr.o tiff.o common.o cv.o convert.o dpx.o

libs=-lImath -lHalf -lIex -lIexMath -lIlmThread -lIlmImf -lz -ltiff

hdr2yuv:  $(objects) 
	$(CC) -o hdr2yuv $(objects) $(libs) 

hdr2yuv.o:  hdr2yuv.cpp
	$(CC) $(CFLAGS) hdr2yuv.cpp

exr.o:  exr.cpp
	$(CC) $(CFLAGS) exr.cpp -I/usr/local/include/OpenEXR 

tiff.o:  tiff.cpp
	$(CC) $(CFLAGS) tiff.cpp -ltiff 

common.o:  common.cpp
	$(CC) $(CFLAGS) common.cpp

cv.o:  cv.cpp
	$(CC) $(CFLAGS) cv.cpp

convert.o:  convert.cpp
	$(CC) $(CFLAGS) convert.cpp

dpx.o:  dpx.cpp
	$(CC) $(CFLAGS) dpx.cpp -l -lImath -lHalf -lIex -lIexMath -lIlmThread -lIlmImf -lz -I/usr/local/include/OpenEXR 



