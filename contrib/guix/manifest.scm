(use-modules (gnu)
             (gnu packages)
             (gnu packages autotools)
             (gnu packages assembly)
             (gnu packages base)
             (gnu packages bash)
             (gnu packages bison)
             (gnu packages certs)
             (gnu packages check)
             (gnu packages cmake)
             (gnu packages commencement)
             (gnu packages compression)
             (gnu packages cross-base)
             (gnu packages curl)
             (gnu packages elf)
             (gnu packages file)
             (gnu packages gawk)
             (gnu packages gcc)
             (gnu packages gettext)
             (gnu packages glib)
             (gnu packages gnome)
             (gnu packages gperf)
             (gnu packages man)
             (gnu packages installers)
             (gnu packages libusb)
             (gnu packages linux)
             (gnu packages llvm)
             (gnu packages m4)
             (gnu packages mingw)
             (gnu packages moreutils)
             (gnu packages package-management)
             (gnu packages perl)
             (gnu packages pkg-config)
             (gnu packages python)
             (gnu packages python-crypto)
             (gnu packages python-web)
             (gnu packages serialization)
             (gnu packages shells)
             (gnu packages tls)
             (gnu packages version-control)
             (gnu packages xorg)
             (guix build-system gnu)
             (guix build-system meson)
             (guix build-system perl)
             (guix build-system python)
             (guix build-system trivial)
             (guix download)
             (guix gexp)
             (guix git-download)
             ((guix licenses) #:prefix license:)
             (guix packages)
             (guix profiles)
             (guix utils))

(define-syntax-rule (search-our-patches file-name ...)
  "Return the list of absolute file names corresponding to each
FILE-NAME found in ./patches relative to the current file."
  (parameterize
      ((%patch-path (list (string-append (dirname (current-filename)) "/patches"))))
    (list (search-patch file-name) ...)))

;(define (make-ssp-fixed-gcc xgcc)
;  "Given a XGCC package, return a modified package that uses the SSP function
;from glibc instead of from libssp.so. Our `symbol-check' script will complain if
;we link against libssp.so, and thus will ensure that this works properly.
;
;Taken from:
;http://www.linuxfromscratch.org/hlfs/view/development/chapter05/gcc-pass1.html"
;  (package
;    (inherit xgcc)
;    (arguments
;     (substitute-keyword-arguments (package-arguments xgcc)
;       ((#:make-flags flags)
;        `(cons "gcc_cv_libc_provides_ssp=yes" ,flags))))))

