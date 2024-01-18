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
#include "vlimits.h"

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

 char *upop = NULL;
 char *uimap = NULL;
 char *udialup = NULL;
 char *upassc = NULL;
 char *uweb = NULL;
 char *urelay = NULL;
 char *uspam = NULL;
 char *usmtp = NULL;
 char *udeletespam  = NULL;
 char *umaildrop  = NULL;

 char *quota = NULL;
 char *maxmsgcount = NULL;
 char *defaultquota = NULL;
 char *defaultmaxmsgcount = NULL;
 storage_t tmpdefaultquota;

 int   ret;
 int limitsmodified = 0;
 struct vlimits defaultlimits;
 struct vlimits limits;

  if (!(acl_features & ACL_DOMAIN_CREATE)) {
    global_warning("Create Domain: Permission denied");
    t_main(1);
  }

  /* get the domain name */
  domain = cgi_is_var("dname");
  if (domain==NULL || strlen(domain)==0) {
    global_warning("Create Domain: Failed: Must supply domain name");
    t_page("html/add_domain.html", 1);
  }

  /* add the domain with defaults */
  ret = vadddomain(domain, VPOPMAILDIR, VPOPMAILUID, VPOPMAILGID );
  if (ret != VA_SUCCESS) {
    global_warning(verror(ret));
    t_page("html/add_domain.html",1);
  } else {
    global_warning("Created Domain");
  }

  /* get the password for postmaster */
  passwd = cgi_is_var("ppass");
  if (passwd==NULL || strlen(passwd)==0 ) {
    global_warning("Create Domain: Failed: Must supply password");
    t_page("html/add_domain.html", 1);
  }

  /* add the postmaster user */
  ret = vadduser("postmaster", domain, passwd, "Postmaster", USE_POP );
  if (ret != VA_SUCCESS) {
    global_warning(verror(ret));
    t_main(1);
  } else {
    global_warning("Domain postmaster added");
  }

