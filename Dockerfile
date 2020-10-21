FROM ubuntu:18.04

ARG THREADS=1

RUN apt clean && apt update
RUN apt install -y gnupg

COPY utils/pubkeys/kitware.asc /kitware.asc
RUN cat /kitware.asc | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt install -y automake git pkg-config python wget python3.6-distutils software-properties-common
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' && apt update

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Amsterdam
RUN apt install -y build-essential nano vim aptitude ccache libusb-1.0-0-dev tzdata

RUN apt install -y xutils-dev && \
    git clone -b xorgproto-2020.1 --depth 1 https://gitlab.freedesktop.org/xorg/proto/xorgproto && \
    cd xorgproto && \
    git reset --hard c62e8203402cafafa5ba0357b6d1c019156c9f36 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b xcb-proto-1.13 --depth 1 https://gitlab.freedesktop.org/xorg/proto/xcbproto && \
    cd xcbproto && \
    git reset --hard 94228cde97d9aecfda04a8e699d462ba2b89e3a0 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y libtool-bin && \
    git clone -b libXau-1.0.9 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxau && \
    cd libxau && \
    git reset --hard d9443b2c57b512cfb250b35707378654d86c7dea && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y libpthread-stubs0-dev && \
    git clone -b 1.13.1 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb && \
    cd libxcb && \
    git reset --hard 8287ebd7b752c33b0cabc4982606fe4831106f7e && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b 0.4.0 --depth 1 --recursive https://gitlab.freedesktop.org/xorg/lib/libxcb-util.git && \
    cd libxcb-util && \
    git reset --hard acf790d7752f36e450d476ad79807d4012ec863b && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b xtrans-1.3.5 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxtrans.git && \
    cd libxtrans && \
    git reset --hard 7cbad9fe2e61cd9d5caeaf361826a6f4bd320f03 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b 0.4.0 --depth 1 --recursive https://gitlab.freedesktop.org/xorg/lib/libxcb-image.git && \
    cd libxcb-image && \
    git reset --hard d882052fb2ce439c6483fce944ba8f16f7294639 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b 0.4.0 --depth 1 --recursive https://gitlab.freedesktop.org/xorg/lib/libxcb-keysyms.git && \
    cd libxcb-keysyms && \
    git reset --hard 0e51ee5570a6a80bdf98770b975dfe8a57f4eeb1 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b 0.3.9 --depth 1 --recursive https://gitlab.freedesktop.org/xorg/lib/libxcb-render-util.git && \
    cd libxcb-render-util && \
    git reset --hard 0317caf63de532fd7a0493ed6afa871a67253747 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b 0.4.1 --depth 1 --recursive https://gitlab.freedesktop.org/xorg/lib/libxcb-wm.git && \
    cd libxcb-wm && \
    git reset --hard 24eb17df2e1245885e72c9d4bbb0a0f69f0700f2 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b libX11-1.6.9 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libx11 && \
    cd libx11 && \
    git reset --hard db7cca17ad7807e92a928da9d4c68a00f4836da2 && \
    ACLOCAL='aclocal -I /usr/local/share/aclocal/' CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b libXext-1.3.4 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxext && \
    cd libxext && \
    git reset --hard ebb167f34a3514783966775fb12573c4ed209625 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y libpthread-stubs0-dev && \
    git clone -b libXinerama-1.1.4 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxinerama.git && \
    cd libxinerama && \
    git reset --hard c3ab2361f13154921df2992f9eacc1ea1b3f946b && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b v1.2.11 --depth 1 https://github.com/madler/zlib && \
    cd zlib && \
    git reset --hard cacf7f1d4e3d44d871b605da3b647f07d718623f && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --static --prefix=/usr/local/zlib && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b VER-2-10-2 https://git.sv.nongnu.org/r/freetype/freetype2.git && \
    cd freetype2 && \
    git reset --hard 132f19b779828b194b3fede187cee719785db4d8 && \
    ./autogen.sh && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static --with-zlib=no && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b R_2_2_9 --depth 1 https://github.com/libexpat/libexpat && \
    cd libexpat/expat && \
    git reset --hard a7bc26b69768f7fb24f0c7976fae24b157b85b13 && \
    ./buildconf.sh && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y autopoint gettext gperf libpng-dev && \
    git clone -b 2.13.92 --depth 1 https://gitlab.freedesktop.org/fontconfig/fontconfig && \
    cd fontconfig && \
    git reset --hard b1df1101a643ae16cdfa1d83b939de2497b1bf27 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static --sysconfdir=/etc --localstatedir=/var && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b release-64-2 --depth 1 https://github.com/unicode-org/icu && \
    cd icu/icu4c/source && \
    git reset --hard e2d85306162d3a0691b070b4f0a73e4012433444 && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static --disable-tests --disable-samples && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y wget && \
    wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz && \
    echo "9995e192e68528793755692917f9eb6422f3052a53c5e13ba278a228af6c7acf boost_1_73_0.tar.gz" > hashsum.txt && \
    sha256sum -c hashsum.txt && \
    tar -xvzf boost_1_73_0.tar.gz && \
    cd boost_1_73_0 && \
    ./bootstrap.sh && \
    ./b2 --with-atomic --with-system --with-filesystem --with-thread --with-date_time --with-chrono --with-regex --with-serialization --with-program_options --with-locale variant=release link=static runtime-link=static cflags='-fPIC' cxxflags='-fPIC' install -a --prefix=/usr

