#!/usr/bin/env bash
# Copyright (c) 2021-2022 The Bitcoin Core developers
# Copyright (c) 2024-2024 The Monero Project
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
export LC_ALL=C
set -e -o pipefail
export TZ=UTC

# Although Guix _does_ set umask when building its own packages (in our case,
# this is all packages in manifest.scm), it does not set it for `guix
# shell`. It does make sense for at least `guix shell --container`
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
    UNSIGNED_FILE: ${UNSIGNED_FILE:?not set}
    GUIX_SIGS_REPO: ${GUIX_SIGS_REPO:?not set}
    DISTNAME: ${DISTNAME:?not set}
    VERSION: ${VERSION:?not set}
    HOST: ${HOST:?not set}
    SOURCE_DATE_EPOCH: ${SOURCE_DATE_EPOCH:?not set}
    DISTSRC: ${DISTSRC:?not set}
    OUTDIR: ${OUTDIR:?not set}
    LOGDIR: ${LOGDIR:?not set}
EOF

ACTUAL_OUTDIR="${OUTDIR}"
OUTDIR="${DISTSRC}/output"

git_head_version() {
    local recent_tag
    if recent_tag="$(git -C "$1" describe --exact-match HEAD 2> /dev/null)"; then
        echo "${recent_tag#v}"
    else
        git -C "$1" rev-parse --short=12 HEAD
    fi
}

mkdir -p "$OUTDIR"

mkdir -p "$DISTSRC"
(
    cd "$DISTSRC"

    case "$HOST" in
        *mingw32*)
            infile_base="$(basename "$UNSIGNED_FILE")"
            outfile_base="${infile_base/-unsigned}"

            # Codesigned *-unsigned.exe and output to OUTDIR
            osslsigncode attach-signature \
                             -in "$UNSIGNED_FILE" \
                             -out "${OUTDIR}/$outfile_base" \
                             -CAfile "$GUIX_ENVIRONMENT/etc/ssl/certs/ca-certificates.crt" \
                             -sigin /guix-sigs/codesignatures/"${VERSION}"/"$outfile_base".pem
            ;;
        *)
            exit 1
            ;;
    esac
)  # $DISTSRC


(
    cd "$OUTDIR"

    case "$HOST" in
        *mingw32.installer)
             find . -print0 \
                 | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
             find . \
                 | sort \
                 | zip -X@ "${OUTDIR}/${DISTNAME}-win-installer.zip" \
                 || ( rm -f "${OUTDIR}/${DISTNAME}-win-installer.zip" && exit 1 )
             ;;
        *mingw32*)
             find . -print0 \
                 | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
             find . \
                 | sort \
                 | zip -X@ "${OUTDIR}/${DISTNAME}-win.zip" \
                 || ( rm -f "${OUTDIR}/${DISTNAME}-win.zip" && exit 1 )
             ;;
    esac
)

rm -rf "$ACTUAL_OUTDIR"
mv --no-target-directory "$OUTDIR" "$ACTUAL_OUTDIR" \
    || ( rm -rf "$ACTUAL_OUTDIR" && exit 1 )

(
    cd /outdir-base
    mkdir -p "$LOGDIR"/codesigned
    {
        find "$ACTUAL_OUTDIR" -type f
    } | xargs realpath --relative-base="$PWD" \
        | xargs sha256sum \
        | sort -k2 \
        | sponge "$LOGDIR"/SHA256SUMS.part
)
