# Local additions to Autoconf macros.
# Copyright (C) 1995 - 1998 Michael Riepe <michael@stud.uni-hannover.de>

# @(#) $Id: aclocal.m4,v 1.1 2004-05-24 17:11:34 fzhao Exp $

AC_PREREQ(2.12)

AC_DEFUN(mr_ENABLE_NLS, [
  AC_PROVIDE([$0])

  # Needed for `make dist' even if NLS is disabled.
  GMOFILES=
  MSGFILES=
  POFILES=
  for mr_lang in $ALL_LINGUAS; do
    GMOFILES="$GMOFILES $mr_lang.gmo"
    MSGFILES="$MSGFILES $mr_lang.msg"
    POFILES="$POFILES $mr_lang.po"
  done
  AC_SUBST(GMOFILES)
  AC_SUBST(MSGFILES)
  AC_SUBST(POFILES)

  AC_MSG_CHECKING([whether NLS is requested])
  AC_ARG_ENABLE(nls,
    [  --disable-nls           do not use Native Language Support],
    [mr_enable_nls="$enableval"],
    [mr_enable_nls=yes])
  AC_MSG_RESULT($mr_enable_nls)

  CATOBJEXT=
  INSTOBJEXT=
  localedir=
  if test "$mr_enable_nls" = yes; then
    mr_PATH=`echo ":$PATH" | sed -e 's,:[^:]*openwin[^:]*,,g' -e 's,^:,,'`
    AC_CACHE_CHECK([for gettext],
      mr_cv_func_gettext, [
	AC_TRY_LINK([#include <libintl.h>],
	  [char *s = gettext(""); return 0],
	  [mr_cv_func_gettext=yes],
	  [mr_cv_func_gettext=no])
    ])
    if test "$mr_cv_func_gettext" = yes; then
      AC_PATH_PROG(MSGFMT, msgfmt, no, $mr_PATH)
      if test "$MSGFMT" != no; then
	AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT, $mr_PATH)
	AC_PATH_PROG(XGETTEXT, xgettext, xgettext, $mr_PATH)
	AC_CACHE_CHECK([for GNU gettext],
	  mr_cv_gnu_gettext, [
	    AC_TRY_LINK([],
	      [extern int _nl_msg_cat_cntr; return _nl_msg_cat_cntr],
	      [mr_cv_gnu_gettext=yes],
	      [mr_cv_gnu_gettext=no])
	])
	if test "$mr_cv_gnu_gettext" = yes; then
	  AC_CACHE_CHECK([for losing catgets-based GNU gettext],
	    mr_cv_catgets_based_gettext, [
	      AC_TRY_LINK([],
		[extern int _msg_tbl_length; return _msg_tbl_length],
		[mr_cv_catgets_based_gettext=yes],
		[mr_cv_catgets_based_gettext=no])
	  ])
	  if test "$mr_cv_catgets_based_gettext" = yes; then
	    # This loses completely. Turn it off and use catgets.
	    LIBS=`echo $LIBS | sed 's,-lintl,,g'`
	    mr_cv_func_gettext=no
	  else
	    # Is there a better test for this case?
	    AC_CACHE_CHECK([for pure GNU gettext],
	      mr_cv_pure_gnu_gettext, [
		AC_TRY_LINK([],
		  [extern int gettext(); return gettext()],
		  [mr_cv_pure_gnu_gettext=yes],
		  [mr_cv_pure_gnu_gettext=no])
	    ])
	    if test "$mr_cv_pure_gnu_gettext" = yes; then
	      CATOBJEXT=.gmo
	      localedir='$(prefix)/share/locale'
	    else
	      CATOBJEXT=.mo
	      localedir='$(prefix)/lib/locale'
	    fi
	    INSTOBJEXT=.mo
	  fi
	else
	  # System provided gettext
	  CATOBJEXT=.mo
	  INSTOBJEXT=.mo
	  localedir='$(prefix)/lib/locale'
	fi
      else
	# Gettext but no msgfmt. Try catgets.
	mr_cv_func_gettext=no
      fi
    fi
    if test "$mr_cv_func_gettext" = yes; then
      AC_DEFINE(HAVE_GETTEXT)
    else
      AC_CACHE_CHECK([for catgets], mr_cv_func_catgets, [
	AC_TRY_LINK([#include <nl_types.h>],
	  [catgets(catopen("",0),0,0,"");return 0;],
	  [mr_cv_func_catgets=yes],
	  [mr_cv_func_catgets=no])
      ])
      if test "$mr_cv_func_catgets" = yes; then
	AC_PATH_PROG(GENCAT, gencat, no, $mr_PATH)
	if test "$GENCAT" != no; then
	  AC_DEFINE(HAVE_CATGETS)
	  AC_PATH_PROG(GMSGFMT, [gmsgfmt msgfmt], msgfmt, $mr_PATH)
	  AC_PATH_PROG(XGETTEXT, xgettext, xgettext, $mr_PATH)
	  CATOBJEXT=.cat
	  INSTOBJEXT=.cat
	  localedir='$(prefix)/lib/locale'
	fi
      else
	AC_MSG_WARN([no NLS support, disabled])
	mr_enable_nls=no
      fi
    fi
  fi
  AC_SUBST(CATOBJEXT)
  AC_SUBST(INSTOBJEXT)
  AC_SUBST(localedir)

  POSUB=
  CATALOGS=
  if test "$mr_enable_nls" = yes; then
    AC_MSG_CHECKING([for catalogs to be installed])
    mr_linguas=
    for mr_lang in ${LINGUAS=$ALL_LINGUAS}; do
      case " $ALL_LINGUAS " in
	*" $mr_lang "*)
	  mr_linguas="$mr_linguas$mr_lang "
	  CATALOGS="$CATALOGS $mr_lang$CATOBJEXT"
	  ;;
      esac
    done
    AC_MSG_RESULT($mr_linguas)
    POSUB=po
  fi
  AC_SUBST(CATALOGS)
  AC_SUBST(POSUB)
])

