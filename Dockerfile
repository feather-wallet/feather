FROM ubuntu:16.04

ARG THREADS=1
ARG QT_VERSION=5.15.2

ENV CFLAGS="-fPIC"
ENV CPPFLAGS="-fPIC"
ENV CXXFLAGS="-fPIC"
ENV SOURCE_DATE_EPOCH=1397818193

RUN apt-get update
RUN apt-get install -y nano vim ccache software-properties-common

RUN add-apt-repository ppa:git-core/ppa
RUN apt-get update

RUN apt install -y automake git pkg-config python xutils-dev && \
    git clone -b xorgproto-2020.1 --depth 1 https://gitlab.freedesktop.org/xorg/proto/xorgproto && \
    cd xorgproto && \
    git reset --hard c62e8203402cafafa5ba0357b6d1c019156c9f36 && \
    ./autogen.sh && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b 1.12 --depth 1 https://gitlab.freedesktop.org/xorg/proto/xcbproto && \
    cd xcbproto && \
    git reset --hard 6398e42131eedddde0d98759067dde933191f049 && \
    ./autogen.sh && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y libtool-bin && \
    git clone -b libXau-1.0.9 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxau && \
    cd libxau && \
    git reset --hard d9443b2c57b512cfb250b35707378654d86c7dea && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y libpthread-stubs0-dev && \
    git clone -b 1.12 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb && \
    cd libxcb && \
    git reset --hard d34785a34f28fa6a00f8ce00d87e3132ff0f6467 && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    make -j$THREADS clean && \
    rm /usr/local/lib/libxcb-xinerama.so && \
    ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    cp src/.libs/libxcb-xinerama.a /usr/local/lib/ && \
    rm -rf $(pwd)

RUN git clone -b 0.4.0 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb-util && \
    cd libxcb-util && \
    git reset --hard acf790d7752f36e450d476ad79807d4012ec863b && \
    git submodule init && \
    git clone --depth 1 https://gitlab.freedesktop.org/xorg/util/xcb-util-m4 m4 && \
    git -C m4 reset --hard f662e3a93ebdec3d1c9374382dcc070093a42fed && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b 0.4.0 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb-image && \
    cd libxcb-image && \
    git reset --hard d882052fb2ce439c6483fce944ba8f16f7294639 && \
    git submodule init && \
    git clone --depth 1 https://gitlab.freedesktop.org/xorg/util/xcb-util-m4 m4 && \
    git -C m4 reset --hard f662e3a93ebdec3d1c9374382dcc070093a42fed && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b 0.4.0 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb-keysyms && \
    cd libxcb-keysyms && \
    git reset --hard 0e51ee5570a6a80bdf98770b975dfe8a57f4eeb1 && \
    git submodule init && \
    git clone --depth 1 https://gitlab.freedesktop.org/xorg/util/xcb-util-m4 m4 && \
    git -C m4 reset --hard f662e3a93ebdec3d1c9374382dcc070093a42fed && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b 0.3.9 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb-render-util && \
    cd libxcb-render-util && \
    git reset --hard 0317caf63de532fd7a0493ed6afa871a67253747 && \
    git submodule init && \
    git clone --depth 1 https://gitlab.freedesktop.org/xorg/util/xcb-util-m4 m4 && \
    git -C m4 reset --hard f662e3a93ebdec3d1c9374382dcc070093a42fed && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b 0.4.1 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb-wm && \
    cd libxcb-wm && \
    git reset --hard 24eb17df2e1245885e72c9d4bbb0a0f69f0700f2 && \
    git submodule init && \
    git clone --depth 1 https://gitlab.freedesktop.org/xorg/util/xcb-util-m4 m4 && \
    git -C m4 reset --hard f662e3a93ebdec3d1c9374382dcc070093a42fed && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y bison && \
    git clone -b xkbcommon-0.5.0 --depth 1 https://github.com/xkbcommon/libxkbcommon && \
    cd libxkbcommon && \
    git reset --hard c43c3c866eb9d52cd8f61e75cbef1c30d07f3a28 && \
    ./autogen.sh --prefix=/usr --enable-shared --disable-static --enable-x11 --disable-docs && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b v1.2.11 --depth 1 https://github.com/madler/zlib && \
    cd zlib && \
    git reset --hard cacf7f1d4e3d44d871b605da3b647f07d718623f && \
    ./configure --static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    ./configure --static --prefix=/usr/local/zlib && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b VER-2-10-2 --depth 1 https://git.savannah.gnu.org/git/freetype/freetype2.git && \
    cd freetype2 && \
    git reset --hard 132f19b779828b194b3fede187cee719785db4d8 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --with-zlib=no && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b R_2_2_9 --depth 1 https://github.com/libexpat/libexpat && \
    cd libexpat/expat && \
    git reset --hard a7bc26b69768f7fb24f0c7976fae24b157b85b13 && \
    ./buildconf.sh && \
    ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y autopoint gettext gperf libpng12-dev && \
    git clone -b 2.13.92 --depth 1 https://gitlab.freedesktop.org/fontconfig/fontconfig && \
    cd fontconfig && \
    git reset --hard b1df1101a643ae16cdfa1d83b939de2497b1bf27 && \
    ./autogen.sh --disable-shared --enable-static --sysconfdir=/etc --localstatedir=/var && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b release-64-2 --depth 1 https://github.com/unicode-org/icu && \
    cd icu/icu4c/source && \
    git reset --hard e2d85306162d3a0691b070b4f0a73e4012433444 && \
    ./configure --disable-shared --enable-static --disable-tests --disable-samples && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y wget && \
    wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz && \
    echo "9995e192e68528793755692917f9eb6422f3052a53c5e13ba278a228af6c7acf boost_1_73_0.tar.gz" | sha256sum -c && \
    tar -xzf boost_1_73_0.tar.gz && \
    rm boost_1_73_0.tar.gz && \
    cd boost_1_73_0 && \
    ./bootstrap.sh && \
    ./b2 --with-atomic --with-system --with-filesystem --with-thread --with-date_time --with-chrono --with-regex --with-serialization --with-program_options --with-locale variant=release link=static runtime-link=static cflags="${CFLAGS}" cxxflags="${CXXFLAGS}" install -a --prefix=/usr && \
    rm -rf $(pwd)

