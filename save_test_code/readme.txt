
怎么样生成./configure :

1.autoscan
2.mv configure.scan configure.ac

添加内容:
AM_INIT_AUTOMAKE 
#AC_CONFIG_FILES([Makefile])
AC_OUTPUT(Makefile)

3.aclocal
4.autoheader
5.autoconf
6.vim Makefile.am  添加内容:
AUTOMAKE_OPTIONS=foreign 
bin_PROGRAMS=test_code
test_code_SOURCES=test_code.c test_code_main.c

7.automake --add-missing