AC_DEFUN(mr_TARGET_ELF, [
  AC_PROVIDE([$0])
  AC_CACHE_CHECK([for native ELF system],
    mr_cv_target_elf,
    [AC_TRY_RUN(changequote(<<, >>)dnl
<<#include <stdio.h>
int
main(int argc, char **argv) {
  char buf[BUFSIZ];
  FILE *fp;
  int n;

  if ((fp = fopen(*argv, "r")) == NULL) {
    exit(1);
  }
  n = fread(buf, 1, sizeof(buf), fp);
  if (n >= 52
   && buf[0] == '\177'
   && buf[1] == 'E'
   && buf[2] == 'L'
   && buf[3] == 'F') {
    exit(0);
  }
  exit(1);
}>>, changequote([, ])dnl
      mr_cv_target_elf=yes,
      mr_cv_target_elf=no,
      mr_cv_target_elf=no)])])

AC_DEFUN(mr_ENABLE_SHARED, [
  AC_PROVIDE([$0])
  PICFLAGS=
  SHLIB=_shlib_dummy_
  SHLINK=
  SONAME=
  LINK_SHLIB=
  INSTALL_SHLIB=
  DEPSHLIBS=
  AC_MSG_CHECKING([whether to build a shared library])
  AC_ARG_ENABLE(shared,
    [  --enable-shared         build shared library],
    [mr_enable_shared="$enableval"],
    [mr_enable_shared=no])
  AC_MSG_RESULT($mr_enable_shared)
  if test "$mr_enable_shared" = yes; then
    AC_MSG_CHECKING([whether GNU naming conventions are requested])
    AC_ARG_ENABLE(gnu_names,
      [  --enable-gnu-names      use GNU naming conventions for shared library],
      [mr_enable_gnu_names="$enableval"],
      [mr_enable_gnu_names=no])
    AC_MSG_RESULT($mr_enable_gnu_names)
    AC_REQUIRE([AC_CANONICAL_HOST])
    AC_REQUIRE([AC_PROG_CC])
    AC_PATH_PROG(LD, ld, ld)
    case "$host" in
      *-linux*)
	if test "$GCC" = yes; then
	  mr_TARGET_ELF
	  if test "$mr_cv_target_elf" = yes; then
	    PICFLAGS='-fPIC'
	    if test "$mr_enable_gnu_names" = yes
	    then
	      SHLIB='$(PACKAGE)-$(VERSION).so'
	    else
	      SHLIB='$(PACKAGE).so.$(VERSION)'
	    fi
	    SHLINK='$(PACKAGE).so'
	    SONAME='$(PACKAGE).so.$(MAJOR)'
	    LINK_SHLIB='$(CC) -shared -Wl,-soname,$(SONAME)'
	    INSTALL_SHLIB='$(INSTALL_PROGRAM)'
	    DEPSHLIBS='-lc'
	  else
	    AC_MSG_WARN([shared libraries not supported for $host])
	    mr_enable_shared=no
	  fi
	else
	  AC_MSG_WARN([GNU CC required for building shared libraries])
	  mr_enable_shared=no
	fi
	;;
      i?86-*-linux)
	if test "$GCC" = yes; then
	  PICFLAGS='-fPIC'
	  if test "$mr_enable_gnu_names" = yes
	  then
	    SHLIB='$(PACKAGE)-$(VERSION).so'
	  else
	    SHLIB='$(PACKAGE).so.$(VERSION)'
	  fi
	  SHLINK='$(PACKAGE).so'
	  SONAME='$(PACKAGE).so.$(MAJOR)'
	  LINK_SHLIB='$(CC) -shared -Wl,-soname,$(SONAME)'
	  INSTALL_SHLIB='$(INSTALL_PROGRAM)'
	  DEPSHLIBS='-lc'
	else
	  AC_MSG_WARN([GNU CC required for building shared libraries])
	  mr_enable_shared=no
	fi
	;;
      sparc-sun-solaris2*)
	if test "$GCC" = yes; then
	  PICFLAGS='-fPIC'
	else
	  PICFLAGS='-K PIC'
	fi
	if test "$mr_enable_gnu_names" = yes
	then
	  SHLIB='$(PACKAGE)-$(MAJOR).so'
	else
	  SHLIB='$(PACKAGE).so.$(MAJOR)'
	fi
	SONAME='$(PACKAGE).so.$(MAJOR)'
	SHLINK='$(PACKAGE).so'
	LINK_SHLIB='$(LD) -G -z text -h $(SONAME)'
	INSTALL_SHLIB='$(INSTALL_PROGRAM)'
	;;
      *)
	AC_MSG_WARN([shared libraries not supported for $host])
	mr_enable_shared=no
	;;
    esac
  else
    mr_enable_shared=no
  fi
  AC_SUBST(PICFLAGS)
  AC_SUBST(SHLIB)
  AC_SUBST(SHLINK)
  AC_SUBST(SONAME)
  AC_SUBST(LINK_SHLIB)
  AC_SUBST(INSTALL_SHLIB)
  AC_SUBST(DEPSHLIBS)
  DO_SHLIB="$mr_enable_shared"
  AC_SUBST(DO_SHLIB)
])

AC_DEFUN(mr_ENABLE_DEBUG, [
  AC_PROVIDE([$0])
  AC_ARG_ENABLE(debug,
    [  --enable-debug          include extra debugging code],
    [mr_enable_debug="$enableval"],
    [mr_enable_debug=no])
  if test "$mr_enable_debug" = yes; then
    AC_DEFINE(ENABLE_DEBUG)
  fi
])

# vi: set ts=8 sw=2 :
