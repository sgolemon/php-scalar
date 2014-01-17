dnl $Id$
dnl config.m4 for extension scalar

PHP_ARG_ENABLE(scalar, whether to enable scalar type hints,
[  --enable-scalar           Enable scalar type hints])

if test "$PHP_SCALAR" != "no"; then
  PHP_NEW_EXTENSION(scalar, php_scalar.c, $ext_shared)
fi