//-------------- FORM VALUES --------------

  //-------------- DOMAIN LIMITS --------------
  lusers = cgi_is_var("lusers");
  lfor = cgi_is_var("lfor");
  lalias = cgi_is_var("lalias");
  lresponder = cgi_is_var("lresponder");
  llists = cgi_is_var("llists");
  quota   = cgi_is_var("quota");
  maxmsgcount = cgi_is_var("maxmsgcount");
  defaultquota = cgi_is_var("defaultquota");
  defaultmaxmsgcount = cgi_is_var("defaultmaxmsgcount");

  //-------------- DOMAIN PERMISSIONS --------------
  upop    = cgi_is_var("upop");
  uimap   = cgi_is_var("uimap");
  udialup = cgi_is_var("udialup");
  upassc  = cgi_is_var("upassc");
  uweb    = cgi_is_var("uweb");
  urelay  = cgi_is_var("urelay");
  uspam  = cgi_is_var("uspam");
  usmtp  = cgi_is_var("usmtp");
  udeletespam  = cgi_is_var("udeletespam");
  umaildrop  = cgi_is_var("umaildrop"); 

  // INITIALIZE DEFAULT STRUCTS
  vdefault_limits (&defaultlimits);

  // FETCH DEFAULT DOMAIN LIMITS FROM "VLIMITS_DEFAULT_FILE"
  if (vlimits_read_limits_file (VLIMITS_DEFAULT_FILE, &defaultlimits) != 0) {
    snprintf(WarningBuff, MAX_WARNING_BUFF,"Failed to get limits from vlimits_default_file for domain %s", domain); 
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_page("html/view_domain.html", 1);
  }

  // CLONE DEFAULT LIMITS TO DETECT IF THE USER CHANGED THE DEFAULTS
  memcpy(&limits, &defaultlimits, sizeof(limits));

  // PROCESS NEW LIMITS
  if (lusers!=NULL && strlen(lusers)>0) {
    limits.maxpopaccounts = atoi(lusers);
  }
  if (lfor!=NULL && strlen(lfor)>0) {
    limits.maxforwards = atoi(lfor);
  }
  if (lalias!=NULL && strlen(lalias)>0) {
    limits.maxaliases = atoi(lalias);
  }
  if (lresponder!=NULL && strlen(lresponder)>0) {
    limits.maxautoresponders = atoi(lresponder);
  }
  if (llists!=NULL && strlen(llists)>0) {
    limits.maxmailinglists = atoi(llists);
  }
  if (quota!=NULL && strlen(quota)>0) {
    limits.diskquota = strtoll(quota, NULL, 10);
  }
  if (maxmsgcount!=NULL && strlen(maxmsgcount)>0) {
    limits.maxmsgcount = strtoll(maxmsgcount, NULL, 10);
  }
  if (defaultquota!=NULL && strlen(defaultquota)>0) {
    // CONVERT bytes->Mbytes
    if ((tmpdefaultquota = strtoll(defaultquota, NULL, 10))) {
        tmpdefaultquota *= 1048576;
        limits.defaultquota = tmpdefaultquota;
    }
  }
  if (defaultmaxmsgcount!=NULL && strlen(defaultmaxmsgcount)>0) {
    limits.defaultmaxmsgcount = strtoll(defaultmaxmsgcount, NULL, 10);
  }

  // PROCESS NEW PERMISSIONS
  if (upop!=NULL) {limits.disable_pop = 1;}
  if (uimap!=NULL) {limits.disable_imap = 1;}
  if (udialup!=NULL) {limits.disable_dialup = 1;}
  if (upassc!=NULL) {limits.disable_passwordchanging = 1;}
  if (uweb!=NULL) {limits.disable_webmail = 1;}
  if (urelay!=NULL) {limits.disable_relay = 1;}
  if (uspam!=NULL) {limits.disable_spamassassin = 1;}
  if (usmtp!=NULL) {limits.disable_smtp = 1;}
  if (udeletespam!=NULL) {limits.delete_spam = 1;}
  if (umaildrop!=NULL) {limits.disable_maildrop = 1;}

  // DETECT DEVIATIONS FROM DEFAULT LIMITS, IF ANY
  if (limits.maxpopaccounts != defaultlimits.maxpopaccounts) {limitsmodified = 1;}
  if (limits.maxaliases != defaultlimits.maxaliases) {limitsmodified = 1;}
  if (limits.maxforwards != defaultlimits.maxforwards) {limitsmodified = 1;}
  if (limits.maxautoresponders != defaultlimits.maxautoresponders) {limitsmodified = 1;}
  if (limits.maxmailinglists != defaultlimits.maxmailinglists) {limitsmodified = 1;}
  if (limits.diskquota != defaultlimits.diskquota) {limitsmodified = 1;}
  if (limits.maxmsgcount != defaultlimits.maxmsgcount) {limitsmodified = 1;}
  if (limits.defaultquota != defaultlimits.defaultquota) {limitsmodified = 1;}
  if (limits.defaultmaxmsgcount != defaultlimits.defaultmaxmsgcount) {limitsmodified = 1;}

  // DETECT DEVIATIONS FROM DEFAULT PERMISSIONS, IF ANY
  if (limits.disable_pop != defaultlimits.disable_pop) {limitsmodified = 1;}
  if (limits.disable_imap != defaultlimits.disable_imap) {limitsmodified = 1;}
  if (limits.disable_dialup != defaultlimits.disable_dialup) {limitsmodified = 1;}
  if (limits.disable_passwordchanging != defaultlimits.disable_passwordchanging) {limitsmodified = 1;}
  if (limits.disable_webmail != defaultlimits.disable_webmail) {limitsmodified = 1;}
  if (limits.disable_relay != defaultlimits.disable_relay) {limitsmodified = 1;}
  if (limits.disable_smtp != defaultlimits.disable_smtp) {limitsmodified = 1;}
  if (limits.disable_spamassassin != defaultlimits.disable_spamassassin) {limitsmodified = 1;}
  if (limits.delete_spam != defaultlimits.delete_spam) {limitsmodified = 1;}
  if (limits.disable_maildrop != defaultlimits.disable_maildrop) {limitsmodified = 1;}

  // APPLY NEW LIMITS, IF ANY CHANGE DETECTED
  if (limitsmodified) {
    if (vset_limits(domain,&limits) != 0) {
      snprintf(WarningBuff, MAX_WARNING_BUFF,"Failed to set limits for domain %s", domain);
      global_warning(WarningBuff);
      global_par("DN", domain);
      t_page("html/view_domain.html", 1);
    }
  }

