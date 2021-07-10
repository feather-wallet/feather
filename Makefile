# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2020-2021, The Monero Project.

CMAKEFLAGS = \
	-DARCH=x86_64 \
	-DBUILD_64=On \
	-DBUILD_TESTS=Off \
	-DLOCALMONERO=On \
	-DXMRIG=On \
	-DCHECK_UPDATES=Off \
	-DTOR_BIN=Off \
	-DCMAKE_CXX_STANDARD=11 \
	-DCMAKE_VERBOSE_MAKEFILE=On \
	-DINSTALL_VENDORED_LIBUNBOUND=Off \
	-DMANUAL_SUBMODULES=1 \
	-DSTATIC=On \
	-DUSE_DEVICE_TREZOR=On \
	$(CMAKEFLAGS_EXTRA)

release-static: CMAKEFLAGS += -DBUILD_TAG="linux-x64"
release-static: CMAKEFLAGS += -DTOR_BIN=$(or ${TOR_BIN}, Off)
release-static: CMAKEFLAGS += -DTOR_VERSION=$(or ${TOR_VERSION}, Off)
release-static: CMAKEFLAGS += -DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off)
release-static: CMAKEFLAGS += -DWITH_SCANNER=$(or ${WITH_SCANNER}, Off)
release-static: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Release
release-static: CMAKEFLAGS += -DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH}, Off)
release-static:
	cmake -Bbuild $(CMAKEFLAGS)
	$(MAKE) -Cbuild

depends:
	mkdir -p build/$(target)/release
	cd build/$(target)/release && cmake -D STATIC=ON -DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH},OFF) -DTOR_VERSION=$(or ${TOR_VERSION}, OFF) -DTOR_BIN=$(or ${TOR_BIN},OFF) -DCHECK_UPDATES=$(or ${CHECK_UPDATES}, OFF) -D DEV_MODE=$(or ${DEV_MODE},OFF) -D BUILD_TAG=$(tag) -D CMAKE_BUILD_TYPE=Release -D CMAKE_TOOLCHAIN_FILE=$(root)/$(target)/share/toolchain.cmake ../../.. && $(MAKE)

mac-release: CMAKEFLAGS += -DSTATIC=Off
mac-release: CMAKEFLAGS += -DTOR_BIN=$(or ${TOR_BIN}, Off)
mac-release: CMAKEFLAGS += -DTOR_VERSION=$(or ${TOR_VERSION}, Off)
mac-release: CMAKEFLAGS += -DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off)
mac-release: CMAKEFLAGS += -DWITH_SCANNER=$(or ${WITH_SCANNER}, On)
mac-release: CMAKEFLAGS += -DBUILD_TAG="mac-x64"
mac-release: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Release
mac-release:
	cmake -Bbuild $(CMAKEFLAGS)
	$(MAKE) -Cbuild
	$(MAKE) -Cbuild deploy
