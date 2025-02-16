#!/usr/bin/env bash
export LC_ALL=C
set -ex -o pipefail
export TZ=UTC

APP_ID="org.featherwallet.Feather"

get_store_path() {
    find gnu/store -maxdepth 1 -type d -name "*$1*" | sort | head -n 1
}

mkdir /tmp-output

mkdir -p /tmp-output/flatpak
cd /tmp-output/flatpak

# Create build dir
mkdir build
cd build

mkdir export
cp -r /feather/contrib/flatpak/share export
rm -rf export/share/app-info

# Copy the metadata file
cp /feather/contrib/flatpak/metadata .

mkdir files
cd files

# Copy flatstart binary
cp /feather/contrib/depends/x86_64-linux-gnu/bin/startup .

# Copy feather binary
cp /outdir/feather feather

# Copy metadata
cp -r /feather/contrib/flatpak/share .
touch --no-dereference --date="@${SOURCE_DATE_EPOCH}" share/metainfo/${APP_ID}.metainfo.xml
gzip -c share/metainfo/${APP_ID}.metainfo.xml > share/app-info/xmls/${APP_ID}.xml.gz

# Copy icons
mkdir -p share/icons
cp -r /feather/contrib/flatpak/icons/gnome share/icons

# Extract guix pack
tar xf /pack .

# Get store paths
GUIX_PROFILE=$(get_store_path "profile")
GUIX_GLIBC=$(get_store_path "glibc")
GUIX_FONTCONFIG=$(get_store_path "fontconfig")
GUIX_GCC=$(get_store_path "gcc")
GUIX_KEYBOARD_CONFIG=$(get_store_path "keyboard-config")

# Patch Feather binary
patchelf --set-interpreter "/${GUIX_GLIBC}/lib/ld-linux-x86-64.so.2" feather
patchelf --set-rpath "/${GUIX_GLIBC}/lib:/${GUIX_FONTCONFIG}/lib:\$ORIGIN/lib" feather

# Copy dynamically linked libraries
mkdir lib
cp "${GUIX_GCC}/lib/libgcc_s.so.1" lib/

# Remove unneeded store items
chmod -R 755 .
rm -rf "$(get_store_path "bash-static")"
rm -rf "$(get_store_path "bash-minimal")"
rm -rf "$(get_store_path "gcc")"
rm -rf "$(get_store_path "font-dejavu")"
rm -rf "$(get_store_path "util-linux")"
rm -rf "$(get_store_path "emacs-subdirs")"
rm -rf "$(get_store_path "info-dir")"

rm -rf "${GUIX_GLIBC:?}/share/i18n/locales"
rm -rf "${GUIX_GLIBC:?}/share/i18n/charmaps"
rm -rf "${GUIX_GLIBC:?}/share/locale"
rm -rf "${GUIX_GLIBC:?}/share/info"
rm -rf "${GUIX_GLIBC:?}/lib/gconv"
rm -rf "${GUIX_GLIBC:?}/include"
rm -rf "${GUIX_GLIBC:?}/sbin"
rm -rf "${GUIX_GLIBC:?}/bin"

# Fonts
# fontconfig looks in /app/share/fonts
ln -s "/run/host/fonts" share/fonts
ln -s "/run/host/fonts-cache" share/fonts-cache
ln -s "/${GUIX_PROFILE}/share/locale" share/locale
ln -s "/${GUIX_PROFILE}/share/xml" share/xml

ln -s "/${GUIX_KEYBOARD_CONFIG}/share/X11" share/X11

# Setup profile symlink
ln -s "/${GUIX_PROFILE}" profile

cd /tmp-output

chmod -R 755 .

find . -print0 \
    | xargs -0r touch --no-dereference --date="@${SOURCE_DATE_EPOCH}"
find . \
    | sort \
    | zip -y -X@ "${DISTNAME}-flatpak.zip" \
    || ( rm -f "${DISTNAME}-flatpak.zip" && exit 1 )

mv "${DISTNAME}-flatpak.zip" /output

cd /output
rm feather

sha256sum "${DISTNAME}-flatpak.zip" > "${LOGDIR}/SHA256SUMS.part"