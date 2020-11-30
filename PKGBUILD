# Maintainer: wowario <wowario[at]protonmail[dot]com>
# Contributor: wowario <wowario[at]protonmail[dot]com>

pkgname='monero-feather-git'
pkgver=0.1.0.925ef5683
pkgrel=1
pkgdesc='a free Monero desktop wallet'
license=('BSD')
arch=('x86_64')
url="https://featherwallet.org"
depends=('boost-libs' 'libunwind' 'openssl' 'readline' 'pcsclite' 'hidapi' 'protobuf' 'miniupnpc' 'libgcrypt' 'qrencode' 'libsodium' 'libpgm' 'expat' 'qt5-base' 'qt5-websockets' 'tor')
makedepends=('git' 'cmake' 'boost')

source=("${pkgname}"::"git+https://git.wownero.com/feather/feather")

sha256sums=('SKIP')

pkgver() {
  cd "${srcdir}/${pkgname}"
  printf "%s.%s" "$(git describe --tags --abbrev=0)" "$(git rev-parse --short=9 HEAD)"
}

build() {
  cd "${srcdir}/${pkgname}"
  git submodule update --init --recursive
  mkdir -p build
  cd build
  cmake ..
  make
}

package_monero-feather-git() {
  install -Dm644 "${srcdir}/${pkgname}/LICENSE" "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
  install -Dm755 "${srcdir}/${pkgname}/build/bin/feather" "${pkgdir}/usr/bin/feather"
}
