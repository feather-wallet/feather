#!/usr/bin/env bash
# Copyright (c) 2019-2021 The Bitcoin Core developers
# Copyright (c) 2022-2022 The Monero Project
# Distributed under the MIT software license, see the accompanying
# file ../LICENSE.txt or http://www.opensource.org/licenses/mit-license.php.
export LC_ALL=C
set -e -o pipefail
export TZ=UTC
export DEBUG_GENID=1
export HOSTNAME=host

# shellcheck source=contrib/shell/git-utils.bash
source contrib/shell/git-utils.bash

# Although Guix _does_ set umask when building its own packages (in our case,
# this is all packages in manifest.scm), it does not set it for `guix
# environment`. It does make sense for at least `guix environment --container`
# to set umask, so if that change gets merged upstream and we bump the
# time-machine to a commit which includes the aforementioned change, we can
# remove this line.
#
# This line should be placed before any commands which creates files.
umask 0022

if [ -n "$V" ]; then
    # Print both unexpanded (-v) and expanded (-x) forms of commands as they are
    # read from this file.
    set -vx
    # Set VERBOSE for CMake-based builds
    export VERBOSE="$V"
fi

# Check that required environment variables are set
cat << EOF
Required environment variables as seen inside the container:
    DIST_ARCHIVE_BASE: ${DIST_ARCHIVE_BASE:?not set}
    DISTNAME: ${DISTNAME:?not set}
    VERSION: ${VERSION:?not set}
    COMMIT: ${COMMIT:?not set}
    HOST: ${HOST:?not set}
    COMMIT_TIMESTAMP: ${COMMIT_TIMESTAMP:?not set}
    JOBS: ${JOBS:?not set}
    DISTSRC: ${DISTSRC:?not set}
    OUTDIR: ${OUTDIR:?not set}
    LOGDIR: ${LOGDIR:?not set}
    OPTIONS: ${OPTIONS}
EOF

mkdir -p ${DISTSRC}
ln -s "${DISTSRC}" /distsrc

ACTUAL_OUTDIR="${OUTDIR}"
OUTDIR="/distsrc/output"

# Use a fixed timestamp for depends builds so hashes match across commits that don't make changes to the build system
export SOURCE_DATE_EPOCH=1397818193

#####################
# Environment Setup #
#####################

# Collect some information about the build environment to help debug potential reproducibility issues
mkdir -p "${LOGDIR}"
ls -1 /gnu/store | sort > ${LOGDIR}/guix-hashes.txt
printenv | sort | grep -v '^\(BASE_CACHE=\|DISTNAME=\|DISTSRC=\|OUTDIR=\|LOGDIR=\|SOURCES_PATH=\|JOBS=\|OPTIONS=\|DEPENDS_ONLY=\)' > ${LOGDIR}/guix-env.txt

# The depends folder also serves as a base-prefix for depends packages for
# $HOSTs after successfully building.
BASEPREFIX="${PWD}/contrib/depends"

# Given a package name and an output name, return the path of that output in our
# current guix environment
store_path() {
    grep --extended-regexp "/[^-]{32}-${1}-[^-]+${2:+-${2}}" "${GUIX_ENVIRONMENT}/manifest" \
        | head --lines=1 \
        | sed --expression='s|\x29*$||' \
              --expression='s|^[[:space:]]*"||' \
              --expression='s|"[[:space:]]*$||'
}


# Set environment variables to point the NATIVE toolchain to the right
# includes/libs
NATIVE_GCC="$(store_path gcc-toolchain)"
NATIVE_GCC_STATIC="$(store_path gcc-toolchain static)"

unset LIBRARY_PATH
unset CPATH
unset C_INCLUDE_PATH
unset CPLUS_INCLUDE_PATH
unset OBJC_INCLUDE_PATH
unset OBJCPLUS_INCLUDE_PATH

