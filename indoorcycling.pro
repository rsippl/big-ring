TEMPLATE = subdirs
include(config.pri)

SUBDIRS += \
    mainlib \
    indoorcycling2 \
    anttestapp \
    test

indoorcycling2.depends = mainlib
anttestapp.depends = mainlib
test.depends = mainlib