RUN wget https://www.openssl.org/source/openssl-1.1.1g.tar.gz && \
    echo "ddb04774f1e32f0c49751e21b67216ac87852ceb056b75209af2443400636d46 openssl-1.1.1g.tar.gz" > hashsum.txt && \
    sha256sum -c hashsum.txt && \
    tar -xzf openssl-1.1.1g.tar.gz && \
    cd openssl-1.1.1g && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./config no-shared no-zlib-dynamic --prefix=/usr/local/openssl && \
    make -j$THREADS && \
    make -j$THREADS install

RUN wget https://download.qt.io/archive/qt/5.15/5.15.0/single/qt-everywhere-src-5.15.0.tar.xz && \
    echo "22b63d7a7a45183865cc4141124f12b673e7a17b1fe2b91e433f6547c5d548c3 qt-everywhere-src-5.15.0.tar.xz"  > hashsum.txt && \
    sha256sum -c hashsum.txt && \
    tar -xf qt-everywhere-src-5.15.0.tar.xz

COPY contrib/Qt5.15_LinuxPatch.json /qt-everywhere-src-5.15.0/qtbase/src/gui/configure.json
RUN apt install -y libgl1-mesa-dev libglib2.0-dev libxkbcommon-dev libxkbcommon-x11-dev

RUN cd /qt-everywhere-src-5.15.0 && \
    sed -ri s/\(Libs:.*\)/\\1\ -lexpat/ /usr/local/lib/pkgconfig/fontconfig.pc && \
    sed -ri s/\(Libs:.*\)/\\1\ -lz/ /usr/local/lib/pkgconfig/freetype2.pc && \
    sed -ri s/\(Libs:.*\)/\\1\ -lXau/ /usr/local/lib/pkgconfig/xcb.pc && \
    OPENSSL_LIBS="-lssl -lcrypto -lpthread -ldl" \
    ./configure --prefix=/usr -platform linux-g++-64 -opensource -confirm-license -release -static -no-avx \
    -no-opengl -qpa xcb -openssl-linked -I /usr/local/openssl/include -L /usr/local/openssl/lib -system-freetype -fontconfig -glib \
    -no-dbus -no-sql-sqlite -no-use-gold-linker -no-kms \
    -qt-harfbuzz -qt-libjpeg -qt-libpng -qt-pcre -qt-zlib \
    -skip qt3d -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d \
    -skip qtdoc -skip qtquickcontrols -skip qtquickcontrols2 -skip qtspeech  -skip qtgamepad \
    -skip qtlocation -skip qtmacextras -skip qtnetworkauth -skip qtpurchasing -optimize-size \
    -skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qttools \
    -skip qtvirtualkeyboard -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebview \
    -skip qtwinextras -skip qtx11extras -skip gamepad -skip serialbus -skip location -skip webengine \
    -skip qtdeclarative \
    -no-feature-cups -no-feature-ftp -no-feature-pdf -no-feature-animation \
    -nomake examples -nomake tests -nomake tools

RUN cd /qt-everywhere-src-5.15.0 && \
    make -j$THREADS && \
    make -j$THREADS install

