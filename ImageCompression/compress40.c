#include "compress40.h"
#include <stdlib.h>
#include <math.h>
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "arith40.h"
#include "bitpack.h"

/* Widths for packing/unpacking values into code word */
#define CODE_WORD_WIDTH 32
#define A_WIDTH 9 //challenge: 9 --> 6
#define B_C_D_WIDTH 5 //challenge: 5 --> 6
#define PB_PR_WIDTH 4

/* LSBs for packing/unpacking values into code word */
#define A_LSB CODE_WORD_WIDTH - A_WIDTH //challenge: 23 --> 26
#define B_LSB A_LSB - B_C_D_WIDTH //challenge: 18 --> 20
#define C_LSB B_LSB - B_C_D_WIDTH //challenge: 13 --> 14
#define D_LSB C_LSB - B_C_D_WIDTH // 8
#define PB_LSB D_LSB - PB_PR_WIDTH // 4
#define PR_LSB PB_LSB - PB_PR_WIDTH // 0

/* Scales for DCT quantization */
#define A_DCT_SCALE 511 //challenge: 511 --> 63
#define B_C_D_DCT_SCALE 50 //challenge: 50 --> 100
#define DCT_RANGE 15 //challenge: 15 --> 31

/* Methods object to be used throughout program */
#define METHODS uarray2_methods_plain

/* Standard RGB denominator for decompression */
#define DENOMINATOR 255

/* Struct representation of component-video */
typedef struct {
	float y;
	float pb;
	float pr;
} component_video;

/* Struct holding every value for code word */
typedef struct {
    uint64_t a;
    int64_t  b;
    int64_t  c;
    int64_t  d;
    uint64_t pb_index;
    uint64_t pr_index; 
} code_word_info;

/* COMPRESSION */
static void compress_apply(int col, int row, 
                           A2Methods_UArray2 arr, 
						   void *elem, 
						   void *cl);
static component_video rgb_to_component_video(Pnm_rgb rgb, int denominator);
static code_word_info create_code_word_info(component_video one, 
                                            component_video two, 
											component_video three, 
											component_video four);
static int quantize_dct(float dct);
static uint64_t pack_code_word(code_word_info info);
static void print_code_word(uint64_t code_word);

/* DECOMPRESSION */
static void decompress_apply(int col, int row, 
                             A2Methods_UArray2 arr, 
							 void *elem, 
							 void *cl);
static code_word_info unpack_code_word(FILE *file);
static uint64_t read_code_word(FILE *file);
static float unquantize_dct(float quantized_dct);
static struct Pnm_rgb component_video_to_rgb(component_video cv);
static float fit_in_interval(float num);

/* extern void compress40(FILE *input)              
 * Parameters: FILE *input - PPM file to be compressed               
 *    Returns: None    
 *       Does: Trims width and height if odd and prints header of compressed
 * 			   image, maps compress_apply which prints all compressed code words 
 *   if Error: raises assertion if input is null                                                                    
 */
extern void compress40(FILE *input) 
{
	assert(input != NULL);

	/* Read image to be compressed */
	Pnm_ppm input_ppm = Pnm_ppmread(input, METHODS);

	/* Trimmed edges so that width and height are even (remove last odd edge) */
	unsigned trimmed_width = (input_ppm->width / 2) * 2;
	unsigned trimmed_height = (input_ppm->height / 2) * 2;

	/* Print compressed image header */
	printf("COMP40 Compressed image format 2\n%u %u\n", trimmed_width, 
	                                                    trimmed_height);

	/* Map compress_apply on all pixels */
	METHODS->map_default(input_ppm->pixels, compress_apply, &input_ppm);

	Pnm_ppmfree(&input_ppm);
}

/* extern void decompress40(FILE *input)              
 * Parameters: FILE *input - Compressed PPM image to be decompressed               
 *    Returns: None    
 *       Does: Reads image header, maps decompress_apply which unpacks all
 * 			   code words and creates a PPM pixel map, prints the PPM image
 *   if Error: raises assertions if 
 * 			       input is null
 * 				   width or height are not able to be read from header
 * 				   header doesn't end with a newline character               
 */
extern void decompress40(FILE *input)
{
	assert(input != NULL);

	/* Get width and height by reading header of compressed image */
	unsigned width, height;
	int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
	                  &width, &height);
	assert(read == 2);
	int c = getc(input);
	assert(c == '\n');

	/* Initialize empty pixel map to be filled from decompression */
	A2Methods_UArray2 input_pixels = METHODS->new(width, height, 
	                                              sizeof(struct Pnm_rgb));

	/* Initialize Pnm_ppm to be filled and printed */
	struct Pnm_ppm pixmap = { .width = width,
	                          .height = height,
					          .denominator = DENOMINATOR,
	                          .pixels = input_pixels,
					          .methods = METHODS
					        };

	/* Map decompress_apply on all pixels with input file as *cl to read from */
	METHODS->map_default(pixmap.pixels, decompress_apply, input);

	/* Print decompressed image */
	Pnm_ppmwrite(stdout, &pixmap);

	METHODS->free(&input_pixels);
}

