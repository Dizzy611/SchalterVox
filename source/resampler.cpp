#include <samplerate.h>
#include <string>
#include <cstring>
#include <malloc.h>
#include <switch.h>
#include "resampler.hpp"

static bool resampler_initialized = false;
static SRC_STATE* resampler_state;
static SRC_DATA resampler_data;
static float* resampler_in;
static float* resampler_out;
static int resampler_error;

void convert_s16_to_float(float *out,
      const int16_t *in, size_t samples);
void convert_float_to_s16(int16_t *out,
      const float *in, size_t samples);

	  
using namespace std;

int do_resample(s16* input_buffer, s16* output_buffer, int input_samplerate, int in_size, int out_size) {
	if (resampler_initialized == false) {
		resampler_state = src_new(SRC_LINEAR, 2, &resampler_error);
		resampler_initialized = true;
	}
	resampler_in = (float*)malloc((in_size/2) * sizeof(float));
	resampler_out = (float*)malloc((out_size/2) * sizeof(float));
	
	convert_s16_to_float(resampler_in, input_buffer, in_size/2);
	
	resampler_data.data_in = resampler_in;
	resampler_data.data_out = resampler_out;
	resampler_data.output_frames = out_size/4;
	resampler_data.input_frames = in_size/4;
	resampler_data.end_of_input = 0;
	resampler_data.src_ratio = (float)(48000)/(input_samplerate*1.0);

	int retval = src_process(resampler_state, &resampler_data);

	free(resampler_in);
	free(resampler_out);	
	
	convert_float_to_s16(output_buffer, resampler_out, (out_size/2));
	return retval;
}

string resampler_get_error(int error) {
	return src_strerror(error);
}


/* Below Copyright  (C) 2010-2018 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to the below code.
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * convert_s16_to_float:
 * @out               : output buffer
 * @in                : input buffer
 * @samples           : size of samples to be converted
 * @gain              : gain applied (.e.g. audio volume)
 *
 * Converts from signed integer 16-bit
 * to floating point.
 **/
void convert_s16_to_float(float *out,
      const int16_t *in, size_t samples)
{
   size_t i      = 0;

   for (; i < samples; i++)
      out[i] = (float)in[i];
}

/**
 * convert_float_to_s16:
 * @out               : output buffer
 * @in                : input buffer
 * @samples           : size of samples to be converted
 *
 * Converts floating point
 * to signed integer 16-bit.
 *
 * C implementation callback function.
 **/
void convert_float_to_s16(int16_t *out,
      const float *in, size_t samples)
{
   size_t i      = 0;

   for (; i < samples; i++)
   {
      int32_t val = (int32_t)(in[i] * 0x8000);
      out[i]      = (val > 0x7FFF) ? 0x7FFF :
         (val < -0x8000 ? -0x8000 : (int16_t)val);
   }
}

