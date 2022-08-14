YS Flight Simulator
Version 20181124

 By CaptainYS (山川機長)
  E-Mail PEB01130@nifty.com
  URL    http://www.ysflight.com



必要環境
  YSFLIGHT for Windows: Windows 7/8.x/10
    Vistaぐらいでも動くと思いますがテストできないので確認できません。
  YSFLIGHT for Mac OSX: Intel Mac OS 10.12 or newer (need 64-bit CPU and OS)
    macOS 10.12未満でも動くと思いますがテストできないので確認できません。
  YSFLIGHT for Linux: Tested on Ubuntu 14
    Ubuntu 14未満でも動くと思いますがテストできないので確認できません。


インストール方法
  この文書を見ているということは、すでにダウンロードして解凍できてい
  ると思います。同じディレクトリにあるSETUP.EXEを実行して画面の指示に
  したがってインストールしてください。

  Windows Vista/7以降では、Administrator権限のあるアカウントでインスト
  ールしてください。

  なお、Windows Vista/7以降のOSで、Administrator権限が無い場合は、
  SETUP.EXEをSETUP.ZIPというファイル名に変更すると、一般的な解凍ツー
  ルで解凍できるようになるので(少なくとも7Zipで可)、適当なディレク
  トリに解凍して、ysflight32_gl1.exe, ysflight32_gl2.exe または 
   ysflight32_d3d9.exeを実行すると、YSFLIGHTを使うことができます。

  なお、最近の解凍ツールではあまり発生しないようですが、単に解凍する
  と、すべてのファイルがひとつのディレクトリに展開されて、YSFLIGHT
  実行に必要なディレクトリ構造が再現されない場合があります。そうな
  ってしまうとYSFLIGHTを実行できないので、手動で解凍する場合は、必ず、
  ディレクトリ構造を再構築するようにしてください。全ファイルがひと
  つのディレクトリにできてしまっていたら、解凍は失敗なのでやりなおし
  てください。



使い方
[Mac OSX]
解凍すると、YSFLIGHTというバンドルができるので、それをダブルクリックしてください。

[Windows]
解凍してできるディレクトリ内に、
    ysflight32_gl1.exe
    ysflight32_gl2.exe
    ysflight32_d3d9.exe
の3本の実行ファイルができるので、そのうちのどれかひとつを実行してください。最もグラフィックスの品質が高いのは ysflight32_gl2.exe ですが、やや高性能なGPUを必要とします。ysflight32_gl1.exe, ysflight32_d3d9.exeはグラフィックスの品質がやや落ちる代わりに比較的旧式のGPUでもそこそこ快適に動作します。


[Linux]
  適当な場所に解凍して、コマンドから
    ysflight/ysflight32_gl1
    ysflight/ysflight32_gl2
    ysflight/ysflight64_gl1
    ysflight/ysflight64_gl2
  とのコマンドのうちのひとつをタイプする、あるいは、マウスでダブルクリックして起動してください。
  ファイル末尾の32/64は32ビットまたは64ビットバイナリの意味で、gl1/gl2は、OpenGL 1.1までの仕様
  によるレンダリングまたはOpenGL 2.0の仕様によるレンダリングの違いです。OpenGL 2.0仕様を使った
  レンダリングの方が、綺麗なグラフィックを表示できますが、PCのグラフィックスチップによっては
  対応できない場合があります。

  また、コンソールサーバーを起動するには、以下のコマンドのうちのどちらかを実行してください。
    ysflight/ysflight32_nownd
    ysflight/ysflight64_nownd



THANK YOU FOR DOWNLOADING MY SOFTWARE.

    Soji Yamakawa
      PEB01130@nifty.com
      http://www.ysflight.com
