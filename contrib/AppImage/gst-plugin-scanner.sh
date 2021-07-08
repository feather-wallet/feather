#!/bin/sh

# Wrapper to launch gst-plugin-scanner inside the AppImage.

HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}:${PATH}"

binary=$(find "$HERE" -name "gst-plugin-scanner-x86_64" | head -n 1)
LD_LINUX=$(find "$HERE/../../../" -name 'ld-*.so.*' | head -n 1)
if [ -e "$LD_LINUX" ] ; then
  case $line in
    "ld-linux"*) exec "${LD_LINUX}" --inhibit-cache "${binary}" "$@" ;;
    *) exec "${LD_LINUX}" "${binary}" "$@" ;;
  esac
else
  exec "${binary}" "$@"
fi