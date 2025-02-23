(use-modules (gnu packages)
             (gnu packages autotools)
             (gnu packages bash)
             (gnu packages bison)
             ((gnu packages build-tools) #:select (meson))
             ((gnu packages certs) #:select (nss-certs))
             ((gnu packages cmake) #:select (cmake-minimal))
             (gnu packages commencement)
             (gnu packages compression)
             (gnu packages cross-base)
             (gnu packages elf)
             (gnu packages file)
             (gnu packages gawk)
             (gnu packages gcc)
             ((gnu packages gettext) #:select (gettext-minimal))
             ((gnu packages installers) #:select (nsis-x86_64))
             ((gnu packages libusb) #:select (libplist))
             ((gnu packages linux) #:select (linux-libre-headers-6.1 util-linux))
             (gnu packages llvm)
             (gnu packages mingw)
             (gnu packages ninja)
             (gnu packages perl)
             (gnu packages pkg-config)
             ((gnu packages python) #:select (python-minimal))
             ((gnu packages python-build) #:select (python-tomli python-poetry-core))
             ((gnu packages python-crypto) #:select (python-asn1crypto))
             ((gnu packages tls) #:select (openssl))
             ((gnu packages version-control) #:select (git-minimal))
             (gnu packages xorg)
             (guix build-system cmake)
             (guix build-system gnu)
             (guix build-system pyproject)
             (guix build-system python)
             (guix build-system trivial)
             (guix download)
             (guix gexp)
             (guix git-download)
             ((guix licenses) #:prefix license:)
             (guix packages)
             ((guix utils) #:select (substitute-keyword-arguments)))

(define-syntax-rule (search-our-patches file-name ...)
  "Return the list of absolute file names corresponding to each
FILE-NAME found in ./patches relative to the current file."
  (parameterize
      ((%patch-path (list (string-append (dirname (current-filename)) "/patches"))))
    (list (search-patch file-name) ...)))

(define building-on (string-append "--build=" (list-ref (string-split (%current-system) #\-) 0) "-guix-linux-gnu"))

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
                                        #:linux-headers base-kernel-headers
                                        #:xgcc xgcc-sans-libc
                                        #:xbinutils xbinutils))
         ;; 3. Build a cross-compiled libc with XGCC-SANS-LIBC and XKERNEL,
         ;; derived from BASE-LIBC
         (xlibc (cross-libc target
                            #:libc base-libc
                            #:xgcc xgcc-sans-libc
                            #:xbinutils xbinutils
                            #:xheaders xkernel))
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

(define base-gcc gcc-13)
(define base-linux-kernel-headers linux-libre-headers-6.1)

(define* (make-bitcoin-cross-toolchain target
                                       #:key
                                       (base-gcc-for-libc linux-base-gcc)
                                       (base-kernel-headers base-linux-kernel-headers)
                                       (base-libc glibc-2.31)
                                       (base-gcc linux-base-gcc))
  "Convenience wrapper around MAKE-CROSS-TOOLCHAIN with default values
desirable for building Feather Wallet release binaries."
  (make-cross-toolchain target
                        base-gcc-for-libc
                        base-kernel-headers
                        base-libc
                        base-gcc))

(define (gcc-mingw-patches gcc)
  (package-with-extra-patches gcc
    (search-our-patches "gcc-remap-guix-store.patch")))

(define (binutils-mingw-patches binutils)
  (package-with-extra-patches binutils
    (search-our-patches "binutils-unaligned-default.patch")))

(define (winpthreads-patches mingw-w64-x86_64-winpthreads)
  (package-with-extra-patches mingw-w64-x86_64-winpthreads
    (search-our-patches "winpthreads-remap-guix-store.patch")))

(define (make-mingw-pthreads-cross-toolchain target)
  "Create a cross-compilation toolchain package for TARGET"
  (let* ((xbinutils (binutils-mingw-patches (cross-binutils target)))
         (machine (substring target 0 (string-index target #\-)))
         (pthreads-xlibc (winpthreads-patches (make-mingw-w64 machine
                                         #:xgcc (cross-gcc target #:xgcc (gcc-mingw-patches base-gcc))
                                         #:with-winpthreads? #t)))
         (pthreads-xgcc (cross-gcc target
                                    #:xgcc (gcc-mingw-patches mingw-w64-base-gcc)
                                    #:xbinutils xbinutils
                                    #:libc pthreads-xlibc)))
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

(define-public mingw-w64-base-gcc
  (package
    (inherit base-gcc)
    (arguments
      (substitute-keyword-arguments (package-arguments base-gcc)
        ((#:configure-flags flags)
          `(append ,flags
            ;; https://gcc.gnu.org/install/configure.html
            (list "--enable-threads=posix",
                  building-on)))))))

(define-public linux-base-gcc
  (package
    (inherit base-gcc)
    (arguments
      (substitute-keyword-arguments (package-arguments base-gcc)
        ((#:configure-flags flags)
          `(append ,flags
            ;; https://gcc.gnu.org/install/configure.html
            (list "--enable-initfini-array=yes",
                  "--enable-default-ssp=yes",
                  "--enable-default-pie=yes",
                  building-on)))
        ((#:phases phases)
          `(modify-phases ,phases
            ;; Given a XGCC package, return a modified package that replace each instance of
            ;; -rpath in the default system spec that's inserted by Guix with -rpath-link
            (add-after 'pre-configure 'replace-rpath-with-rpath-link
             (lambda _
               (substitute* (cons "gcc/config/rs6000/sysv4.h"
                                  (find-files "gcc/config"
                                              "^gnu-user.*\\.h$"))
                 (("-rpath=") "-rpath-link="))
               #t))))))))

(define-public glibc-2.31
  (let ((commit "8e30f03744837a85e33d84ccd34ed3abe30d37c3"))
  (package
    (inherit glibc) ;; 2.35
    (version "2.31")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://sourceware.org/git/glibc.git")
                    (commit commit)))
              (file-name (git-file-name "glibc" commit))
              (sha256
               (base32
                "1zi0s9yy5zkisw823vivn7zlj8w6g9p3mm7lmlqiixcxdkz4dbn6"))
              (patches (search-our-patches "glibc-guix-prefix.patch"
                                           "glibc-2.31-riscv64-fix-incorrect-jal-with-HIDDEN_JUMPTARGET.patch"))))
    (arguments
      (substitute-keyword-arguments (package-arguments glibc)
        ((#:configure-flags flags)
          `(append ,flags
            ;; https://www.gnu.org/software/libc/manual/html_node/Configuring-and-compiling.html
            (list "--enable-stack-protector=all",
                  "--enable-bind-now",
                  "--disable-werror",
                  building-on)))
    ((#:phases phases)
        `(modify-phases ,phases
           (add-before 'configure 'set-etc-rpc-installation-directory
             (lambda* (#:key outputs #:allow-other-keys)
               ;; Install the rpc data base file under `$out/etc/rpc'.
               ;; Otherwise build will fail with "Permission denied."
               ;; Can be removed when we are building 2.32 or later.
               (let ((out (assoc-ref outputs "out")))
                 (substitute* "sunrpc/Makefile"
                   (("^\\$\\(inst_sysconfdir\\)/rpc(.*)$" _ suffix)
                    (string-append out "/etc/rpc" suffix "\n"))
                   (("^install-others =.*$")
                    (string-append "install-others = " out "/etc/rpc\n")))))))))))))

(define osslsigncode
  (package
    (name "osslsigncode")
    (version "2.9")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                     (url "https://github.com/mtrojnar/osslsigncode")
                     (commit version)))
              (sha256
                (base32
                  "160dwjzpwaxism6r7ryn7dgfq78rk3nkbg9m2kwg512hhn20blqh"))))
    (build-system cmake-build-system)
    (inputs (list openssl zlib))
    (home-page "https://github.com/mtrojnar/osslsigncode")
    (synopsis "Authenticode signing and timestamping tool")
    (description "osslsigncode is a small tool that implements part of the
functionality of the Microsoft tool signtool.exe - more exactly the Authenticode
signing and timestamping. But osslsigncode is based on OpenSSL and cURL, and
thus should be able to compile on most platforms where these exist.")
    (license license:gpl3+))) ; license is with openssl exception

(define-public python-elfesteem
  (let ((commit "2eb1e5384ff7a220fd1afacd4a0170acff54fe56"))
    (package
      (name "python-elfesteem")
      (version (git-version "0.1" "1" commit))
      (source
        (origin
          (method git-fetch)
          (uri (git-reference
                 (url "https://github.com/LRGH/elfesteem")
                 (commit commit)))
          (file-name (git-file-name name commit))
          (sha256
            (base32
              "07x6p8clh11z8s1n2kdxrqwqm2almgc5qpkcr9ckb6y5ivjdr5r6"))))
      (build-system python-build-system)
      ;; There are no tests, but attempting to run python setup.py test leads to
      ;; PYTHONPATH problems, just disable the test
      (arguments '(#:tests? #f))
      (home-page "https://github.com/LRGH/elfesteem")
      (synopsis "ELF/PE/Mach-O parsing library")
      (description "elfesteem parses ELF, PE and Mach-O files.")
      (license license:lgpl2.1))))

(define-public python-oscrypto
  (package
    (name "python-oscrypto")
    (version "1547f535001ba568b239b8797465536759c742a3")
    (source
      (origin
        (method git-fetch)
        (uri (git-reference
               (url "https://github.com/wbond/oscrypto")
               (commit version)))
        (file-name (git-file-name name version))
        (sha256
          (base32
            "1iis5mrgqcapzrrc5p9x4jz5cmrn0lb9di6vrh3pld7dprz5g0qk"))
        (patches (search-our-patches "oscrypto-hard-code-openssl.patch"))))
    (build-system python-build-system)
    (native-search-paths
      (list (search-path-specification
              (variable "SSL_CERT_FILE")
              (file-type 'regular)
              (separator #f)                ;single entry
              (files '("etc/ssl/certs/ca-certificates.crt")))))

    (propagated-inputs
      (list python-asn1crypto openssl))
    (arguments
      `(#:phases
         (modify-phases %standard-phases
           (add-after 'unpack 'hard-code-path-to-libscrypt
             (lambda* (#:key inputs #:allow-other-keys)
               (let ((openssl (assoc-ref inputs "openssl")))
                 (substitute* "oscrypto/__init__.py"
                   (("@GUIX_OSCRYPTO_USE_OPENSSL@")
                     (string-append openssl "/lib/libcrypto.so" "," openssl "/lib/libssl.so")))
                 #t)))
           (add-after 'unpack 'disable-broken-tests
             (lambda _
               ;; This test is broken as there is no keyboard interrupt.
               (substitute* "tests/test_trust_list.py"
                 (("^(.*)class TrustListTests" line indent)
                   (string-append indent
                     "@unittest.skip(\"Disabled by Guix\")\n"
                     line)))
               (substitute* "tests/test_tls.py"
                 (("^(.*)class TLSTests" line indent)
                   (string-append indent
                     "@unittest.skip(\"Disabled by Guix\")\n"
                     line)))
               #t))
           (replace 'check
             (lambda _
               (invoke "python" "run.py" "tests")
               #t)))))
    (home-page "https://github.com/wbond/oscrypto")
    (synopsis "Compiler-free Python crypto library backed by the OS")
    (description "oscrypto is a compilation-free, always up-to-date encryption library for Python.")
    (license license:expat)))

(define-public python-oscryptotests
  (package (inherit python-oscrypto)
    (name "python-oscryptotests")
    (propagated-inputs
      (list python-oscrypto))
    (arguments
      `(#:tests? #f
         #:phases
         (modify-phases %standard-phases
           (add-after 'unpack 'hard-code-path-to-libscrypt
             (lambda* (#:key inputs #:allow-other-keys)
               (chdir "tests")
               #t)))))))

(define-public python-certvalidator
  (let ((commit "a145bf25eb75a9f014b3e7678826132efbba6213"))
    (package
      (name "python-certvalidator")
      (version (git-version "0.1" "1" commit))
      (source
        (origin
          (method git-fetch)
          (uri (git-reference
                 (url "https://github.com/achow101/certvalidator")
                 (commit commit)))
          (file-name (git-file-name name commit))
          (sha256
            (base32
              "1qw2k7xis53179lpqdqyylbcmp76lj7sagp883wmxg5i7chhc96k"))))
      (build-system python-build-system)
      (propagated-inputs
        (list python-asn1crypto
              python-oscrypto
              python-oscryptotests)) ;; certvalidator tests import oscryptotests
      (arguments
        `(#:phases
           (modify-phases %standard-phases
             (add-after 'unpack 'disable-broken-tests
               (lambda _
                 (substitute* "tests/test_certificate_validator.py"
                   (("^(.*)class CertificateValidatorTests" line indent)
                     (string-append indent
                       "@unittest.skip(\"Disabled by Guix\")\n"
                       line)))
                 (substitute* "tests/test_crl_client.py"
                   (("^(.*)def test_fetch_crl" line indent)
                     (string-append indent
                       "@unittest.skip(\"Disabled by Guix\")\n"
                       line)))
                 (substitute* "tests/test_ocsp_client.py"
                   (("^(.*)def test_fetch_ocsp" line indent)
                     (string-append indent
                       "@unittest.skip(\"Disabled by Guix\")\n"
                       line)))
                 (substitute* "tests/test_registry.py"
                   (("^(.*)def test_build_paths" line indent)
                     (string-append indent
                       "@unittest.skip(\"Disabled by Guix\")\n"
                       line)))
                 (substitute* "tests/test_validate.py"
                   (("^(.*)def test_revocation_mode_hard" line indent)
                     (string-append indent
                       "@unittest.skip(\"Disabled by Guix\")\n"
                       line)))
                 (substitute* "tests/test_validate.py"
                   (("^(.*)def test_revocation_mode_soft" line indent)
                     (string-append indent
                       "@unittest.skip(\"Disabled by Guix\")\n"
                       line)))
                 #t))
             (replace 'check
               (lambda _
                 (invoke "python" "run.py" "tests")
                 #t)))))
      (home-page "https://github.com/wbond/certvalidator")
      (synopsis "Python library for validating X.509 certificates and paths")
      (description "certvalidator is a Python library for validating X.509
certificates or paths. Supports various options, including: validation at a
specific moment in time, whitelisting and revocation checks.")
      (license license:expat))))

(define-public python-signapple
  (let ((commit "a9bf003d87e17f744a2791b86348abc5377ebed2"))
    (package
      (name "python-signapple")
      (version (git-version "0.2.0" "1" commit))
      (source
        (origin
          (method git-fetch)
          (uri (git-reference
                 (url "https://github.com/achow101/signapple")
                 (commit commit)))
          (file-name (git-file-name name commit))
          (sha256
            (base32
              "0p250z2ai04a7fgr1b9kmc8samz4bkpqprx3az0k0jp6b310p2nz"))))
      (build-system pyproject-build-system)
      (propagated-inputs
        (list python-asn1crypto
              python-oscrypto
              python-certvalidator
              python-elfesteem))
      (native-inputs (list python-poetry-core))
      ;; There are no tests, but attempting to run python setup.py test leads to
      ;; problems, just disable the test
      (arguments '(#:tests? #f))
      (home-page "https://github.com/achow101/signapple")
      (synopsis "Mach-O binary signature tool")
      (description "signapple is a Python tool for creating, verifying, and
inspecting signatures in Mach-O binaries.")
      (license license:expat))))

(packages->manifest
 (append
  (list ;; The Basics
        bash
        coreutils-minimal ; includes basic shell utilities: cat, cp, echo, mkdir, etc
        which

        ;; File(system) inspection
        file
        grep
        diffutils ; provides diff
        findutils ; provides find and xargs

        ;; File transformation
        patch
        gawk
        sed
        patchelf

        ;; Compression and archiving
        tar
        gzip ; used to unpack most packages in depends
        xz   ; used to unpack some packages in depends
        zip  ; used to create release archives

        ;; Build tools
        gcc-toolchain-13
        (list gcc-toolchain-13 "static")
        gnu-make
        pkg-config
        cmake-minimal
        meson ; used to build libfuse, wayland, libXau, libxkbcommon in depends
        ninja ; used to build qt in depends

        ;; Scripting
        perl           ; required to build openssl in depends
        python-minimal

        ;; Git
        git-minimal ; used to create the release source archive
    )
  (let ((target (getenv "HOST")))
    (cond ((string-contains target "-mingw32")
           ;; Windows
           (list
             (make-mingw-pthreads-cross-toolchain "x86_64-w64-mingw32")
             nsis-x86_64     ;; used to build the installer
             nss-certs
             osslsigncode
             gettext-minimal ;; used to build libgpg-error in depends
             ))
          ((string-contains target "-linux-")
           (list
             (make-bitcoin-cross-toolchain target)
             squashfs-tools
             bison ; used to build libxkbcommon in depends
          ))
          ((string-contains target "darwin")
           (list
             clang-toolchain-18
             lld-18
             (make-lld-wrapper lld-18 #:lld-as-ld? #t)
             python-signapple
             p7zip ;; needed to extract tor_darwin .dmg
             ))
          (else '())))))
