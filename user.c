/*
 * vQadmin Virtual Administration Interface
 * Copyright (C) 2000-2002 Inter7 Internet Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License  
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * vol@inter7.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <time.h>
#include <memory.h>
#include <ctype.h>
#include "config.h"
#include "global.h"
/* undefine vaiables conflicting with analog vpopmail vars imported from vpopmail config.h */
#undef VERSION
#undef QMAILDIR
#undef PACKAGE_VERSION
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_NAME
#undef PACKAGE
#include "vpopmail.h"
#include "vpopmail_config.h"
#include "vauth.h"

extern unsigned int acl_features;
extern char WarningBuff[MAX_WARNING_BUFF];

char Crypted[100];


void del_user()
{
 char *eaddr = NULL;
 static char user[156];
 static char domain[156];
 int   ret;

  if (!(acl_features & ACL_USER_DELETE)) {
    global_warning("Delete Email Account: Permission denied");
    t_page(T_MAIN, 1);
  }

  eaddr = cgi_is_var("eaddr");

  if ( eaddr==NULL || strlen(eaddr)==0 ) {
    global_warning("Delete Email Account: Failed: no email address given");
    t_page("html/del_user.html", 1);
  }

  parse_email( eaddr, user, domain, 156);

  ret = vdeluser(user, domain);
  if (ret != VA_SUCCESS) {
     global_warning(verror(ret));
  } else {
    snprintf(WarningBuff, MAX_WARNING_BUFF, 
        "Email Account %s deleted", eaddr);
    global_warning(WarningBuff);
  }

  t_main(1);
}

void view_user()
{
 char *eaddr = NULL;
 static char user[156];
 static char domain[156];
 struct vqpasswd *vpw;

  if (!(acl_features & ACL_USER_VIEW)) {
    global_warning("View Email Account: Permission denied");
    t_main(1);
  }

  eaddr = cgi_is_var("eaddr");

  if ( eaddr==NULL || strlen(eaddr)==0 ) {
    global_warning("Add Email Account: Failed: no email address given");
    t_page("html/view_user.html", 1);
  }

  parse_email( eaddr, user, domain, 156);

  vpw = vauth_getpw(user,domain);
  if (vpw == NULL) {
    global_warning("View User: account does not exist");
    t_page("html/view_user.html", 1);
  }

  post_email_info( eaddr, vpw, domain);

  t_page("html/mod_user.html", 1);

}

