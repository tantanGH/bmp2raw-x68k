#ifndef __H_BMP_DECODE__
#define __H_BMP_DECODE__

#include <stdint.h>
#include <stddef.h>

typedef struct {
  int32_t width;
  int32_t height;
  uint16_t* rgb555_r;
  uint16_t* rgb555_g;
  uint16_t* rgb555_b;
} BMP_DECODE_HANDLE;

int32_t bmp_decode_open(BMP_DECODE_HANDLE* bmp);
void bmp_decode_close(BMP_DECODE_HANDLE* bmp);
int32_t bmp_decode_exec(BMP_DECODE_HANDLE* bmp, uint8_t* bmp_file_name, uint16_t* decode_buffer, size_t decode_buffer_len, size_t* decoded_len);

#endif