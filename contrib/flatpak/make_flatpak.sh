#!/usr/bin/env bash
set -ex

APP_ID="org.featherwallet.Feather"

get_store_path() {
    find gnu/store -maxdepth 1 -type d -name "*$1*" | sort | head -n 1
}

mkdir -p /output/flatpak
cd /output/flatpak

# Create build dir
mkdir build
cd build

mkdir export
cp -a /feather/contrib/flatpak/share export
rm -rf export/share/app-info

# Copy the metadata file
cp /feather/contrib/flatpak/metadata .

mkdir files
cd files

# Copy metadata
cp -a /feather/contrib/flatpak/share .
touch --no-dereference --date="@${SOURCE_DATE_EPOCH}" share/metainfo/${APP_ID}.metainfo.xml
gzip -c share/metainfo/${APP_ID}.metainfo.xml > share/app-info/xmls/${APP_ID}.xml.gz

# Extract guix pack
tar xf /pack .

# Get store paths
GUIX_BASH_STATIC=$(get_store_path "bash-static")
GUIX_COREUTILS=$(get_store_path "coreutils-minimal")
GUIX_GLIBC=$(get_store_path "glibc")
GUIX_PROFILE=$(get_store_path "profile")

GLIBC_VERSION="${GUIX_GLIBC##*-}"

# Patch ln
LN_PATH="${GUIX_COREUTILS}/bin/ln"

chmod 655 "${LN_PATH}"

patchelf --set-rpath "/app/${GUIX_GLIBC}/lib" "${LN_PATH}"
patchelf --set-interpreter "/app/${GUIX_GLIBC}/lib/ld-linux-x86-64.so.2" "${LN_PATH}"

# Fonts
# fontconfig looks in /app/share/fonts
ln -s "/${GUIX_PROFILE}/share/fonts" share/fonts
ln -s "/${GUIX_PROFILE}/share/locale" share/locale
ln -s "/${GUIX_PROFILE}/share/dbus-1" share/dbus-1
ln -s "/${GUIX_PROFILE}/share/xml" share/xml

chmod 555 "${LN_PATH}"

# create startup.sh
cat << EOF > startup.sh
#!/app/${GUIX_BASH_STATIC}/bin/bash
/app/${LN_PATH} -s /app/gnu /gnu
export PATH="/${GUIX_PROFILE}/bin"
mkdir -p /etc/ssl
# Qt expects certs to be here, see: qtbase/src/network/ssl/qsslsocket.cpp
ln -s /${GUIX_PROFILE}/etc/ssl/certs /etc/ssl/certs
mkdir -p /run/current-system/locale/${GLIBC_VERSION}
ln -s /${GUIX_PROFILE}/lib/locale/${GLIBC_VERSION}/en_US.UTF-8 /run/current-system/locale/${GLIBC_VERSION}
# Feather interpreter is set to /lib64/ld-linux-x86-64.so.2
# Guix pack includes glibc twice if we add glibc to the inputs in flatpak.scm, so this is an alternative to patching
ln -s /${GUIX_GLIBC}/lib /lib64
feather
EOF

chmod 555 startup.sh