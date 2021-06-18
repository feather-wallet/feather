#!/usr/bin/env bash
set -e

cd /deps
for target in bionic bionic-updates bionic-security
do
  pushd "$target"

  # Verify Releases
  gpg --no-default-keyring --keyring /usr/share/keyrings/ubuntu-archive-keyring.gpg --verify Release.gpg Release

  for repo in main multiverse restricted universe
  do
    pushd "$repo"

    # Verify Packages.xz
    sha256=`cat ../Release | grep "$repo/binary-amd64/Packages.xz" | tail -n 1 | awk '{print $1}'`
    echo "$sha256 Packages.xz" | sha256sum -c

    xz -d -c Packages.xz >> ../../Packages-all
    popd
  done
  popd
done

# Verify individual .deb files
cd /archives
for deb in *.deb; do
  file_name=`echo $deb | sed 's/[0-9]*%3a//g'` # --download-only uses this version format sometimes, not sure what that is all about
  sha256=`sed -n "/\/${file_name}$"'/{:start /SHA256: /!{N;b start};//p}' /deps/Packages-all | tail -n 1 | awk '{print $2}'`
  echo "$sha256 $deb" | sha256sum -c
done

dpkg -i --force-depends *.deb