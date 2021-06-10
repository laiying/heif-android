# heif格式图片在Android端处理，例如heif的编码、解码以及和jpg、png等格式互相转换、
## 依赖以下第三方库
- 必须[libheif v1.12.0](https://github.com/strukturag/libheif)
- 必须[libde265 v1.0.8](https://github.com/strukturag/libde265)
- 必须[x265 master](https://github.com/videolan/x265)
- 可选[libjpeg v2.0.x](https://github.com/libjpeg-turbo/libjpeg-turbo)
- 可选[libpng v1.6.37](https://libpng.sourceforge.io/)
## 难点在交叉编译这些库的过程[参考](https://github.com/WanghongLin/miscellaneous/blob/master/docs/libheif4Android.md),这个参考链接有几个小误区,下面会介绍
- 本人在linux环境下编译，下载Android ndk android-ndk-r17c,装好automake,cmake等工具
- Create a standalone toolchains(tmp目录系统重启貌似会擦除上次新增的)
```
$ cd /tmp/android-ndk-r17c/build/tools
$ ./make_standalone_toolchain.py --arch arm --api 21 --stl libc++ --install-dir /tmp/ndk
```
##Build x265 from CMakeLists.txt
- Get the [source](https://github.com/videolan/x265)
```
$ cd /tmp
$ git clone https://github.com/videolan/x265
```
- Configure and build
Assembly and cli should disable, solve the TEXTREL problem, so cpu detect should comment also in file source/common/cpu.cpp
```
extern "C" {
  void PFX(cpu_neon_test)(void) {} // add empty define
  int PFX(cpu_fast_neon_mrc_test)(void) { return 0; } // add empty define
}
```
```
$ cd /tmp/x265/build
$ mkdir arm-android && cd arm-android
$ cmake -DCROSS_COMPILE_ARM=1 -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7l \
        -DCMAKE_C_COMPILER=/tmp/ndk/bin/arm-linux-androideabi-clang -DCMAKE_CXX_COMPILER=/tmp/ndk/bin/arm-linux-androideabi-clang++ \
        -DCMAKE_FIND_ROOT_PATH=/tmp/ndk/sysroot -DENABLE_ASSEMBLY=OFF -DENABLE_CLI=OFF \
        -DENABLE_PIC=ON -DENABLE_SHARED=OFF -DCMAKE_INSTALL_PREFIX=/tmp/out/x265 -DCMAKE_C_FLAGS="" \
        -G "Unix Makefiles" ../../source
$ make -j8 && make install
```
After build done, the generated static library should be found at `/tmp/out/x265`

##Build libde265 for Android
- Get the [source](https://github.com/videolan/x265)
```
$ cd /tmp
$ git clone https://github.com/strukturag/libde265
```
- Configure and build
- 误区一:需要安装automake工具，然后执行sudo sh autogen.sh 命令，生成configure等文件
- 误区二:注意这里export和参考链接的不同,CC和CXX需要保持编译的平台一样,CFLAGS、CXXFLAGS、LDFLAGS需要设置为-fPIE,不然后面编译libheif会报错
```
$ cd libde265
$ sudo sh autogen.sh
$ export CC=/tmp/ndk/bin/arm-linux-android-clang \
      CXX=/tmp/ndk/bin/arm-linux-android-clang++ \
      CFLAGS="-fPIE" \
      CXXFLAGS="-fPIE" \
      LDFLAGS="-fPIE" \
      PATH=$PATH:/tmp/ndk/bin
$ ./configure --prefix=/tmp/out/libde265 --enable-shared=no --host=arm-linux-androideabi \
      --disable-arm --disable-sse \
      && make -j8 && make install
```
After done, the output can be found at `/tmp/out/libde265`

##Build libpng for Android(如果需要heif和png格式互相转换)
- Get and extract the source Download the source from [Source Forge](https://libpng.sourceforge.io/)
```
$ cd /tmp
$ tar Jxvf ~/Downloads/libpng-1.6.37.tar.xz
```
- Configure and build
```
$ export CC=/tmp/ndk/bin/arm-linux-androideabi-clang CFLAGS='-fPIE' LDFLAGS='-fPIE -pie' PATH=$PATH:/tmp/ndk/bin
$ ./configure --prefix=/tmp/out/libpng --host=arm-linux-androideabi --enable-shared=no --enable-arm-neon
$ make -j8 && make install
```
After build done, the installed files can be found at `/tmp/out/libpng`

##Build libjpeg for Android(如果需要heif和jpeg格式互相转换)
- 注意:libjpeg在libheif中是以外部so库链接,在Android项目中加载libjpeg.so库;这点和libpng不同,libpng是以内部库链接方式编译在libheif.so中
- Get and extract the source Download the source from [Source Forge](https://github.com/libjpeg-turbo/libjpeg-turbo)
```
$ cd /tmp
$ git clone https://github.com/libjpeg-turbo/libjpeg-turbo
```
- Configure and build(在项目根目录中新建build.sh文件)
```
$ cd libjpeg-turbo
$ vim build.sh
```
接着复制下面内容到build.sh中
```
# Set these variables to suit your needs
NDK_PATH=/home/Android/android-ndk-r17c
TOOLCHAIN=gcc
ANDROID_VERSION=17
#源码目录 这里是当前脚本所在目录
MY_SOURCE_DIR=/tmp/libjpeg-turbo
# 生成目标文件目录
PREFIX=/tmp/out/libjpeg-turbo

cmake -G"Unix Makefiles" \
  -DANDROID_ABI=armeabi-v7a \
  -DANDROID_ARM_MODE=arm \
  -DANDROID_PLATFORM=android-${ANDROID_VERSION} \
  -DANDROID_TOOLCHAIN=${TOOLCHAIN} \
  -DCMAKE_ASM_FLAGS="--target=arm-linux-androideabi${ANDROID_VERSION}" \
  -DCMAKE_TOOLCHAIN_FILE=${NDK_PATH}/build/cmake/android.toolchain.cmake \
  -DCMAKE_INSTALL_PREFIX=${PREFIX} \
  ${MY_SOURCE_DIR}
make clean
make
make install
```
最后保存退出vim,执行build.sh
```
$ sudo sh build.sh
```

After build done, the installed files can be found at `/tmp/out/libjpeg`

##Build libheif for Android
- Get the [source code](https://github.com/strukturag/libheif)
```
$ cd /tmp
$ git clone https://github.com/strukturag/libheif
```
- Configure and build
- 误区三:ndk-r17c引用别的库有bug LDFLAGS="-fPIE -pie"值编译会报错,需要设置LDFLAGS="-Wl,-Bsymbolic"
- 注意:如果不需要libpng库,PKG_CONFIG_PATH去掉就行，PKG_CONFIG_PATH=PKG_CONFIG_PATH=/tmp/out/x265/lib/pkgconfig:/tmp/out/libde265/lib/pkgconfig
```
$ export CC=/tmp/ndk/bin/arm-linux-androideabi-clang \
    CXX=/tmp/ndk/bin/arm-linux-androideabi-clang++ \
    CFLAGS="-fPIE -Wno-tautological-constant-compare" \
    CXXFLAGS="-fPIE -Wno-tautological-constant-compare" \
    LDFLAGS="-Wl,-Bsymbolic" \
    PKG_CONFIG_PATH=/tmp/out/x265/lib/pkgconfig:/tmp/out/libde265/lib/pkgconfig:/tmp/out/libpng/lib/pkgconfig \
    PATH=$PATH:/tmp/ndk/bin
$ ./configure --prefix=/tmp/out/libheif --host=arm-linux-androideabi
```
After done, the installed files can be found at `/tmp/out/libheif`.
Now we can integrate libheif into our Android project and export the native API to Java by JNI.