/* static void compress_apply(int col, int row, 
                              A2Methods_UArray2 arr, 
						      void *elem, 
						      void *cl)                  
 * Parameters: int col - column index of current pixel                                        
 *             int row - row index of current pixel
 * 			   A2Methods_UArray2 arr - UArray2 of pixels being mapped on and
 *                                     compressed
 * 			   void *elem - current pixel Pnm_rgb (void)
 * 			   void *cl - Pnm_ppm being mapped on/being compressed      
 *    Returns: None    
 *       Does: For each top-left corner pixel of 2x2 blocks, converts RGBs
 * 			   to compressed code words and prints the code words
 *   if Error: No errors to be handled                        
 */  
static void compress_apply(int col, int row, 
                           A2Methods_UArray2 arr, 
						   void *elem, 
						   void *cl)
{
	(void) elem;

	/* Create local variables */
	Pnm_ppm input_ppm = *(Pnm_ppm *) cl;
	int width = input_ppm->width;
	int height = input_ppm->height;
	unsigned denominator = input_ppm->denominator;
	
	/* Skip pixels that are trimmed off */
	if ((col >= width - 1) || (row >= height - 1)) {
		return;
	}

	/* Apply on top-left corners of 2x2 pixel blocks */
	if (col % 2 == 0 && row % 2 == 0) {
		component_video one, two, three, four;

		/* Create component-video representations of each pixel in the 2x2 */
		Pnm_rgb pixel1, pixel2, pixel3, pixel4;

		pixel1 = METHODS->at(arr, col, row);
		pixel2 = METHODS->at(arr, col + 1, row);
		pixel3 = METHODS->at(arr, col, row + 1);
		pixel4 = METHODS->at(arr, col + 1, row + 1);
		
		one = rgb_to_component_video(pixel1, denominator);
		two = rgb_to_component_video(pixel2, denominator);
		three = rgb_to_component_video(pixel3, denominator);
		four = rgb_to_component_video(pixel4, denominator);
		
		/* Prepare info to be packed */
		code_word_info info = create_code_word_info(one, two, three, four);

		/* Pack info into code word */
		uint64_t code_word = pack_code_word(info);

		print_code_word(code_word);
	}
}

/* static component_video rgb_to_component_video(Pnm_rgb rgb, int denominator)           
 * Parameters: Pnm_rgb rgb - RGB pixel to be converted to component-video
 * 			   int denominator - denominator of original PPM image  
 *    Returns: component_video - struct representation of component-video
 * 								 form of inputted RGB pixel
 *       Does: Converts RGB values of inputted pixel to component-video form 
 * 			   (i.e. Y, Pb, Pr)
 *   if Error: No errors to be handled                        
 */  
static component_video rgb_to_component_video(Pnm_rgb rgb, int denominator)
{
	component_video new_cv;
    
	/* Scale RGB values by denominator */
    float red = (float) rgb->red / denominator;
    float green = (float) rgb->green / denominator;
    float blue = (float) rgb->blue / denominator;

	/* Convert scaled RGB values to Y, Pb, and Pr */
    new_cv.y = 0.299 * red + 0.587 * green + 0.114 * blue;
    new_cv.pb = -0.168736 * red - 0.331264 * green + 0.5 * blue;
    new_cv.pr = 0.5 * red - 0.418688 * green - 0.081312 * blue;
    
    return new_cv;
}

/* static code_word_info create_code_word_info(component_video one, 
                                               component_video two, 
											   component_video three, 
											   component_video four)      
 * Parameters: component_video one, two, three, four
 * 			   - component-video form of four pixels from a 2x2 block
 *    Returns: code_word_info - struct representation of the values needed
 * 			   to be packed into a code word
 *       Does: Converts component-video form to values to be packed into
 * 			   code word (i.e. a, b, c, d, pb_index, pr_index)
 *   if Error: No errors to be handled                        
 */  
