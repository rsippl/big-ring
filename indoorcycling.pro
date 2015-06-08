TEMPLATE = subdirs
include(config.pri)

SUBDIRS += \
    mainlib \
    indoorcycling2 \
    antlib \
    anttestapp \
    test 

indoorcycling2.depends = antlib mainlib
anttestapp.depends = antlib
test.depends = antlib mainlib
