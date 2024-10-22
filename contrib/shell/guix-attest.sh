#!/usr/bin/env bash

set -e -o pipefail

# Convenience wrapper around contrib/guix/guix-attest

if [ -z "${GUIX_SIGS_REPO}" ]; then
    echo "[HINT] Fork and clone the feather-sigs repo:"
    echo "https://github.com/feather-wallet/feather-sigs"
    echo ""

    printf "Enter path to 'feather-sigs' repo: "
    read -r repo

    if [ ! -d "${repo}" ]; then
        echo "Directory does not exist"
        exit 1
    fi

    export GUIX_SIGS_REPO="$repo"
fi

if [ -z "${SIGNER}" ]; then
    printf "Enter your GitHub username: "
    read -r signer

    echo ""
    echo "[HINT] To find your GPG fingerprint use:"
    echo "gpg --list-secret-keys --keyid-format=long"
    echo "It should look like: E87BD921CDD885C9D78A38C5E45B10DD027D2472"
    echo ""

    printf "Enter fingerprint of your GPG key: "
    read -r fingerprint

    export SIGNER="${fingerprint}=${signer}"
fi

#echo "To skip these steps, invoke this command as:"
#echo "GUIX_SIGS_REPO=${GUIX_SIGS_REPO} SIGNER=${SIGNER} ./contrib/guix/guix-attest"

./contrib/guix/guix-attest

read -p "Commit changes? [Yn]: "