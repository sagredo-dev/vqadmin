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
#include <string.h>
#include <unistd.h>
#include <memory.h>
#include "config.h"
#include "global.h"
#include "vpopmail.h"
#include "vpopmail_config.h"
#include "vauth.h"

#define TOKENS " :\t\n\r"

extern unsigned int acl_features;
extern char WarningBuff[MAX_WARNING_BUFF];
extern vdir_type vdir;

#ifdef ENABLE_ISOQLOG
void add_isoqlog (char *domain);
void del_isoqlog(char *domain);
#endif

void add_domain()
{
 char *domain = NULL;
 char *passwd = NULL;
 char *lusers = NULL;
 char *lfor = NULL;
 char *lalias = NULL;
 char *lresponder = NULL;
 char *llists = NULL;
 char *quota = NULL;
 char *upop = NULL;
 char *uimap = NULL;
 char *udialup = NULL;
 char *upassc = NULL;
 char *uweb = NULL;
 char *urelay = NULL;
 char *uspam = NULL;
 int   ret;
 char dir[156];
 uid_t uid;
 gid_t gid;
 FILE *fs;

  if (!(acl_features & ACL_DOMAIN_CREATE)) {
    global_warning("Create Domain: Permission denied");
    t_open(T_MAIN, 1);
  }

  domain = cgi_is_var("dname");
  passwd = cgi_is_var("pp");

  lusers = cgi_is_var("lusers");
  lfor = cgi_is_var("lfor");
  lalias = cgi_is_var("lalias");
  lresponder = cgi_is_var("lresponder");
  llists = cgi_is_var("llists");

  quota   = cgi_is_var("quota");
  upop    = cgi_is_var("upop");
  uimap   = cgi_is_var("uimap");
  udialup = cgi_is_var("udialup");
  upassc  = cgi_is_var("upassc");
  uweb    = cgi_is_var("uweb");
  urelay  = cgi_is_var("urelay");
  uspam  = cgi_is_var("uspam");

  /* get the domain name */
  if (domain==NULL || strlen(domain)==0) {
    global_warning("Create Domain: Failed: Must supply domain name");
    t_open("html/add_domain.html", 1);
  }

  /* get the password */
  if (passwd==NULL || strlen(passwd)==0 ) {
    global_warning("Create Domain: Failed: Must supply password");
    t_open("html/add_domain.html", 1);
  }

  /* add the domain with defaults */
  ret = vadddomain(domain, VPOPMAILDIR, VPOPMAILUID, VPOPMAILGID );
  if (ret != VA_SUCCESS) {
    global_warning(verror(ret));
    t_open(T_MAIN, 1);
  } else {
    global_warning("Created Domain");
  }

  /* setup the .qmailadmin-limits file */
  vget_assign(domain,dir,156,&uid,&gid);
  strncat(dir,"/.qmailadmin-limits", 156);
  if ( (fs = fopen(dir,"w+")) == NULL ) {
    global_warning("Create Domain: open .qmailadmin-limits failed");
    t_open(T_MAIN, 1);
  }

  if (lusers!=NULL&&strlen(lusers)>0)  
    fprintf(fs, "maxpopaccounts: %s\n", lusers);

  if (lalias!=NULL&&strlen(lalias)>0)  
    fprintf(fs, "maxaliases: %s\n", lalias);

  if (lfor!=NULL&&strlen(lfor)>0)  
    fprintf(fs, "maxforwards: %s\n", lfor);

  if (lresponder!=NULL&&strlen(lresponder)>0)  
    fprintf(fs, "maxautoresponders: %s\n", lresponder);

  if (llists!=NULL&&strlen(llists)>0)  
    fprintf(fs, "maxmailinglists: %s\n", llists);

  if (quota!=NULL && strlen(quota)>0) 
    fprintf(fs,"default_quota: %s\n",quota);

  if (upop!=NULL)  fprintf(fs, "disable_pop\n");
  if (uimap!=NULL) fprintf(fs, "disable_imap\n");
  if (udialup!=NULL) fprintf(fs, "disable_dialup\n");
  if (upassc!=NULL) fprintf(fs, "disable_password_changing\n");
  if (uweb!=NULL) fprintf(fs, "disable_webmail\n");
  if (urelay!=NULL) fprintf(fs, "disable_external_relay\n");
  if (uspam!=NULL) fprintf(fs, "disable_spamassasin\n");
  fclose(fs);
  chown(dir,uid, gid);
  chmod(dir, S_IRUSR | S_IWUSR);

  ret = vadduser("postmaster", domain, passwd, "Postmaster", USE_POP );
  if (ret != VA_SUCCESS) {
    global_warning(verror(ret));
    t_open(T_MAIN, 1);
  } else {
    global_warning("Domain postmaster added");
  }

#ifdef ENABLE_ISOQLOG
  add_isoqlog(domain); /* add the domain to isoqlog's domains file */
#endif

  t_open(T_MAIN, 1);

}

