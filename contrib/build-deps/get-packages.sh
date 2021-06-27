#!/usr/bin/env bash
for target in bionic bionic-updates bionic-security
do
  mkdir "$target"
  pushd "$target"
  target_uri="http://archive.ubuntu.com/ubuntu/dists/$target/"
  wget "$target_uri/Release"
  wget "$target_uri/Release.gpg"
  for repo in main multiverse restricted universe
  do
    mkdir "$repo"
    pushd "$repo"
    wget "$target_uri/$repo/binary-amd64/Packages.xz"
    popd
  done
  popd
done