hdr2yuv
=======

Conversion of High Dynamic Range (HDR) files  (.exr, .tiff) to/from .yuv input to codecs such as HEVC and AVC 


this tool is an update of JCTVC-Q0085 (Universal Pictures & MovieLabs)

   http://phenix.int-evry.fr/jct/doc_end_user/current_document.php?id=8888


The install instructions to build the supporting tools (OpenEXR, LibTIFF..) are described below:


1. Install zlib

curl -OL http://zlib.net/zlib-1.2.8.tar.gz
tar xvf zlib-1.2.8.tar.gz
cd zlib-1.2.8/
./configure 
make
make install
cd ..


2. Install ilmbase

curl -OL http://download.savannah.nongnu.org/releases/openexr/ilmbase-2.1.0.tar.gz

tar xvf ilmbase-2.1.0.tar 
cd ilmbase-2.1.0/
./configure
make 
make install
cd ..


3. Install OpenEXR

curl -OL http://download.savannah.nongnu.org/releases/openexr/openexr-2.1.0.tar.gz

tar xvf openexr-2.1.0.tar.gz 
cd openexr-2.1.0
./configure 
make
make install
cd ..

4. Tifflib

If you don’t have tiff library, you can use brew to install it:

	brew install tifflib

or FTP it from remotesensing.org/libtiff, or use its CVS server to check out a copy (use “brew install cvs” if you don’t have CVS installed):

   brew tap epichub/homebrew-epicbrews
   brew install cvs

   export CVSROOT=:pserver:cvsanon@cvs.maptools.org:/cvs/maptools/cvsroot
   cvs login
   [ use empty password -- hit return key ]
   cvs checkout libtiff

cd libtiff
./configure
make 
make install
cd ..

5. ACES

Prior to installing ACES tools (ctlrender), the documentation 

	https://github.com/ampas/CTL

recommends either building ilmBase, OpenEXR as above or use home-brew to automatically retrieve dependent libraries and build each tool: cake, ilmBase, openexr.

	brew install cmake
	brew install ilmBase
	brew install openexr


To build the ctlrender program:

	curl -OL https://github.com/ampas/CTL/archive/ctl-1.5.zip
	unzip ctl-1.5.zip
	cd CTL-ctl-1.5/
	mkdir build
	cd build
	cmake ..
	make 
	sudo make install
        cd ..

To get the standard library of ctlrender scripts

	curl -OL https://github.com/ampas/aces-dev/archive/v0.7.1.zip
	unzip v0.7.1.zip
	cd aces-dev-0.7.1/
        cd ..


6. Build the tiff2yuv tools

make a directory with the JCTVC-Q0085 sources files:

	2020-2-709-500 1.sh		
	709-2-OCES.ctl			
	encodeHDY500.sh
	encodeHD2020.sh
	encodeHD.sh			
	INVPQ10k-2-XYZ.ctl		
	INVPQ10k2020-2-XYZ.ctl		
	makeYUV.sh
	odt_PQ10k.ctl
	odt_PQ10k2020.ctl
	OutputConversion.sh		
	odt_rec709_full_500nits.ctl
	OutputConversion2020.sh		
	OutputConversionXYZ.sh		
	PQ.ctl				
	PQ.sh				
	scale10k.ctl
	scaleR179.ctl
	technicolor.sh
	tiff2ydzdx.cpp
	yuv.sh
	ydzdx2tiff.cpp
	XYZ2ACES.ctl			

Compile the tiff <—> yuv conversion programs:

	cd source
	g++ -O3 tiff2ydzdx.cpp -o tiff2ydzdx -ltiff
	g++ -O3 ydzdx2tiff.cpp -o ydzdx2tiff -ltiff

[ on Mac, I had to change the main() in both tiff2ydzdx.cpp and ydzdx2tiff.cpp to return int ]

7.  Test one Technicolor .exr file

In order to run ctlrender on these scripts, a few control scripts must be in the path or copied in the current directory:

 cp ../aces-dev-0.7.1/transforms/ctl/utilities/utilities-color.ctl .
 cp ../aces-dev-0.7.1/transforms/ctl/utilities/utilities.ctl .
 cp ../aces-dev-0.7.1/transforms/ctl/utilities/transforms-common.ctl .
 cp ../aces-dev-0.7.1/transforms/ctl/utilities/rrt-transform-common.ctl .

First, the .exr file is converted from 709-color to OCES RGB (709-2-OCES.ctl), then the scene-referred RGB is ranged to display range by a 1/179 scale factor of the Sim2 projector.  Finally a minor rolloff is applied (rrt.ctl) 

ctlrender -force -ctl 709-2-OCES.ctl -ctl scaleR179.ctl -ctl ../aces-dev-0.7.1/transforms/ctl/rrt/rrt.ctl -param1 aIn 1.0 Market3_1920x1080p_50_hf_709_00000.exr -format exr32 Market3_1920x1080p_50_hf_709_00000_scaled_rrt_tmp.exr

Note that rrt.ctl needs an Alpha parameter (-param1 aIn 1.0) to be specified. 

The temporary file (Market3_1920x1080p_50_hf_709_00000_scaled_rrt_tmp.exr) we just created from the original source file (Market3_1920x1080p_50_hf_709_00000.exr) has all samples scaled by 1/179 and the standard rolloff (rrt.ctl) has been applied. Color space is RGB.

Next, we convert rendered RGB to XYZ, then apple the PQ EOTF and quantize the samples to 16-bit integers and stuff them in a .tiff container:

ctlrender -force -ctl odt_PQ10k.ctl Market3_1920x1080p_50_hf_709_00000_scaled_rrt_tmp.exr -format tiff16 Market3_1920x1080p_50_hf_709_00000_XYZ_444_PQ-ETOF_10k-nits.tiff 

From this tiff (Market3_1920x1080p_50_hf_709_00000_XYZ_444_PQ-ETOF_10k-nits.tiff), which contains 16-bit 10,000 nit display-ranged XYZ 4:4:4 samples, we can now derive a .yuv encoding input container that is formatted to BT.2020, YDzDx, BT.709, etc. color spaces.


