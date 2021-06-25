# Copyright (c) 2014-2021, The Monero Project
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other
#    materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be
#    used to endorse or promote products derived from this software without specific
#    prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY

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
	-DUSE_DEVICE_TREZOR=Off \
	$(CMAKEFLAGS_EXTRA)

release-static: CMAKEFLAGS += -DBUILD_TAG="linux-x64"
release-static: CMAKEFLAGS += -DTOR_BIN=$(or ${TOR_BIN},OFF)
release-static: CMAKEFLAGS += -DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off)
release-static: CMAKEFLAGS += -DWITH_SCANNER=$(or ${WITH_SCANNER}, Off)
release-static: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Release
release-static: CMAKEFLAGS += -DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH},OFF)
release-static:
	cmake -Bbuild $(CMAKEFLAGS)
	$(MAKE) -Cbuild

depends:
	mkdir -p build/$(target)/release
	cd build/$(target)/release && cmake -D STATIC=ON -DREPRODUCIBLE=$(or ${SOURCE_DATE_EPOCH},OFF) -DTOR_VERSION=$(or ${TOR_VERSION}, OFF) -DTOR_BIN=$(or ${TOR_BIN},OFF) -DCHECK_UPDATES=$(or ${CHECK_UPDATES}, OFF) -D DEV_MODE=$(or ${DEV_MODE},OFF) -D BUILD_TAG=$(tag) -D CMAKE_BUILD_TYPE=Release -D CMAKE_TOOLCHAIN_FILE=$(root)/$(target)/share/toolchain.cmake ../../.. && $(MAKE)

mac-release: CMAKEFLAGS += -DSTATIC=Off
mac-release: CMAKEFLAGS += -DTOR_BIN=$(or ${TOR_BIN},OFF)
mac-release: CMAKEFLAGS += -DCHECK_UPDATES=$(or ${CHECK_UPDATES}, Off)
mac-release: CMAKEFLAGS += -DBUILD_TAG="mac-x64"
mac-release: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Release
mac-release:
	cmake -Bbuild $(CMAKEFLAGS)
	$(MAKE) -Cbuild
	$(MAKE) -Cbuild deploy