void mod_user()
{
 char *eaddr = NULL;
 static char user[156];
 static char domain[156];
 char *gecos;
 char *passwd;
 char *uquota;

 char *upop;
 char *uimap;
 char *udialup;
 char *upassc;
 char *uweb;
 char *ubounce;
 char *urelay;
 char *uspam;
 char *usmtp;
 char *umaildrop;
 char *udeletespam;
 char *uqadmin;

 char *formattedquota = NULL;
 char qconvert[11];
 int   ret;
 int   i;
 int   GidFlag = 0;
 struct vqpasswd *vpw;

  if (!(acl_features & ACL_USER_MOD)){
    global_warning("Modify Email Account: Permission denied");
    t_main(1);
  }

  eaddr = cgi_is_var("eaddr");
  gecos = cgi_is_var("fname");
  passwd = cgi_is_var("cpass");
  uquota = cgi_is_var("quota");

  upop = cgi_is_var("upop");
  uimap = cgi_is_var("uimap");
  udialup = cgi_is_var("udialup");
  upassc = cgi_is_var("upassc");
  uweb = cgi_is_var("uweb");
  ubounce = cgi_is_var("ubounce");

  urelay = cgi_is_var("urelay"); 
  uspam = cgi_is_var("uspam"); 
  usmtp = cgi_is_var("usmtp"); 
  umaildrop = cgi_is_var("umaildrop"); 
  udeletespam = cgi_is_var("udeletespam"); 
  uqadmin = cgi_is_var("uqadmin");

  if ( eaddr==NULL || strlen(eaddr)==0 ) {
    global_warning("Modify Email Account: Failed: no email address given");
    t_page("html/mod_user.html", 1);
  }
 
  parse_email( eaddr, user, domain, 156);

  vpw = vauth_getpw(user, domain);
  if (vpw == NULL) {
    global_warning("Modify User: account does not exist");
    t_page("html/mod_user.html", 1);
  }

  if ( passwd!=NULL && strlen(passwd)>=0 ){
    if ( !(vpw->pw_gid & NO_PASSWD_CHNG) ) {
      if (strlen(passwd) == 0 ) {
        vpw->pw_passwd = "";
        vpw->pw_clear_passwd = "";
      } else {
        mkpasswd3(passwd,Crypted, 100);
        vpw->pw_passwd = Crypted;
        vpw->pw_clear_passwd = passwd;
      }
    }
  }

  if ( gecos!=NULL && strlen(gecos)>0 ){
    for(i=0;gecos[i]!=0;++i) if (gecos[i] == '+') gecos[i] = ' ';
    vpw->pw_gecos = gecos;
  }

  // UPDATE USER QUOTA ("maildirsize" file will be created at the end, only if the update is successfull.
  formattedquota="NOQUOTA"; // START WITH DEAFULT "NOQUOTA"
  if (uquota!=NULL) { // QUOTA FIELD IS SET
    if (strlen(uquota)==0 || strcmp(uquota, "NOQUOTA")==0 || strcmp(uquota, "BADQUOTA")==0) {  // RESET QUOTA
      formattedquota = "NOQUOTA";
    } else { // QUOTA IS REQUESTED, CONVERT bytes->Mbytes
      if (quota_to_bytes(qconvert, uquota)) {
        global_warning("Invalid quota string.");
        t_page("html/mod_user.html", 1);
      } else {
        formattedquota = format_maildirquota(qconvert);
      }
    }
  }
  vpw->pw_shell = formattedquota;

  // UPDATE PERMISSIONS
  if (upop!=NULL) GidFlag |= NO_POP;
  if (uimap!=NULL) GidFlag |= NO_IMAP;
  if (udialup!=NULL) GidFlag |= NO_DIALUP;
  if (upassc!=NULL) GidFlag |= NO_PASSWD_CHNG;
  if (uweb!=NULL) GidFlag |= NO_WEBMAIL;
  if (ubounce!=NULL) GidFlag |= BOUNCE_MAIL;
  if (urelay!=NULL) GidFlag |= NO_RELAY;
  if (usmtp!=NULL) GidFlag |= NO_SMTP;
  if (uspam!=NULL) GidFlag |= NO_SPAMASSASSIN;
  if (udeletespam!=NULL) GidFlag |= DELETE_SPAM;
  if (umaildrop!=NULL) GidFlag |= NO_MAILDROP;
  if (uqadmin!=NULL) GidFlag |= QA_ADMIN;

  vpw->pw_gid = GidFlag;

  ret = vauth_setpw(vpw, domain);
  if ( ret != VA_SUCCESS ) {
   snprintf(WarningBuff, MAX_WARNING_BUFF,
        "Modify Account %s error %s", eaddr, verror(ret));
    global_warning(WarningBuff);
  } else {
    update_maildirsize(domain, vpw->pw_dir, formattedquota); // CREATE "maildirsize" file in user's Maildir.
  }
  post_email_info( eaddr, vpw, domain);

  t_page("html/mod_user.html", 1);
}

void post_email_info( char *eaddr, struct vqpasswd *vpw, char *domain)
{
 char qconvert[11];
#ifdef ENABLE_AUTH_LOGGING
 time_t mytime;
 char *authip;
#endif

  global_par("UA", eaddr);
  global_par("UN", vpw->pw_gecos);

// SHOW USER QUOTA IN MB
  if (strncmp(vpw->pw_shell, "NOQUOTA", 2) != 0) {
    if(quota_to_megabytes(qconvert, vpw->pw_shell)) {
      global_par("UQ", "BADQUOTA");
    }
    else {
      global_par("UQ", qconvert);
    }
  }

  global_par("UD", vpw->pw_dir);

#ifdef CLEAR_PASS
  global_par("UO", vpw->pw_clear_passwd);
#endif

  if (vpw->pw_gid & NO_POP) global_par("MP", "CHECKED");
  if (vpw->pw_gid & NO_IMAP) global_par("MI", "CHECKED");
  if (vpw->pw_gid & NO_DIALUP) global_par("MD", "CHECKED");
  if (vpw->pw_gid & NO_PASSWD_CHNG) global_par("MC", "CHECKED");
  if (vpw->pw_gid & NO_WEBMAIL) global_par("MW", "CHECKED");
  if (vpw->pw_gid & BOUNCE_MAIL) global_par("MB", "CHECKED");

  if (vpw->pw_gid & NO_RELAY) global_par("MS", "CHECKED");
  if (vpw->pw_gid & NO_SMTP) global_par("MH", "CHECKED");
  if (vpw->pw_gid & NO_SPAMASSASSIN) global_par("MZ", "CHECKED");
  if (vpw->pw_gid & DELETE_SPAM) global_par("ML", "CHECKED");
  if (vpw->pw_gid & NO_MAILDROP) global_par("MN", "CHECKED");
  if (vpw->pw_gid & QA_ADMIN) global_par("MK", "CHECKED");

#ifdef ENABLE_AUTH_LOGGING
  mytime = vget_lastauth(vpw, domain);
  authip = vget_lastauthip(vpw, domain);
  if ( mytime == 0 ) global_par("UT", "never logged in");
  else global_par("UT", asctime(localtime(&mytime)));

  if ( authip == 0 || strcmp(authip,NULL_REMOTE_IP) == 0 ) {
    global_par("UZ", "never logged in");
  } else {
    global_par("UZ", authip); 
  }
#else
  global_par("UT", "auth logging not enabled");
  global_par("UZ", "auth logging not enabled");
#endif

  t_page("html/mod_user.html", 1);
}