//-------------- DOMAIN LIMITS FINISHED --------------

#ifdef ENABLE_ISOQLOG
  add_isoqlog(domain); /* add the domain to isoqlog's domains file */
#endif

  t_page("html/view_domain.html", 1);
}

void del_domain()
{
 char *domain;
 int   ret;

  if (!(acl_features & ACL_DOMAIN_DELETE)) {
    global_warning("Delete Domain: Permission denied");
    t_main(1);
  }

  domain = cgi_is_var("dname");
  if (domain==NULL || strlen(domain)==0 ) {
    global_warning("Delete Domain: Failed: Must supply domain name");
    t_page("html/del_domain.html", 1);
  }

  ret = vdeldomain(domain);
  if (ret != VA_SUCCESS) global_warning("Delete Domain: Failed");
  else global_warning("Deleted Domain");

#ifdef ENABLE_ISOQLOG
  del_isoqlog(domain); /* remove the domain from the isoqlog domains file */
#endif

  t_main(1);
}

void view_domain()
{
 char *domain;


  if (!(acl_features & ACL_DOMAIN_VIEW)) {
    global_warning("View Domain: Permission denied");
    t_main(1);
  }

  domain = cgi_is_var("dname");
  if (domain==NULL || strlen(domain)==0 ) {
    global_warning("View Domain: Failed: Must supply domain name");
    t_page("html/view_domain.html", 1);
  }

  post_domain_info(domain);

  t_page("html/mod_domain.html", 1);
}


void mod_domain()
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
 char *usmtp = NULL;
 char *udeletespam  = NULL;
 char *umaildrop  = NULL;
 char *maxmsgcount = NULL;
 char *defaultquota = NULL;
 char *defaultmaxmsgcount = NULL;
 storage_t tmpdefaultquota;

 int ret;
 int limitsmodified = 0;
 struct vlimits defaultlimits;
 struct vlimits limits;

  if (!(acl_features & ACL_DOMAIN_MOD)) {
    global_warning("Mod Domain: Permission denied");
    t_main(1);
  }

  /* get the domain name */
  domain = cgi_is_var("dname");
  if (domain==NULL || strlen(domain)==0) {
    global_warning("Mod Domain: Failed: Missing domain name");
    t_page("html/mod_domain.html", 1);
  }

  /* Change the postmaster password (if requested) */
  passwd = cgi_is_var("ppass");
  if (passwd!=NULL && strlen(passwd)>0) {
    ret = vpasswd("postmaster", domain, passwd, USE_POP);
    if ( ret != VA_SUCCESS ) {
      snprintf(WarningBuff, MAX_WARNING_BUFF,
          "Postmaster Password error %s", verror(ret));
      global_warning(WarningBuff);
    } else {
      global_warning("Postmaster password set");
    }
  }

//-------------- FORM VALUES --------------

  //-------------- DOMAIN LIMITS --------------
  lusers = cgi_is_var("lusers");
  lfor = cgi_is_var("lfor");
  lalias = cgi_is_var("lalias");
  lresponder = cgi_is_var("lresponder");
  llists = cgi_is_var("llists");

  quota   = cgi_is_var("quota");
  maxmsgcount = cgi_is_var("maxmsgcount");
  defaultquota = cgi_is_var("defaultquota");
  defaultmaxmsgcount = cgi_is_var("defaultmaxmsgcount");

  //-------------- DOMAIN PERMISSIONS --------------
  upop    = cgi_is_var("upop");
  uimap   = cgi_is_var("uimap");
  udialup = cgi_is_var("udialup");
  upassc  = cgi_is_var("upassc");
  uweb    = cgi_is_var("uweb");
  urelay  = cgi_is_var("urelay");
  uspam  = cgi_is_var("uspam");
  usmtp  = cgi_is_var("usmtp");
  udeletespam  = cgi_is_var("udeletespam");
  umaildrop  = cgi_is_var("umaildrop");


  // INITIALIZE DEFAULT STRUCT
  vdefault_limits (&defaultlimits);

  // FETCH DEFAULT DOMAIN LIMITS FROM "VLIMITS_DEFAULT_FILE"
  if (vlimits_read_limits_file (VLIMITS_DEFAULT_FILE, &defaultlimits) != 0) {
    snprintf(WarningBuff, MAX_WARNING_BUFF,"Failed to get limits from vlimits_default_file for domain %s", domain); 
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_page("html/view_domain.html", 1);
  }

