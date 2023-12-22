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
#include "global.h"

char WarningBuff[MAX_WARNING_BUFF];

int warning_len = 0;
char vqa_error[MAX_GLOBAL_LENGTH],
     vqa_user[MAX_GLOBAL_LENGTH],
     vqa_group[MAX_GLOBAL_LENGTH],
     vqa_warning[MAX_GLOBAL_LENGTH];

struct par_t {
  char id[3],
       *val;

  struct par_t *next;
};


struct par_t *parlist = NULL, *pt = NULL;

void global_init(void)
{
  memset((char *)vqa_error, 0, MAX_GLOBAL_LENGTH);
  memset((char *)vqa_user, 0, MAX_GLOBAL_LENGTH);
  memset((char *)vqa_group, 0, MAX_GLOBAL_LENGTH);
  memset((char *)vqa_warning, 0, MAX_GLOBAL_LENGTH);

  warning_len = 0;

  parlist = (struct par_t *)malloc(sizeof(struct par_t));
  parlist->next = NULL;

  pt = parlist;
}

/*
   Defines a global error

   err         - Error
   init        - Error occured during initialization process
   recoverable - Error is recoverable

   vol@inter7.com
*/
void global_error(char *err, char init, char recoverable)
{
  memset((char *)vqa_error, 0, MAX_GLOBAL_LENGTH);
  memcpy((char *)vqa_error, (char *)err, MAX_GLOBAL_LENGTH);

  if (init) t_page(T_INIT_ERROR, 1);

  if (!recoverable) t_page(T_ERROR, 1);
}

void global_warning(char *warn)
{
  int len = 0;

  len = strlen(warn);
 
  if ((len + 1) >= (MAX_GLOBAL_LENGTH - warning_len)) return;  

  memcpy((char *)(vqa_warning + warning_len), 
         (char *)warn, (MAX_GLOBAL_LENGTH - warning_len));

  warning_len += len;

  *(vqa_warning + warning_len) = '$';

  warning_len++;

  *(vqa_warning + warning_len) = '\0';
}

void global_par(char *id, char *val)
{
 struct par_t *p = NULL;
 struct par_t *tmp_pt;

  for( tmp_pt = parlist; tmp_pt != NULL; tmp_pt = tmp_pt->next) {
    if ( tmp_pt->id[0] == id[0] && tmp_pt->id[1] == id[1] ) {
      free(tmp_pt->val);
      tmp_pt->val = (char *)malloc(strlen(val) + 1);
      if (tmp_pt->val == NULL) global_error("Out of memory", 0, 0);
      memset((char *)tmp_pt->val, 0, strlen(val) + 1);
      memcpy((char *)tmp_pt->val, (char *)val, strlen(val));
      return;
    }
  }

  if ( *val == 0 ) return;
  p = (struct par_t *)malloc(sizeof(struct par_t));
  if (p == NULL) global_error("Out of memory", 0, 0);
  memset((char *)p->id, 0, 3);
  memcpy((char *)p->id, (char *)id, 2);
  p->next = NULL;

  p->val = (char *)malloc(strlen(val) + 1);
  if (p->val == NULL) global_error("Out of memory", 0, 0);

  memset((char *)p->val, 0, strlen(val) + 1);
  memcpy((char *)p->val, (char *)val, strlen(val));

  pt->next = p;
  pt = p;

/*
  struct par_t *p = NULL;

  if ( *val == 0 ) return;
  p = (struct par_t *)malloc(sizeof(struct par_t));
  if (p == NULL) global_error("Out of memory", 0, 0);

  memset((char *)p->id, 0, 3);
  memcpy((char *)p->id, (char *)id, 2);

  p->next = NULL;

  p->val = (char *)malloc(strlen(val) + 1);
  if (p->val == NULL) global_error("Out of memory", 0, 0);
  
  memset((char *)p->val, 0, strlen(val) + 1);
  memcpy((char *)p->val, (char *)val, strlen(val));

  pt->next = p;
  pt = p;  
*/
}

char *f_global_par(char *id)
{
  struct par_t *p = NULL;
  
  for (p = parlist; p->next; p = p->next) {
    if (!(strcmp(p->next->id, id))) {
      return p->next->val;      
    }
  }
  return NULL;
}

/*
   Flush all warnings
*/
void global_f_warning(void)
{
  char *h = NULL, *t = NULL;

  if (!(*vqa_warning)) return;

  for (h = t = vqa_warning;;) {
    if ((*h == '$') || (!(*h))) {
      if (*h) *h++ = '\0';
      else h = NULL;

      if (h != NULL) printf("<p>%s</p>", t);

      if (h == NULL) break;

      t = h;
    } else {
      h++;
    }
  }
}

void global_exit(int exit_code)
{
  vclose();
  exit(exit_code);
}

/* quota_to_bytes: used to convert user entered quota (given in MB)
                   back to bytes for vpasswd file
   return value: 0 for success, 1 for failure
*/
int quota_to_bytes(char returnval[], char *quota) {
    storage_t tmp;

    if (quota == NULL) { return 1; }
    if ((tmp = strtoll(quota, NULL, 10))) {
        tmp *= 1048576;
        sprintf(returnval, "%.0lf", (double)tmp);
        return 0;
    } else {
	strcpy (returnval, "");
	return 1;
    }
}

/* quota_to_megabytes: used to convert vpasswd representation of quota
                       to number of megabytes.
   return value: 0 for success, 1 for failure
*/
int quota_to_megabytes(char *returnval, char *quota) {
    storage_t tmp;
    int i;

    if (quota == NULL) { return 1; }
    i = strlen(quota);
    if ((quota[i-1] == 'M') || (quota[i-1] == 'm')) {
        tmp = strtoll(quota, NULL, 10);  /* already in megabytes */
    } else if ((quota[i-1] == 'K') || (quota[i-1] == 'k')) {
	tmp = strtoll(quota, NULL, 10) * 1024;  /* convert kilobytes to megabytes */
    } else if ((tmp = strtoll(quota, NULL, 10))) {
        tmp /= 1048576.0;
    } else {
	strcpy (returnval, "");
	return 1;
    }
    sprintf(returnval, "%.2lf", (double)tmp);
    return 0;
}
