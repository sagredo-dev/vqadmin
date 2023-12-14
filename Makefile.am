@SET_MAKE@

SUBDIRS=

bin_PROGRAMS=vqadmin

COMMONSOURCES=cgi.c template.c global.c acl.c misc.c domain.c lang.c \
              user.c cedit.c

vqadmin_SOURCES = vqadmin.c $(COMMONSOURCES)
vqadmin_LDADD = @lib_deps@ 

DEFS=-I. @inc_deps@

install-exec-local:
	$(mkinstalldirs)  @cgibindir@
	$(mkinstalldirs)  @cgibindir@/vqadmin
	$(mkinstalldirs)  @cgibindir@/vqadmin/html
	$(mkinstalldirs)  @cgibindir@/vqadmin/lang
	$(mkinstalldirs)  @cgibindir@/vqadmin/assets

install-data-local:
	cp vqadmin         @cgibindir@/vqadmin/vqadmin.cgi
	strip              @cgibindir@/vqadmin/vqadmin.cgi
	cp -R html/*       @cgibindir@/vqadmin/html
	cp -R lang/*       @cgibindir@/vqadmin/lang
	cp -R assets/*     @cgibindir@/vqadmin/assets

	if test -f @cgibindir@/vqadmin/vqadmin.acl ; then \
		echo "vqadmin.acl file already in place" ; \
	else \
		cp vqadmin.acl     @cgibindir@/vqadmin ; \
	fi

	chown              @vpopuser@ @cgibindir@/vqadmin
	chgrp              @vpopgroup@ @cgibindir@/vqadmin
	chown -R           @vpopuser@ @cgibindir@/vqadmin/*
	chgrp -R           @vpopgroup@ @cgibindir@/vqadmin/*
	chown root         @cgibindir@/vqadmin/vqadmin.cgi
	chgrp `id -g root` @cgibindir@/vqadmin/vqadmin.cgi
	chmod u+s,g+s      @cgibindir@/vqadmin/vqadmin.cgi
	chmod 755          @cgibindir@/vqadmin
	chmod 755          @cgibindir@/vqadmin/html
	chmod 755          @cgibindir@/vqadmin/lang
	chmod 755          @cgibindir@/vqadmin/assets

	if test -f @cgibindir@/vqadmin/.htaccess ; then \
		echo ".htaccess file already in place" ; \
	else \
		cp htaccess        @cgibindir@/vqadmin/.htaccess ; \
	fi

	chown nobody       @cgibindir@/vqadmin/.htaccess
	chmod u+rw         @cgibindir@/vqadmin/.htaccess
	chmod go+r         @cgibindir@/vqadmin/.htaccess

AUTOMAKE_OPTIONS = foreign no-dependencies

acl.o: config.h
cedit.o: config.h
cgi.o: config.h
domain.o: config.h
global.o: config.h
lang.o: config.h
misc.o: config.h
template.o: config.h
user.o: config.h
vqadmin.o: config.h
