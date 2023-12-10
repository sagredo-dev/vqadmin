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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "global.h"

struct var_t {
  char *name,
       *data;

  struct var_t *next;
};

struct env_t {
  char *name,
       *data;

  struct env_t *next;
};

extern char **environ;
extern char vqa_user[],
            vqa_group[];

unsigned int content_length = 0;
char *gdata = NULL;
struct var_t *varlist = NULL;
struct env_t *envlist = NULL;

/*
   Determine what user is trying to do based on variables supplied
*/
void cgi_nav(void)
{
  char *r = NULL, i = 0;

  i = 0;

  r = cgi_is_var("nav");
  if (r) {
    if (!(strcasecmp(r, "add_domain"))) {
      add_domain();     
    } else if (!(strcasecmp(r, "del_domain"))) {
      del_domain();     
    } else if (!(strcasecmp(r, "view_domain"))) {
      view_domain();     
    } else if (!(strcasecmp(r, "add_user"))) {
      add_user();     
    } else if (!(strcasecmp(r, "del_user"))) {
      del_user();     
    } else if (!(strcasecmp(r, "view_user"))) {
      view_user();     
    } else if (!(strcasecmp(r, "mod_user"))) {
      mod_user();     
    } else if (!(strcasecmp(r, "show_users"))) {
      show_users();     
    } else if (!(strcasecmp(r, "show_controls"))) {
      show_controls();     
    } else if (!(strcasecmp(r, "mod_domain"))) {
      mod_domain();     
    } else if (!(strcasecmp(r, "list_domains"))) {
      list_domains();     
    } else if (!(strcasecmp(r, "add_alias_domain"))) {
      add_alias_domain();     
    } else if (!(strcasecmp(r, "display_file"))) {
      display_file();
    } else if (!(strcasecmp(r, "modify_file"))) {
      modify_file();
    } else if (!(strcasecmp(r, "delete_file"))) {
      delete_file();
    }		  
  }
  
  t_open(T_MAIN, 1);
}

/*
   Parse out information in stdin, if any

   gdata contains parsed out data, which is pointed to by
   the var_t linked list.
*/
void cgi_var(void)
{
 struct var_t *v = NULL, *vt = NULL;
 char *h = NULL, *t = NULL, *n = NULL, *d = NULL;  

  cgi_parse_hex();

  varlist = (struct var_t *)malloc(sizeof(struct var_t));
  varlist->next = NULL;
  vt = varlist;

  for (n = NULL, d = NULL, h = t = gdata;;) {
    if ((*h == '=') && (n == NULL)) {
      *h++ = '\0';
      n = t;
      t = h;
    } else if (((*h == '&') && (n != NULL) && (d == NULL)) ||
               ((!(*h)) && (n != NULL) && (d == NULL))) {
      if (*h) *h++ = '\0';
      else h = NULL;

      d = t;
      t = h;
       
      v = (struct var_t *)malloc(sizeof(struct var_t));
      if (v == NULL) global_error("Out of memory", 1, 0);
         
      v->name = n;
      v->data = d;
      v->next = NULL;

      vt->next = v;
      vt = v;

      n = d = NULL;         

      if (h == NULL) break;
    } else {
      h++;
    }
  }
}

/*
   Convert all hex codes in content to ascii
*/
void cgi_parse_hex(void)
{
 unsigned int len = 0;
 unsigned char r = 0;
 char *p = NULL;

 len = content_length;

  for (p = gdata; *p; p++) {
    len--;

    if (*p == '%') {
      if ((*(p + 1)) && (*(p + 2))) {
        r = hex2asc(*(p + 1), *(p + 2));
            
        *p = r;

        memmove((char *)(p + 1), (char *)(p + 3), (len - 2));

        *(p + len - 1) = '\0';
      }         
    }
  }
}

