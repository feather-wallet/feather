#!/usr/bin/env bash

base_uri="http://archive.ubuntu.com/ubuntu/dists"
arch="amd64"
if [ "$(uname -m)" = "aarch64" ]; then
  base_uri="http://ports.ubuntu.com/ubuntu-ports/dists"
  arch="arm64"
fi

for target in bionic bionic-updates bionic-security
do
  mkdir "$target"
  pushd "$target"
  target_uri="$base_uri/$target/"
  wget "$target_uri/Release"
  wget "$target_uri/Release.gpg"
  for repo in main multiverse restricted universe
  do
    mkdir "$repo"
    pushd "$repo"
    wget "$target_uri/$repo/binary-$arch/Packages.xz"
    popd
  done
  popd
done