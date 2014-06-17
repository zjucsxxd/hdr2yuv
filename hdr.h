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

enum input_file_type_list {
    INPUT_FILE_TYPE_UNDEFINED = 0,
    INPUT_FILE_TYPE_YUV,       // raw
    INPUT_FILE_TYPE_TIFF,
    INPUT_FILE_TYPE_EXR,
    INPUT_FILE_TYPE_Y4M,
    INPUT_FILE_TYPE_DPX,
    INPUT_FILE_TYPE_RGB,
    INPUT_FILE_TYPE_LAST,
};

enum output_file_type_list {
    OUTPUT_FILE_TYPE_UNDEFINED = 0,
    OUTPUT_FILE_TYPE_YUV,       // raw
    OUTPUT_FILE_TYPE_TIFF,
    OUTPUT_FILE_TYPE_EXR,
    OUTPUT_FILE_TYPE_Y4M,
    OUTPUT_FILE_TYPE_DPX,
    OUTPUT_FILE_TYPE_RGB,
    OUTPUT_FILE_TYPE_LAST,
};

struct type_info_t
{
    int idx;
    const char *name;
    int is_supported;
} ;

static struct type_info_t input_file_types[] =
{
    { INPUT_FILE_TYPE_UNDEFINED, "UNDEFINED", 0 },
    { INPUT_FILE_TYPE_YUV, "yuv", 1 },
    { INPUT_FILE_TYPE_TIFF, "tiff", 1 },
    { INPUT_FILE_TYPE_EXR, "exr", 1 },
    { INPUT_FILE_TYPE_Y4M, "y4m", 0 },
    { INPUT_FILE_TYPE_DPX, "dpx", 1 },
    { INPUT_FILE_TYPE_RGB, "rgb", 1 }
};

static struct type_info_t output_file_types[] =
{
    { OUTPUT_FILE_TYPE_UNDEFINED, "UNDEFINED", 0 },
    { OUTPUT_FILE_TYPE_YUV, "yuv", 1 },
    { OUTPUT_FILE_TYPE_TIFF, "tiff", 0 },
    { OUTPUT_FILE_TYPE_EXR, "exr", 1 },
    { OUTPUT_FILE_TYPE_Y4M, "y4m", 0 },
    { OUTPUT_FILE_TYPE_DPX, "dpx", 1 },
    { OUTPUT_FILE_TYPE_RGB, "rgb", 0 }
};





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
    TRANSFER_BT709,
 //   TRANSFER_BT1361 = 1,
    TRANSFER_UNSPECIFIED,
    TRANSFER_RESERVED3,
    TRANSFER_BT470M,
//    TRANSFER_BT1700_PAL_SECAM = 4,
    TRANSFER_BT470BG = 5,
    TRANSFER_BT601 = 6,
//    TRANSFER_BT1358 = 6,
//    TRANSFER_SMPTE170M = 6,
//    TRANSFER_BT1700_NTSC = 6,
    TRANSFER_SMPTE240M,
    TRANSFER_LINEAR,
    TRANSFER_LOG1,
    TRANSFER_LOG2,
    TRANSFER_IEC61966_2_4,
//    TRANSFER_BT1361_EXTENDED_GAMUT,
    TRANSFER_XVYCC = 12,
//    TRANSFER_IEC61966_2_1 = 13,
    TRANSFER_SRGB,
//    TRANSFER_YCC = 13,
    TRANSFER_BT2020_10bit,
    TRANSFER_BT2020_12bit,
    TRANSFER_PQ,
    TRANSFER_SMPTE428,
//    TRANSFER_DCI = 17,
    
    TRANSFER_RHO_GAMMA,
};

static struct type_info_t transfer_types_table[] =
{
    { TRANSFER_RESERVED0, "RESERVED0", 0 },
    { TRANSFER_BT709, "BT709", 1 },
//    { TRANSFER_BT1361, "BT1361", 0 },
    { TRANSFER_UNSPECIFIED, "UNSPECIFIED", 0 },
    { TRANSFER_RESERVED3, "RESERVED3", 0 },
    { TRANSFER_BT470M, "BT470M", 0 },
//    { TRANSFER_BT1700_PAL_SECAM, "BT1700_PAL_SECAM", 0 },
    { TRANSFER_BT470BG, "BT470BG", 0 },
    { TRANSFER_BT601, "BT601", 1 },
//    { TRANSFER_BT1358, "BT1358", 0 },
 //   { TRANSFER_SMPTE170M, "SMPTE170M", 0 },
 //   { TRANSFER_BT1700_NTSC, "BT1700_NTSC", 0 },
    { TRANSFER_SMPTE240M, "SMPTE240M", 0 },
    { TRANSFER_LINEAR, "LINEAR", 1 },
    { TRANSFER_LOG1, "LOG1", 0 },
    { TRANSFER_LOG2, "LOG2", 0 },
    { TRANSFER_IEC61966_2_4, "IEC61966_2_4", 0 },
//    { TRANSFER_BT1361_EXTENDED_GAMUT, "BT1361_EXTENDED_GAMUT", 0 },
    { TRANSFER_XVYCC, "XVYCC", 0 },
//    { TRANSFER_IEC61966_2_1, "IEC61966_2_1", 0 },
    { TRANSFER_SRGB, "SRGB", 0 },
//    { TRANSFER_YCC, "YCC", 0 },
    { TRANSFER_BT2020_10bit, "BT2020_10bit", 0 },
    { TRANSFER_BT2020_12bit, "BT2020_12bit", 0 },
    { TRANSFER_PQ, "PQ", 1 },
    { TRANSFER_SMPTE428, "SMPTE428", 0 },
 //   { TRANSFER_DCI, "DCI", 0 },
    { TRANSFER_RHO_GAMMA, "RHO_GAMMA", 1 },
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
    char *src_filename;
    char *dst_filename;
    int verbose_level;
    int src_start_frame;
    int n_frames;
    