RUN wget https://www.openssl.org/source/openssl-1.1.1g.tar.gz && \
    echo "ddb04774f1e32f0c49751e21b67216ac87852ceb056b75209af2443400636d46 openssl-1.1.1g.tar.gz" | sha256sum -c && \
    tar -xzf openssl-1.1.1g.tar.gz && \
    rm openssl-1.1.1g.tar.gz && \
    cd openssl-1.1.1g && \
    ./config no-shared no-zlib-dynamic --prefix=/usr/local/openssl && \
    make -j$THREADS && \
    make -j$THREADS install && \
    ./config no-shared no-zlib-dynamic --openssldir=/usr && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y libgl1-mesa-dev libglib2.0-dev mesa-common-dev && \
    rm /usr/lib/x86_64-linux-gnu/libX11.a && \
    rm /usr/lib/x86_64-linux-gnu/libXext.a && \
    rm /usr/lib/x86_64-linux-gnu/libX11-xcb.a && \
    git clone git://code.qt.io/qt/qt5.git -b ${QT_VERSION} --depth 1 && \
    cd qt5 && \
    git clone git://code.qt.io/qt/qtbase.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qtimageformats.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qtmultimedia.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qtsvg.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qttools.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qttranslations.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qtx11extras.git -b ${QT_VERSION} --depth 1 && \
    git clone git://code.qt.io/qt/qtwebsockets.git -b ${QT_VERSION} --depth 1 && \
    sed -ri s/\(Libs:.*\)/\\1\ -lexpat/ /usr/local/lib/pkgconfig/fontconfig.pc && \
    sed -ri s/\(Libs:.*\)/\\1\ -lz/ /usr/local/lib/pkgconfig/freetype2.pc && \
    sed -ri s/\(Libs:.*\)/\\1\ -lXau/ /usr/local/lib/pkgconfig/xcb.pc && \
    sed -i s/\\/usr\\/X11R6\\/lib64/\\/usr\\/local\\/lib/ qtbase/mkspecs/linux-g++-64/qmake.conf && \
    OPENSSL_LIBS="-lssl -lcrypto -lpthread -ldl" \
    ./configure --prefix=/usr -platform linux-g++-64 -opensource -confirm-license -release -static -no-avx \
    -no-opengl -qpa xcb --xcb -xcb-xlib -feature-xlib -openssl-linked -I /usr/local/openssl/include \
    -L /usr/local/openssl/lib -system-freetype -fontconfig -glib \
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
    -nomake examples -nomake tests -nomake tools && \
    make -j$THREADS && \
    make -j$THREADS install && \
    cd qttools/src/linguist/lrelease && \
    ../../../../qtbase/bin/qmake && \
    make -j$THREADS && \
    make -j$THREADS install && \
    cd ../../../.. && \
    rm -rf $(pwd)

