# Create the super cache so modules will add themselves to it.
cache(, super)

!QTDIR_build: cache(CONFIG, add, $$list(QTDIR_build))

#prl = no_install_prl
#CONFIG += $$prl
cache(CONFIG, add stash, prl)

TEMPLATE = subdirs
SUBDIRS = qtbase qttools qttranslations qtsvg qtwebsockets

qtwebsockets.depends = qtbase
qtsvg.depends = qtbase
qttools.depends = qtbase
qttranslations.depends = qttools

load(qt_configure)
