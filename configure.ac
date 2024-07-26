dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([vqadmin],[2.4.1])
AM_INIT_AUTOMAKE()

AC_USE_SYSTEM_EXTENSIONS

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

AC_CANONICAL_HOST
AC_CONFIG_HEADERS([config.h])

AC_SEARCH_LIBS([strerror],[cposix])

AC_DEFINE_UNQUOTED(VQA_PACKAGE,"$PACKAGE_NAME",Package name)
AC_DEFINE_UNQUOTED(VQA_VERSION,"$PACKAGE_VERSION",Package version)

if test "$GCC" = yes ; then
        CXXFLAGS="$CXXFLAGS -Wall"
        CFLAGS="$CFLAGS -Wall"
fi

qmaildir=""
for f in /var/qmail
do
    if test -d $f
    then
        qmaildir=$f
    fi
done

AC_ARG_ENABLE(qmaildir, [  --enable-qmaildir=dir   directory where qmail is installed.],
    qmaildir="$enableval",
    [
        if test "$qmaildir" = ""
        then
            AC_MSG_ERROR([Unable to find your qmaildir directory, specify --enable-qmaildir.])
        fi
    ] )
AC_DEFINE_UNQUOTED(QMAILDIR,"$qmaildir","")

vpopuser="vpopmail"
AC_ARG_ENABLE(vpopuser, [  --enable-vpopuser=vpopmail   user vchkpw was installed as.],
    vpopuser="$enableval",
    [
        if test "$vpopuser" = ""
        then
            AC_MSG_ERROR([Unable to find your vpopuser user, specify --enable-vpopuser.])
        fi
    ] )
AC_DEFINE_UNQUOTED(VPOPUSER,"$vpopuser","")
AC_SUBST(vpopuser)
echo $vpopuser > vpopusername

vpopgroup="vchkpw"
AC_ARG_ENABLE(vpopgroup, [  --enable-vpopgroup=vchkpw   group vchkpw was installed as.],
    vpopgroup="$enableval",
    [
        if test "$vpopgroup" = ""
        then
            AC_MSG_ERROR([Unable to find your vpopgroup group, specify --enable-vpopgroup.])
        fi
    ] )
AC_DEFINE_UNQUOTED(VPOPGROUP,"$vpopgroup","")
AC_SUBST(vpopgroup)

AC_RUN_IFELSE([AC_LANG_SOURCE([[

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>

int main()
{
struct passwd *pw;
char tmpbuf[100];
FILE *f;

        unlink("vpopmail.dir");
        unlink("vpopmail.uid");
        unlink("vpopmail.gid");

        f=fopen("vpopusername","r+");
        fgets(tmpbuf,100,f);
        fclose(f);
        tmpbuf[strlen(tmpbuf)-1] = 0;

        if (( pw=getpwnam(tmpbuf)) != 0 ) {
                f=fopen("vpopmail.dir","a+");
                fprintf(f,"%s\n", pw->pw_dir);
                fclose(f);

                f=fopen("vpopmail.uid", "a+");
                fprintf(f,"%d\n", pw->pw_uid);
                fclose(f);

                f=fopen("vpopmail.gid", "a+");
                fprintf(f,"%d\n", pw->pw_gid);
                fclose(f);
        }
        return(0);
}
]])],[AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.)],[AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.)])

if test ! -f vpopmail.uid
then
        AC_MSG_ERROR(No vpopmail user.)
fi

if test ! -f vpopmail.gid
then
        AC_MSG_ERROR(No vpopmail group.)
fi

if test ! -f vpopmail.dir
then
        AC_MSG_ERROR(No vpopmail home directory user.)
fi

vpopmaildir=`cat vpopmail.dir`
AC_SUBST(vpopmaildir)
rm -f vpopmail.dir

vpopmailuid=`cat vpopmail.uid`
AC_SUBST(vpopmailuid)
rm -f vpopmail.uid

vpopmailgid=`cat vpopmail.gid`
AC_SUBST(vpopmailgid)
rm -f vpopmail.gid
rm -f vpopusername

if test ! -f $vpopmaildir/etc/lib_deps
then
        AC_MSG_ERROR(No vpopmail etc/lib_deps file. Upgrade to vpopmail-4.9.8 or above)
fi
lib_deps=`cat $vpopmaildir/etc/lib_deps`
AC_SUBST(lib_deps)

if test ! -f $vpopmaildir/etc/inc_deps
then
        AC_MSG_ERROR(No vpopmail etc/inc_deps file. Upgrade to vpopmail-4.9.8 or above)
fi
inc_deps=`cat $vpopmaildir/etc/inc_deps`
AC_SUBST(inc_deps)

cgibindir=""

for f in /home/httpd /usr/local/apache /usr/local/www /var/lib/httpd /var/www/htdocs /usr/local/www /var/www
do
    if test -d $f/cgi-bin
    then
        cgibindir=$f/cgi-bin
    fi
done

AC_ARG_ENABLE(cgibindir, [  --enable-cgibindir={dir}   HTTP server's cgi-bin directory.],
    cgibindir="$enableval",
    [
        if test "$cgibindir" = ""
        then
            AC_MSG_ERROR([Unable to find your cgi-bin directory, specify --enable-cgibindir.])
        fi
    ] )
AC_SUBST(cgibindir)
AC_DEFINE_UNQUOTED(CGIBINDIR,"$cgibindir",cgi bin directory)

AC_ARG_ENABLE(isoqlog,
        [  --enable-isoqlog=n|y   enable support for isoqlog, default is no],
        ENABLE_ISOQLOG=$enableval,
                [
                        ENABLE_ISOQLOG=no
                ])
case $ENABLE_ISOQLOG in
1*|y*|Y*)
        ENABLE_ISOQLOG=1
        AC_DEFINE_UNQUOTED(ENABLE_ISOQLOG,$ENABLE_ISOQLOG,"")
        ;;
*)
        ENABLE_ISOQLOG=0
        ;;
esac

AC_ARG_ENABLE(isoqlogpath, [  --enable-isoqlogpath={path}   override default path to isoqlog file, default /usr/local/etc/isoqlog.domains],
    isoqlogpath="$enableval",
    [
        if test "$isoqlogpath" = ""
        then
		isoqlogpath="/usr/local/etc/isoqlog.domains"
        fi
    ] )
AC_DEFINE_UNQUOTED(ISOQLOGPATH,"$isoqlogpath","")

dnl Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP
# AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library functions.
AC_CHECK_LIB(crypt,crypt)
AC_CHECK_LIB(m,floor)
AC_CHECK_LIB(z,compress)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo ""
echo "            $PACKAGE_NAME $PACKAGE_VERSION           "
echo "            Current settings"
echo "---------------------------------------"
echo "vpopmail directory = "$vpopmaildir""
echo "               uid = "$vpopmailuid""
echo "               gid = "$vpopmailgid""
echo "       cgi-bin dir = $cgibindir"
echo "       vqadmin dir = $cgibindir/vqadmin"


