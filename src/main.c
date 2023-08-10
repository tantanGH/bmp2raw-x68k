// base
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// dos/iocs call
#include <doslib.h>
#include <iocslib.h>

// devices
#include "keyboard.h"
#include "himem.h"

// decoder/encoder
#include "bmp_decode.h"
#include "raw_encode.h"

// application
#include "cp932rsc.h"
#include "bmp2raw.h"

// bmp file names
uint8_t* bmp_file_names = NULL;

// frame buffer
uint16_t* frame_buffer = NULL;

// bmp decoder
BMP_DECODE_HANDLE bmp = { 0 };

// raw encoder
RAW_ENCODE_HANDLE raw = { 0 };

// abort vector handler
static void abort_application() {
  
  // close bmp decoder
  bmp_decode_close(&bmp);

  // close raw encoder
  raw_encode_close(&raw);

  // reclaim frame buffer memory
  if (frame_buffer != NULL) {
    himem_free(frame_buffer, 0);
    frame_buffer = NULL;
  }

  // reclaim bmp file name buffer memory
  if (bmp_file_names != NULL) {
    himem_free(bmp_file_names, 0);
    bmp_file_names = NULL;
  }

  // flush key buffer
  KFLUSHIO(0xff);

  // cursor on
  C_CURON();

  // abort message
  printf(cp932rsc_aborted);
  printf("\n");

  // abort exit
  EXIT2(1);
}

// quick sort comparator
static int compare_bmp_file_names(const void* n1, const void* n2) {
  return stricmp((uint8_t*)n1, (uint8_t*)n2);
}

//
//  show help message
//
static void show_help_message() {
  printf("usage: bmp2raw <bmp-files-dir> <output-raw-name>\n");
}

