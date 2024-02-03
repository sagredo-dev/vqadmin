# vQadmin

vQadmin is a web based control panel that allows system administrators to perform actions which require root access — for example, adding and deleting domains.

## Installation and Configuration of vQadmin

vQadmin comes with full HTML templates.  This means that the CGI
does not generate any HTML at all, and that the templates
are responsible for providing the correct information to the CGI.
Generally a CGI will generate things such as forms, or portions
of forms like input tags.  This is not the case with vQadmin, so
be careful not to remove, or change any variable names inside
the template forms.  It also comes with an example access list file,
which explains pretty well how to set access restrictions upon users,
and their usergroups.

##  Contents
1.  What's needed to install vQadmin
2.  Installing and configuring vQadmin
3.  Setting up Apache
4.  All of the above key-by-key
5.  Troubleshooting
6.  More info and support

## 1.  What's needed to install vQadmin
vQadmin will run on any Unix-based system that has
vpopmail 4.9.8 or higher, Apache, and a C compiler installed.

## 2.  Installing and configuring vQadmin
```
$ ./configure 
# make
# make install-strip
```
Now you want to edit your vqadmin.acl file, which is your access list
definitions.  Please read that file for information on how to define users
and usergroups.

If you haven't changed anything else, and your libraries are set properly,
typing 'make' here should compile the CGI with no errors.  Once that's done,
typing 'make install' should install the CGI.  Any errors that appear during
these two command-line operations are going to be very hard to document
because of the system-specific nature of this portion of the installation.
(See section 5)

## 3. Setting up Apache

vQadmin will require it's own CGI-allowed, access-protected,
directory to operate.  First, you will need to create a <Directory>
tag inside your Apache configuration, which sets the directory
to have ExecCGI permissions, allows the directory to override
authority, and sets the directory to deny everyone by default.
vQadmin will not function without this setup.
```
Define QMAILROOT /var/www/qmail/
<VirtualHost *:80>
  ServerName vqadminyourdomain.tld
  DocumentRoot ${QMAILROOT}
  ScriptAlias /cgi-bin/ ${QMAILROOT}/cgi-bin/
  AddHandler cgi-script .cgi
  <Directory ${QMAILROOT}>
    AllowOverride None
    Require all granted
  </Directory>
  <Directory ${QMAILROOT}/cgi-bin>
    AllowOverride None
    Options ExecCGI
    Require all granted
  </Directory>
  <Directory ${QMAILROOT}/cgi-bin/vqadmin>
    Require all denied
    Options ExecCGI
    AllowOverride AuthConfig
  </Directory>
  Alias /assets/ ${QMAILROOT}/cgi-bin/vqadmin/assets/
  <Directory ${QMAILROOT}/cgi-bin/vqadmin/assets>
    Require all granted
  </Directory>
</VirtualHost>
```
After you've created the directory, you will need to create an
htaccess for the directory so Apache knows how to authenticate
users trying to access the directory.  In our example directory
/usr/local/apache/cgi-bin/vqadmin, you'd create a '.htaccess' file
describing the authentication we're using.  You should store the
password file somewhere the webserver isn't capable of displaying,
such as the conf directory.  The realm (AuthName) is not important,
so you may call it whatever you'd like.  You will want to chown
the file to the webserver user, and chmod it 600.
```
AuthType Basic
AuthUserFile /etc/httpdpwd/vqadmin.passwd
AuthName "Authentication required"
Require valid-user
```
Now, create a user.  In your Apache installation root directory, under
the bin subdirectory is a program called 'htpasswd'.  This program is used
to create, and maintain the vqadmin.passwd file.
```
  Usage:
	htpasswd [-cmdps] passwordfile username
	htpasswd -b[cmdps] passwordfile username password

   -c  Create a new file.
   -m  Force MD5 encryption of the password.
   -d  Force CRYPT encryption of the password (default).
   -p  Do not encrypt the password (plaintext).
   -s  Force SHA encryption of the password.
   -b  Use the password from the command line rather than prompting for it.
  On Windows and TPF systems the '-m' flag is used by default.
  On all other systems, the '-p' flag will probably not work.
```
We're only interested in the c (or maybe b) option for now.
To create a vqadmin.passwd file, with a login of 'test', and a
password of 'test'.
```
  htpasswd -bc /etc/httpdpwd/vqadmin.passwd test test
```
That's it.  Just remember that you made a user named 'test'!  You need
to know this for configuring vqadmin.

After you've done all this, you'll need to reload your configuration
files.

Note for isoqlog support: the isoqlog.domains file must exist (but can be
empty) for adding or deleting domains to/from isoqlog to work.  This may
be changed in a later version.

## 4.  All of the above key-by-key

Here we go, a very quick and dirty key-by-key installation guide...
```
# tar zxf vqadmin-X.X.tar.gz 
# cd vqadmin* 
# ./configure
# make
# make install
# cd /etc/httpd
# vi httpd.conf
Define QMAILROOT /var/www/qmail/
<VirtualHost *:80>
  ServerName vqadminyourdomain.tld
  DocumentRoot ${QMAILROOT}
  ScriptAlias /cgi-bin/ ${QMAILROOT}/cgi-bin/
  AddHandler cgi-script .cgi    
  <Directory ${QMAILROOT}>
    AllowOverride None
    Require all granted
  </Directory>
  <Directory ${QMAILROOT}/cgi-bin>
    AllowOverride None
    Options ExecCGI
    Require all granted
  </Directory>
  <Directory ${QMAILROOT}/cgi-bin/vqadmin>
    Require all denied
    Options ExecCGI
    AllowOverride AuthConfig
  </Directory>
  Alias /assets/ ${QMAILROOT}/cgi-bin/vqadmin/assets/
  <Directory ${QMAILROOT}/cgi-bin/vqadmin/assets>
    Require all granted
  </Directory>
</VirtualHost>
# cd /var/www//cgi-bin/vqadmin
# vi vqadmin.acl
# vi .htaccess
  AuthType Basic
  AuthUserFile /etc/httpd/httpdpwd/vqadmin.passwd
  AuthName "Authentication required"
  Require valid-user
# chown nobody .htaccess
# chmod 600 .htaccess
# htpasswd -bc /etc/httpd/httpdpwd/vqadmin.passwd admin adminpass
# apachectl restart 
```
As you can see, once you've done it once, it's pretty simple to just run
through it in 5 or 10 minutes.

## 5.  Troubleshooting

I could only think of a few troubleshooting things to document.
Just little problems I ran into when coding, or installing it during
production testing.
```
  * Apache shuts down after re-starting, and wont re-start
  + The <Directory> information you entered was probably in an invalid
    format, or you forgot your </Directory> tag. 
   
  * Compile-time errors
  + vQadmin compiles on all Unix-based systems.  If you get any
    errors when compiling, check to see if you have the required
    libraries compiled in at link time.
 
  * '500 Internal Server Error' message from the webserver
    when trying to use the CGI.
  + This could be a great deal of things.  Most likely it's
    related to the .htaccess file you setup.  Make sure it's
    readable by the webserver.
```

## 6. More info and support

You can find more info and ask for support at https://notes.sagredo.eu/en/qmail-notes-185/vqadmin-26.html.