static code_word_info create_code_word_info(component_video one, 
                                            component_video two, 
											component_video three, 
											component_video four)
{
	code_word_info info;

	/* Calculate average Pb and Pr of the four pixels */
	float avg_pb = (one.pb + two.pb + three.pb + four.pb) / 4.0;
	float avg_pr = (one.pr + two.pr + three.pr + four.pr) / 4.0;

	/* Calculate index of Pb and Pr with given function */
	info.pb_index = Arith40_index_of_chroma(avg_pb);
	info.pr_index = Arith40_index_of_chroma(avg_pr);

	/* Quantize a, b, c, and d */
	info.a = ((four.y + three.y + two.y + one.y) / 4.0) * A_DCT_SCALE; //challenge: 511 --> 63
	info.b = quantize_dct((four.y + three.y - two.y - one.y) / 4.0);
	info.c = quantize_dct((four.y - three.y + two.y - one.y) / 4.0);
	info.d = quantize_dct((four.y - three.y - two.y + one.y) / 4.0);

	return info;
}

/* static int quantize_dct(float dct)    
 * Parameters: float dct - DCT value to be quantized
 *    Returns: int - quantized DCT
 *       Does: Quantizes DCT by fitting DCT into (-0.3, 0.3) and scaling
 *   if Error: No errors to be handled                        
 */  
static int quantize_dct(float dct)
{
	/* Quantize large DCT into range of (-15, 15) */
	if (dct < -0.3) {
        return -DCT_RANGE;
    }
    if (dct > 0.3) {
        return DCT_RANGE;
    }

	/* Convert small DCT to 5-bit scaled integer (-15, 15) */
	int quantized_dct = dct * B_C_D_DCT_SCALE;

    return quantized_dct;
}

/* static uint64_t pack_code_word(code_word_info info)  
 * Parameters: code_word_info info - struct of values to be packed
 *    Returns: uint64_t - compressed code word to be printed
 *       Does: Utilizes bitpack functions to pack inputted values into code word
 *   if Error: No errors to be handled
 */  
static uint64_t pack_code_word(code_word_info info)
{
	uint64_t code_word = 0;

	code_word = Bitpack_newu(code_word, A_WIDTH, A_LSB, info.a);
    code_word = Bitpack_news(code_word, B_C_D_WIDTH, B_LSB, info.b);
    code_word = Bitpack_news(code_word, B_C_D_WIDTH, C_LSB, info.c);
    code_word = Bitpack_news(code_word, B_C_D_WIDTH, D_LSB, info.d);
    code_word = Bitpack_newu(code_word, PB_PR_WIDTH, PB_LSB, info.pb_index);
    code_word = Bitpack_newu(code_word, PB_PR_WIDTH, PR_LSB, info.pr_index);

    return code_word;
}

/* static void print_code_word(uint64_t code_word)
 * Parameters: uint64_t code_word - code word to be printed to stdout
 *    Returns: None
 *       Does: Utilizes bitpack to print code word in big-endian order to stdout
 *   if Error: No errors to be handled
 */  
static void print_code_word(uint64_t code_word)
{
    putchar(Bitpack_getu(code_word, 8, 24));
    putchar(Bitpack_getu(code_word, 8, 16));
    putchar(Bitpack_getu(code_word, 8, 8));
    putchar(Bitpack_getu(code_word, 8, 0));
}

/* static void decompress_apply(int col, int row, 
                                A2Methods_UArray2 arr, 
							    void *elem, 
							    void *cl)              
 * Parameters: int col - column index of current pixel                                        
 *             int row - row index of current pixel
 * 			   A2Methods_UArray2 arr - UArray2 of pixels being mapped on and
 *                                     filled from decompressing
 * 			   void *elem - current pixel (void)
 * 			   void *cl - compressed image file being decompressed 
 *    Returns: None
 *       Does: For each top-left corner pixel of 2x2 blocks, reads and unpacks 
 * 			   the next code word from the inputted compressed image, stores
 * 			   the decompressed RGB values into the inputted UArray2 of pixels
 *   if Error: No errors to be handled
 */  
static void decompress_apply(int col, int row, 
                             A2Methods_UArray2 arr, 
							 void *elem, 
							 void *cl)
{
	(void) elem;

	/* Create local variables */
	FILE *file = (FILE *) cl;

	/* Apply on top-left corners of 2x2 pixel blocks */
	if (col % 2 == 0 && row % 2 == 0) {

		/* Get info from code word */
		code_word_info info = unpack_code_word(file);

		float a, b, c, d;

		/* Unquantize DCTs */
		a = info.a / (float) A_DCT_SCALE;
		b = unquantize_dct(info.b);
		c = unquantize_dct(info.c);
		d = unquantize_dct(info.d);

		component_video one, two, three, four;

		/* Get component-video for all four pixels from code word info */
		one.y = a - b - c + d;
		two.y = a - b + c - d;
		three.y = a + b - c - d;
		four.y = a + b + c + d;

		one.pb = two.pb = three.pb = four.pb 
		                           = Arith40_chroma_of_index(info.pb_index);
		one.pr = two.pr = three.pr = four.pr 
		                           = Arith40_chroma_of_index(info.pr_index);

		/* Convert component-video to RGB and insert into Pnm_ppm pixel map */
		*(Pnm_rgb) METHODS->at(arr, col, row) 
		                      = component_video_to_rgb(one);
		*(Pnm_rgb) METHODS->at(arr, col + 1, row) 
		                      = component_video_to_rgb(two);
		*(Pnm_rgb) METHODS->at(arr, col, row + 1) 
		                      = component_video_to_rgb(three);
		*(Pnm_rgb) METHODS->at(arr, col + 1, row + 1) 
		                      = component_video_to_rgb(four);
	}
}

