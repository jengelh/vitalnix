
# GCC 4.x -fvisibility=hidden

AC_DEFUN([CHECK_GCC_FVISIBILITY], [
	AC_LANG_PUSH(C)
	saved_CFLAGS="$CFLAGS"
	CFLAGS="$saved_CFLAGS -fvisibility=hidden"
	AC_CACHE_CHECK([whether compiler accepts -fvisibility=hidden],
	  [cv_fvisibility_hidden], AC_COMPILE_IFELSE(
		AC_LANG_PROGRAM([], []),
		[cv_fvisibility_hidden=yes],
		[cv_fvisibility_hidden=no]
	))
	if test "$cv_fvisibility_hidden" = "yes"; then
		AC_DEFINE(HAVE_VISIBILITY_HIDDEN, [1],
		  [True if compiler supports -fvisibility=hidden])
		AC_SUBST(GCC_FVISIBILITY_HIDDEN, [-fvisibility=hidden])
	fi
	CFLAGS="$saved_CFLAGS"
	AC_LANG_POP(C)
])dnl
CHECK_GCC_FVISIBILITY()
