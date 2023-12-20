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
#include "global.h"

int main(int argc, char *argv[])
{
  char *pi;

  printf("Content-type: text/html\n\n");

  global_init();
  set_language();
  cgi_init();

  pi = getenv("PATH_INFO");

  if ( pi && strncmp(pi,"/show/", 6) == 0 ) send_html(pi);
  else cgi_nav();

  return 0;
}

/* 
 * send an html file and run it
 * through the variable substitution code
 */
void send_html(char *command)
{
  char tmpbuf[255];

  /* only open files in the local directory */
  if ( strstr(&command[6], "..")!=NULL || strstr(&command[6], "/")!=NULL ) {
    global_error("invalid file",1,0); 
  }

  /* header */
  memset(tmpbuf, 0, 255);
  snprintf(tmpbuf, 254, T_HEADER);
  t_open(tmpbuf, 0);

  /* requested file */
  memset(tmpbuf, 0, 255);
  snprintf(tmpbuf, 254, "html/%s", &command[6]);
  t_open(tmpbuf, 0);

  /* colonna dx */
  memset(tmpbuf, 0, 255);
  snprintf(tmpbuf, 254, T_COL);
  t_open(tmpbuf, 0);

  /* footer and close*/
  memset(tmpbuf, 0, 255);
  snprintf(tmpbuf, 254, T_FOOTER);
  t_open(tmpbuf, 1);
}