void del_domain()
{
 char *domain;
 int   ret;

  if (!(acl_features & ACL_DOMAIN_DELETE)) {
    global_warning("Delete Domain: Permission denied");
    t_open(T_MAIN, 1);
  }

  domain = cgi_is_var("dname");
  if (domain==NULL || strlen(domain)==0 ) {
    global_warning("Delete Domain: Failed: Must supply domain name");
    t_open("html/del_domain.html", 1);
  }
  
  ret = vdeldomain(domain);
  if (ret != VA_SUCCESS) global_warning("Delete Domain: Failed");
  else global_warning("Deleted Domain");
  
#ifdef ENABLE_ISOQLOG
  del_isoqlog(domain); /* remove the domain from the isoqlog domains file */
#endif
  
  t_open(T_MAIN, 1);
}

void view_domain()
{
 char *domain;


  if (!(acl_features & ACL_DOMAIN_VIEW)) {
    global_warning("View Domain: Permission denied");
    t_open(T_MAIN, 1);
  }

  domain = cgi_is_var("dname");
  if (domain==NULL || strlen(domain)==0 ) {
    global_warning("View Domain: Failed: Must supply domain name");
    t_open("html/view_domain.html", 1);
  }

  post_domain_info(domain);

  t_open("html/mod_domain.html", 1);

}


void mod_domain()
{
 char *domain = NULL;
 char *ppass = NULL;
 char *lusers = NULL;
 char *lfor = NULL;
 char *lalias = NULL;
 char *lresponder = NULL;
 char *llists = NULL;
 char *quota = NULL;
 char *upop = NULL;
 char *uimap = NULL;
 char *udialup = NULL;
 char *upassc = NULL;
 char *uweb = NULL;
 char *urelay = NULL;
 int   ret;
 char dir[156];
 uid_t uid;
 gid_t gid;
 FILE *fs;

  if (!(acl_features & ACL_DOMAIN_MOD)) {
    global_warning("Mod Domain: Permission denied");
    t_open(T_MAIN, 1);
  }

  domain = cgi_is_var("dname");

  /* get the domain name */
  if (domain==NULL || strlen(domain)==0) {
    global_warning("Mod Domain: Failed: Must supply domain name");
    t_open("html/mod_domain.html", 1);
  }

  lusers = cgi_is_var("lusers");
  lfor = cgi_is_var("lfor");
  lalias = cgi_is_var("lalias");
  lresponder = cgi_is_var("lresponder");
  llists = cgi_is_var("llists");
  quota   = cgi_is_var("quota");
  upop    = cgi_is_var("upop");
  uimap   = cgi_is_var("uimap");
  udialup = cgi_is_var("udialup");
  upassc  = cgi_is_var("upassc");
  uweb    = cgi_is_var("uweb");
  urelay  = cgi_is_var("urelay");

  vget_assign(domain,dir,156,&uid,&gid);
  strncat(dir,"/.qmailadmin-limits", 156);
  if ( (fs = fopen(dir,"w+")) == NULL ) {
    global_warning("Create Domain: open .qmailadmin-limits failed");
    t_open(T_MAIN, 1);
  }
  if ( lusers!=NULL && strlen(lusers) > 0 ) 
    fprintf(fs, "maxpopaccounts: %s\n", lusers);
  if ( lalias!=NULL && strlen(lalias) > 0 ) 
    fprintf(fs, "maxaliases: %s\n", lalias);
  if ( lfor!=NULL && strlen(lfor) > 0 ) 
    fprintf(fs, "maxforwards: %s\n", lfor);
  if ( lresponder!=NULL && strlen(lresponder) > 0 ) 
    fprintf(fs, "maxautoresponders: %s\n", lresponder);
  if ( llists!=NULL && strlen(llists) > 0 ) 
    fprintf(fs, "maxmailinglists: %s\n", llists);
  if (quota!=NULL && strlen(quota)>0) 
    fprintf(fs,"default_quota: %s\n",quota);

  if (upop!=NULL)  fprintf(fs, "disable_pop\n");
  if (uimap!=NULL) fprintf(fs, "disable_imap\n");
  if (udialup!=NULL) fprintf(fs, "disable_dialup\n");
  if (upassc!=NULL) fprintf(fs, "disable_password_changing\n");
  if (uweb!=NULL) fprintf(fs, "disable_webmail\n");
  if (urelay!=NULL) fprintf(fs, "disable_external_relay\n");

  fclose(fs);
  chown(dir,uid, gid);
  chmod(dir, S_IRUSR | S_IWUSR);

  ppass = cgi_is_var("ppass");
  if (ppass!=NULL && strlen(ppass)>0) {
    ret = vpasswd("postmaster", domain, ppass, USE_POP);
    if ( ret != VA_SUCCESS ) {
      snprintf(WarningBuff, MAX_WARNING_BUFF, 
          "Postmaster Password error %s", verror(ret)); 
      global_warning(WarningBuff);
    } else {
      global_warning("Postmaster password set");
    }
  } 

  post_domain_info(domain);

  t_open("html/mod_domain.html", 1);

}

