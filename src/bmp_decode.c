#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "himem.h"
#include "bmp_decode.h"

//
//  init BMP decode handler
//
int32_t bmp_decode_open(BMP_DECODE_HANDLE* bmp, int16_t dither) {

  int32_t rc = -1;

  bmp->width = 0;
  bmp->height = 0;
  bmp->brightness = 100;
  bmp->dither = dither;

  bmp->rgb555_r = (uint16_t*)himem_malloc(sizeof(uint16_t) * 256, 0);
  bmp->rgb555_g = (uint16_t*)himem_malloc(sizeof(uint16_t) * 256, 0);
  bmp->rgb555_b = (uint16_t*)himem_malloc(sizeof(uint16_t) * 256, 0);

  if (bmp->rgb555_r == NULL || bmp->rgb555_g == NULL || bmp->rgb555_b == NULL) goto exit;

  bmp->rgb555_e1 = (int8_t*)himem_malloc(sizeof(int8_t) * 256, 0);
  bmp->rgb555_e3 = (int8_t*)himem_malloc(sizeof(int8_t) * 256, 0);
  bmp->rgb555_e5 = (int8_t*)himem_malloc(sizeof(int8_t) * 256, 0);
  bmp->rgb555_e7 = (int8_t*)himem_malloc(sizeof(int8_t) * 256, 0);
  if (bmp->rgb555_e1 == NULL || bmp->rgb555_e3 == NULL || bmp->rgb555_e5 == NULL || bmp->rgb555_e7 == NULL) goto exit;

  for (int16_t i = 0; i < 256; i++) {
    uint32_t c = (uint32_t)(i * 32 * bmp->brightness / 100) >> 8;
    bmp->rgb555_r[i] = (uint16_t)(c <<  6);
    bmp->rgb555_g[i] = (uint16_t)(c << 11);
    bmp->rgb555_b[i] = (uint16_t)(c <<  1);

    bmp->rgb555_e1[i] = ((i & 0xf8) - i) * 1 / 16;
    bmp->rgb555_e3[i] = ((i & 0xf8) - i) * 3 / 16;
    bmp->rgb555_e5[i] = ((i & 0xf8) - i) * 5 / 16;
    bmp->rgb555_e7[i] = ((i & 0xf8) - i) * 7 / 16;
  }

  rc = 0;

exit:
  return rc;
}

//
//  close BMP decode handler
//
void bmp_decode_close(BMP_DECODE_HANDLE* bmp) {
  if (bmp->rgb555_r != NULL) {
    himem_free(bmp->rgb555_r, 0);
    bmp->rgb555_r = NULL;
  }
  if (bmp->rgb555_g != NULL) {
    himem_free(bmp->rgb555_g, 0);
    bmp->rgb555_g = NULL;
  }
  if (bmp->rgb555_b != NULL) {
    himem_free(bmp->rgb555_b, 0);
    bmp->rgb555_b = NULL;
  }

  if (bmp->rgb555_e1 != NULL) {
    himem_free(bmp->rgb555_e1, 0);
    bmp->rgb555_e1 = NULL;
  }
  if (bmp->rgb555_e3 != NULL) {
    himem_free(bmp->rgb555_e3, 0);
    bmp->rgb555_e3 = NULL;
  }
  if (bmp->rgb555_e5 != NULL) {
    himem_free(bmp->rgb555_e5, 0);
    bmp->rgb555_e5 = NULL;
  }
  if (bmp->rgb555_e7 != NULL) {
    himem_free(bmp->rgb555_e7, 0);
    bmp->rgb555_e7 = NULL;
  }
}