export LIBRARY_PATH="${NATIVE_GCC}/lib:${NATIVE_GCC}/lib64:${NATIVE_GCC_STATIC}/lib:${NATIVE_GCC_STATIC}/lib64"
export C_INCLUDE_PATH="${NATIVE_GCC}/include"
export CPLUS_INCLUDE_PATH="${NATIVE_GCC}/include/c++:${NATIVE_GCC}/include"
export OBJC_INCLUDE_PATH="${NATIVE_GCC}/include"
export OBJCPLUS_INCLUDE_PATH="${NATIVE_GCC}/include/c++:${NATIVE_GCC}/include"

prepend_to_search_env_var() {
    export "${1}=${2}${!1:+:}${!1}"
}

# Set environment variables to point the CROSS toolchain to the right
# includes/libs for $HOST
case "$HOST" in
    *mingw*)
        # Determine output paths to use in CROSS_* environment variables
        CROSS_GLIBC="$(store_path "mingw-w64-x86_64-winpthreads")"
        CROSS_GCC="$(store_path "gcc-cross-${HOST}")"
        CROSS_GCC_LIB_STORE="$(store_path "gcc-cross-${HOST}" lib)"
        CROSS_GCC_LIBS=( "${CROSS_GCC_LIB_STORE}/lib/gcc/${HOST}"/* ) # This expands to an array of directories...
        CROSS_GCC_LIB="${CROSS_GCC_LIBS[0]}" # ...we just want the first one (there should only be one)

        # The search path ordering is generally:
        #    1. gcc-related search paths
        #    2. libc-related search paths
        #    2. kernel-header-related search paths (not applicable to mingw-w64 hosts)
        export CROSS_C_INCLUDE_PATH="${CROSS_GCC_LIB}/include:${CROSS_GCC_LIB}/include-fixed:${CROSS_GLIBC}/include"
        export CROSS_CPLUS_INCLUDE_PATH="${CROSS_GCC}/include/c++:${CROSS_GCC}/include/c++/${HOST}:${CROSS_GCC}/include/c++/backward:${CROSS_C_INCLUDE_PATH}"
        export CROSS_LIBRARY_PATH="${CROSS_GCC_LIB_STORE}/lib:${CROSS_GCC_LIB}:${CROSS_GLIBC}/lib"

        WMF_LIBS="$(store_path "mingw-w64-x86_64-winpthreads")"
        export WMF_LIBS
        ;;
    *darwin*)
        # The CROSS toolchain for darwin uses the SDK and ignores environment variables.
        # See depends/hosts/darwin.mk for more details.
        ;;
    *linux*)
        CROSS_GLIBC="$(store_path "glibc-cross-${HOST}")"
        CROSS_GLIBC_STATIC="$(store_path "glibc-cross-${HOST}" static)"
        CROSS_KERNEL="$(store_path "linux-libre-headers-cross-${HOST}")"
        CROSS_GCC="$(store_path "gcc-cross-${HOST}")"
        CROSS_GCC_LIB_STORE="$(store_path "gcc-cross-${HOST}" lib)"
        CROSS_GCC_LIBS=( "${CROSS_GCC_LIB_STORE}/lib/gcc/${HOST}"/* ) # This expands to an array of directories...
        CROSS_GCC_LIB="${CROSS_GCC_LIBS[0]}" # ...we just want the first one (there should only be one)

        export CROSS_C_INCLUDE_PATH="${CROSS_GCC_LIB}/include:${CROSS_GCC_LIB}/include-fixed:${CROSS_GLIBC}/include:${CROSS_KERNEL}/include"
        export CROSS_CPLUS_INCLUDE_PATH="${CROSS_GCC}/include/c++:${CROSS_GCC}/include/c++/${HOST}:${CROSS_GCC}/include/c++/backward:${CROSS_C_INCLUDE_PATH}"
        export CROSS_LIBRARY_PATH="${CROSS_GCC_LIB_STORE}/lib:${CROSS_GCC_LIB}:${CROSS_GLIBC}/lib:${CROSS_GLIBC_STATIC}/lib"
        ;;
    *)
        exit 1 ;;
esac

# Sanity check CROSS_*_PATH directories
IFS=':' read -ra PATHS <<< "${CROSS_C_INCLUDE_PATH}:${CROSS_CPLUS_INCLUDE_PATH}:${CROSS_LIBRARY_PATH}"
for p in "${PATHS[@]}"; do
    if [ -n "$p" ] && [ ! -d "$p" ]; then
        echo "'$p' doesn't exist or isn't a directory... Aborting..."
        exit 1
    fi
done

# Disable Guix ld auto-rpath behavior
export GUIX_LD_WRAPPER_DISABLE_RPATH=yes

# Make /usr/bin if it doesn't exist
[ -e /usr/bin ] || mkdir -p /usr/bin

# Symlink file and env to a conventional path
[ -e /usr/bin/file ] || ln -s --no-dereference "$(command -v file)" /usr/bin/file
[ -e /usr/bin/env ]  || ln -s --no-dereference "$(command -v env)"  /usr/bin/env

# Determine the correct value for -Wl,--dynamic-linker for the current $HOST
case "$HOST" in
    *linux*)
        glibc_dynamic_linker=$(
            case "$HOST" in
                x86_64-linux-gnu)      echo /lib64/ld-linux-x86-64.so.2 ;;
                arm-linux-gnueabihf)   echo /lib/ld-linux-armhf.so.3 ;;
                aarch64-linux-gnu)     echo /lib/ld-linux-aarch64.so.1 ;;
                riscv64-linux-gnu)     echo /lib/ld-linux-riscv64-lp64d.so.1 ;;
                powerpc64-linux-gnu)   echo /lib64/ld64.so.1;;
                powerpc64le-linux-gnu) echo /lib64/ld64.so.2;;
                *)                     exit 1 ;;
            esac
        )
        ;;
esac

export GLIBC_DYNAMIC_LINKER=${glibc_dynamic_linker}

# Environment variables for determinism
export TAR_OPTIONS="--owner=0 --group=0 --numeric-owner --mtime='@${SOURCE_DATE_EPOCH}' --sort=name"
export TZ="UTC"

####################
# Depends Building #
####################

# LDFLAGS
case "$HOST" in
    *linux*)  HOST_LDFLAGS="-Wl,--as-needed -Wl,--dynamic-linker=$glibc_dynamic_linker -static-libstdc++ -Wl,-O2" ;;
    *mingw*)  HOST_LDFLAGS="-Wl,--no-insert-timestamp" ;;
esac

mkdir -p "$OUTDIR"

# Log the depends build ids
make -C contrib/depends --no-print-directory HOST="$HOST" print-final_build_id_long | tr ':' '\n' > ${LOGDIR}/depends-hashes.txt

export CMAKE_BUILD_PARALLEL_LEVEL=$JOBS

# Build the depends tree, overriding variables that assume multilib gcc
make -C contrib/depends --jobs="$JOBS" HOST="$HOST" \
                                   ${V:+V=1} \
                                   ${SOURCES_PATH+SOURCES_PATH="$SOURCES_PATH"} \
                                   ${BASE_CACHE+BASE_CACHE="$BASE_CACHE"} \
                                   ${SDK_PATH+SDK_PATH="$SDK_PATH"} \
                                   OUTDIR="$OUTDIR" \
                                   LOGDIR="$LOGDIR" \
                                   x86_64_linux_CC=x86_64-linux-gnu-gcc \
                                   x86_64_linux_CXX=x86_64-linux-gnu-g++ \
                                   x86_64_linux_AR=x86_64-linux-gnu-gcc-ar \
                                   x86_64_linux_RANLIB=x86_64-linux-gnu-gcc-ranlib \
                                   x86_64_linux_NM=x86_64-linux-gnu-gcc-nm \
                                   x86_64_linux_STRIP=x86_64-linux-gnu-strip \
                                   guix_ldflags="$HOST_LDFLAGS" \
                                   build_CC="${NATIVE_GCC}/bin/gcc -isystem ${NATIVE_GCC}/include" \
                                   build_CXX="${NATIVE_GCC}/bin/g++ -isystem ${NATIVE_GCC}/include/c++ -isystem ${NATIVE_GCC}/include"

# Log the depends package hashes
DEPENDS_PACKAGES="$(make -C contrib/depends --no-print-directory HOST="$HOST" print-all_packages)"
DEPENDS_CACHE="$(make -C contrib/depends --no-print-directory ${BASE_CACHE+BASE_CACHE="$BASE_CACHE"} print-BASE_CACHE)"

{
    for package in ${DEPENDS_PACKAGES}; do
        cat "${DEPENDS_CACHE}/${HOST}/${package}"/*.hash
    done
} | sort -k2 > "${LOGDIR}/depends-packages.txt"

###########################
# Source Tarball Building #
###########################

# Use COMMIT_TIMESTAMP for the source and release binary archives
export SOURCE_DATE_EPOCH=${COMMIT_TIMESTAMP}
export TAR_OPTIONS="--owner=0 --group=0 --numeric-owner --mtime='@${SOURCE_DATE_EPOCH}' --sort=name"

GIT_ARCHIVE="${DIST_ARCHIVE_BASE}/${DISTNAME}.tar.gz"

# Create the source tarball if not already there
if [ ! -e "$GIT_ARCHIVE" ]; then
    mkdir -p "$(dirname "$GIT_ARCHIVE")"
    git ls-files --recurse-submodules \
    | sort \
    | tar --create --transform "s,^,${DISTNAME}/," --mode='u+rw,go+r-w,a+X' --files-from=- \
    | gzip -9n > ${GIT_ARCHIVE}
    sha256sum "$GIT_ARCHIVE"
fi

###########################
# Binary Tarball Building #
###########################

# CFLAGS
HOST_CFLAGS="-O2"
HOST_CFLAGS+=$(find /gnu/store -maxdepth 1 -mindepth 1 -type d -exec echo -n " -ffile-prefix-map={}=/usr" \;)
case "$HOST" in
    *linux*)  HOST_CFLAGS+=" -ffile-prefix-map=${PWD}=." ;;
    *mingw*)  HOST_CFLAGS+=" -fno-ident" ;;
    *darwin*) unset HOST_CFLAGS ;;
esac

# CXXFLAGS
HOST_CXXFLAGS="$HOST_CFLAGS"

case "$HOST" in
    arm-linux-gnueabihf) HOST_CXXFLAGS="${HOST_CXXFLAGS} -Wno-psabi" ;;
esac

export USE_DEVICE_TREZOR_MANDATORY=1

# Make $HOST-specific native binaries from depends available in $PATH
export PATH="${BASEPREFIX}/${HOST}/native/bin:${PATH}"
(
    cd "/distsrc"

    # Extract the source tarball
    tar --strip-components=1 -xf "${GIT_ARCHIVE}"

    # Setup the directory where our Bitcoin Core build for HOST will be
    # installed. This directory will also later serve as the input for our
    # binary tarballs.
    INSTALLPATH="/distsrc/installed"
    mkdir -p "${INSTALLPATH}"


    # Set appropriate CMake options for build type
    CMAKEVARS="-DWITH_SCANNER=On -DCHECK_UPDATES=On -DSELF_CONTAINED=On -DDONATE_BEG=On -DFEATHER_TARGET_TRIPLET=${HOST} -DWITH_PLUGIN_REDDIT=Off"

    if [[ -n "${TAG}" ]]; then
        CMAKEVARS+=" -DOFFICIAL_BUILD=On"
    fi

    case "$HOST" in
        *mingw32)
            case "$OPTIONS" in
                installer)
                    CMAKEVARS+=" -DPLATFORM_INSTALLER=On"
                    ;;
            esac
            ;;
        *linux*)
            CMAKEVARS+=" -DSTACK_TRACE=ON"
            case "$OPTIONS" in
                pack)
                    CMAKEVARS+=" -DCHECK_UPDATES=Off -DSELF_CONTAINED=Off"
                    ;;
            esac
            ;;
        *gnueabihf)
            CMAKEVARS+=" -DNO_AES=On" # Raspberry Pi
            ;;
    esac

    # Configure this DISTSRC for $HOST
    # shellcheck disable=SC2086
    env CFLAGS="${HOST_CFLAGS}" CXXFLAGS="${HOST_CXXFLAGS}" \
    cmake --toolchain "${BASEPREFIX}/${HOST}/share/toolchain.cmake" -S . -B build \
      -DCMAKE_INSTALL_PREFIX="${INSTALLPATH}" \
      -DCCACHE=OFF \
      ${CONFIGFLAGS} \
      -DCMAKE_EXE_LINKER_FLAGS="${HOST_LDFLAGS}" \
      -DCMAKE_SHARED_LINKER_FLAGS="${HOST_LDFLAGS}" \
      ${CMAKEVARS}

    make -C build --jobs="$JOBS"

    LINUX_ARCH=""
    case "$HOST" in
        aarch64-linux*)
            LINUX_ARCH="-arm64"
            ;;
        arm-linux*)
            LINUX_ARCH="-arm"
            ;;
        riscv64-linux*)
            LINUX_ARCH="-riscv64"
            ;;
    esac

    DARWIN_ARCH=""
    case "$HOST" in
        arm64-apple-darwin)
            DARWIN_ARCH="-arm64"
            ;;
    esac

    case "$HOST" in
        *linux*)
            if [ "$OPTIONS" != "pack" ]; then
                bash contrib/AppImage/build-appimage.sh
                APPIMAGENAME=${DISTNAME}${LINUX_ARCH}.AppImage
                mv feather.AppImage "${APPIMAGENAME}"
                cp "${APPIMAGENAME}" "${INSTALLPATH}/"
                cp "${APPIMAGENAME}" "${OUTDIR}/"
            fi
            ;;
    esac

    mkdir -p "$OUTDIR"

    # Make the os-specific installers
    case "$HOST" in
        *mingw*)
            case "$OPTIONS" in
                installer)
                    makensis -DCUR_PATH=$PWD -V2 contrib/installers/windows/setup.nsi
                    cp contrib/installers/windows/FeatherWalletSetup.exe "${INSTALLPATH}/"
                    mv contrib/installers/windows/FeatherWalletSetup.exe "${OUTDIR}/"
                    ;;
            esac
            ;;
    esac

    # Install built Feather to $INSTALLPATH
    case "$HOST" in
        *darwin*)
            make -C build install/strip ${V:+V=1}
            ;;
        *.installer)

            ;;
        *)
            case "$OPTIONS" in
                installer)
                    # do nothing, we don't want feather.exe in the final .zip
                    ;;
                *)
                    make -C build install ${V:+V=1}
                    ;;
            esac
    esac

    (
        cd installed

        case "$HOST" in
            *darwin*)
                mv "feather.app" "Feather.app"
                mkdir -p Feather.app/Contents/bin
                ;;
        esac

        # Code-signing
        if [[ -n "${TAG}" ]]; then
            if [[ "${TAG}" != *"-rc"* ]]; then
                case "$HOST" in
                    *darwin*)
                        signapple apply Feather.app "/distsrc/external/feather-codesigning/signatures/${HOST}/Feather.app"
                        ;;
                    *mingw*)
                        case "$OPTIONS" in
                            installer)
                                osslsigncode attach-signature \
                                                 -in "FeatherWalletSetup.exe" \
                                                 -out "FeatherWalletSetup.exe.tmp" \
                                                 -CAfile "$GUIX_ENVIRONMENT/etc/ssl/certs/ca-certificates.crt" \
                                                 -sigin "/distsrc/external/feather-codesigning/signatures/${HOST}/feather.installer.pem" || true
                                cp FeatherWalletSetup.exe.tmp "${OUTDIR}/FeatherWalletSetup-${TAG}.exe"
                                mv FeatherWalletSetup.exe.tmp "FeatherWalletSetup-${TAG}.exe"
                                rm FeatherWalletSetup.exe
                                rm "${OUTDIR}/FeatherWalletSetup.exe"
                                ;;
                            "")
                                osslsigncode attach-signature \
                                                 -in "feather.exe" \
                                                 -out "feather.exe.tmp" \
                                                 -CAfile "$GUIX_ENVIRONMENT/etc/ssl/certs/ca-certificates.crt" \
                                                 -sigin "/distsrc/external/feather-codesigning/signatures/${HOST}/feather.pem" || true
                                mv feather.exe.tmp feather.exe
                                ;;
                        esac
                        ;;
                esac
            fi
        fi

        # Finally, deterministically produce {non-,}debug binary tarballs ready
        # for release
        case "$HOST" in
            *mingw*)
                if [[ "${TAG}" == *"-rc"* ]]; then
                    if [ -z "$OPTIONS" ]; then
                        mv feather.exe "${OUTDIR}/${DISTNAME}.exe"
                    fi
                else
                      case "$OPTIONS" in
                          installer)
                              find . -print0 \
                                  | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
                              find . \
                                  | sort \
                                  | zip -X@ "${OUTDIR}/${DISTNAME}-win-installer.zip" \
                                  || ( rm -f "${OUTDIR}/${DISTNAME}-win-installer.zip" && exit 1 )
                              ;;
                          "")
                              mv feather.exe ${DISTNAME}.exe && \
                              find . -print0 \
                                  | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
                              find . \
                                  | sort \
                                  | zip -X@ "${OUTDIR}/${DISTNAME}-win.zip" \
                                  || ( rm -f "${OUTDIR}/${DISTNAME}-win.zip" && exit 1 )
                              ;;
                      esac
                fi
                ;;
            *linux*)
                if [ "$OPTIONS" != "pack" ]; then
                    mv feather "${DISTNAME}"
                    case "$OPTIONS" in
                        "")
                            find . -not -name "*.AppImage" -print0 \
                                | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
                            find . -not -name "*.AppImage" \
                                | sort \
                                | zip -X@ "${OUTDIR}/${DISTNAME}-linux${LINUX_ARCH}.zip" \
                                || ( rm -f "${OUTDIR}/${DISTNAME}-linux${LINUX_ARCH}.zip" && exit 1 )
                            ;;
                    esac
                    find . -name "*.AppImage" -print0 \
                        | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
                    find . -name "*.AppImage" \
                        | sort \
                        | zip -X@ "${OUTDIR}/${DISTNAME}-linux${LINUX_ARCH}-appimage.zip" \
                        || ( rm -f "${OUTDIR}/${DISTNAME}-linux${LINUX_ARCH}-appimage.zip" && exit 1 )
                else
                    find . -print0 \
                        | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
                    cp feather "${OUTDIR}"
                fi
                ;;
            *darwin*)
                find . -print0 \
                    | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
                find . \
                    | sort \
                    | zip -X@ "${OUTDIR}/${DISTNAME}-mac${DARWIN_ARCH}.zip" \
                    || ( rm -f "${OUTDIR}/${DISTNAME}-mac${DARWIN_ARCH}.zip" && exit 1 )
                ;;
        esac

    )
)  # $DISTSRC

rm -rf "$ACTUAL_OUTDIR"
mv --no-target-directory "$OUTDIR" "$ACTUAL_OUTDIR" \
    || ( rm -rf "$ACTUAL_OUTDIR" && exit 1 )

(
    cd /outdir-base
    {
        echo "$GIT_ARCHIVE"
        find "$ACTUAL_OUTDIR" -type f -not -name "*.txt"
    } | xargs realpath --relative-base="$PWD" \
      | xargs sha256sum \
      | sort -k2 -o "$LOGDIR"/SHA256SUMS.part
)
