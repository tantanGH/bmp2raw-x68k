# BMP2RAW.X

BMP files to RAW movie file converter for Human68k/X680x0

指定したディレクトリの中にある連番BMPファイル群を無圧縮 X680x0 GVRAM形式に変換して連結し、[RAWMVP.X](https://github.com/tantanGH/rawmvp/) で再生できるファイルを生成します。

---

## インストール方法

BMP2Rxxx.ZIP をダウンロードして展開し、BMP2RAW.X をパスの通ったディレクトリに置きます。

---

## 使用方法

    bmp2raw [option] <BMPファイルディレクトリ名> <出力RAWファイル名>
    options:
      -r<source-fps>:<target-fps> ... 連番BMPのフレームレート:出力RAWファイルのフレームレート
      -d ... dithering
      -h ... show help message
    target fps:
      w256/512 ... 27.729(30) / 22.183(24) / 18.486(20) / 13.865(15) / 11.092(12) / 9.243(10)
          w384 ... 28.136(30) / 22.509(24) / 18.757(20) / 14.068(15) / 11.254(12) / 9.379(10)

BMPファイルは横幅256,384,512pxのいずれか、縦幅は256px以下、24bitカラーのみサポートしています。

ファイル名の辞書順にソートされて連結されていきます。ヘッダなどは特になく単なるベタファイルのベタ連結です。

注意：

* Human68k の扱えるファイルサイズ上限は約2GBです。256x256の画像サイズの場合約16000フレーム、384x256/512x256の場合約8000フレームが理論上限となります。
* 出力ファイルは圧縮が掛かっていませんのでディスクの空き容量に注意してください。24bit -> 16bit の色変換が行われるため、目安として元のBMPファイルの総サイズ * 0.67 くらいの大きさのファイルが出力されることになります。

### フレームレート変換

フレームレート変換はオプションです。アップサンプリング・ダウンサンプリングの両方に対応しています。

例：29.97fps で出力された連番BMPを 27.729fps でRAW出力したい場合

    -r29.97:27.729

指定の無い場合はすべてのBMPファイルからの画像を単純に連結します。

### マッハバンド対策

減色は8bitから5bitへ単純に下位3bit落としますが、グラデーション部分でのバンディング(マッハバンド)が気になる場合があります。

`-d` オプションをつけると誤差拡散を行い多少緩和します。ただし少しノイジーになります。お好みでどうぞ。

---

## Special Thanks

品質向上のための貴重な動作報告と動作確認にご協力頂きました。この場を借りてお礼申し上げます。

- ネコサダさん
- カタさん

---

## History

* 0.4.1 (2023/08/15) ... ヘルプメッセージ内のfps値表示の追加
* 0.4.0 (2023/08/12) ... 処理終了時にBMP画像サイズと出力フレーム数を表示するようにした、コード整理、ドキュメント更新
* 0.3.6 (2023/08/12) ... ファイル名ソートの再実装
* 0.3.5 (2023/08/11) ... メモリチェック、エラーメッセージ強化、-rオプションの後ろのスペースが不要だったのをdoc修正
* 0.3.0 (2023/08/11) ... ディザリングに対応した
* 0.2.0 (2023/08/11) ... fps変換に対応した
* 0.1.0 (2023/08/10) ... 初版
