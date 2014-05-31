typedef struct
{
    const char *ext_name;
    int n_components;
    int SubWidthC; // horizontal luma/chroma subsample ratio
    int SubHeightC;  // vertical luma/chroma subsample ratio
} chroma_format_t ;

// http://www.ffmpeg.org/doxygen/2.0/pixfmt_8h_source.html
// ffmpeg -pix_fmts

// table 6.1 from the HEVC spec.

#define CHROMA_400          0  // monochrome (B&W, grayscale)
#define CHROMA_420          1
#define CHROMA_422          2
#define CHROMA_444          3

const chroma_format_t chroma_format_list[] = {
    { "mono",    1, 1, 1 },
    { "yuv420p", 3, 2, 2  },
    { "yuv422p", 3, 2, 1 },
    { "yuv444p", 3, 1, 1 },
    
};


typedef struct
{
    const char *name;
    float cie_chromaticy_coordinates[3][2];
    const char *white_name;
    float cie_white_coordinate[2];
    
    
} color_primary_info_t;


enum  color_primaries_list {
    COLOR_PRIMARY_RESERVED0 = 0,
    COLOR_PRIMARY_BT709 = 1,
    COLOR_PRIMARY_UNSPECIFIED = 2,
    COLOR_PRIMARY_RESERVED3 = 3,
    COLOR_PRIMARY_BT470 = 4,
    COLOR_PRIMARY_BT601 = 5,
    COLOR_PRIMARY_SMPTE_240M = 6,
    COLOR_PRIMARY_FILM = 7,
    COLOR_PRIMARY_BT2020_10bit = 8,
    COLOR_PRIMARY_BT2020_12bit = 9,
    COLOR_PRIMARY_XYZ = 10
};

enum  transfer_characteristics_list {
    TRANSFER_RESERVED0 = 0,
    TRANSFER_BT709 = 1,
    TRANSFER_BT1361 = 1,
    TRANSFER_UNSPECIFIED = 2,
    TRANSFER_RESERVED3 = 3,
    TRANSFER_BT470M = 4,
    TRANSFER_BT1700_PAL_SECAM = 4,
    TRANSFER_BT470BG = 5,
    TRANSFER_BT601 = 6,
    TRANSFER_BT1358 = 6,
    TRANSFER_SMPTE170M = 6,
    TRANSFER_BT1700_NTSC = 6,
    TRANSFER_SMPTE240M = 7,
    TRANSFER_LINEAR = 8,
    TRANSFER_LOG1 = 9,
    TRANSFER_LOG2 = 10,
    TRANSFER_IEC61966_2_4 =11,
    TRANSFER_BT1361_EXTENDED_GAMUT = 12,
    TRANSFER_XVYCC = 12,
    TRANSFER_IEC61966_2_1 = 13,
    TRANSFER_SRGB = 13,
    TRANSFER_YCC = 13,
    TRANSFER_BT2020_10bit = 14,
    TRANSFER_BT2020_12bit = 15,
    TRANSFER_PQ = 16,
    TRANSFER_SMPTE428 = 17,
    TRANSFER_DCI = 17,
};

enum matrix_list {
    MATRIX_GBR,  // 0
    MATRIX_BT709, // 1
    MATRIX_UNSPECIFIED, // 2
    MATRIX_RESERVED, // 3
    MATRIX_FCC, // 4
    MATRIX_BT601_625line, // 5
    MATRIX_BT601_525line, // 6
    MATRIX_SMPTE240M, // 7
    MATRIX_YCoCg, // 8
    MATRIX_BT2020nc, // 9
    MATRIX_BT2020c, // 10
    
    MATRIX_YDzDx, // 11
    MATRIX_YDzDx_Y500, // 12
    MATRIX_YDzDx_Y100, // 13
    
    MATRIX_YUVPRIME1, // 14
    MATRIX_YUVPRIME2, // 15
    
    
};


const color_primary_info_t color_primaries_table_E3[11] = {
    // {"name", {{green-x,green-y} {blue-x, blue-y}, {red-x, red-y}},"white point", {white-x, white-y}},
    
    {"Reserved", {{0.0,0.0},{0.0,0.0},{0.0,0.0}}, "", {0.0, 0.0}}, // 0
    {"ITU-R BT.709-5", {{0.300,0.600},{0.150,0.060},{0.640,0.330}}, "white D65", {0.3127,0.3290}},  // 1
    {"Unspecified", {{0.0,0.0},{0.0,0.0},{0.0,0.0}}, "", {0.0,0.0}}, // 2
    {"Reserved", {{0.0,0.0},{0.0,0.0},{0.0,0.0}}, "", {0.0, 0.0}}, // 3
    {"ITU-R BT.470-6 System M", {{ 0.21, 0.71 }, {0.14,  0.08}, {0.67,  0.33 }},"white C", {0.310, 0.316}}, // 4
    {"ITU-R BT.601-6 625", {{0.29, 0.60}, {0.15, 0.06}, {0.64, 0.33}}, "white D65", {0.3127, 0.3290}}, // 5
    {"ITU-R BT.601-6 525", {{0.310, 0.595}, {0.155, 0.070}, {0.630, 0.340}}, "white D65", {0.3127, 0.3290}}, // 6
    {"SMPTE 240M", {{ 0.310, 0.595}, {0.155, 0.070}, {0.630, 0.340}}, "white D65", {0.3127, 0.3290}}, // 7
    {"Generic film", {{0.243, 0.692},{0.145, 0.049}, {0.681, 0.319}}, "white C", {0.310, 0.316}}, // 8
    {"ITU-R BT.2020", {{0.170, 0.797}, {0.131, 0.046}, {0.708, 0.292}}, "white D65", {0.3127, 0.3290}}, // 9
    {"XYZ (SMPTE ST 428-1)", {{0.000, 1.000}, {0.000, 0.000}, {1.000, 0.0000}}, "center white", {0.333, 0.333}}, // 10
};

