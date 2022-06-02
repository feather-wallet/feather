# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2022 The Monero Project

CMAKEFLAGS = \
	-DTOR_DIR=$(or ${TOR_DIR}, Off) \
	-DTOR_VERSION=$(or ${TOR_VERSION}, Off) \
	-DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off) \
	-DWITH_SCANNER=$(or ${WITH_SCANNER}, Off) \
	-DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH}, Off)

guix:
	mkdir -p build/$(target)/release && \
	cd build/$(target)/release && \
	cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
		-DCMAKE_PREFIX_PATH=/gnu/store \
		-DCMAKE_PREFIX_PATH=$(CURDIR)/contrib/depends/$(target) \
		-DCMAKE_PREFIX_PATH=$(CURDIR)/contrib/depends/$(target)/native/bin \
		-DCMAKE_TOOLCHAIN_FILE=/feather/contrib/depends/$(target)/share/toolchain.cmake ../../.. \
		$(CMAKEFLAGS) && \
	$(MAKE)

release:
	mkdir -p build/release && \
	cd build/release && \
	cmake \
		-DARCH=x86-64 \
		-D BUILD_TAG="linux-x64" \
		-D CMAKE_BUILD_TYPE=Release \
		$(CMAKEFLAGS) \
		../.. && \
	$(MAKE)

release-static:
	mkdir -p build/release && \
	cd build/release && \
	cmake \
		-DARCH=x86-64 \
		-D BUILD_TAG="linux-x64" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=On \
		$(CMAKEFLAGS) \
		../.. && \
	$(MAKE)

release-static-linux-arm64:
	mkdir -p build/release && \
	cd build/release && \
	cmake \
		-D ARCH="armv8-a" \
		-D BUILD_TAG="linux-armv8" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=On \
		$(CMAKEFLAGS) \
		../.. && \
	$(MAKE)

release-static-linux-arm64-rpi:
	mkdir -p build/release && \
	cd build/release && \
	cmake \
		-D ARCH="armv8-a" \
		-D NO_AES=On \
		-D BUILD_TAG="linux-armv8-noaes" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=On \
		$(CMAKEFLAGS) \
		../.. && \
	$(MAKE)

release-static-windows:
	mkdir -p build/$(target)/release && \
	cd build/$(target)/release && \
	cmake \
	    -DARCH=x86-64 \
		-D BUILD_TAG=$(tag) \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=ON \
		-D CMAKE_TOOLCHAIN_FILE=$(root)/$(target)/share/toolchain.cmake \
		$(CMAKEFLAGS) \
		../../.. && \
	$(MAKE)

release-static-windows-installer:
	mkdir -p build/$(target)/release && \
	cd build/$(target)/release && \
	cmake \
	    -D PLATFORM_INSTALLER=On \
		-DARCH=x86-64 \
		-D BUILD_TAG=$(tag) \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=ON \
		-D CMAKE_TOOLCHAIN_FILE=$(root)/$(target)/share/toolchain.cmake \
		$(CMAKEFLAGS) \
		-D TOR_DIR=Off \
		-D TOR_VERSION=Off \
		../../.. && \
	$(MAKE)

mac-release:
	mkdir -p build && \
	cd build && \
	cmake \
	    -DARCH=native \
        -D BUILD_TAG="mac-x64" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=Off \
		$(CMAKEFLAGS) \
		.. && \
	$(MAKE) && \
	$(MAKE) deploy
