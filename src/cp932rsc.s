  .align 2

  .globl _cp932rsc_output_file_exists
  .globl _cp932rsc_fps_convert
  .globl _cp932rsc_dither
  .globl _cp932rsc_start_process
  .globl _cp932rsc_bmp_dir_not_available
  .globl _cp932rsc_bmp_file_not_available
  .globl _cp932rsc_bmp_files_found
  .globl _cp932rsc_bmp_files_sorting
  .globl _cp932rsc_bmp_files_sorted
  .globl _cp932rsc_bmp_file_decode_error
  .globl _cp932rsc_bmp_size_error
  .globl _cp932rsc_raw_file_open_error
  .globl _cp932rsc_raw_file_output_error
  .globl _cp932rsc_out_of_memory
  .globl _cp932rsc_aborted
  .globl _cp932rsc_completed

  .data

_cp932rsc_output_file_exists:
  .dc.b '出力先のファイルが存在します。上書きしますか？(Y/N)',$00

_cp932rsc_fps_convert:
  .dc.b '%.3f fps から %.3f fps に変換します。',$00

_cp932rsc_dither:
  .dc.b 'ディザリングを行います。',$00

_cp932rsc_start_process:
  .dc.b '処理を開始します。ESCキーで中断できます。',$00

_cp932rsc_bmp_dir_not_available:
  .dc.b 'BMPファイルディレクトリが見つかりません。',$00

_cp932rsc_bmp_file_not_available:
  .dc.b 'BMPファイルが見つかりません。',$00

_cp932rsc_bmp_files_found:
  .dc.b '%d 件のBMPファイルが %s ディレクトリに見つかりました。',$00

_cp932rsc_bmp_files_sorting:
  .dc.b 'BMPファイル名をソートしています。',$00

_cp932rsc_bmp_files_sorted:
  .dc.b 'BMPファイル名のソートが完了しました。',$00

_cp932rsc_bmp_file_decode_error:
  .dc.b 'BMPファイルのデコードに失敗しました。(%d,%s)',$00

_cp932rsc_bmp_size_error:
  .dc.b 'BMP画像の大きさが最初のフレームと違います。(%s)',$00

_cp932rsc_raw_file_open_error:
  .dc.b 'RAWファイルを書き込みオープンできませんでした。(%d,%s)',$00

_cp932rsc_raw_file_output_error:
  .dc.b 'RAWファイルの出力に失敗しました。(%d,%s)',$00

_cp932rsc_out_of_memory:
  .dc.b 'メモリが足りません。',$00

_cp932rsc_aborted:
  .dc.b '処理を中断しました。(出力画像サイズ:%dx%d 出力フレーム数:%d)',$00

_cp932rsc_completed:
  .dc.b '正常終了しました。(出力画像サイズ:%dx%d 出力フレーム数:%d)',$00

  .end