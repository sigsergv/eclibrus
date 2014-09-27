TEMPLATE = subdirs

# Directories
SUBDIRS += fb2 quazip webdav src

isEmpty(PREFIX) {
PREFIX = /usr
}

DATADIR =$$PREFIX/share

macx {
    update-translations.commands = lupdate src/src.pro
    update-translations-clean.commands = lupdate -noobsolete src/src.pro
    compile-translations.commands = lrelease src/src.pro
}

linux-g++ {
    update-translations.commands = lupdate -qt=5 src/src.pro
    update-translations-clean.commands = lupdate -qt=5 -noobsolete src/src.pro
    compile-translations.commands = lrelease -qt=5 src/src.pro
}

QMAKE_EXTRA_TARGETS = update-translations compile-translations update-translations-clean

translations.depends = compile-translations
translations.path = $$DATADIR/eclibrus/translations
translations.files = translations/*.qm
#make_default.depends += compile-translations

INSTALLS += translations
