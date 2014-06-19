# check combinations
rm *.yuv 

# tiff RGB 4:4:4  --> YCbCr 4:2:0 
echo 
echo tiff RGB 4:4:4  --> YCbCr 4:2:0 
./hdr2yuv --src_matrix_coeffs 0 --dst_matrix_coeffs 1 \
--src_transfer_characteristics 1 --dst_transfer_characteristics 1 \
--src_colour_primaries 1 --dst_colour_primaries 1 \
--src_pic_width 1920 --src_pic_height 1080 \
--src_filename ~/Documents/Clips/Balloon_1920x1080p_25_hf_709_00000_RGB-PQ_for_BT2020-YCbCr.tiff \
--dst_filename Balloon_1920x1080_420p_10bits_1.yuv \
--src_bit_depth 12  --dst_bit_depth 10 \
--src_chroma_format_idc 3 --dst_chroma_format_idc 1 \
--verbose_level 4 


echo 
# YCbCr 4:4:4  --> YCbCr 4:4:4 
echo YCbCr 4:4:4  --> YCbCr 4:4:4 
./hdr2yuv  --src_matrix_coeffs 1 --dst_matrix_coeffs 1 \
--src_transfer_characteristics 1  --dst_transfer_characteristics 1 \
--src_colour_primaries 1 --dst_colour_primaries 1 \
--src_filename ref/traffic_2560x1600_444p_12bits_BT709-3.yuv \
--dst_filename traffic_2560x1600_444p_12bits_1.yuv  \
--src_pic_width 2560 --src_pic_height 1600 \
--src_bit_depth 12 --dst_bit_depth 12 \
--src_chroma_format_idc 3 --dst_chroma_format_idc 3 \
--verbose_level 4 --src_start_frame 0 \


echo 
echo YCbCr 4:4:4  --> YCbCr 4:2:0 
# YCbCr 4:4:4  --> YCbCr 4:2:0 
./hdr2yuv --src_matrix_coeffs 1 --dst_matrix_coeffs 1 \
--src_transfer_characteristics 1 --dst_transfer_characteristics 1 \
--src_colour_primaries 1 --dst_colour_primaries 1 \
--src_filename ref/traffic_2560x1600_444p_12bits_BT709-3.yuv \
--dst_filename traffic_2560x1600_420p_12bits_1.yuv \
--src_pic_width 2560 --src_pic_height 1600 \
--src_bit_depth 12 --dst_bit_depth 12 \
--src_chroma_format_idc 3 --dst_chroma_format_idc 1 \
--src_start_frame 0 --verbose_level 4 


# RGB 4:4:4  --> YCbCr 4:2:0 
echo 
echo RGB 4:4:4  --> YCbCr 4:2:0 
./hdr2yuv --src_matrix_coeffs 0 --dst_matrix_coeffs 1 \
--src_transfer_characteristics 1 --dst_transfer_characteristics 1 \
--src_colour_primaries 1 --dst_colour_primaries 1 \
--src_filename ~/Documents/Clips/Traffic_2560x1600_30_12bit_444_rgb/Traffic_2560x1600_30_12bit_444.rgb \
--dst_filename traffic_2560x1600_420p_10bits_1.yuv \
--src_pic_width 2560 --src_pic_height 1600 \
--src_bit_depth 12 --dst_bit_depth 10 \
--src_chroma_format_idc 3 --dst_chroma_format_idc 1 \
--src_start_frame 0 --verbose_level 4 

# dpx RGB 4:4:4  --> YCbCr 4:2:0 



# .exr  --> YCbCr 4:2:0 
echo 
echo exr to YCbCr 4:2:0 
./hdr2yuv --src_matrix_coeffs 0 --dst_matrix_coeffs 1 \
--src_transfer_characteristics 8 --dst_transfer_characteristics 1 \
--src_colour_primaries 1 --dst_colour_primaries 1 \
--src_filename ~/Documents/Clips/Technicolor/half-float-sequences/Market3_1920x1080p_50_hf_709/Market3_1920x1080p_50_hf_709_00000.exr \
--dst_filename market_1920x1080_420p_10bits_1.yuv \
--src_pic_width 1920 --src_pic_height 1080 \
--src_bit_depth 16 --dst_bit_depth 10 \
--src_chroma_format_idc 3 --dst_chroma_format_idc 1 \
--verbose_level 4 --src_start_frame 0


echo YCbCr 4:4:4  --> RGB tiff
./hdr2yuv  --src_matrix_coeffs 1 --dst_matrix_coeffs 0 \
--src_transfer_characteristics 1  --dst_transfer_characteristics 1 \
--src_colour_primaries 1 --dst_colour_primaries 1 \
--src_filename ref/traffic_2560x1600_444p_12bits.yuv \
--dst_filename traffic_1.tiff \
--src_pic_width 2560 --src_pic_height 1600 \
--src_bit_depth 12 --dst_bit_depth 16 \
--src_chroma_format_idc 3 --dst_chroma_format_idc 3 \
--verbose_level 4 --src_start_frame 0 \


cmp ref/traffic_2560x1600_444p_12bits.yuv  traffic_2560x1600_444p_12bits_1.yuv  
cmp ref/traffic_2560x1600_420p_12bits.yuv  traffic_2560x1600_420p_12bits_1.yuv  
cmp ref/traffic_2560x1600_420p_10bits.yuv  traffic_2560x1600_420p_10bits_1.yuv  
cmp ref/Balloon_1920x1080_420p_10bits.yuv   Balloon_1920x1080_420p_10bits_1.yuv 
cmp ref/market_1920x1080_420p_10bits.yuv   market_1920x1080_420p_10bits_1.yuv
cmp ref/traffic.tiff traffic_1.tiff