(define-public mingw-w64-x86_64-winpthreads-10.0.0
  (package (inherit mingw-w64-x86_64-winpthreads)
    (name "mingw-w64-x86_64-winpthreads-10.0.0")
    (version "10.0.0")
    (source
      (origin
        (method url-fetch)
        (uri (string-append
              "mirror://sourceforge/mingw-w64/mingw-w64/"
              "mingw-w64-release/mingw-w64-v" version ".tar.bz2"))
        (sha256
        (base32 "15089y4rlj6g1m2m3cm3awndw3rbzhznl7skd0vkmikjxl546sxs"))
        (patches
          (search-patches "mingw-w64-6.0.0-gcc.patch"
                          "mingw-w64-dlltool-temp-prefix.patch"
                          "mingw-w64-reproducible-gendef.patch"))))
    (arguments
      (substitute-keyword-arguments (package-arguments mingw-w64-x86_64-winpthreads)
               ((#:parallel-build? _ #f) #f)))))

(define (make-gcc-rpath-link xgcc)
  "Given a XGCC package, return a modified package that replace each instance of
-rpath in the default system spec that's inserted by Guix with -rpath-link"
  (package
    (inherit xgcc)
    (arguments
     (substitute-keyword-arguments (package-arguments xgcc)
       ((#:phases phases)
        `(modify-phases ,phases
           (add-after 'pre-configure 'replace-rpath-with-rpath-link
             (lambda _
               (substitute* (cons "gcc/config/rs6000/sysv4.h"
                                  (find-files "gcc/config"
                                              "^gnu-user.*\\.h$"))
                 (("-rpath=") "-rpath-link="))
               #t))))))))

(define (make-cross-toolchain target
                              base-gcc-for-libc
                              base-kernel-headers
                              base-libc
                              base-gcc)
  "Create a cross-compilation toolchain package for TARGET"
  (let* ((xbinutils (cross-binutils target))
         ;; 1. Build a cross-compiling gcc without targeting any libc, derived
         ;; from BASE-GCC-FOR-LIBC
         (xgcc-sans-libc (cross-gcc target
                                    #:xgcc base-gcc-for-libc
                                    #:xbinutils xbinutils))
         ;; 2. Build cross-compiled kernel headers with XGCC-SANS-LIBC, derived
         ;; from BASE-KERNEL-HEADERS
         (xkernel (cross-kernel-headers target
                                        base-kernel-headers
                                        xgcc-sans-libc
                                        xbinutils))
         ;; 3. Build a cross-compiled libc with XGCC-SANS-LIBC and XKERNEL,
         ;; derived from BASE-LIBC
         (xlibc (cross-libc target
                            base-libc
                            xgcc-sans-libc
                            xbinutils
                            xkernel))
         ;; 4. Build a cross-compiling gcc targeting XLIBC, derived from
         ;; BASE-GCC
         (xgcc (cross-gcc target
                          #:xgcc base-gcc
                          #:xbinutils xbinutils
                          #:libc xlibc)))
    ;; Define a meta-package that propagates the resulting XBINUTILS, XLIBC, and
    ;; XGCC
    (package
      (name (string-append target "-toolchain"))
      (version (package-version xgcc))
      (source #f)
      (build-system trivial-build-system)
      (arguments '(#:builder (begin (mkdir %output) #t)))
      (propagated-inputs
       `(("binutils" ,xbinutils)
         ("libc" ,xlibc)
         ("libc:static" ,xlibc "static")
         ("gcc" ,xgcc)
         ("gcc-lib" ,xgcc "lib")))
      (synopsis (string-append "Complete GCC tool chain for " target))
      (description (string-append "This package provides a complete GCC tool
chain for " target " development."))
      (home-page (package-home-page xgcc))
      (license (package-license xgcc)))))

(define base-gcc gcc-10)
(define base-linux-kernel-headers linux-libre-headers-5.15)

;; https://gcc.gnu.org/install/configure.html
(define (hardened-gcc gcc)
  (package-with-extra-configure-variable (
    package-with-extra-configure-variable gcc
    "--enable-default-ssp" "yes")
    "--enable-default-pie" "yes"))

(define* (make-bitcoin-cross-toolchain target
                                       #:key
                                       (base-gcc-for-libc base-gcc)
                                       (base-kernel-headers base-linux-kernel-headers)
                                       (base-libc (hardened-glibc (make-glibc-without-werror glibc-2.27)))
                                       (base-gcc (make-gcc-rpath-link (hardened-gcc base-gcc))))
  "Convenience wrapper around MAKE-CROSS-TOOLCHAIN with default values
desirable for building Feather Wallet release binaries."
  (make-cross-toolchain target
                        base-gcc-for-libc
                        base-kernel-headers
                        base-libc
                        base-gcc))

(define (make-gcc-with-pthreads gcc)
  (package-with-extra-configure-variable
    (package-with-extra-patches gcc
      (search-our-patches "gcc-10-remap-guix-store.patch"))
    "--enable-threads" "posix"))

(define (make-mingw-w64-cross-gcc cross-gcc)
  (package-with-extra-patches cross-gcc
    (search-our-patches "vmov-alignment.patch"
                        "gcc-broken-longjmp.patch")))

(define (make-mingw-pthreads-cross-toolchain target)
  "Create a cross-compilation toolchain package for TARGET"
  (let* ((xbinutils (cross-binutils target))
         (pthreads-xlibc mingw-w64-x86_64-winpthreads-10.0.0)
         (pthreads-xgcc (make-gcc-with-pthreads
                         (cross-gcc target
                                    #:xgcc (make-mingw-w64-cross-gcc base-gcc)
                                    #:xbinutils xbinutils
                                    #:libc pthreads-xlibc))))
    ;; Define a meta-package that propagates the resulting XBINUTILS, XLIBC, and
    ;; XGCC
    (package
      (name (string-append target "-posix-toolchain"))
      (version (package-version pthreads-xgcc))
      (source #f)
      (build-system trivial-build-system)
      (arguments '(#:builder (begin (mkdir %output) #t)))
      (propagated-inputs
       `(("binutils" ,xbinutils)
         ("libc" ,pthreads-xlibc)
         ("gcc" ,pthreads-xgcc)
         ("gcc-lib" ,pthreads-xgcc "lib")))
      (synopsis (string-append "Complete GCC tool chain for " target))
      (description (string-append "This package provides a complete GCC tool
chain for " target " development."))
      (home-page (package-home-page pthreads-xgcc))
      (license (package-license pthreads-xgcc)))))

(define (make-nsis-for-gcc-10 base-nsis)
  (package-with-extra-patches base-nsis
    (search-our-patches "nsis-gcc-10-memmove.patch"
                        "nsis-disable-installer-reloc.patch")))

(define (make-glibc-without-werror glibc)
  (package-with-extra-configure-variable glibc "enable_werror" "no"))

;; https://www.gnu.org/software/libc/manual/html_node/Configuring-and-compiling.html
(define (hardened-glibc glibc)
  (package-with-extra-configure-variable (
    package-with-extra-configure-variable glibc
    "--enable-stack-protector" "all")
    "--enable-bind-now" "yes"))

(define-public glibc-2.27
  (package
    (inherit glibc-2.31)
    (version "2.27")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://sourceware.org/git/glibc.git")
                    (commit "73886db6218e613bd6d4edf529f11e008a6c2fa6")))
              (file-name (git-file-name "glibc" "73886db6218e613bd6d4edf529f11e008a6c2fa6"))
              (sha256
               (base32
                "0azpb9cvnbv25zg8019rqz48h8i2257ngyjg566dlnp74ivrs9vq"))
              (patches (search-our-patches "glibc-ldd-x86_64.patch"
                                           "glibc-versioned-locpath.patch"
                                           "glibc-2.27-riscv64-Use-__has_include-to-include-asm-syscalls.h.patch"
                                           "glibc-2.27-fcommon.patch"
                                           "glibc-2.27-guix-prefix.patch"))))))

(define-public ldid
  (package
    (name "ldid")
    (version "v2.1.5-procursus7")
    (source (origin
              (method url-fetch)
              (uri (string-append "https://github.com/ProcursusTeam/"
                     name "/archive/refs/tags/" version ".tar.gz"))
              (sha256
                (base32
                  "0ppzy4d9sl4m0sn8nk8wpi39qfimvka6h2ycr67y8r97y3363r04"))))
    (build-system gnu-build-system)
    (arguments
      `(#:phases
         (modify-phases %standard-phases
           (delete 'configure)
           (replace 'build (lambda _ (invoke "make")))
           (delete 'check)
           (replace 'install
             (lambda* (#:key outputs #:allow-other-keys)
               (let* ((out (assoc-ref outputs "out"))
                       (bin (string-append out "/bin")))
                 (install-file "ldid" bin)
                 #t)))
           )))
    (native-inputs (list pkg-config))
    (inputs (list openssl libplist))
    (home-page "https://github.com/ProcursusTeam/ldid")
    (synopsis "Link Identity Editor.")
    (description "Put real or fake signatures in a Mach-O.")
    (license license:gpl3+)))

(define-public debugedit
  (package
    (name "debugedit")
    (version "5.0")
    (source
      (origin
        (method git-fetch)
        (uri (git-reference
               (url "https://sourceware.org/git/debugedit.git")
               (commit (string-append name "-" version) )))
        (file-name (git-file-name name version))
        (sha256
          (base32 "1jxiizzzvx89dhs99aky48kl5s49i5zr9d7j4753gp0knk4pndjm"))))
    (build-system gnu-build-system)
    (arguments '(#:tests? #f))
    (propagated-inputs (list elfutils))
    (inputs (list zlib xz))
    (native-inputs
      (list
        autoconf automake m4 util-linux libtool help2man pkg-config))
    (home-page "https://sourceware.org/git/debugedit.git")
    (synopsis "Tool for debugging")
    (description
      "The debugedit project provides programs and scripts for creating
 debuginfo and source file distributions, collect build-ids and rewrite
 source paths in DWARF data for debugging, tracing and profiling.

 It is based on code originally from the rpm project plus libiberty and
 binutils.  It depends on the elfutils libelf and libdw libraries to
 read and write ELF files, DWARF data and build-ids.")
    (license license:lgpl2.1)))

(define-public flatpak-builder
  (package
    (name "flatpak-builder")
    (version "1.3.3")
    (source
      (origin
        (method git-fetch)
        (uri (git-reference
               (url "https://github.com/flatpak/flatpak-builder.git")
               (commit version)
               (recursive? #t)))
        (file-name (git-file-name name version))
        (sha256
          (base32 "01pvkayamvgqrv62msqncz61xd0w8h3rr3bycy408s69j3zhbpsd"))))
    (build-system gnu-build-system)
    (arguments
      '(#:configure-flags
         (list
           "--enable-documentation=no"
           "--with-system-debugedit")
         #:phases
         (modify-phases %standard-phases
           ;; Test are supposed to be done in /var/tmp because of the need for
           ;; xattrs. Nonetheless, moving it back to /tmp makes tests suceed.
           (add-before 'check 'allow-tests
             (lambda _
               (substitute* '("buildutil/tap-test" "tests/libtest.sh")
                 (("\\/var\\/tmp\\/")
                   "/tmp/")))))))
    (propagated-inputs (list flatpak debugedit elfutils))
    (inputs
      (list libsoup-minimal-2
            libostree
            json-glib
            curl
            libyaml))
    (native-inputs
      `(("autoconf" ,autoconf)
        ("automake" ,automake)
        ("m4" ,m4)
         ("libtool" ,libtool)
         ("pkg-config" ,pkg-config)
         ("gettext-minimal" ,gettext-minimal)
         ("which" ,which)
        ))
    (home-page "https://github.com/flatpak/flatpak-builder.git")
    (synopsis "Tool to build flatpaks from source")
    (description "@code{flatpak-builder} is a wrapper around the flatpak build
command that automates the building of applications and their dependencies.
It is one option you can use to build applications.

The goal of flatpak-builder is to push as much knowledge about how to build
modules to the individual upstream projects.  An invocation of flatpak-builder
proceeds in these stages, each being specified in detail in json format in
the file MANIFEST :

@itemize
@item Download all sources
@item Initialize the application directory with flatpak build-init
@item Build and install each module with flatpak build
@item Clean up the final build tree by removing unwanted files and
e.g. stripping binaries
@item Finish the application directory with flatpak build-finish
@end itemize")
    (license license:lgpl2.1)))

(packages->manifest
 (append
  (list ;; The Basics
        bash
        which
        coreutils-minimal
        util-linux
        ;; File(system) inspection
        file
        grep
        diffutils
        findutils
        ;; File transformation
        patch
        gawk
        sed
        moreutils
        patchelf
        ;; Compression and archiving
        tar
        bzip2
        gzip
        xz
        p7zip
        zip
        unzip
        ;; Build tools
        gnu-make
        libtool
        autoconf-2.71
        automake
        pkg-config
        bison
        gperf
        gettext-minimal
        squashfs-tools
        cmake-minimal
        ;; Native GCC 10 toolchain
        gcc-toolchain-10
        (list gcc-toolchain-10 "static")
        ;; Scripting
        perl
        python-minimal
        ;; Git
        git-minimal
        ;; Xcb
        xcb-util
        xcb-util-cursor
        xcb-util-image
        xcb-util-keysyms
        xcb-util-renderutil
        xcb-util-wm
        ;; Flatpak
        debugedit
        flatpak-builder
    )
  (let ((target (getenv "HOST")))
    (cond ((string-suffix? "-mingw32" target)
           ;; Windows
           (list (make-mingw-pthreads-cross-toolchain "x86_64-w64-mingw32")
                 (make-nsis-for-gcc-10 nsis-x86_64)))
          ((string-contains target "-linux-")
           (list (make-bitcoin-cross-toolchain target)))
          ((string-contains target "darwin")
           (list clang-toolchain-10 binutils ldid))
          (else '())))))
