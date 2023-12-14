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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vpopmail.h>

#include "global.h"
#include "vauth.h"

#define MAX_TMPBUF 200
char tmpbuf[MAX_TMPBUF];
char Lang[MAX_TMPBUF];
FILE *lang_fs = NULL;

void set_language()
{
 char *tmpstr;
 int lang_err;

  if ( lang_fs != NULL ) {
    fclose(lang_fs);
    lang_fs = NULL;
  }

  memset(Lang, 0, MAX_TMPBUF);
  tmpstr = getenv("HTTP_ACCEPT_LANGUAGE");

  if ( tmpstr != NULL ) {
    strncpy(tmpbuf, tmpstr, MAX_TMPBUF);
    tmpstr = strtok(tmpbuf, " ,\n");

    for(lang_err = -1;tmpstr!=NULL && lang_err!=0;) {
      lang_err = open_lang(tmpstr);
      if ( lang_err == 0 ) snprintf(Lang, MAX_TMPBUF - 1, "%s", tmpstr);
      else tmpstr = strtok(NULL, " ,\n");
    }

    if ( tmpstr == NULL ) strcpy(Lang, "en");

  } else {
    strcpy(Lang, "en");
  }
  if ( lang_fs == NULL ) open_lang(Lang);

}

int open_lang( char *lang)
{
 char tmpfile[MAX_TMPBUF];
 struct stat mystat;

  /* Lowercase the language name to fix a bug where chrome users can't access
   * the page. */
  lowerit(lang);

  /* only open files in the local directory */
  if ( strstr(lang, ".") != NULL || strstr(lang, "/") != NULL ) {
    strcpy(lang, "en");
  }

  if ( lang_fs == NULL ) {
    memset(tmpfile, 0, MAX_TMPBUF);
    snprintf(tmpfile, MAX_TMPBUF - 1, "lang/%s", lang);

    /* check for symbolic link */
    if ( lstat(tmpfile, &mystat) == 0 && S_ISLNK(mystat.st_mode) ) {
      global_error("invalid file",1,0); 
    } 

    if ( (lang_fs=fopen(tmpfile, "r"))==NULL) return(-1);
  }
  return(0);
}

void put_lang_code( char *index ) 
{
 char *tmpstr;
 char lang_index[4];
 int i;

  for(i=0;i<3;++i) lang_index[i] = *(index+i);
  lang_index[3] = 0; 

  if (lang_fs == NULL) return;

  rewind(lang_fs);
  while(fgets(tmpbuf,MAX_TMPBUF,lang_fs)!=NULL){
    tmpstr = strtok(tmpbuf, " ");
    if (strcmp(tmpstr, lang_index) == 0 ) {
      tmpstr = strtok(NULL, "\n");
      fputs(tmpstr, stdout);
      return;
    }
  }
}

char *get_lang_code( char *index ) 
{
 char *tmpstr;
 char lang_index[4];
 int i;

  for(i=0;i<3;++i) lang_index[i] = *(index+i);
  lang_index[3] = 0; 

  if (lang_fs == NULL) return("");

  rewind(lang_fs);
  while(fgets(tmpbuf,MAX_TMPBUF,lang_fs)!=NULL){
    tmpstr = strtok(tmpbuf, " ");
    if (strcmp(tmpstr, lang_index) == 0 ) {
      tmpstr = strtok(NULL, "\n");
      return(tmpstr);
    }
  }
  return("");
}
