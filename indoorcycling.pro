
TEMPLATE = subdirs

SUBDIRS += mainlib \
    indoorcycling \
    antlib

indoorcycling.depends = antlib mainlib