void show_users()
{
 char *domain;
 struct vqpasswd *vpw;
 int count;
 char *tmpstr;
 static char dir[500];
 static char workdir[500]; 
 static char tmpbuf[500]; 
 FILE *fs;
 DIR *mydir;
 struct dirent *mydirent;
 int i, j;
 int found;
#ifdef ENABLE_AUTH_LOGGING
 time_t mytime;
#endif

  if (!(acl_features & ACL_USER_VIEW)) {
     global_warning("View Email Account: Permission denied");
     t_main(1);
  }

  domain = cgi_is_var("dname");
  if ( domain==NULL ) {
    global_warning("Show Users: Failed: no domain name given");
    t_page("html/show_users.html",1);
  }

  if ( vget_assign(domain,dir,156,NULL,NULL) == NULL ) {
    global_warning("Show Users: Failed: domain does not exist");
    t_page("html/show_users.html",1);
  }

  /* initialize html template */
  t_open(T_HEADER,0);
  /* ended. go on filling with the contents */

  printf("<h2>Users for <mark>%s</mark></h2>\n", domain);

  vpw = vauth_getall(domain,1,0);
  if ( vpw == NULL ) {
    printf("<div class=\"alert alert-danger\" role=\"alert\">Domain %s does not exist</div>\n", domain);
  } else {
    printf("<table class=\"table table-striped table-hover\">\n");
    printf("<thead><tr>\n");
    printf("<th>User</th>\n");
#ifdef CLEAR_PASS
    printf("<th>Password</th>\n");
#endif
    printf("<th>Forward</th>\n");
    printf("<th>Vacation</th>\n");
    printf("<th>Quota</th>\n");
    printf("<th>Domain Administrator</th>\n");
    printf("<th>Last Logon</th></tr>\n");
    printf("</tr></thead><tbody>\n");
  }

  count = 0;
  while(vpw != NULL && count < 128000 ){
      ++count;

      printf("<tr><td>");
      printf("<a href=vqadmin.cgi?nav=view_user&eaddr=%s@%s>", vpw->pw_name, domain);
      printf("%s</a></td>\n", vpw->pw_name);
#ifdef CLEAR_PASS
      printf("<td>%s</td>\n", vpw->pw_clear_passwd);
#endif
      printf("<td>");
      snprintf(workdir, 156, "%s/.qmail", vpw->pw_dir);
      fs=fopen(workdir,"r");
      if ( fs == NULL ) {
          printf("No");
      } else {
         found = 0;
     while( fgets(workdir,156,fs) != NULL ) {
             if ( workdir[0] == '&' ) {
                 printf("%s", &workdir[1]);
                 found = 1;
             }
         }
         if ( found == 0 ) {
        printf("No");
         }
      }
      printf("</td>\n");

      printf("<td>");
      if ( fs == NULL ) {
          printf("No");
      } else {
         rewind(fs);
         found = 0;
     while( fgets(workdir,156,fs) != NULL ) {
             if ( strstr(workdir, "autorespond") != NULL ) {
                found = 1;
             }
         }

         if ( found == 1 ) {
                 printf("Yes");
         } else {
             printf("No");
         }
      }
      printf("</td>\n");
      if ( fs!=NULL) fclose(fs);


      printf("<td>%s</td>", vpw->pw_shell);

      if (vpw->pw_gid & QA_ADMIN) {
        printf("<td>Yes</td>\n");
      }    else  printf("<td>No</td>\n");

#ifdef ENABLE_AUTH_LOGGING
  mytime = vget_lastauth(vpw, domain);
  if ( mytime == 0 ) printf("<td>NEVER LOGGED IN</td></tr>\n");
  else printf("<td>%s</td></tr>\n", asctime(localtime(&mytime)));
#else
  printf("<td>* auth logging not enabled *</td></tr>\n");
#endif

      vpw = vauth_getall(domain,0,0);
  }
  printf("</tbody></table>\n");

  printf("<h2 class=\"mt-5\">Alias/Forwards for <mark>%s</mark></h2>\n", domain);
  printf("<table class=\"table table-striped table-hover\">\n");
  printf("<thead><tr>\n");
  printf("<th>name</th>\n");
  printf("<th>Alias/Forward</th>\n");
  printf("</tr></thead><tbody>\n");

  chdir(dir);
  mydir = opendir(".");
  count = 0;
  while( (mydirent=readdir(mydir)) != NULL ) {
      if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {

          if ( strstr(mydirent->d_name, "-owner") != NULL ) continue;
          if ( strstr( mydirent->d_name, "-default") != NULL ) continue;

          fs=fopen(mydirent->d_name,"r");
          memset(tmpbuf,0,sizeof(tmpbuf));
      if ( fgets( tmpbuf, 156, fs ) != NULL ) {
        if ( tmpbuf[0] == '|' ) {
            fclose(fs);
            continue;
        }
      }

          for(i=7,j=0;j<156-1&&mydirent->d_name[i]!=0;++i,++j) {
              workdir[j] = mydirent->d_name[i];
          }
          workdir[j] = 0;
          ++count;

          printf("<tr><td>%s@%s</td>", workdir, domain);

          printf("<td>\n");
          fs=fopen(mydirent->d_name,"r");
      while ( fgets( tmpbuf, 156, fs ) != NULL ) {
                if ( tmpbuf[0] == '#' || isspace(tmpbuf[0]) ) {
            printf("&nbsp\n");
                } else if ( strstr(tmpbuf, "@") != NULL ) {
                    if ( tmpbuf[0] == '&' ) i = 1;
                    else i = 0;

            printf("forward: %s<br>\n", &tmpbuf[i]);
                } else {
            tmpstr = &tmpbuf[strlen(tmpbuf)-2];
            *tmpstr = 0;
                    while (*tmpstr!='/') --tmpstr;
            *tmpstr = 0;
                    while (*tmpstr!='/') --tmpstr;
                    ++tmpstr;
            printf("alias: %s<br>\n", tmpstr);
                }
      }
          printf("</td></tr>\n");

          fclose(fs);
      }
  }
  closedir(mydir);
  printf("</tbody></table>\n");

  printf("<h2 class=\"mt-5\">Mailing lists for <mark>%s</mark></h2>\n", domain);
  printf("<table class=\"table table-striped table-hover\"><tbody>\n");

  chdir(dir);
  mydir = opendir(".");
  count = 0;
  while( (mydirent=readdir(mydir)) != NULL ) {
      if ( strncmp(".qmail-", mydirent->d_name, 7) == 0 ) {
          fs=fopen(mydirent->d_name,"r");
          fgets( workdir, 156, fs);
          if ( strstr( workdir, "ezmlm-reject") != 0 ) {
              for(i=7,j=0;j<156-1&&mydirent->d_name[i]!=0;++i,++j) {
                  workdir[j] = mydirent->d_name[i];
              }
              workdir[j] = 0;
              ++count;

              printf("<tr><td>%s@%s</td></tr>", workdir, domain);
          }
          fclose(fs);
      }
  }
  closedir(mydir);
  printf("</tbody></table>\n");

  /* close the html template */
  snprintf(tmpbuf, sizeof(tmpbuf), "%s/vqadmin/%s", CGIBINDIR, T_COL);
  t_open(tmpbuf,0);
  memset(tmpbuf,0,sizeof(tmpbuf));
  snprintf(tmpbuf, sizeof(tmpbuf), "%s/vqadmin/%s", CGIBINDIR, T_FOOTER);
  t_open(tmpbuf,1);

  vexit(0);
}