#ifdef ENABLE_MYSQL_LIMITS
  // DUMMY CALL TO "vget_limits", OTHERWISE "vdel_limits" WILL CORE DUMP AT THE END
  if (vget_limits(domain, &limits) != 0) {
    snprintf(WarningBuff, MAX_WARNING_BUFF,"Failed to vget_limits for domain %s", domain);
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_page("html/view_domain.html", 1);
  }
#endif

  // CLONE LIMITS TO CHECK IF THE USER CHANGED THE DEFAULTS
  memcpy(&limits, &defaultlimits, sizeof(limits));

  // PROCESS NEW LIMITS
  if (lusers!=NULL && strlen(lusers)>0) {
    limits.maxpopaccounts = atoi(lusers);
  }
  if (lfor!=NULL && strlen(lfor)>0) {
    limits.maxforwards = atoi(lfor);
  }
  if (lalias!=NULL && strlen(lalias)>0) {
    limits.maxaliases = atoi(lalias);
  }
  if (lresponder!=NULL && strlen(lresponder)>0) {
    limits.maxautoresponders = atoi(lresponder);
  }
  if (llists!=NULL && strlen(llists)>0) {
    limits.maxmailinglists = atoi(llists);
  }
  if (quota!=NULL && strlen(quota)>0) {
    limits.diskquota = strtoll(quota, NULL, 10);
  }
  if (maxmsgcount!=NULL && strlen(maxmsgcount)>0) {
    limits.maxmsgcount = strtoll(maxmsgcount, NULL, 10);
  }
  if (defaultquota!=NULL && strlen(defaultquota)>0) {
    // CONVERT bytes->Mbytes
    if ((tmpdefaultquota = strtoll(defaultquota, NULL, 10))) {
        tmpdefaultquota *= 1048576;
        limits.defaultquota = tmpdefaultquota;
    }
  }
  if (defaultmaxmsgcount!=NULL && strlen(defaultmaxmsgcount)>0) {
    limits.defaultmaxmsgcount = strtoll(defaultmaxmsgcount, NULL, 10);
  }

  // PROCESS NEW PERMISSIONS
  if (upop!=NULL) {limits.disable_pop = 1;}
  if (uimap!=NULL) {limits.disable_imap = 1;}
  if (udialup!=NULL) {limits.disable_dialup = 1;}
  if (upassc!=NULL) {limits.disable_passwordchanging = 1;}
  if (uweb!=NULL) {limits.disable_webmail = 1;}
  if (urelay!=NULL) {limits.disable_relay = 1;}
  if (uspam!=NULL) {limits.disable_spamassassin = 1;}
  if (usmtp!=NULL) {limits.disable_smtp = 1;}
  if (udeletespam!=NULL) {limits.delete_spam = 1;}
  if (umaildrop!=NULL) {limits.disable_maildrop = 1;}

  // DETECT DEVIATIONS FROM DEFAULT, IF ANY
  if (limits.maxpopaccounts != defaultlimits.maxpopaccounts) {limitsmodified = 1;}
  if (limits.maxaliases != defaultlimits.maxaliases) {limitsmodified = 1;}
  if (limits.maxforwards != defaultlimits.maxforwards) {limitsmodified = 1;}
  if (limits.maxautoresponders != defaultlimits.maxautoresponders) {limitsmodified = 1;}
  if (limits.maxmailinglists != defaultlimits.maxmailinglists) {limitsmodified = 1;}
  if (limits.diskquota != defaultlimits.diskquota) {limitsmodified = 1;}
  if (limits.maxmsgcount != defaultlimits.maxmsgcount) {limitsmodified = 1;}
  if (limits.defaultquota != defaultlimits.defaultquota) {limitsmodified = 1;}
  if (limits.defaultmaxmsgcount != defaultlimits.defaultmaxmsgcount) {limitsmodified = 1;}

  // DETECT DEVIATIONS FROM DEFAULT PERMISSIONS, IF ANY
  if (limits.disable_pop != defaultlimits.disable_pop) {limitsmodified = 1;}
  if (limits.disable_imap != defaultlimits.disable_imap) {limitsmodified = 1;}
  if (limits.disable_dialup != defaultlimits.disable_dialup) {limitsmodified = 1;}
  if (limits.disable_passwordchanging != defaultlimits.disable_passwordchanging) {limitsmodified = 1;}
  if (limits.disable_webmail != defaultlimits.disable_webmail) {limitsmodified = 1;}
  if (limits.disable_relay != defaultlimits.disable_relay) {limitsmodified = 1;}
  if (limits.disable_smtp != defaultlimits.disable_smtp) {limitsmodified = 1;}
  if (limits.disable_spamassassin != defaultlimits.disable_spamassassin) {limitsmodified = 1;}
  if (limits.delete_spam != defaultlimits.delete_spam) {limitsmodified = 1;}
  if (limits.disable_maildrop != defaultlimits.disable_maildrop) {limitsmodified = 1;}

  // PURGE EXISTING DOMAIN LIMITS
  if (vdel_limits(domain)!=0) {
    snprintf(WarningBuff, MAX_WARNING_BUFF,"Failed to reset limits for domain %s", domain); 
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_page("html/view_domain.html", 1);
  }

  // APPLY NEW LIMITS, IF ANY CHANGE DETECTED
  if (limitsmodified) {
    if (vset_limits(domain,&limits) != 0) {
      snprintf(WarningBuff, MAX_WARNING_BUFF,"Failed to set limits for domain %s", domain);
      global_warning(WarningBuff);
      global_par("DN", domain);
      t_page("html/view_domain.html", 1);
    }
  }
