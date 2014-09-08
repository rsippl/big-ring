TEMPLATE = subdirs

include(indoorcycling.pri)

SUBDIRS += \
    mainlib \
    indoorcycling \
    antlib

!isEmpty(GOOGLE_TEST_SOURCE_DIR) {
    SUBDIRS += test google_test
    test.depends = mainlib antlib indoorcycling google_test
}


indoorcycling.depends = antlib mainlib