void add_user()
{
 char *eaddr = NULL;
 static char user[156];
 static char domain[156];
 char *gecos;
 char *passwd;
 char *uquota;

 char *upop;
 char *uimap;
 char *udialup;
 char *upassc;
 char *uweb;
 char *ubounce;
 char *urelay;
 char *uspam;
 char *usmtp;
 char *umaildrop;
 char *udeletespam;
 char *uqadmin;

 char *formattedquota = NULL;
 char qconvert[11];
 int   ret;
 int   GidFlag = 0;
 struct vqpasswd *vpw;

  if (!(acl_features & ACL_USER_CREATE)) {
    global_warning("Add Email Account: Permission denied");
    t_main(1);
  }

  eaddr = cgi_is_var("eaddr");
  gecos = cgi_is_var("fname");
  passwd = cgi_is_var("cpass");
  uquota = cgi_is_var("quota");

  upop = cgi_is_var("upop");
  uimap = cgi_is_var("uimap");
  udialup = cgi_is_var("udialup");
  upassc = cgi_is_var("upassc");
  uweb = cgi_is_var("uweb");
  ubounce = cgi_is_var("ubounce");

  urelay = cgi_is_var("urelay"); 
  uspam = cgi_is_var("uspam"); 
  usmtp = cgi_is_var("usmtp"); 
  umaildrop = cgi_is_var("umaildrop"); 
  udeletespam = cgi_is_var("udeletespam"); 
  uqadmin = cgi_is_var("uqadmin");


  if ( eaddr==NULL || strlen(eaddr)==0 ) {
    global_warning("Add Email Account: Failed: no email address given");
    t_page("html/add_user.html", 1);
  }

  if ( parse_email( eaddr, user, domain, 156) == -1 ) {
    global_warning("Add Email Account: Failled: invalid characters in email address ");
    t_page("html/add_user.html", 1);
  }

  vpw = vauth_getpw(user,domain);
  if (vpw != NULL) {
    global_warning("Add User: Failed: account already exist");
    t_page("html/add_user.html", 1);
  }

  if ( gecos==NULL || strlen(gecos)==0 ){
    gecos = user;
  }

  ret = vadduser(user, domain, passwd, gecos, USE_POP );
  if (ret != VA_SUCCESS) {
    snprintf(WarningBuff, MAX_WARNING_BUFF, "Create Account %s error: %s", eaddr, verror(ret));
    global_warning(WarningBuff);
    t_page("html/add_user.html", 1);
  } else {
    snprintf(WarningBuff, MAX_WARNING_BUFF, "Email Account %s created", eaddr);
    global_warning(WarningBuff);
  }

  vpw = vauth_getpw(user, domain);

  // UPDATE USER QUOTA ("maildirsize" file will be created at the end, only if the update is successfull.
  formattedquota="NOQUOTA"; // START WITH DEAFULT "NOQUOTA"
  if (uquota!=NULL) { // QUOTA FIELD IS SET
    if (strlen(uquota)==0 || strcmp(uquota, "NOQUOTA")==0 || strcmp(uquota, "BADQUOTA")==0) {  // RESET QUOTA
      formattedquota = "NOQUOTA";
    } else { // QUOTA IS REQUESTED, CONVERT bytes->Mbytes
      if (quota_to_bytes(qconvert, uquota)) {
        global_warning("Invalid quota string.");
        t_page("html/mod_user.html", 1);
      } else {
        formattedquota = format_maildirquota(qconvert);
      }
    }
  }
  vpw->pw_shell = formattedquota;

  // USER PERMISSIONS
  if (upop!=NULL) GidFlag |= NO_POP;
  if (uimap!=NULL) GidFlag |= NO_IMAP;
  if (udialup!=NULL) GidFlag |= NO_DIALUP;
  if (upassc!=NULL) GidFlag |= NO_PASSWD_CHNG;
  if (uweb!=NULL) GidFlag |= NO_WEBMAIL;
  if (ubounce!=NULL) GidFlag |= BOUNCE_MAIL;
  if (urelay!=NULL) GidFlag |= NO_RELAY;
  if (usmtp!=NULL) GidFlag |= NO_SMTP;
  if (uspam!=NULL) GidFlag |= NO_SPAMASSASSIN;
  if (udeletespam!=NULL) GidFlag |= DELETE_SPAM;
  if (umaildrop!=NULL) GidFlag |= NO_MAILDROP;
  if (uqadmin!=NULL) GidFlag |= QA_ADMIN;

  vpw->pw_gid = GidFlag;

  ret = vauth_setpw(vpw, domain);
  if ( ret != VA_SUCCESS ) {
    snprintf(WarningBuff, MAX_WARNING_BUFF, 
        "Add Account: Failure:  %s set options", eaddr);
    global_warning(WarningBuff);
  } else {
    update_maildirsize(domain, vpw->pw_dir, formattedquota); // CREATE "maildirsize" file in user's Maildir.
  }

  post_email_info(eaddr, vpw, domain);

  t_main(1);
}
