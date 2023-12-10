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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <vauth.h>
#include <stdint.h>

#define MAX_WARNING_BUFF 500

/*
   Access list stuff
*/
#define ACL_NONE            0L
#define ACL_USER_CREATE     1
#define ACL_USER_DELETE     2
#define ACL_USER_VIEW       4 
#define ACL_USER_MOD        8 
#define ACL_DOMAIN_CREATE  16
#define ACL_DOMAIN_DELETE  32
#define ACL_DOMAIN_VIEW    64
#define ACL_DOMAIN_MOD    128

#define ACL_ALL ( \
ACL_USER_CREATE |  \
ACL_USER_DELETE |  \
ACL_USER_VIEW |  \
ACL_USER_MOD |  \
ACL_DOMAIN_CREATE |  \
ACL_DOMAIN_DELETE |  \
ACL_DOMAIN_VIEW |  \
ACL_DOMAIN_MOD ) 

#define ACL_FILENAME "vqadmin.acl"

/*
   Variable maximum lengths
*/
#define MAX_CONTENT_LENGTH 10000
#define MAX_GLOBAL_LENGTH 500
#define MAX_TEMPLATE_LINE_LENGTH 500
#define MAX_QUOTA_LENGTH 13
#define MAX_PASSWORD_LENGTH 28
#define MAX_USERNAME_LENGTH 28
#define MAX_EMAIL_LENGTH (64 + 1 + MAX_USERNAME_LENGTH + 1)

/*
   Template definitions
*/
#define T_INIT_ERROR "html/init_error.html"
#define T_ERROR "html/error.html"
#define T_AUTH_FAILED "html/auth_failed.html"
#define T_MAIN "html/main.html"
#define T_EDIT "html/edit.html"
#define T_QONTROL "html/control.html"
#define T_CTRL_FILE "html/ctrl_file.html"

/* main.c */
void send_html(char *command);

/* cgi.c */
void cgi_nav(void);
void cgi_var(void);
void cgi_parse_hex(void);
void cgi_env(void);
void cgi_init(void);
char *cgi_is_env(char *);
char *cgi_is_var(char *);
char matoh(char);
unsigned char hex2asc(char, char);

/* template.c */
void t_code(char);
void g_code(char *);
void t_printf(char *);
void t_open(char *template, int exit_when_done);

/* global.c */
void global_init(void);
void global_error(char *, char, char);
void global_warning(char *);
void global_par(char *, char *);
char *f_global_par(char *);
void global_f_warning(void);
void global_exit(int exit_code);

/* acl.c */
void acl_init(void);
void acl_read(void);
void acl_parse(char *);
char acl_parse_features(char *);
int acl_parse_multi(char *);

/* edit.c */
void go_edit(char *);
void u_edit(void);
int check_box(char *, char, char, struct vqpasswd *, struct vqpasswd *, char *);

/* misc.c */
char *mstrdup(char *);
void tfatal(void);

/* domain.c */
void add_domain();
void del_domain();
void view_domain();
void list_domains();
void mod_domain();
void post_domain_info(char *domain);
void add_alias_domain();

/* lang.c */
void set_language();
int open_lang( char *lang);
void put_lang_code( char *index );
char *get_lang_code( char *index );

/* user.c */
void add_user();
void del_user();
void view_user();
void mod_user();
void post_email_info( char *eaddr, struct vqpasswd *vpwd, char *domain);
void show_users();

/* cedit.c */
void display_file();
void modify_file();
void show_controls();
void delete_file();

typedef uint64_t storage_t;
int quota_to_bytes(char[], char*);     //jhopper prototype
int quota_to_megabytes(char[], char*); //jhopper prototype