void post_domain_info(char *domain)
{
 char Dir[156];
 char cuid[10];
 char cgid[10];
 char cusers[10];
 char *tmpstr1;
 char *tmpstr2;
 uid_t uid;
 gid_t gid;
 FILE *fs;
 struct vqpasswd *vpw;

  if ( vget_assign(domain,Dir,156,&uid,&gid) == NULL ) {
    snprintf(WarningBuff, MAX_WARNING_BUFF, 
        "Domain %s does not exist", domain); 
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_open("html/view_domain.html", 1);
  }
  global_par("DN", domain);
  global_par("DD", Dir);

  sprintf(cuid,"%lu", (long unsigned)uid);
  global_par("DU", cuid);

  sprintf(cgid,"%lu", (long unsigned)gid);
  global_par("DG", cgid);

  open_big_dir(domain, uid, gid);
  close_big_dir(domain,uid,gid);

  sprintf(cusers,"%lu", (long unsigned)vdir.cur_users);
  global_par("DS", cusers);

  vpw = vauth_getpw("postmaster", domain);
  if ( vpw != NULL ) global_par("DP", vpw->pw_clear_passwd);
  else global_par("DP", "Domain has no postmaster!!");

  strncat(Dir,"/.qmailadmin-limits", 156);
  fs = fopen(Dir,"r");
  if ( fs != NULL ) {
    global_par("QL", "CHECKED");
    while(fgets(Dir,156,fs)!=NULL) {
      if ( (tmpstr1 = strtok(Dir,TOKENS))==NULL) continue;

      if ( strcmp(tmpstr1, "maxpopaccounts") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("MU", tmpstr2);

      } else if ( strcmp(tmpstr1, "maxaliases") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("MA", tmpstr2);

      } else if ( strcmp(tmpstr1, "maxforwards") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("MF", tmpstr2);

      } else if ( strcmp(tmpstr1, "maxautoresponders") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("MR", tmpstr2);

      } else if ( strcmp(tmpstr1, "maxmailinglists") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("ML", tmpstr2);

      } else if ( strcmp(tmpstr1, "quota") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("MQ", tmpstr2);

      } else if ( strcmp(tmpstr1, "default_quota") == 0 ) {
        if ( (tmpstr2 = strtok(NULL,TOKENS))==NULL) continue;
        global_par("MQ", tmpstr2);

      } else if ( strcmp(tmpstr1, "disable_pop") == 0 ) {
        global_par("MP", "checked");

      } else if ( strcmp(tmpstr1, "disable_imap") == 0 ) {
        global_par("MI", "checked");

      } else if ( strcmp(tmpstr1, "disable_dialup") == 0 ) {
        global_par("MD", "checked");

      } else if ( strcmp(tmpstr1, "disable_password_changing") == 0 ) {
        global_par("MC", "checked");

      } else if ( strcmp(tmpstr1, "disable_external_relay") == 0 ) {
        global_par("MS", "checked");
		
	  } else if ( strcmp(tmpstr1, "disable_spamassassin") == 0 ) {
		global_par("MZ", "checked");

      } else if ( strcmp(tmpstr1, "disable_webmail") == 0 ) {
        global_par("MW", "checked");

      }
    }
    fclose(fs);
  } else {
    global_par("QU", "CHECKED");
  }

}

