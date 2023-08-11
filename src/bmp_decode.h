#ifndef __H_BMP_DECODE__
#define __H_BMP_DECODE__

#include <stdint.h>
#include <stddef.h>

typedef struct {
  int32_t width;
  int32_t height;
  int16_t brightness;
  int16_t dither;
  uint16_t* rgb555_r;
  uint16_t* rgb555_g;
  uint16_t* rgb555_b;
//  uint16_t* rgb555_r2;
//  uint16_t* rgb555_g2;
//  uint16_t* rgb555_b2;
  int8_t* rgb555_e1;
  int8_t* rgb555_e3;
  int8_t* rgb555_e5;
  int8_t* rgb555_e7;
} BMP_DECODE_HANDLE;

int32_t bmp_decode_open(BMP_DECODE_HANDLE* bmp, int16_t dither);
void bmp_decode_close(BMP_DECODE_HANDLE* bmp);
int32_t bmp_decode_exec(BMP_DECODE_HANDLE* bmp, uint8_t* bmp_file_name, uint16_t* decode_buffer, size_t decode_buffer_len, size_t* decoded_len);

#endif