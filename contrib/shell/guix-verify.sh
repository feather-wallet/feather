#!/usr/bin/env bash

set -e

# Convenience wrapper around contrib/guix/guix-verify

if [ -z "${GUIX_SIGS_REPO}" ]; then
    printf "Enter path to 'feather-sigs' repo: "
    read -r repo

    if [ ! -d "${repo}" ]; then
        echo "Directory does not exist"
        exit 1
    fi

    export GUIX_SIGS_REPO="$repo"
fi

if [ -z "${SIGNER}" ]; then
    printf "Enter name of signer: "
    read -r signer
    export SIGNER="$signer"
fi

./contrib/guix/guix-verify