typedef struct
{
    const char *name;
    float Kr, Kb;
    
} matrix_info_t;



typedef struct
{
    const char *name;
    int number;
} matrix_coeffs_info_t;

const matrix_coeffs_info_t matrix_table[16] =
{
        {"GBR", MATRIX_GBR },
        {"709", MATRIX_BT709 },
        {"UNSPECIFIED", MATRIX_UNSPECIFIED },
        {"RESERVED", MATRIX_RESERVED },
        {"FCC", MATRIX_FCC },
        {"601_625line", MATRIX_BT601_625line },
        {"601_525line", MATRIX_BT601_525line },
        {"240M", MATRIX_SMPTE240M },
        {"YCoCg", MATRIX_YCoCg },
        {"2020nc", MATRIX_BT2020nc },
        {"2020c", MATRIX_BT2020c},
        
        {"YDzDx", MATRIX_YDzDx },
        {"Y500", MATRIX_YDzDx_Y500 },
        {"Y100", MATRIX_YDzDx_Y100 },
        {"YUVPrime1", MATRIX_YUVPRIME1},
        {"YUVPrime2", MATRIX_YUVPRIME2},
};

// indexed by matrix_coeffs
const matrix_info_t matrix_coefficients_table_E5[12] = {
    {"GBR", 0.0, 0.0}, // 0
    {"ITU-R 709", 0.2126, 0.0722}, // 1
    {"Unspecified", 0.0, 0.0 }, // 2
    {"Reserved", 0.0, 0.0 }, // 3
    {"FCC 73.682 (a)(20)", 0.0, 0.11}, // 4
    {"ITU-R BT.601-6 625", 0.299, 0.114 }, // 5
    {"ITU-R BT.601-6 525;  SMPTE 170M", 0.299, 0.114 }, // 6
    {"SMPTE 240M", 0.212, 0.087 }, // 7
    {"YCoCg", 0.0, 0.0 }, // 8
    {"ITU-R BT.2020 non-constant luminance", 0.2627, 0.0593}, // 9
    {"ITU-R BT.2020 constant luminance", 0.2627, 0.0593}, // 10
    {"YZX", 0.0, 0.0 } // 11
    
};

typedef struct
{
    int width;
    int height;
    int bit_depth;
    unsigned short *buf;
    
} t_component;

#define MAX_NUM_CC  3

typedef struct
{
    int chroma_format_idc;
    int transfer_characteristics;
    int colour_primaries;
    int matrix_coeffs;
    int chroma_sample_loc_type;
    
    int bit_depth;
    int video_full_range_flag;
    
    //int bit_depth_Y;
    //int bit_depth_C;
    int pic_width;
    int pic_height;
    
    
} t_vui;



typedef struct
{
    t_vui vui;
    int n_components;
    t_component cc[MAX_NUM_CC];
    
} t_yuv_frame;

typedef struct
{
    char *src_filename;
    char *dst_filename;
    int verbose_level;
    int src_start_frame;
    int n_frames;
    
    t_vui src_vui;
    t_vui dst_vui;
    
    int src_pic_width;
    int src_pic_height;
    int dst_pic_width;
    int dst_pic_height;
    
    int src_bit_depth;
    int dst_bit_depth;
    
    int src_chroma_format_idc;
    int dst_chroma_format_idc;
    
    int src_colour_primaries;
    int dst_colour_primaries;
    int src_transfer_characteristics;
    int dst_transfer_characteristics;
    int src_matrix_coeffs;
    int dst_matrix_coeffs;
    
    int src_video_full_range_flag;  // 0: 16,235   1:  0,255
    int dst_video_full_range_flag;
    
    int src_chroma_sample_loc_type;
    int dst_chroma_sample_loc_type;
    
    int chroma_resampler_type;    // BOX, FIR
    
    int alpha_channel;   // 0: off,  1: on
    int cutout_hd;
    int cutout_qhd;
    
    float bright_means_threshold;
    
    // SMPTE 2050 types
    
    
    
} t_user_args;

#if 0
typedef struct
{
    t_yuv_frame src;
    t_yuv_frame dst;
    t_user_args args;
} t_hdr;
#endif

typedef struct {
    unsigned long long Y;
    unsigned long long Cb;
    unsigned long long Cr;
    
} t_mse;

typedef struct
{
    double Y;
    double Cb;
    double Cr;
} t_psnr;



typedef struct
{
    int width;
    int height;
    
    int chroma_format_idc;
    int transfer_characteristics;
    int colour_primaries;
    int matrix_coeffs;
    int chroma_sample_loc_type;
    
    int bit_depth;
    int video_full_range_flag;

    unsigned short *buf[3];
 
} t_pic;


typedef struct
{
  //  unsigned long Half; // e.g half value 12 bits would have been 2048
    unsigned long minCV; //12 bits
    unsigned long maxCV;
   // unsigned short D; //D=4 for 12 bits =1 10bits = 16 for 14 bits
    unsigned short minVR, maxVR, minVRC,maxVRC,Half;
    
    t_user_args user_args;
    
    t_pic in_pic;
    t_pic out_pic;
    
} t_hdr;


void exr_test( char *fn );

int convert(  t_hdr *h );
int write_tiff(  char *filename, t_hdr *h );
int matrix_convert( t_pic *out_pic, t_hdr *h, t_pic *in_pic );
int read_tiff( t_pic *tif_pic, t_hdr *h, char* filename );


