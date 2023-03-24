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
             (gnu packages gnome)
             (gnu packages gperf)
             (gnu packages installers)
             (gnu packages libusb)
             (gnu packages linux)
             (gnu packages llvm)
             (gnu packages mingw)
             (gnu packages moreutils)
             (gnu packages perl)
             (gnu packages pkg-config)
             (gnu packages python)
             (gnu packages python-crypto)
             (gnu packages python-web)
             (gnu packages shells)
             (gnu packages tls)
             (gnu packages version-control)
             (guix build-system gnu)
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

(define (fix-ppc64-nx-default lief)
  (package-with-extra-patches lief
    (search-our-patches "lief-fix-ppc64-nx-default.patch")))

(define-public lief
  (package
   (name "python-lief")
   (version "0.12.1")
   (source
    (origin
     (method git-fetch)
     (uri (git-reference
           (url "https://github.com/lief-project/LIEF.git")
           (commit version)))
     (file-name (git-file-name name version))
     (sha256
      (base32
       "1xzbh3bxy4rw1yamnx68da1v5s56ay4g081cyamv67256g0qy2i1"))))
   (build-system python-build-system)
   (arguments
    `(#:phases
      (modify-phases %standard-phases
        (add-after 'unpack 'parallel-jobs
          ;; build with multiple cores
          (lambda _
            (substitute* "setup.py" (("self.parallel if self.parallel else 1") (number->string (parallel-job-count)))))))))
   (native-inputs
    `(("cmake" ,cmake)))
   (home-page "https://github.com/lief-project/LIEF")
   (synopsis "Library to Instrument Executable Formats")
   (description "Python library to to provide a cross platform library which can
parse, modify and abstract ELF, PE and MachO formats.")
   (license license:asl2.0)))

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

(packages->manifest
 (append
  (list ;; The Basics
        bash
        which
        coreutils
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
        cmake
        ;; Native GCC 10 toolchain
        gcc-toolchain-10
        (list gcc-toolchain-10 "static")
        ;; Scripting
        perl
        python-3
        ;; Git
        git
        ;; Tests
        lief
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
