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
#include <sys/types.h>
#include "global.h"
#include "vauth.h"

char *mstrdup(char *str)
{
 int len = 0;
 char *r = NULL;

  len = strlen(str);
  
  r = (char *)malloc(len + 1);
  if (r == NULL) global_error("Out of memory", 0, 0);

  memset((char *)r, 0, len + 1);
  memcpy((char *)r, (char *)str, len);

  return r;
}

/*
   An error occured in which there is absolutely no way to recover,
   or report outside of internal code.
*/
void tfatal(void)
{
  
  printf("<HTML><HEAD><TITLE>Fatal Error</TITLE></HEAD><BODY>\n" \
         "<CENTER>Fatal Error</CENTER>\n" \
         "<PRE>\n" \
         "An unrecoverable, fatal error has occured.  Please e-mail\n" \
         "the webmaster immediately and report this error, as it will\n" \
         "continue to occur.\n" \
         "</PRE></BODY></HTML>");

  vclose();
  exit(1);         
}
