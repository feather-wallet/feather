#!/usr/bin/env bash
set -ex

mkdir empty
cd empty

mkdir export files usr

# Needs to exists, otherwise flatpak doesn't mount zoneinfo
# https://github.com/flatpak/flatpak/blob/8b4f523c4f8287d57f1a84a3a8216efe200c5fbf/common/flatpak-run.c#L1605
mkdir -p usr/share/zoneinfo

cat << EOF > metadata
[Runtime]
name=org.featherwallet.Empty
runtime=org.featherwallet.Empty/x86_64/empty
sdk=org.featherwallet.Empty/x86_64/empty
EOF