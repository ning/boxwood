dnl $Id$
dnl config.m4 for extension boxwood

PHP_ARG_ENABLE(boxwood, whether to enable boxwood support,
               [  --enable-boxwood           Enable boxwood support])

if test "$PHP_BOXWOOD" != "no"; then
  PHP_NEW_EXTENSION(boxwood, php_boxwood.c boxwood.c case-fold.c, $ext_shared)
  PHP_ADD_SOURCES_X(PHP_EXT_DIR(boxwood),case-fold-map.c,"-std=c99",shared_objects_boxwood,yes)
fi

