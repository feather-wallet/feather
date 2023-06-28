(use-modules
  (gnu packages)
  (gnu packages certs)
  (gnu packages fontutils)
  (gnu packages zig)
  (guix build-system gnu)
  (guix download)
  ((guix licenses) #:prefix license:)
  (guix packages)
  (guix utils))

(define-public ln-guix-store
  (package
    (name "ln-guix-store")
    (version "a148fb86c30968eeb30dc6ac3384ad2a16690520")
    (source (origin
              (method url-fetch)
              (uri (string-append
                     "https://github.com/tobtoht/ln-guix-store/archive/" version ".tar.gz"))
              (sha256 (base32 "0nrncjix2c18px2cm67acz7c15hji2dl6ynsfh1v1z7rlk6lysg9"))))
    (build-system gnu-build-system)
    (arguments
      (list
        #:make-flags
        #~(list (string-append "PREFIX=" #$output)
                (string-append "CC=" #$(cc-for-target)))
        #:phases
        #~(modify-phases %standard-phases
            (delete 'configure)      ; No configure script.
            (add-before 'build 'pre-build
              (lambda _
                (setenv "ZIG_GLOBAL_CACHE_DIR"
                  (mkdtemp "/tmp/zig-cache-XXXXXX"))))
            (delete 'check))))
    (native-inputs
      (list zig))
    (synopsis "Symlink /app/gnu to /gnu")
    (description "Tiny program to symlink /app/gnu to /gnu inside a Flatpak")
    (license license:bsd-3)
    (home-page "https://featherwallet.org/")))

(define utf8-locales
  (make-glibc-utf8-locales
    glibc
    #:locales (list "en_US")
    #:name "utf8-locales"))

(packages->manifest
  (append
    (list nss-certs
          fontconfig
          ln-guix-store
          utf8-locales)))