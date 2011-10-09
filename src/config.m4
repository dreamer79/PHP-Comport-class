PHP_ARG_ENABLE(phpcomport, whether to enable HTML crunch support,
[ --enable-phpcomport   Enable HTML crunch support])

if test "$PHP_PHPCOMPORT" = "yes"; then
  AC_DEFINE(HAVE_PHPCOMPORT, 1, [Whether you have HTML crunch])
   PHP_REQUIRE_CXX()
   PHP_SUBST(PHPCOMPORT_SHARED_LIBADD)
   PHP_ADD_LIBRARY(stdc++, 1, PHPCOMPORT_SHARED_LIBADD)
  PHP_NEW_EXTENSION(phpcomport, phpcomport.cpp comport.cpp, $ext_shared)
fi

