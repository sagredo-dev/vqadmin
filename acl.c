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

struct acl_t_l {
  char id;
  unsigned int bit;
};

extern char vqa_group[],
            vqa_user[];

struct acl_t_l acl_types[] = {
  { 'C', ACL_USER_CREATE },
  { 'D', ACL_USER_DELETE },
  { 'V', ACL_USER_VIEW },
  { 'M', ACL_USER_MOD },
  { 'A', ACL_DOMAIN_CREATE },
  { 'X', ACL_DOMAIN_DELETE },
  { 'I', ACL_DOMAIN_VIEW },
  { 'U', ACL_DOMAIN_MOD },
  { '\0', ACL_NONE },
};

unsigned int acl_features = ACL_NONE, acl_d_features = ACL_NONE;

void acl_init(void)
{
  acl_read();

  if (!(vqa_group[0])) {
    acl_features = acl_d_features;
    memcpy((char *)vqa_group, (char *)"default", 7);
  }
}

void acl_read(void)
{
 FILE *stream = NULL;
 char b[80], *p = NULL;  

  stream = fopen(ACL_FILENAME, "r");
  if (stream == NULL) global_error("Unable to read access lists", 1, 0);

  while(1) {
    memset((char *)b, 0, 80);
    fgets(b, 80, stream);

    if (feof(stream)) break;
    
    if ((*b) && (*b != '#') && (*b != '\n') && (*b != '\r')) {
      for (p = b; *p; p++) {
        if ((*p == '\n') || (*p == '\r')) {
          *p = '\0';
          break;
        }
      }

      acl_parse(b);

      if (vqa_group[0]) break;
    }
  }

  fclose(stream);
}

void acl_parse(char *b)
{ 
 char *h = NULL, *t = NULL, i = 0, *group = NULL;
 unsigned int f=0;

  for (h = t = b; *h; h++) {
    if (*h == ' ') {
      i++;

      if (i > 2) break;
    }
  }
 
  if (i != 2) global_error("Syntax erorr in access lists", 1, 0);

  for (h = b; *h != ' '; h++); *h++ = '\0'; group = t;
  for (t = h; *h != ' '; h++); *h++ = '\0';
  
  if (*t == '*') f = ACL_ALL;
  else f = acl_parse_features(t);
  
  if (!(strcasecmp(group, "default"))) acl_d_features = f;
    
  if (acl_parse_multi(h)) {
    memcpy((char *)vqa_group, (char *)group, MAX_GLOBAL_LENGTH);
    acl_features = f;
  }
}

char acl_parse_features(char *b)
{
 int i = 0; 
 int bits = 0;
 char *p = NULL;

  bits = ACL_NONE;    
  
  for (p = b; *p; p++) {
    for (i = 0; acl_types[i].id!='\0'; i++) {
      if (*p == acl_types[i].id) {
        if (!(bits & acl_types[i].bit)) bits |= acl_types[i].bit;
      }
    }
  }

  return bits;
}

int acl_parse_multi(char *b)
{
  char *h = NULL, *t = NULL;

  for (h = t = b;;) {
    if ((*h == ',') || (*h == '\0') || (*h == ' ') ) {

      /* spaces or comma are separators */
      if (*h == ',' || *h == ' ') *h = '\0';
      else h = NULL;

      if (!(strcmp(t, vqa_user))) return 1;

      if (h == NULL) break;

      h++;
      t = h;
    } else {
      h++;
    }
  }

  return 0;
}
