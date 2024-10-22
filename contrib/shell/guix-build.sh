#!/usr/bin/env bash

source contrib/guix/libexec/hosts.sh

echo "Available targets:"
echo "---"
echo "0) build all targets"
for i in "${!DEFAULT_HOSTS[@]}"; do
    echo "$((i+1))) ${DEFAULT_HOSTS[$i]}"
done

echo "---"
read -p "Select target to build [0]: "

./contrib/guix/guix-build