RUN apt install -y libudev-dev && \
    git clone -b v1.0.23 --depth 1 https://github.com/libusb/libusb && \
    cd libusb && \
    git reset --hard e782eeb2514266f6738e242cdcb18e3ae1ed06fa && \
    ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b hidapi-0.9.0 --depth 1 https://github.com/libusb/hidapi && \
    cd hidapi && \
    git reset --hard 7da5cc91fc0d2dbe4df4f08cd31f6ca1a262418f && \
    ./bootstrap && \
    ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y libsodium-dev && \
    git clone -b v4.3.2 --depth 1 https://github.com/zeromq/libzmq && \
    cd libzmq && \
    git reset --hard a84ffa12b2eb3569ced199660bac5ad128bff1f0 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --disable-libunwind --with-libsodium && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b libgpg-error-1.38 --depth 1 git://git.gnupg.org/libgpg-error.git && \
    cd libgpg-error && \
    git reset --hard 71d278824c5fe61865f7927a2ed1aa3115f9e439 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --disable-doc --disable-tests && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b libgcrypt-1.8.5 --depth 1 git://git.gnupg.org/libgcrypt.git && \
    cd libgcrypt && \
    git reset --hard 56606331bc2a80536db9fc11ad53695126007298 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --disable-doc && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b v3.10.0 --depth 1 https://github.com/protocolbuffers/protobuf && \
    cd protobuf && \
    git reset --hard 6d4e7fd7966c989e38024a8ea693db83758944f1 && \
    ./autogen.sh && \
    ./configure --enable-static --disable-shared && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b v3.18.4 --depth 1 https://github.com/Kitware/CMake && \
    cd CMake && \
    git reset --hard 3cc3d42aba879fff5e85b363ae8f21386a3f9f9b && \
    ./bootstrap && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y libusb-1.0-0-dev

RUN apt install -y zip && git clone -b v4.0.2 --depth 1 https://github.com/fukuchi/libqrencode.git && \
    cd libqrencode && \
    git reset --hard 59ee597f913fcfda7a010a6e106fbee2595f68e4 && \
    cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr . && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN apt install -y libmbedtls-dev && git clone -b release-2.1.12-stable --depth 1 https://github.com/libevent/libevent.git && \
    cd libevent && \
    git reset --hard 5df3037d10556bfcb675bc73e516978b75fc7bc7 && \
    mkdir build && cd build && \
    cmake -DEVENT_LIBRARY_STATIC=ON -DOPENSSL_ROOT_DIR=/usr/local/openssl -DCMAKE_INSTALL_PREFIX=/usr/local/libevent .. && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone -b tor-0.4.4.6 --depth 1 https://git.torproject.org/tor.git && \
    cd tor && \
    git reset --hard 2a8b789ea6f308d081f369d78fa7cfdc9d00bf90 && \
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
    make -j$THREADS install && \
    rm -rf $(pwd)

RUN git clone https://git.wownero.com/feather/monero-seed.git && \
    cd monero-seed && \
    git reset --hard 4674ef09b6faa6fe602ab5ae0b9ca8e1fd7d5e1b && \
    cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && \
    make -Cbuild -j$THREADS && \
    make -Cbuild install && \
    rm -rf $(pwd)

RUN apt install -y curl && \
    curl -LO "https://github.com/xmrig/xmrig/releases/download/v6.3.5/xmrig-6.3.5-linux-static-x64.tar.gz" && \
    echo "24d4f07cf5850f00ab513b228f95769a5a5ed68d35808d98f9959b58d97985a0 xmrig-6.3.5-linux-static-x64.tar.gz" | sha256sum -c && \
    tar xvf xmrig-6.3.5-linux-static-x64.tar.gz --one-top-level=/xmrig --strip 1