RUN cd qt-everywhere-src-5.15.0/qttools/src/linguist/lrelease && \
    qmake && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y libudev-dev && \
    git clone -b v1.0.23 --depth 1 https://github.com/libusb/libusb && \
    cd libusb && \
    git reset --hard e782eeb2514266f6738e242cdcb18e3ae1ed06fa && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b hidapi-0.9.0 --depth 1 https://github.com/libusb/hidapi && \
    cd hidapi && \
    git reset --hard 7da5cc91fc0d2dbe4df4f08cd31f6ca1a262418f && \
    ./bootstrap && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y libsodium-dev && \
    git clone -b v4.3.2 --depth 1 https://github.com/zeromq/libzmq && \
    cd libzmq && \
    git reset --hard a84ffa12b2eb3569ced199660bac5ad128bff1f0 && \
    ./autogen.sh && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static --disable-libunwind --with-libsodium && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b libgpg-error-1.38 --depth 1 git://git.gnupg.org/libgpg-error.git && \
    cd libgpg-error && \
    git reset --hard 71d278824c5fe61865f7927a2ed1aa3115f9e439 && \
    ./autogen.sh && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static --disable-doc --disable-tests && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b libgcrypt-1.8.5 --depth 1 git://git.gnupg.org/libgcrypt.git && \
    cd libgcrypt && \
    git reset --hard 56606331bc2a80536db9fc11ad53695126007298 && \
    ./autogen.sh && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --disable-shared --enable-static --disable-doc && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b v3.10.0 --depth 1 https://github.com/protocolbuffers/protobuf && \
    cd protobuf && \
    git reset --hard 6d4e7fd7966c989e38024a8ea693db83758944f1 && \
    ./autogen.sh && \
    CFLAGS='-fPIC' CXXFLAGS='-fPIC' ./configure --enable-static --disable-shared && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y zip cmake && git clone -b v4.0.2 --depth 1 https://github.com/fukuchi/libqrencode.git && \
    cd libqrencode && \
    git reset --hard 59ee597f913fcfda7a010a6e106fbee2595f68e4 && \
    cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr . && \
    make -j$THREADS && \
    make -j$THREADS install

RUN apt install -y libmbedtls-dev && git clone -b release-2.1.12-stable --depth 1 https://github.com/libevent/libevent.git && \
    cd libevent && \
    mkdir build && cd build && \
    cmake -DEVENT_LIBRARY_STATIC=ON -DOPENSSL_ROOT_DIR=/usr/local/openssl -DCMAKE_INSTALL_PREFIX=/usr/local/libevent .. && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone -b tor-0.4.3.5 --depth 1 https://git.torproject.org/tor.git && \
    cd tor && \
    bash autogen.sh && \
    LDFLAGS="-L/usr/local/openssl/lib/" LIBS="-lssl -lcrypto -lpthread -ldl" CPPFLAGS="-I/usr/local/openssl/include/" ./configure \
    --enable-static-zlib \
    --enable-static-openssl \
    --enable-static-libevent \
    --disable-system-torrc \
    --with-libevent-dir=/usr/local/libevent \
    --with-openssl-dir=/usr/local/openssl/ \
    --with-zlib-dir=/usr/local/zlib \
    --disable-system-torrc \
    --disable-tool-name-check \
    --disable-systemd \
    --disable-lzma \
    --disable-unittests \
    --disable-zstd \
    --disable-seccomp \
    --disable-asciidoc \
    --disable-manpage \
    --disable-html-manual \
    --disable-system-torrc \
    --prefix=/usr/local/tor && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone https://git.torproject.org/torsocks.git && \
    cd torsocks && \
    bash autogen.sh && \
    ./configure --prefix=/usr/local/torsocks && \
    make -j$THREADS && \
    make -j$THREADS install

RUN git clone https://git.wownero.com/feather/monero-seed.git && \
    cd monero-seed && \
    cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && \
    make -Cbuild -j$THREADS && \
    make -Cbuild install

RUN apt install -y curl && \
    curl -LO "https://github.com/xmrig/xmrig/releases/download/v6.3.5/xmrig-6.3.5-linux-static-x64.tar.gz" && \
    echo "24d4f07cf5850f00ab513b228f95769a5a5ed68d35808d98f9959b58d97985a0 xmrig-6.3.5-linux-static-x64.tar.gz" > hashsum.txt && \
    sha256sum -c hashsum.txt && \
    tar xvf xmrig-6.3.5-linux-static-x64.tar.gz --one-top-level=/xmrig --strip 1
