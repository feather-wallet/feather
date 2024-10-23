#!/usr/bin/env bash
# Copyright (c) 2021-2022 The Bitcoin Core developers
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
    UNSIGNED_TARBALL: ${UNSIGNED_TARBALL:?not set}
    DETACHED_SIGS_REPO: ${DETACHED_SIGS_REPO:?not set}
    DIST_ARCHIVE_BASE: ${DIST_ARCHIVE_BASE:?not set}
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
        *mingw*)
            find "$PWD" -name "*-unsigned.exe" | while read -r infile; do
                infile_base="$(basename "$infile")"

                # Codesigned *-unsigned.exe and output to OUTDIR
                osslsigncode attach-signature \
                                 -in "$infile" \
                                 -out "${OUTDIR}/${infile_base/-unsigned}" \
                                 -CAfile "$GUIX_ENVIRONMENT/etc/ssl/certs/ca-certificates.crt" \
                                 -sigin /detached-sigs/codesignatures/"${VERSION}"/"$infile_base".pem
            done
            ;;
        *)
            exit 1
            ;;
    esac
)  # $DISTSRC

rm -rf "$ACTUAL_OUTDIR"
mv --no-target-directory "$OUTDIR" "$ACTUAL_OUTDIR" \
    || ( rm -rf "$ACTUAL_OUTDIR" && exit 1 )

(
    cd /outdir-base
    {
        echo "$UNSIGNED_TARBALL"
        echo "$CODESIGNATURE_GIT_ARCHIVE"
        find "$ACTUAL_OUTDIR" -type f
    } | xargs realpath --relative-base="$PWD" \
        | xargs sha256sum \
        | sort -k2 \
        | sponge "$LOGDIR"/codesigned/SHA256SUMS.part
)
