# Maintainer: wowario <wowario[at]protonmail[dot]com>
# Contributor: wowario <wowario[at]protonmail[dot]com>

pkgbase=('monero-feather-git')
pkgname=('monero-feather-git')
pkgver=v0.1.0.0.cd1cd5cb75
pkgrel=1
pkgdesc="a free Monero desktop wallet"
license=('BSD')
arch=('x86_64')
url="https://featherwallet.org"
depends=('boost-libs' 'libunwind' 'openssl' 'readline' 'zeromq' 'pcsclite' 'hidapi' 'protobuf' 'miniupnpc' 
	'libgcrypt' 'qrencode' 'ccache' 'libsodium' 'libpgm' 'expat' 'base-devel' 'qt5-base')
makedepends=('git' 'cmake' 'boost')
provides=('monero-feather-git')

source=("${pkgname}"::"git+https://git.wownero.com/feather/feather")

sha256sums=('SKIP')

pkgver() {
    cd "$srcdir/$pkgname"
    git describe --long --tags | sed 's/\([^-]*-\)g/r\1/;s/-/./g'
}

build() {
  cd "${srcdir}/${pkgname}"
  mkdir build && cd build && cmake .. && make release-static -j2
}

package_feather-git() {
  install -Dm644 "${srcdir}/${pkgname}/LICENSE" "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
  install -Dm755 "${srcdir}/${pkgname}/build/bin/feather" "${pkgdir}/usr/bin/feather"
}