    int chroma_resampler_type;    // BOX, FIR
    
    int alpha_channel;   // 0: off,  1: on
    int cutout_hd;
    int cutout_qhd;
    
    float bright_means_threshold;
    
    // SMPTE 2050 types
    
    int input_file_type;
    int output_file_type;
    
} user_args_t;


#define MIN_BIT_DEPTH   10
#define MAX_BIT_DEPTH   16

#define PIC_TYPE_U16    1
#define PIC_TYPE_F32  2

#define MAX_CC  3       // maximum component channels (color sample planes)

typedef struct
{
    int init;
    
    float f_max[MAX_CC];
    float f_min[MAX_CC];
    float f_avg[MAX_CC];
    
    unsigned short i_max[MAX_CC];
    unsigned short i_min[MAX_CC];
    unsigned short i_avg[MAX_CC];
    
    
    int estimated_ceiling[MAX_CC];
    int estimated_floor[MAX_CC];
    
} pic_stats_t;

typedef struct
{
    int init;
    
    float f_max;
    float f_min;
    float f_avg;
    float f_var;
    
    unsigned short i_max;
    unsigned short i_min;
    unsigned short i_avg;
    unsigned short i_var;
    
    int estimated_ceiling;
    int estimated_floor;
    
} plane_stats_t;

typedef struct
{
    int width;
    int height;
} pic_plane_t;

typedef struct
{
    unsigned long minCV; //12 bits
    unsigned long maxCV;
    // unsigned short D; //D=4 for 12 bits =1 10bits = 16 for 14 bits
    unsigned short minVR;
    unsigned short maxVR;
    unsigned short minVRC;
    unsigned short maxVRC;
    unsigned short Half;

} clip_limits_t;


typedef struct
{
    int init;
    
    int n_components;
    
    int width;
    int height;
    
    int chroma_format_idc;
    int transfer_characteristics;
    int colour_primaries;
    int matrix_coeffs;
    int chroma_sample_loc_type;
    
    int bit_depth;
    int video_full_range_flag;

    int pic_buffer_type;
    int half_float_flag;
    
    clip_limits_t clip;

    unsigned short *buf[MAX_CC];
    float *fbuf[MAX_CC];
    
    const char *name;
    
    pic_stats_t stats;

    pic_plane_t plane[MAX_CC];


} pic_t;


typedef struct
{
#if 0
    //  unsigned long Half; // e.g half value 12 bits would have been 2048
    unsigned long minCV; //12 bits
    unsigned long maxCV;
   // unsigned short D; //D=4 for 12 bits =1 10bits = 16 for 14 bits
    unsigned short minVR, maxVR, minVRC,maxVRC,Half;
#endif
    
    user_args_t user_args;
    
    pic_t in_pic;
    pic_t out_pic;
    
} hdr_t;


//void exr_test( char *fn );

// exr.cpp
void read_exr( pic_t *dst_pic, char *filename );
int write_exr_file( char *filename, int pic_width, int pic_height, int half_float_flag, pic_t *src_pic );

// convert.cpp
int convert( pic_t *out_pic, hdr_t *h, pic_t *in_pic );
int write_yuv(  char *filename, hdr_t *h, pic_t *in_pic, int src_bit_depth );
int matrix_convert( pic_t *out_pic, hdr_t *h, pic_t *in_pic );

// tiff.cpp
int read_tiff( pic_t *tif_pic, char* filename, hdr_t *h );

// dpx.cpp from Gary Demos
void dpx_write_float(char *outname, float *pixel_result, short width, short height);
void dpx_read (char *inname, float **pixels_read, short *width, short *height, short cineon, short half_flag);
void dpx_write_10bit_from_float(char *outname, float *pixel_result, short width, short height);
void  dpx_write_10bit_from_float(char *outname, float *pixel_result, short width, short height);


// common.cpp
int planar_float_to_muxed_dpx_buf( float *dst, int width, int height, pic_t *src );
int muxed_dpx_to_planar_float_buf( pic_t *dst, int width, int height, float *src );
void pic_stats( pic_t *pic, pic_stats_t *stats, int print_stats );
void copy_pic_vars( pic_t *dst, pic_t *src );
int init_pic( pic_t *pic, int width, int height, int chroma_format_idc, int bit_depth, int video_full_range_flag, int colour_primaries, int transfer_characteristics, int matrix_coeffs, int chroma_sample_loc_type, int sample_type, int half_float_flag, int verbose_level, const char *name );
int deinit_pic( pic_t *pic );
void set_pic_clip( pic_t *pic );

#ifdef OPENCV_ENABLED
int openCV_resize_picture(  pic_t *dst_pic, pic_t *src_pic  );
#endif