//
//  main
//
int32_t main(int32_t argc, uint8_t* argv[]) {

  // default return code
  int32_t rc = -1;

  // error message
  uint8_t error_message[ MAX_MES_LEN ];
  error_message[0] = '\0';

  // credit
  printf("BMP2RAW.X - BMP files to RAW movie file converter version " PROGRAM_VERSION " by tantan\n");

  // argument check
  if (argc < 3) {
    show_help_message();
    goto exit;
  }

  uint8_t* bmp_dir_name = argv[1];
  uint8_t* raw_file_name = argv[2];

  // set abort vectors
  uint32_t abort_vector1 = INTVCS(0xFFF1, (int8_t*)abort_application);
  uint32_t abort_vector2 = INTVCS(0xFFF2, (int8_t*)abort_application);  

  // check output file existence
  FILE* fp = fopen(raw_file_name, "rb");
  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
    printf(cp932rsc_output_file_exists);
    int32_t ik = INKEY();
    printf("\n");
    if (ik != 'y' && ik != 'Y') {
      printf(cp932rsc_aborted);
      printf("\n");
      goto exit;
    }
  }

  C_CUROFF();

  // open BMP folder
  uint8_t bmp_wild_name[ MAX_PATH_LEN ];
  strcpy(bmp_wild_name, bmp_dir_name);
  uint8_t* c = bmp_wild_name + strlen(bmp_wild_name) - 1;
  if (*c == '\\' || *c == '/') *c = '\0';

  // pass1: count BMP files
  struct FILBUF filbuf;
  if (FILES(&filbuf, bmp_wild_name, 0x10) < 0) {
    strcpy(error_message, cp932rsc_bmp_dir_not_available);
    goto exit;
  }

  strcat(bmp_wild_name, "\\*.BMP");
  if (FILES(&filbuf, bmp_wild_name, 0x20) < 0) {
    strcpy(error_message, cp932rsc_bmp_file_not_available);
    goto exit;
  }

  size_t num_bmp_files = 0;

  do {
    num_bmp_files++;  
  } while (NFILES(&filbuf) >= 0);

  printf(cp932rsc_bmp_files_found, num_bmp_files, bmp_dir_name);
  printf("\n");

  // allocate BMP file name buffer
  bmp_file_names = himem_malloc(BMP_NAME_SIZE * num_bmp_files, 0);
  memset(bmp_file_names, 0, BMP_NAME_SIZE * num_bmp_files);

  // pass2: read BMP file names into buffer 
  if (FILES(&filbuf, bmp_wild_name, 0x20) < 0) {
    strcpy(error_message, cp932rsc_bmp_file_not_available);
    goto exit;
  }

  size_t i = 0;

  do {
    strcpy(bmp_file_names + i * 24, filbuf.name);
    i++;
  } while (NFILES(&filbuf) >= 0);

  // sort BMP file names in dictionary order
  qsort(bmp_file_names, num_bmp_files, 24, &compare_bmp_file_names);
  printf(cp932rsc_bmp_files_sorted);
  printf("\n");

  // allocate frame buffer memory
  frame_buffer = himem_malloc(FRAME_BUFFER_BYTES, 0);
  if (frame_buffer == NULL) {
    strcpy(error_message, cp932rsc_out_of_memory);
    goto exit;
  }
  memset(frame_buffer, 0, FRAME_BUFFER_BYTES);

  // open BMP decoder handle
  bmp_decode_open(&bmp);

  strcpy(bmp_wild_name, bmp_dir_name);
  c = bmp_wild_name + strlen(bmp_wild_name) - 1;
  if (*c == '\\' || *c == '/') *c = '\0';
  strcat(bmp_wild_name, "\\");

  printf(cp932rsc_start_process);
  printf("\n");

  // decode BMP and add to raw movie file
  for (size_t i = 0; i < num_bmp_files; i++) {
    uint8_t bmp_path_name[ MAX_PATH_LEN ];
    strcpy(bmp_path_name, bmp_wild_name);
    strcat(bmp_path_name, bmp_file_names + 24 * i);
    printf("\r%d/%d", i + 1, num_bmp_files);
    size_t written_len;
    if (bmp_decode_exec(&bmp, bmp_path_name, frame_buffer, FRAME_BUFFER_LEN, &written_len) < 0) {
      strcpy(error_message, cp932rsc_bmp_file_decode_error);
      goto exit;      
    }
    int32_t rc_raw = 0;
    if (i == 0) {
      // open RAW encoder handle
      rc_raw = raw_encode_open(&raw, raw_file_name, bmp.width, bmp.height);
      if (rc_raw < 0) {
        sprintf(error_message, cp932rsc_raw_file_open_error, rc_raw);
        goto exit;
      }
    }
    rc_raw = raw_encode_add_frame(&raw, frame_buffer, written_len);
    if (rc_raw == -2) {
      strcpy(error_message, cp932rsc_bmp_size_error);
      goto exit;
    } else if(rc_raw < 0) {
      sprintf(error_message, cp932rsc_raw_file_output_error, rc_raw);
      goto exit;
    }
    if (B_KEYSNS() != 0) {
      int16_t scan_code = B_KEYINP() >> 8;
      if (scan_code == KEY_SCAN_CODE_ESC) {
        printf("\n");
        printf(cp932rsc_aborted);
        printf("\n");
        goto exit;
      }
    }
    //printf("%s\n", bmp_file_names + 24 * i);
  }

  printf("\n");

  rc = 0;

exit:

  // close bmp decoder
  bmp_decode_close(&bmp);

  // close raw encoder
  raw_encode_close(&raw);

  // reclaim frame buffer memory
  if (frame_buffer != NULL) {
    himem_free(frame_buffer, 0);
    frame_buffer = NULL;
  }

  // reclaim bmp file name buffer memory
  if (bmp_file_names != NULL) {
    himem_free(bmp_file_names, 0);
    bmp_file_names = NULL;
  }

  // show error message if exists
  if (error_message[0] != '\0') {
    printf("error: %s\n", error_message);
  }

  // cursor on
  C_CURON();

  // resume abort vectors
  INTVCS(0xFFF1, (int8_t*)abort_vector1);
  INTVCS(0xFFF2, (int8_t*)abort_vector2);  

  return rc;
}