void list_domains()
{
 char *domain;
 int   matchit = 0;
 FILE *fs;
 char *tmpbuf;
 char *assign_domain;
 char *assign_alias_domain;
 char bgcolor[30];
 char fgcolor[30];
 char face[30];
 char size[30];

  tmpbuf = malloc(500);
  if (!(acl_features & ACL_DOMAIN_VIEW)) {
    global_warning("List Domains: Permission denied");
    t_open(T_MAIN, 1);
  }

  domain = cgi_is_var("dname");
  if (domain!=NULL && strlen(domain)>0 ) {
    matchit = 1;
  }

  memset(bgcolor, 0, 30);
  memset(fgcolor, 0, 30);
  memset(face, 0, 30);
  memset(size, 0, 30);
  strncpy( bgcolor, get_lang_code("055"), 30);
  strncpy( fgcolor, get_lang_code("056"), 30);
  strncpy( face, get_lang_code("057"), 30);
  strncpy( size, get_lang_code("058"), 30);

  printf("<HTML><HEAD><TITLE>List Domains</TITLE></HEAD>\n");
  printf("<body bgcolor=%s vlink=%s link=%s alink=%s>\n",
    bgcolor, fgcolor, fgcolor, fgcolor);
  printf("<FONT face=\"%s\" SIZE=\"%s\" color=\"%s\">\n",
    face, size, fgcolor);

  if ( matchit == 1 ) printf("<B>Domains containing %s</B><BR>\n", domain);
  else printf("<B>All domains</B><BR>\n");

  snprintf(tmpbuf, 500, "%s/users/assign", QMAILDIR);
  if ( (fs = fopen(tmpbuf, "r")) == NULL ) {
    global_warning("List Domains: could not open assign file");
    t_open(T_MAIN, 1);
  }

  while( fgets(tmpbuf,500,fs) != NULL ) {
    if ( (assign_domain = strtok(tmpbuf, TOKENS)) == NULL ) continue;
    if ( (assign_alias_domain = strtok(NULL, TOKENS)) == NULL ) continue;

    /* skip the first + character */
    ++assign_domain;

    /* skip the last - character */
    assign_domain[strlen(assign_domain)-1] = 0;
    if ( matchit == 1 && strstr(assign_domain, domain) == NULL ) continue;

    if ( strcmp(assign_domain, assign_alias_domain) == 0 ) {
      printf("<a href=vqadmin.cgi?nav=view_domain&dname=%s>%s</a><BR>\n",
        assign_alias_domain, assign_alias_domain);
    } else {
      printf(
"<a href=vqadmin.cgi?nav=view_domain&dname=%s>%s</a> Aliased to %s<BR>\n",
        assign_alias_domain, assign_domain, assign_alias_domain);
    }
  }
  fclose(fs);
    
  printf("<HR>\n");
  printf("<a href=\"/cgi-bin/vqadmin/vqadmin.cgi\">Main VqAdmin Menu</a><BR><BR>\n");
  printf("<a href=http://www.inter7.com/vqadmin/>%s</a> %s<BR>\n", 
    VQA_PACKAGE, VQA_VERSION);
  printf("<a href=http://www.inter7.com/vpopmail/>%s</a> %s<BR>\n", 
    PACKAGE, VERSION);

  free(tmpbuf);
  vexit(0);

}

