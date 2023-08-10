#ifndef __H_RAW_ENCODE__
#define __H_RAW_ENCODE__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
  uint8_t* file_name;
  uint16_t width;
  uint16_t height;
  uint32_t num_frames;
  FILE* fp;
  uint16_t* raw_buffer;
} RAW_ENCODE_HANDLE;

int32_t raw_encode_open(RAW_ENCODE_HANDLE* raw, uint8_t* file_name, uint16_t width, uint16_t height);
int32_t raw_encode_close(RAW_ENCODE_HANDLE* raw);
int32_t raw_encode_add_frame(RAW_ENCODE_HANDLE* raw, uint16_t* frame_buffer, size_t frame_buffer_len);

#endif