//-------------- DOMAIN LIMITS FINISHED --------------

  post_domain_info(domain);
  t_page("html/mod_domain.html", 1);
}

void post_domain_info(char *domain)
{
 char Dir[156];
 char cuid[11];
 char cgid[11];
 char cusers[11];
 char qconvert[11];
 uid_t uid;
 gid_t gid;
 struct vlimits limits;
 struct vqpasswd *vpw;
 storage_t tmpdefaultquota;

  if ( vget_assign(domain,Dir,156,&uid,&gid) == NULL ) {
    snprintf(WarningBuff, MAX_WARNING_BUFF, 
        "Domain %s does not exist", domain); 
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_page("html/view_domain.html", 1);
  }
  global_par("DN", domain);
  global_par("DD", Dir);

  snprintf(cuid, sizeof(cuid), "%lu", (long unsigned)uid);
  global_par("DU", cuid);

  snprintf(cgid, sizeof(cgid), "%lu", (long unsigned)gid);
  global_par("DG", cgid);

  open_big_dir(domain, uid, gid);
  close_big_dir(domain,uid,gid);

  snprintf(cusers, sizeof(cusers), "%lu", (long unsigned)vdir.cur_users);
  global_par("DS", cusers);

  vpw = vauth_getpw("postmaster", domain);
  if ( vpw != NULL ) global_par("DP", vpw->pw_clear_passwd);
  else global_par("DP", "Domain has no postmaster!!");

//	FETCH DOMAIN LIMITS

  if (vget_limits(domain, &limits) != 0) {
    snprintf(WarningBuff, MAX_WARNING_BUFF,
        "Failed to vget_limits for domain %s", domain); 
    global_warning(WarningBuff);
    global_par("DN", domain);
    t_page("html/view_domain.html", 1);
  } else {
    char buffer[20];

    if(limits.maxpopaccounts != -1) {
      snprintf(buffer, sizeof(buffer), "%d", limits.maxpopaccounts);
      global_par("MU", buffer);
    }

    if(limits.maxaliases != -1) {
      snprintf(buffer, sizeof(buffer), "%d", limits.maxaliases);
      global_par("MA", buffer);
    }

    if(limits.maxforwards != -1) {
      snprintf(buffer, sizeof(buffer), "%d", limits.maxforwards);
      global_par("MF", buffer);
    }

    if(limits.maxautoresponders != -1) {
      snprintf(buffer, sizeof(buffer), "%d", limits.maxautoresponders);
      global_par("MR", buffer);
    }

    if(limits.maxmailinglists != -1) {
      snprintf(buffer, sizeof(buffer), "%d", limits.maxmailinglists);
      global_par("ML", buffer);
    }

    if(limits.diskquota != 0) {
      snprintf(buffer, sizeof(buffer)+1, "%lu", limits.diskquota);
      global_par("MQ", buffer);
    }

    if(limits.maxmsgcount != 0) {
      snprintf(buffer, sizeof(buffer)+1, "%lu", limits.maxmsgcount);
      global_par("ME", buffer);
    }

    if(limits.defaultquota != 0) {
      tmpdefaultquota = limits.defaultquota/1048576.0;
      sprintf(qconvert, "%.2lf", (double)tmpdefaultquota);
      global_par("MB", qconvert);
    }

    if(limits.defaultmaxmsgcount != 0) {
      snprintf(buffer, sizeof(buffer)+1, "%lu", limits.defaultmaxmsgcount);
      global_par("MG", buffer);
    }

    if (limits.disable_pop) global_par("MP", "checked");
    if (limits.disable_imap) global_par("MI", "checked");
    if (limits.disable_dialup) global_par("MD", "checked");
    if (limits.disable_passwordchanging) global_par("MC", "checked");
    if (limits.disable_webmail) global_par("MW", "checked");
    if (limits.disable_relay) global_par("MS", "checked");
    if (limits.disable_smtp) global_par("MH", "checked");
    if (limits.disable_spamassassin) global_par("MZ", "checked");
    if (limits.delete_spam) global_par("ML", "checked");
    if (limits.disable_maildrop) global_par("MN", "checked");
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
    t_main(1);
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

  /* initialize html template */
  t_open(T_HEADER,0);
  /* ended. go on filling with the contents */

  if ( matchit == 1 ) printf("<h2>Domains containing <mark>%s</mark></h2>\n", domain);
  else printf("<h2>All domains</h2>\n");

  snprintf(tmpbuf, 500, "%s/users/assign", QMAILDIR);
  if ( (fs = fopen(tmpbuf, "r")) == NULL ) {
    global_warning("List Domains: could not open assign file");
    /* close the html template */
    t_open(T_COL,0);
    t_open(T_FOOTER,1);
  }

  printf("<nav class=\"nav flex-column\">\n");
  while( fgets(tmpbuf,500,fs) != NULL ) {
    if ( (assign_domain = strtok(tmpbuf, TOKENS)) == NULL ) continue;
    if ( (assign_alias_domain = strtok(NULL, TOKENS)) == NULL ) continue;

    /* skip the first + character */
    ++assign_domain;

    /* skip the last - character */
    assign_domain[strlen(assign_domain)-1] = 0;
    if ( matchit == 1 && strstr(assign_domain, domain) == NULL ) continue;

    if ( strcmp(assign_domain, assign_alias_domain) == 0 ) {
      printf("<a href=\"vqadmin.cgi?nav=view_domain&dname=%s\">%s</a>\n",
        assign_alias_domain, assign_alias_domain);
    } else {
      printf("<a href=\"vqadmin.cgi?nav=view_domain&dname=%s\">%s</a> Aliased to %s\n",
        assign_alias_domain, assign_domain, assign_alias_domain);
    }
  }
  printf("</nav>\n");
  fclose(fs);

  /* close the html template */
  t_open(T_COL,0);
  t_open(T_FOOTER,1);

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
    t_main(1);
  }

  domain = cgi_is_var("dname");
  alias_domain = cgi_is_var("adname");

  /* get the domain name */
  if (domain==NULL || strlen(domain)==0) {
    global_warning("Add Alias Domain: Failed: Must supply domain name");
    t_page("html/add_alias_domain.html", 1);
  }

  /* get the domain name */
  if (alias_domain==NULL || strlen(alias_domain)==0) {
    global_warning("Add Alias Domain: Failed: Must supply alias domain name");
    t_page("html/add_alias_domain.html", 1);
  }

  /* add the domain with defaults */
  ret = vaddaliasdomain(alias_domain, domain);
  if (ret != VA_SUCCESS) {
    global_warning(verror(ret));
    t_page("html/add_alias_domain.html", 1);
  } else {
    global_warning("Alias Domain Added");
  }

  t_main(1);
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