/*
   Fetch HTTP environment variables
*/
void cgi_env(void)
{
 int i = 0;
 struct env_t *et = NULL, *e = NULL;
 char *t = NULL, *n = NULL, *v = NULL; 
 int elen = 0, nlen = 0, vlen = 0;

  envlist = (struct env_t *)malloc(sizeof(struct env_t));
  if (envlist == NULL) global_error("Out of memory", 1, 0);

  envlist->next = NULL;
  et = envlist;  

  for (i = 0; environ[i]; i++) {
    for (t = environ[i]; *t != '='; t++);

    elen = strlen(environ[i]);
    nlen = (elen - strlen(t));
    vlen = (elen - nlen);

    e = (struct env_t *)malloc(sizeof(struct env_t));
    if (e == NULL) global_error("Out of memory", 1, 0);         
          
    n = (char *)malloc(nlen + 1);
    if (n == NULL) global_error("Out of memory", 1, 0);

    v = (char *)malloc(vlen + 1);
    if (v == NULL) global_error("Out of memory", 1, 0);

    e->name = n;
    e->data = v;
    e->next = NULL;

    memset(n, 0, (nlen + 1));
    memset(v, 0, (vlen + 1));

    memcpy(n, environ[i], nlen);
    memcpy(v, (++t), vlen);

    et->next = e;
    et = e;
  }
}

void cgi_init(void)
{
 int val = 0;
 char *env = NULL;
 int ret;

  cgi_env();

  env = cgi_is_env("REMOTE_USER");
  if (!env) {
    global_error("Username unknown", 0, 1);
    t_open(T_AUTH_FAILED, 1);
  }

  memcpy((char *)vqa_user, (char *)env, MAX_GLOBAL_LENGTH);
   
  acl_init();

  env = cgi_is_env("REQUEST_METHOD");
  if (env == NULL) global_error("Unknown request method", 1, 0);

  if (strcasecmp(env, "POST")==0) {
    env = cgi_is_env("CONTENT_LENGTH");
    if (env == NULL) global_error("Unknown content", 1, 0);

    val = atoi(env);
    if (val < 1) global_error("Invalid content length", 1, 0);

    if (val > MAX_CONTENT_LENGTH) val = (MAX_CONTENT_LENGTH - 1);

    content_length = val;
    gdata = malloc(content_length + 1);
    if (gdata == NULL) global_error("Out of memory", 1, 0);
 
    memset((unsigned char *)gdata, 0, (content_length + 1));

    ret = read(0, (unsigned char *)gdata, content_length);
    if (ret != content_length) global_error("Invalid content length", 1, 0);

    cgi_var();

  } else if (strcasecmp(env, "GET") == 0 ) {
    env = getenv("QUERY_STRING");
    if (env == NULL) global_error("No Query String", 1, 0);

    content_length = strlen(env);
    gdata = env; 
    cgi_var();

    /*t_open(T_MAIN);*/

  } else {
    global_error("Unsupported request method", 1, 0);
    t_open(T_MAIN, 1);
  }
}

char *cgi_is_env(char *name)
{
 struct env_t *e = NULL;

  for (e = envlist; e->next; e = e->next) {
    if (!(strcmp(e->next->name, name))) return e->next->data;
  }
  return NULL;
}

char *cgi_is_var(char *name)
{
 struct var_t *v = NULL;

  for (v = varlist; v->next; v = v->next) {
    if (!(strcmp(v->next->name, name))) return v->next->data;
  }

  return NULL;
}

char matoh(char x)
{
  char ret = 0;

  switch(x) {
   case '0':
    ret = 0;
    break;
   case '1':
    ret = 1;
    break;
   case '2':
    ret = 2;
    break;
   case '3':
    ret = 3;
    break;
   case '4':
    ret = 4;
    break;
   case '5':
    ret = 5;
    break;
   case '6':
    ret = 6;
    break;
   case '7':
    ret = 7;
    break;
   case '8':
    ret = 8;
    break;
   case '9':
    ret = 9;
    break;
   case 'A':
    ret = 10;
    break;
   case 'B':
    ret = 11;
    break;
   case 'C':
    ret = 12;
    break;
   case 'D':
    ret = 13;
    break;
   case 'E':
    ret = 14;
    break;
   case 'F':
    ret = 15;
    break;
   default:
    ret = -1;
    break;
  }
  return ret;
}

unsigned char hex2asc(char s, char f)
{
 char ret = 0;
 unsigned char val1 = 0, val2 = 0, val3 = 0;

  ret = matoh(f);
  if (ret == -1) return 0;

  val1 = ret;

  ret = matoh(s);
  if (ret == -1) return 0;

  val2 = ret;

  val3 = val1 + (val2 * 15) + val2;
  return val3;
}
