(use-modules
  (gnu packages)
  (gnu packages nss)
  (gnu packages fontutils)
  (guix build-system gnu)
  (guix download)
  ((guix licenses) #:prefix license:)
  (guix packages)
  (guix utils)
  (gnu packages xorg)
  )

(define utf8-locales
  (make-glibc-utf8-locales
    glibc
    #:locales (list "en_US")
    #:name "utf8-locales"))

(packages->manifest
  (append
    (list nss-certs
          fontconfig
          utf8-locales
          xkeyboard-config)))