/* static code_word_info unpack_code_word(FILE *file)
 * Parameters: FILE *file - compressed image file to read code words from
 *    Returns: code_word_info - struct of values unpacked from code word
 *       Does: Gets code word from read_code_word() and utilizes bitpack to 
 * 			   unpack the values from the code word (a, b, c, d, Pb, Pr)
 *   if Error: Raises assertion is file is null
 */  
static code_word_info unpack_code_word(FILE *file)
{
	assert(file != NULL);

	/* Get code word by reading from file */
	uint64_t code_word = read_code_word(file);

	code_word_info info;

	/* Unpack each value from the retrieved code word */
	info.pr_index = Bitpack_getu(code_word, PB_PR_WIDTH, PR_LSB);
	info.pb_index = Bitpack_getu(code_word, PB_PR_WIDTH, PB_LSB);
	info.d = Bitpack_gets(code_word, B_C_D_WIDTH, D_LSB);
	info.c = Bitpack_gets(code_word, B_C_D_WIDTH, C_LSB);
	info.b = Bitpack_gets(code_word, B_C_D_WIDTH, B_LSB);
	info.a = Bitpack_getu(code_word, A_WIDTH, A_LSB);

	return info;
}

/* static uint64_t read_code_word(FILE *file)
 * Parameters: FILE *file - compressed image file to read code words from
 *    Returns: uint64_t - code word read from inputted compressed image file
 *       Does: Utilizes bitpack to read the next code word
 *   if Error: Raises assertion is file is null
 */  
static uint64_t read_code_word(FILE *file)
{
	assert(file != NULL);

	uint64_t code_word = 0;

	code_word = Bitpack_newu(code_word, 8, 24, getc(file));
	code_word = Bitpack_newu(code_word, 8, 16, getc(file));
	code_word = Bitpack_newu(code_word, 8, 8, getc(file));
	code_word = Bitpack_newu(code_word, 8, 0, getc(file));

	return code_word;
}

/* static float unquantize_dct(float quantized_dct)
 * Parameters: float quantized_dct - quantized DCT read from code word
 *    Returns: float - unquantized DCT
 *       Does: Unquantizes DCT by dividing it by DCT scale
 *   if Error: No errors to be handled
 */  
static float unquantize_dct(float quantized_dct)
{
	return quantized_dct / (float) B_C_D_DCT_SCALE;
}

/* static struct Pnm_rgb component_video_to_rgb(component_video cv)
 * Parameters: component_video cv - struct of component-video values of a pixel
 *    Returns: struct Pnm_rgb - struct of RGB values for inputted pixel
 *       Does: Converts inputted component-video values (Y, Pb, Pr) to RGB
 * 			   values
 *   if Error: No errors to be handled
 */  
static struct Pnm_rgb component_video_to_rgb(component_video cv)
{
    struct Pnm_rgb new_rgb;

	/* Calculate RGB values from component-video values */
	float red = 1.0 * cv.y + 1.402 * cv.pr;
    float green = 1.0 * cv.y - 0.344136 * cv.pb - 0.714136 * cv.pr;
    float blue = 1.0 * cv.y + 1.772 * cv.pb;

	/* Force RGB values that are beyond (-1, 1) interval into the interval */
    new_rgb.red = fit_in_interval(red) * DENOMINATOR;
    new_rgb.green = fit_in_interval(green) * DENOMINATOR;
    new_rgb.blue = fit_in_interval(blue) * DENOMINATOR;

    return new_rgb;
}

/* static float fit_in_interval(float value)
 * Parameters: float value - RGB value to be forced into (0, 1) range
 *    Returns: float - RGB value after being forced into (0, 1) range
 *       Does: Fits RGB value into (0, 1) range if it is < 0 or > 1
 *   if Error: No errors to be handled
 */  
static float fit_in_interval(float value)
{
	return (value > 1) ? 1 : (value < 0) ? 0 : value;
}