//
//  decode
//
int32_t bmp_decode_exec(BMP_DECODE_HANDLE* bmp, uint8_t* bmp_file_name, uint16_t* decode_buffer, size_t decode_buffer_len, size_t* decoded_len) {

  int32_t rc = -1;
  FILE* fp = NULL;
  uint8_t* bmp_buffer = NULL;

  *decoded_len = 0;

  // open bmp file
  fp = fopen(bmp_file_name, "rb");
  if (fp == NULL) {
    rc = -1;
    goto exit;
  }

  // get bmp file size
  fseek(fp, 0, SEEK_END);
  size_t bmp_file_len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // allocate memory buffer
  bmp_buffer = himem_malloc(bmp_file_len, 0);
  if (bmp_buffer == NULL) {
    rc = -2;
    goto exit;
  }

  // read bmp data
  size_t read_len = fread(bmp_buffer, 1, bmp_file_len, fp);
  if (read_len != bmp_file_len) {
    rc = -3;
    goto exit;
  }

  // close bmp file
  fclose(fp);
  fp = NULL;

  // bmp header check
  if (bmp_buffer[0] != 0x42 || bmp_buffer[1] != 0x4d) {
    rc = -4;
    goto exit;
  }

  // bmp width and height
  int32_t bmp_width  = bmp_buffer[18] + (bmp_buffer[19] << 8) + (bmp_buffer[20] << 16) + (bmp_buffer[21] << 24);
  int32_t bmp_height = bmp_buffer[22] + (bmp_buffer[23] << 8) + (bmp_buffer[24] << 16) + (bmp_buffer[25] << 24);
  bmp->width = bmp_width;
  bmp->height = bmp_height;
  if (bmp_width != 256 && bmp_width != 384 && bmp_width != 512) {
    rc = -5;
    goto exit;
  }
  if (bmp_height > 256) {
    rc = -6;
    goto exit;
  }
  if (bmp_width * bmp_height > decode_buffer_len) {
    rc = -7;
    goto exit;
  }

  // bmp bit depth
  int32_t bmp_bit_depth = bmp_buffer[28] + (bmp_buffer[29] << 8);
  if (bmp_bit_depth != 24) {
    rc = -8;
    goto exit;
  }

  // bmp bitmap data pointer
  uint8_t* bmp_bitmap = bmp_buffer + 54;
  size_t padding = (4 - ((bmp_width * 3) % 4)) % 4;

  if (bmp->dither) {

    for (int32_t y = bmp_height - 1; y >= 0; y--) {
    
      uint16_t* vvram = decode_buffer + y * 512;

      for (int32_t x = 0; x < bmp_width; x++) {
        uint8_t b = *bmp_bitmap++;
        uint8_t g = *bmp_bitmap++;
        uint8_t r = *bmp_bitmap++;
        uint16_t c = bmp->rgb555_g[ g ] | bmp->rgb555_r[ r ] | bmp->rgb555_b[ b ];
        vvram[ x ] = (c == 0) ? 0 : c + 1;

        if (x < bmp_width - 1) {
          int16_t b2 = bmp_bitmap[0] + bmp->rgb555_e7[b];
          int16_t g2 = bmp_bitmap[1] + bmp->rgb555_e7[g];
          int16_t r2 = bmp_bitmap[2] + bmp->rgb555_e7[r];
          if (b2 < 0) b2 = 0;
          if (g2 < 0) g2 = 0;
          if (r2 < 0) r2 = 0;
          if (b2 > 255) b2 = 255;
          if (g2 > 255) g2 = 255;
          if (r2 > 255) r2 = 255;
          bmp_bitmap[0] = b2;
          bmp_bitmap[1] = g2;
          bmp_bitmap[2] = r2;
        }

        if (y > 0) {
          int16_t b2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 0 ] + bmp->rgb555_e5[b];
          int16_t g2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 1 ] + bmp->rgb555_e5[g];
          int16_t r2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 2 ] + bmp->rgb555_e5[r];
          if (b2 < 0) b2 = 0;
          if (g2 < 0) g2 = 0;
          if (r2 < 0) r2 = 0;
          if (b2 > 255) b2 = 255;
          if (g2 > 255) g2 = 255;
          if (r2 > 255) r2 = 255;
          bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 0 ] = b2;
          bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 1 ] = g2;
          bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 2 ] = r2;

          if (x > 0) {
            int16_t b2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 - 3 ] + bmp->rgb555_e3[b];
            int16_t g2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 - 2 ] + bmp->rgb555_e3[g];
            int16_t r2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 - 1 ] + bmp->rgb555_e3[r];
            if (b2 < 0) b2 = 0;
            if (g2 < 0) g2 = 0;
            if (r2 < 0) r2 = 0;
            if (b2 > 255) b2 = 255;
            if (g2 > 255) g2 = 255;
            if (r2 > 255) r2 = 255;
            bmp_bitmap[ bmp_width * 3 + padding + x * 3 - 3 ] = b2;
            bmp_bitmap[ bmp_width * 3 + padding + x * 3 - 2 ] = g2;
            bmp_bitmap[ bmp_width * 3 + padding + x * 3 - 1 ] = r2;
          }
          if (x < bmp_width - 1) {
            int16_t b2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 3 ] + bmp->rgb555_e1[b];
            int16_t g2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 4 ] + bmp->rgb555_e1[g];
            int16_t r2 = bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 5 ] + bmp->rgb555_e1[r];
            if (b2 < 0) b2 = 0;
            if (g2 < 0) g2 = 0;
            if (r2 < 0) r2 = 0;
            if (b2 > 255) b2 = 255;
            if (g2 > 255) g2 = 255;
            if (r2 > 255) r2 = 255;
            bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 3 ] = b2;
            bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 4 ] = g2;
            bmp_bitmap[ bmp_width * 3 + padding + x * 3 + 5 ] = r2;
          }
        }

      }

      bmp_bitmap += padding;
      *decoded_len = *decoded_len + bmp_width;

    }

  } else {

    for (int32_t y = bmp_height - 1; y >= 0; y--) {
    
      uint16_t* vvram = decode_buffer + y * 512;

      for (int32_t x = 0; x < bmp_width; x++) {
        uint8_t b = *bmp_bitmap++;
        uint8_t g = *bmp_bitmap++;
        uint8_t r = *bmp_bitmap++;
        uint16_t c = bmp->rgb555_g[ g ] | bmp->rgb555_r[ r ] | bmp->rgb555_b[ b ];
        vvram[ x ] = (c == 0) ? 0 : c + 1;
      }

      bmp_bitmap += padding;
      *decoded_len = *decoded_len + bmp_width;
    }

  }

  rc = 0;

exit:

  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }

  if (bmp_buffer != NULL) {
    himem_free(bmp_buffer, 0);
    bmp_buffer = NULL;
  }

  return rc;
}