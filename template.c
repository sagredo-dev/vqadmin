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

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "config.h"
/* undefine vaiables conflicting with analog vpopmail vars imported from vpopmail config.h */
#undef VERSION
#undef QMAILDIR
#undef PACKAGE_VERSION
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_NAME
#undef PACKAGE
#include "vpopmail_config.h"

extern char vqa_error[],
            vqa_user[],
            vqa_group[],
            vqa_warning[];

void t_code(char code)
{
  switch(code) {
   case 'V':
    printf("<a href=\"https://notes.sagredo.eu/en/qmail-notes-185/vqadmin-26.html\" target=\"_blank\">%s</a> %s ~ ", VQA_PACKAGE, VQA_VERSION);
    printf("<a href=\"https://notes.sagredo.eu/en/qmail-notes-185/installing-and-configuring-vpopmail-81.html\">%s</a> %s", PACKAGE, VERSION);
    break;
   case 'E':
    t_printf(vqa_error);
    break;
   case 'W':
    global_f_warning();
    break;
   case 'U':
    t_printf(vqa_user);
    break;
   case 'G':
    t_printf(vqa_group);
    break;
   default:
    break;
  }
}

void g_code(char *id)
{
  char *r = NULL;

  r = f_global_par(id);
  t_printf(r);
}

void t_printf(char *str)
{
  if ((str) && (str[0])) printf("%s", str);
}

void t_open(char *filename, int exit_when_done)
{
  FILE *stream = NULL;
  char *p = NULL, b[MAX_TEMPLATE_LINE_LENGTH], t = 0;
  struct stat mystat;

  if ( lstat( filename, &mystat ) == -1 || S_ISLNK(mystat.st_mode) ) {
    printf("Unable to retrieve file informations: %s\n",filename);
    vclose();
    exit(-1);
  }

  stream = fopen(filename, "r");
  if (stream == NULL) {
    printf("Dag Nabit\n");
    tfatal();
  }

  while(1) {
    memset((char *)b, 0, MAX_TEMPLATE_LINE_LENGTH);
    fgets(b, MAX_TEMPLATE_LINE_LENGTH, stream);

    if (feof(stream)) break;

    for (p = b; *p; p++) {
      if ((*p == '$') && (*(p + 1) == '-')) {
        t = *(p + 4);
        *(p + 4) = '\0';
        g_code(p + 2);
        *(p + 4) = t;
        p += 3;
      } else if ((*p == '%') && (*(p + 1) == '-')) {
        t_code(*(p + 2));
        p += 2;
      } else if ((*p == '#') && (*(p + 1) == '-')) {
        put_lang_code((p+2));
	p += 4;
      } else {
        putchar(*p);
      }
    }
  }

  fclose(stream);

  if ( exit_when_done == 1 ) {
    vclose();
    exit(0);
  }
}

/* internal page template */
void t_page(char *filename, int exit_when_done) {
  char tmpbuf[256];

  snprintf(tmpbuf, sizeof(tmpbuf), "%s/vqadmin/%s", CGIBINDIR, T_HEADER);
  t_open(tmpbuf,0);
  memset(tmpbuf,0,sizeof(tmpbuf));

  snprintf(tmpbuf, sizeof(tmpbuf), "%s/vqadmin/%s", CGIBINDIR, filename);
  t_open(tmpbuf,0);
  memset(tmpbuf,0,sizeof(tmpbuf));

  if ( strcmp(filename, T_MAIN) != 0 ) {
    snprintf(tmpbuf, sizeof(tmpbuf), "%s/vqadmin/%s", CGIBINDIR, T_COL);
    t_open(tmpbuf,0);
    memset(tmpbuf,0,sizeof(tmpbuf));
  }

  snprintf(tmpbuf, sizeof(tmpbuf), "%s/vqadmin/%s", CGIBINDIR, T_FOOTER);
  if ( exit_when_done == 1 ) {
    t_open(tmpbuf,1);
    vclose();
    exit(0);
  }
  else t_open(tmpbuf,0);
}

/* home page template (no side menu) */
void t_main(int exit_when_done) {
  t_page(T_MAIN, exit_when_done);
}
