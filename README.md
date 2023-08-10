# BMP2RAW.X

BMP files to RAW movie file converter for Human68k/X680x0

指定したディレクトリの中にあるBMPファイルを無圧縮X680x0GVRAM形式に変換して連結し、[RAWMVP.X](https://github.com/tantanGH/rawmvp/)で再生できるファイルを生成します。

---

### インストール方法

BMP2Rxxx.ZIP をダウンロードして展開し、BMP2RAW.X をパスの通ったディレクトリに置きます。

---

### 使用方法

    bmp2raw <BMPファイルディレクトリ名> <出力RAWファイル名>

オプションなどは特にありません。

BMPファイルは横幅は256,384,512pxのいずれか、縦幅は256px以下、24bitカラーのみサポートしています。

ファイル名の辞書順にソートされて連結されていきます。

---

### History

* 0.1.0 (2023/08/10) ... 初版
