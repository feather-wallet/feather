# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2020-2021, The Monero Project.

CMAKEFLAGS = \
	-DARCH=x86_64 \
	-DTOR_BIN=$(or ${TOR_BIN}, Off) \
	-DTOR_VERSION=$(or ${TOR_VERSION}, Off) \
	-DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off) \
	-DWITH_SCANNER=$(or ${WITH_SCANNER}, Off) \
	-DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH}, Off)

release:
	mkdir -p build/release && \
	cd build/release && \
	cmake -D CMAKE_BUILD_TYPE=Release $(CMAKEFLAGS) ../.. && \
	$(MAKE)

release-static:
	mkdir -p build/release && \
	cd build/release && \
	cmake \
		-D BUILD_TAG="linux-x64" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=On \
		$(CMAKEFLAGS) \
		../.. && \
	$(MAKE)

depends:
	mkdir -p build/$(target)/release && \
	cd build/$(target)/release && \
	cmake \
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
        -D BUILD_TAG="mac-x64" \
		-D CMAKE_BUILD_TYPE=Release \
		-D STATIC=Off \
		$(CMAKEFLAGS) \
		.. && \
	$(MAKE) && \
	$(MAKE) deploy
