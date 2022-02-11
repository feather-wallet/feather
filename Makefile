# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2022 The Monero Project

CMAKEFLAGS = \
	-DTOR_BIN=$(or ${TOR_BIN}, Off) \
	-DTOR_VERSION=$(or ${TOR_VERSION}, Off) \
	-DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off) \
	-DWITH_SCANNER=$(or ${WITH_SCANNER}, Off) \
	-DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH}, Off)

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

depends:
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

win-installer:
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
		../../.. && \
	$(MAKE)

mac-release:
	mkdir -p build && \
	cd build && \
	cmake \
	    -DARCH=x86-64 \
        -D BUILD_TAG="mac-x64" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=Off \
		$(CMAKEFLAGS) \
		.. && \
	$(MAKE) && \
	$(MAKE) deploy
