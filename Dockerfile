# bionic-20210615.1 (18.04)
FROM ubuntu@sha256:ce1e17c0e0aa9db95cf19fb6ba297eb2a52b9ba71768f32a74ab39213c416600

ARG THREADS=1

ENV CFLAGS="-fPIC"
ENV CPPFLAGS="-fPIC"
ENV CXXFLAGS="-fPIC"
ENV SOURCE_DATE_EPOCH=1397818193
ENV DEBIAN_FRONTEND=noninteractive

# Feather build flags
ENV CHECK_UPDATES=ON

COPY --from=featherwallet/feather-deps:linux-beta-8 /deps /deps
COPY --from=featherwallet/feather-deps:linux-beta-8 /var/cache/apt/archives /archives

RUN apt-get update && \
    apt-get install -y gpg xz-utils ca-certificates wget && \
    rm -rf /var/lib/apt/lists/*

COPY contrib/build-deps/verify-packages.sh .
RUN bash verify-packages.sh && rm -rf /deps /archives

# OpenSSL: Required for CMake, Qt 5.15.2, libwallet, Tor
ENV OPENSSL_ROOT_DIR=/usr/local/openssl/
RUN git clone -b OpenSSL_1_1_1k --depth 1 https://github.com/openssl/openssl.git && \
    cd openssl && \
    git reset --hard fd78df59b0f656aefe96e39533130454aa957c00 && \
    ./config no-shared no-dso --prefix=/usr/local/openssl && \
    make -j$THREADS && \
    make -j$THREADS install_sw && \
    rm -rf $(pwd)

# CMake: Required to build libqrencode, monero-seed, libzip
RUN git clone -b v3.18.4 --depth 1 https://github.com/Kitware/CMake && \
    cd CMake && \
    git reset --hard 3cc3d42aba879fff5e85b363ae8f21386a3f9f9b && \
    ./bootstrap && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# xorg-macros: Required for xorgproto, other xcb libs
RUN git clone -b util-macros-1.19.3 --depth 1 https://gitlab.freedesktop.org/xorg/util/macros.git && \
    cd macros && \
    git reset --hard b8766308d2f78bc572abe5198007cf7aeec9b761 && \
    ./autogen.sh --prefix=/usr && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# xorgproto: Required for Qt 5.15.2
RUN git clone -b xorgproto-2020.1 --depth 1 https://gitlab.freedesktop.org/xorg/proto/xorgproto && \
    cd xorgproto && \
    git reset --hard c62e8203402cafafa5ba0357b6d1c019156c9f36 && \
    ./autogen.sh && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# xcbproto: Required for Qt 5.15.2
RUN git clone -b xcb-proto-1.14.1 --depth 1 https://gitlab.freedesktop.org/xorg/proto/xcbproto && \
    cd xcbproto && \
    git reset --hard 496e3ce329c3cc9b32af4054c30fa0f306deb007 && \
    ./autogen.sh && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libxau: Required for Qt 5.15.2
RUN git clone -b libXau-1.0.9 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxau && \
    cd libxau && \
    git reset --hard d9443b2c57b512cfb250b35707378654d86c7dea && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# pthread-stubs: Required for libxcb
RUN git clone -b 0.4 --depth 1 https://gitlab.freedesktop.org/xorg/lib/pthread-stubs.git && \
    cd pthread-stubs && \
    git reset --hard 50f0755a7f894acae168f19c66e52a3f139ca4ec && \
    ./autogen.sh && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libxcb: Required for Qt 5.15
RUN git clone -b libxcb-1.14 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxcb && \
    cd libxcb && \
    git reset --hard 4b40b44cb6d088b6ffa2fb5cf3ad8f12da588cef && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libxcb-util: Required for Qt 5.15
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

# libxcb-image: Required for Qt 5.15
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

# libxcb-keysyms: Required for Qt 5.15
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

# libxcb-render-util: Required for Qt 5.15
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

# libxcb-wm: Required for Qt 5.15
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

# libkcbcommon: Required for Qt 5.15
RUN git clone -b xkbcommon-0.5.0 --depth 1 https://github.com/xkbcommon/libxkbcommon && \
    cd libxkbcommon && \
    git reset --hard c43c3c866eb9d52cd8f61e75cbef1c30d07f3a28 && \
    ./autogen.sh --prefix=/usr --enable-shared --disable-static --enable-x11 --disable-docs && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libXext: Required for Qt 5.15
RUN git clone -b libXext-1.3.4 --depth 1 https://gitlab.freedesktop.org/xorg/lib/libxext.git && \
    cd libxext && \
    git reset --hard ebb167f34a3514783966775fb12573c4ed209625 && \
    ./autogen.sh --enable-shared --disable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# freetype2: Required for Qt 5.15
RUN git clone -b VER-2-10-2 --depth 1 https://git.savannah.gnu.org/git/freetype/freetype2.git && \
    cd freetype2 && \
    git reset --hard 132f19b779828b194b3fede187cee719785db4d8 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --with-zlib=no && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# expat: Required for fontconfig
RUN git clone -b R_2_2_9 --depth 1 https://github.com/libexpat/libexpat && \
    cd libexpat/expat && \
    git reset --hard a7bc26b69768f7fb24f0c7976fae24b157b85b13 && \
    ./buildconf.sh && \
    ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# fontconfig: Required for Qt 5.15
RUN git clone -b 2.13.92 --depth 1 https://gitlab.freedesktop.org/fontconfig/fontconfig && \
    cd fontconfig && \
    git reset --hard b1df1101a643ae16cdfa1d83b939de2497b1bf27 && \
    ./autogen.sh --disable-shared --enable-static --sysconfdir=/etc --localstatedir=/var && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

ENV QT_VERSION=v5.15.2
RUN git clone git://code.qt.io/qt/qt5.git -b ${QT_VERSION} --depth 1 && \
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
    sed -i s/\\/usr\\/X11R6\\/lib64/\\/usr\\/local\\/lib/ qtbase/mkspecs/linux-g++-64/qmake.conf

RUN cd qt5 && \
    rm /usr/lib/x86_64-linux-gnu/libX11.a && \
    rm /usr/lib/x86_64-linux-gnu/libX11-xcb.a && \
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
        -skip qtdeclarative -gstreamer \
        -no-feature-cups -no-feature-ftp -no-feature-pdf -no-feature-animation \
        -nomake examples -nomake tests -nomake tools && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# boost: Required for libwallet
RUN wget https://downloads.sourceforge.net/project/boost/boost/1.73.0/boost_1_73_0.tar.bz2 && \
    echo "4eb3b8d442b426dc35346235c8733b5ae35ba431690e38c6a8263dce9fcbb402 boost_1_73_0.tar.bz2" | sha256sum -c && \
    tar -xvf boost_1_73_0.tar.bz2 && \
    rm boost_1_73_0.tar.bz2 && \
    cd boost_1_73_0 && \
    ./bootstrap.sh && \
    ./b2 --with-atomic --with-system --with-filesystem --with-thread --with-date_time --with-chrono --with-regex --with-serialization --with-program_options --with-locale variant=release link=static runtime-link=static cflags="${CFLAGS}" cxxflags="${CXXFLAGS}" install -a --prefix=/usr && \
    rm -rf $(pwd)

# libusb: Required for libwallet
RUN git clone -b v1.0.24 --depth 1 https://github.com/libusb/libusb && \
    cd libusb && \
    git reset --hard c6a35c56016ea2ab2f19115d2ea1e85e0edae155 && \
    ./autogen.sh --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# hidapi: Required for libwallet
RUN git clone -b hidapi-0.10.1 --depth 1 https://github.com/libusb/hidapi && \
    cd hidapi && \
    git reset --hard f6d0073fcddbdda24549199445e844971d3c9cef && \
    ./bootstrap && \
    ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libsodium: Required for libzmq
RUN git clone -b 1.0.18-RELEASE --depth 1 https://github.com/jedisct1/libsodium.git && \
    cd libsodium && \
    git reset --hard 940ef42797baa0278df6b7fd9e67c7590f87744b && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libzmq: Required for libwallet
RUN git clone -b v4.3.2 --depth 1 https://github.com/zeromq/libzmq && \
    cd libzmq && \
    git reset --hard a84ffa12b2eb3569ced199660bac5ad128bff1f0 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --disable-libunwind --with-libsodium && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# protobuf: Required for libwallet
RUN git clone -b v3.10.0 --depth 1 https://github.com/protocolbuffers/protobuf && \
    cd protobuf && \
    git reset --hard 6d4e7fd7966c989e38024a8ea693db83758944f1 && \
    ./autogen.sh && \
    ./configure --enable-static --disable-shared && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# appimagetool: Used to created Feather AppImage
RUN mkdir appimagetool && \
    cd appimagetool && \
    wget https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage && \
    echo "d918b4df547b388ef253f3c9e7f6529ca81a885395c31f619d9aaf7030499a13 appimagetool-x86_64.AppImage" | sha256sum -c && \
    chmod +x appimagetool-x86_64.AppImage && \
    ./appimagetool-x86_64.AppImage --appimage-extract && \
    rm appimagetool-x86_64.AppImage

# squashfs-tools: Used to create Feather AppImage
RUN git clone https://github.com/plougher/squashfs-tools.git && \
    cd squashfs-tools/squashfs-tools && \
    git reset --hard 38fa0720526222827da44b3b6c3f7eb63e8f5c2f && \
    make && \
    make install && \
    rm -rf $(pwd)

# patchelf: Required by linuxdeployqt
RUN git clone -b 0.12 --depth 1 https://github.com/NixOS/patchelf.git && \
    cd patchelf && \
    git reset --hard 8d3a16e97294e3c5521c61b4c8835499c9918264 && \
    ./bootstrap.sh && \
    ./configure && \
     make -j$THREADS && \
    make install && \
    rm -rf $(pwd)

# linuxdeployqt: Used to create Feather AppImage
# build from source because latest release does not allow glib 2.27
RUN git clone https://github.com/probonopd/linuxdeployqt.git && \
    cd linuxdeployqt && \
    git reset --hard b4697483c98120007019c3456914cfd1dba58384 && \
    qmake && \
    make -j$THREADS && \
    make install && \
    rm -rf $(pwd)

# libevent: Required for Tor
RUN wget https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz && \
    echo "92e6de1be9ec176428fd2367677e61ceffc2ee1cb119035037a27d346b0403bb libevent-2.1.12-stable.tar.gz" | sha256sum -c && \
    tar -zxvf libevent-2.1.12-stable.tar.gz && \
    cd libevent-2.1.12-stable && \
    PKG_CONFIG_PATH=/usr/local/openssl/lib/pkgconfig/ \
    ./configure --prefix=/usr/local/libevent \
                --disable-shared \
                --enable-static \
                --with-pic && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# zlib: Required for Tor
RUN git clone -b v1.2.11 --depth 1 https://github.com/madler/zlib && \
    cd zlib && \
    git reset --hard cacf7f1d4e3d44d871b605da3b647f07d718623f && \
    ./configure --static --prefix=/usr/local/zlib && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# Tor: Optional for Feather (-DTOR_BIN)
# Binary can be embedded in Feather
ENV TOR_BIN=/usr/local/tor/bin/tor
RUN git clone -b tor-0.4.5.7 --depth 1 https://git.torproject.org/tor.git && \
    cd tor && \
    git reset --hard 83f895c015de55201e5f226f84a866f30f5ee14b && \
    ./autogen.sh && \
    ./configure \
    --disable-asciidoc \
    --disable-manpage \
    --disable-html-manual \
    --disable-system-torrc \
    --disable-module-relay \
    --disable-lzma \
    --disable-zstd \
    --enable-static-tor \
    --with-libevent-dir=/usr/local/libevent \
    --with-openssl-dir=/usr/local/openssl \
    --with-zlib-dir=/usr/local/zlib \
    --disable-tool-name-check \
    --enable-fatal-warnings \
    --prefix=/usr/local/tor && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd) && \
    strip -s -D /usr/local/tor/bin/tor

# libqrencode: Required for Feather
# Used to display QR Codes
RUN git clone -b v4.1.1 --depth 1 https://github.com/fukuchi/libqrencode.git && \
    cd libqrencode && \
    git reset --hard 715e29fd4cd71b6e452ae0f4e36d917b43122ce8 && \
    cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr . && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# monero-seed: Required for Feather
# Tevador's 14 word seed library
RUN git clone https://git.featherwallet.org/feather/monero-seed.git && \
    cd monero-seed && \
    git reset --hard 4674ef09b6faa6fe602ab5ae0b9ca8e1fd7d5e1b && \
    cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && \
    make -Cbuild -j$THREADS && \
    make -Cbuild install && \
    rm -rf $(pwd)

# libzip: Required for Feather
# Used to unzip updates downloaded by the built-in updater
RUN git clone -b v1.7.3 --depth 1 https://github.com/nih-at/libzip.git && \
    cd libzip && \
    git reset --hard 66e496489bdae81bfda8b0088172871d8fda0032 && \
    cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr . && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libgpg-error: Required for libgcrypt
RUN git clone -b libgpg-error-1.38 --depth 1 git://git.gnupg.org/libgpg-error.git && \
    cd libgpg-error && \
    git reset --hard 71d278824c5fe61865f7927a2ed1aa3115f9e439 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --disable-doc --disable-tests && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# libgcrypt: Required for Feather
# Used in src/openpgp to verify updates downloaded by the built-in updater
RUN git clone -b libgcrypt-1.8.5 --depth 1 git://git.gnupg.org/libgcrypt.git && \
    cd libgcrypt && \
    git reset --hard 56606331bc2a80536db9fc11ad53695126007298 && \
    ./autogen.sh && \
    ./configure --disable-shared --enable-static --disable-doc && \
    make -j$THREADS && \
    make -j$THREADS install && \
    rm -rf $(pwd)

# zbar: Optional for Feather (-DWITH_SCANNER)
# Used to scan for QR Codes
RUN git clone -b stable-0.21 --recursive https://github.com/mchehab/zbar.git && \
    cd zbar && \
    git reset --hard 505f1a87b32cb7bb0edbaf37e20ccdd46bbae2a3 && \
    autoreconf -vfi && \
    ./configure --enable-static --disable-shared --without-imagemagick --with-gtk=no --with-python=no --enable-doc=no && \
    make -j$THREADS && \
    make install && \
    rm -rf $(pwd)