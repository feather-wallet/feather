#!/usr/bin/env bash
set -ex

mkdir empty
cd empty

mkdir export files usr

cat << EOF > metadata
[Runtime]
name=org.featherwallet.Empty
runtime=org.featherwallet.Empty/x86_64/empty
sdk=org.featherwallet.Empty/x86_64/empty
EOF