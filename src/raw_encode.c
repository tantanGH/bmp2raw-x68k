#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <doslib.h>
#include <iocslib.h>
#include "himem.h"
#include "raw_encode.h"

//
//  init RAW encode handler
//
int32_t raw_encode_open(RAW_ENCODE_HANDLE* raw, uint8_t* file_name, uint16_t width, uint16_t height) {

  int32_t rc = -1;

  // initialize attributes
  raw->file_name = file_name;
  raw->width = width;
  raw->height = height;
  raw->fp = NULL;
  raw->raw_buffer = NULL;

  // check width
  if (raw->width != 256 && raw->width != 384 && raw->width != 512) {
    rc = -1;
    goto exit;
  }

  // check height
  if (raw->height < 1 || raw->height > 256 ) {
    rc = -2;
    goto exit;
  }

  // allocate raw data buffer (512 * 256 words)
  raw->raw_buffer = himem_malloc(512 * 256 * 2, 0);
  if (raw->raw_buffer == NULL) {
    rc = -3;
    goto exit;
  }

  // open output raw file in binary write mode
  raw->fp = fopen(raw->file_name, "wb");
  if (raw->fp == NULL) {
    rc = -4;
    goto exit;
  }

  rc = 0;

exit:
  return rc;
}

//
//  close RAW encode handler
//
int32_t raw_encode_close(RAW_ENCODE_HANDLE* raw) {

  int32_t rc = -1;

  // if unwritten data are in the raw data buffer, flush it in 256 width mode
  if (raw->width == 256 && (raw->num_frames % 2) == 1) {

    uint8_t* dst = (uint8_t*)(raw->raw_buffer + 256);
    uint16_t pitch = raw->width * 2;

    for (int16_t i = 0; i < raw->height; i++) {
      memset(dst, 0, pitch);
      dst += 512 * 2;
    }

    size_t len = fwrite(raw->raw_buffer, 2, 512 * raw->height, raw->fp);
    if (len != 512 * raw->height) {
      goto exit;
    }

    raw->num_frames++;

  }

  rc = 0;

exit:

  // close output file handle
  if (raw->fp != NULL) {
    fclose(raw->fp);
    raw->fp = NULL;
  }

  // reclaim raw data buffer memory
  if (raw->raw_buffer != NULL) {
    himem_free(raw->raw_buffer, 0);
    raw->raw_buffer = NULL;
  }

  return rc;
}

//
//  add a frame
//
int32_t raw_encode_add_frame(RAW_ENCODE_HANDLE* raw, uint16_t* frame_buffer, size_t frame_buffer_len) {

  int32_t rc = -1;

  // check input data size consistency
  if (frame_buffer_len != raw->width * raw->height) {
    rc = -2;
    goto exit;
  }

  if (raw->width == 256) {

    // 256 width mode ... 2 frames / 1 raw data buffer
  
    // even frame ... left side / odd frame ... right side
    uint8_t* dst = (raw->num_frames % 2) == 0 ? (uint8_t*)(raw->raw_buffer) : (uint8_t*)(raw->raw_buffer + 256);
    uint8_t* src = (uint8_t*)frame_buffer;
    size_t pitch = raw->width * 2;

    for (int16_t i = 0; i < raw->height; i++) {
      memcpy(dst, src, pitch);
      dst += 512 * 2;
      src += 512 * 2;
    }

    // write to disk once raw data buffer is full
    if ((raw->num_frames % 2) == 1) {
      size_t len = fwrite(raw->raw_buffer, 2, 512 * raw->height, raw->fp);
      if (len != 512 * raw->height) {
        rc = -1;
        goto exit;
      }
    }

    raw->num_frames++;

  } else {

    // 384/512 width mode ... 1 frame / 1 raw data buffer

    uint8_t* dst = (uint8_t*)raw->raw_buffer;
    uint8_t* src = (uint8_t*)frame_buffer;
    size_t pitch = raw->width * 2;

    for (int16_t i = 0; i < raw->height; i++) {
      memcpy(dst, src, pitch);
      dst += 512 * 2;
      src += 512 * 2;
    }

    // write to disk at every frame
    size_t len = fwrite(raw->raw_buffer, 2, 512 * raw->height, raw->fp);
    if (len != 512 * raw->height) {
      rc = -1;
      goto exit;
    }

    raw->num_frames++;

  }

  rc = 0;

exit:

  return rc;
}