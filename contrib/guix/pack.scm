(use-modules (gnu)
  (gnu packages)
  (gnu packages base)
  (gnu packages bash)
  (gnu packages certs)
  (gnu packages check)
  (gnu packages compression)
  (gnu packages elf)
  (gnu packages fonts)
  (gnu packages fontutils)
  (gnu packages gcc)
  (gnu packages gettext)
  (gnu packages glib)
  (gnu packages libusb)
  (gnu packages linux)
  (gnu packages moreutils)
  (gnu packages shells)
  (gnu packages tls)
  (gnu packages xorg)
  (gnu packages xdisorg)
  (guix build-system gnu)
  (guix download)
  ((guix licenses) #:prefix license:)
  (guix packages)
  (guix profiles)
  (guix utils))

(define-public feather-binary
  (package
    (name "feather-binary")
    (version (getenv "VERSION"))
    (source (origin
              (method url-fetch)
              (uri (getenv "FILE"))
              (sha256 (base32 (getenv "HASH")))))
    (build-system gnu-build-system)
    (propagated-inputs
      (list nss-certs
            dbus
            coreutils-minimal))
    (inputs (list fontconfig
                  (list gcc "lib")
                  glibc
                  libxkbcommon
                  libxcb
                  xcb-util-cursor
                  xcb-util-wm
                  xcb-util-image
                  xcb-util-keysyms
                  xcb-util-renderutil))
    (arguments
      (list
        #:strip-binaries? #f
        #:phases
        #~(modify-phases %standard-phases
            (delete 'bootstrap)
            (delete 'configure)
            (delete 'build)
            (delete 'check)
            (add-before 'install 'patchelff
              (lambda* (#:key inputs outputs propagated-inputs #:allow-other-keys)
                (let ((interpreter (string-append (assoc-ref inputs "glibc") "/lib/ld-linux-x86-64.so.2"))
                       (binary "feather")
                       (runpath '("gcc"
                                   "glibc"
                                   "dbus"
                                   "fontconfig-minimal"
                                   "libxkbcommon"
                                   "libxcb"
                                   "xcb-util-cursor"
                                   "xcb-util-wm"
                                   "xcb-util-image"
                                   "xcb-util-keysyms"
                                   "xcb-util-renderutil")))

                  (define* (maybe-make-rpath entries name)
                    (let ((entry (assoc-ref entries name)))
                      (if entry
                        (string-append entry "/lib")
                        #f)))

                  (define* (make-rpath name)
                    (or
                      (maybe-make-rpath inputs name)
                      (maybe-make-rpath propagated-inputs name)
                      (error (format #f "`~a' not found among the inputs nor the outputs." name))))

                  (system* "patchelf" "--set-interpreter" interpreter binary)
                  (let ((rpath (string-join
                                 (map make-rpath runpath)
                                 ":")))
                    (invoke "patchelf" "--set-rpath" rpath binary))

                  #t)))
            (replace 'install
              (lambda* (#:key outputs #:allow-other-keys)
                (let ((target (string-append (assoc-ref outputs "out") "/bin/feather")))
                  (mkdir-p (string-append (assoc-ref outputs "out") "/bin/"))
                  (copy-file "./feather" target)))))))
    (native-inputs (list
                     patchelf
                     unzip))
    (home-page "https://featherwallet.org/")
    (synopsis "A free, open-source Monero wallet")
    (description
      "Feather is a free, open-source Monero wallet for Linux, Tails, Windows and macOS.")
    (license license:bsd-3)))

(define utf8-locales
  (make-glibc-utf8-locales
    glibc
    #:locales (list "en_US")
    #:name "utf8-locales"))

(packages->manifest
  (append
    (list feather-binary
          strace
          utf8-locales
          font-wqy-zenhei
          font-gnu-unifont)))