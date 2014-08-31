cache()

TEMPLATE = subdirs

SUBDIRS += mainlib \
    indoorcycling

#LIBS +=  -lusb-1.0 -lavcodec -lavformat -lavutil

indoorcycling.depends = mainlib