void add_alias_domain()
{
 char *domain = NULL;
 char *alias_domain = NULL;
 int ret;

  if (!(acl_features & ACL_DOMAIN_CREATE)) {
    global_warning("Add Alias Domain: Permission denied");
    t_open(T_MAIN, 1);
  }

  domain = cgi_is_var("dname");
  alias_domain = cgi_is_var("adname");

  /* get the domain name */
  if (domain==NULL || strlen(domain)==0) {
    global_warning("Add Alias Domain: Failed: Must supply domain name");
    t_open("html/add_domain.html", 1);
  }

  /* get the domain name */
  if (alias_domain==NULL || strlen(alias_domain)==0) {
    global_warning("Add Alias Domain: Failed: Must supply alias domain name");
    t_open("html/add_domain.html", 1);
  }

  /* add the domain with defaults */
  ret = vaddaliasdomain(alias_domain, domain);
  if (ret != VA_SUCCESS) {
    global_warning(verror(ret));
    t_open(T_MAIN, 1);
  } else {
    global_warning("Alias Domain Added");
  }

  t_open(T_MAIN, 1);

}



#ifdef ENABLE_ISOQLOG

void add_isoqlog (char *domain) 
{
 FILE *infile; /* file pointer for existing domains file */
 FILE *tmpfile; /* file pointer for temporary file */
 char buffer[100]; /* buffer for handling file I/O */
 char *dom; /* pointer to temp buffer */
 char tmpbuf[100];
 char status[100];
	
  snprintf(tmpbuf, 100, "%s.tmp", ISOQLOGPATH);
  infile = fopen(ISOQLOGPATH, "a+"); /* open the existing domains file */
  if (infile == NULL) {
    snprintf(status, 100, "Error: Unable to open input file %s You may \
have to add the domain to isoqlog manually", ISOQLOGPATH);
    global_warning(status);
    return;
  } /* reports error opening input file */
	
  tmpfile = fopen(tmpbuf,"w+"); /* open the temporary file */
  if (tmpfile == NULL) {
   snprintf(status, 100, "Error: Unable to open temporary file %s You \
may have to add the domain to isoqlog manually.", tmpbuf);
    global_warning(status);
    return;
  } /* reports error opening temp file */
	
	
  /* 
   * this loop is pretty pointless.  it simply copies one file into the other
   * and then we add to it right after the loop completes.  However, it
   * does attempt to clean up a messy file, which is good, who likes
   * messy files? :)
   */
	
  /* while there's something to be read */
  while( fgets (buffer, 100, infile) != NULL ) {
    /* munge */
    dom = strtok( buffer, " \n\t\r"); /* munge */

    if (dom == NULL) continue; /* don't write blank lines */
    fprintf(tmpfile, "%s\n", dom); /* write the line */
  }	
	
  fprintf(tmpfile, "%s\n", domain); /* finally we write our new domain */
	
  fclose(infile); fclose(tmpfile); /* close the files */
	
  rename ( tmpbuf, ISOQLOGPATH ); /* rename the temp file */
}

void del_isoqlog (char *domain) 
{
 FILE *infile; /* file pointer for existing domains file */
 FILE *tmpfile; /* file pointer for temporary file */
 char buffer[100]; /* buffer for handling file I/O */
 char *dom; /* pointer to temp buffer */
 char tmpbuf[100];
 char status[100];
	
  snprintf(tmpbuf, 100, "%s.tmp", ISOQLOGPATH);
	
  infile = fopen(ISOQLOGPATH, "r+"); /* open the existing domains file */
  if (infile == NULL) {
    snprintf(status, 100, "Error: Unable to open input file %s you may \
have to remove the domain from isoqlog manually.", ISOQLOGPATH);
    global_warning(status);
    return;
  } /* reports error opening input file */
	
  tmpfile = fopen(tmpbuf,"w+"); /* open the temporary file */
  if (tmpfile == NULL) {
    snprintf(status, 100, "Error: Unable to open temporary file %s you may have to remove the domain from isoqlog manually", tmpbuf);
    global_warning(status);
    return;
  } /* reports error opening temp file */
	
  while( fgets (buffer, 100, infile) != NULL ) { /* while there's something to be read */
    dom = strtok( buffer, " \n\t\r"); /* munge */
    if (dom == NULL) continue; /* blank line */
    if (strcmp(dom, domain) == 0) continue; /* hey! we found our domain! */
    fprintf(tmpfile, "%s\n", dom); /* nope, wasn't him, spit it out */
  }	
	
   fclose(infile); fclose(tmpfile); /* close files */
	
   rename ( tmpbuf, ISOQLOGPATH ); /* move the modified file into place */
}

#endif
