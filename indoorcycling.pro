TEMPLATE = subdirs
CONFIG += debug
include(config.pri)

SUBDIRS += \
    mainlib \
    indoorcycling2 \
    antlib \
    anttestapp \
    test \
    videotest

indoorcycling2.depends = antlib mainlib
anttestapp.depends = antlib
test.depends = antlib mainlib
