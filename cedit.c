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


char *ControlFiles[] = {
"badmailfrom",
"bouncefrom",
"bouncehost",
"concurrencylocal",
"concurrencyremote",
"defaultdomain",
"defaulthost",
"databytes",
"doublebouncehost",
"doublebounceto",
"envnoathost",
"helohost",
"idhost",
"localiphost",
"me",
"percenthack",
"plusdomain",
"qmapservers",
"queuelifetime",
"smtpgreeting",
"smtproutes",
"timeoutconnect",
"timeoutremote",
"timeoutsmtpd",
NULL
};

char *ControlDefaults[] = {
"none",
"MAILER-DAEMON",
"me",
"10",
"20",
"me",
"me",
"0",
"me",
"postmaster",
"me",
"me",
"me",
"me",
"me",
"none",
"me",
"none",
"60480 (7 days)",
"me",
"none",
"60",
"1200",
"1200",
NULL
};

/*
 * Opens the requested file and displays it in a textarea
 */
void display_file()
{
 struct stat finfo;
 FILE *f=NULL;
 char *fname=NULL, *fcontent=NULL, *ptr=NULL, *rows=NULL, *cols=NULL;
 char path[255];
 memset(path, 0, 255);

  if((fname=cgi_is_var("fname")) == NULL) {
    global_error(": cannot find file", 1, 0);
  }

  for(ptr=fname; *ptr; ptr++) {
    if((*ptr == '.') || (*ptr == '/')) {
      global_error(": invalid file name", 1, 0);
    }
  }

  if((rows=cgi_is_var("rows")) == NULL) rows="20";
  if((cols=cgi_is_var("cols")) == NULL) cols="80";
  global_par("NM", fname);
  global_par("RW", rows);
  global_par("CL", cols);
		
  snprintf(path, 254, "%s/control/%s", QMAILDIR, fname);
  if( lstat(path, &finfo) != 0) {
    global_par("CF", "");
    t_open(T_CTRL_FILE, 1);
  }
	
  if(S_ISLNK(finfo.st_mode) != 0) global_error(": not a file", 1, 0);
	
  if((f=fopen(path, "r")) == NULL) {
    snprintf(path, 254, "could not open %s", fname);
    global_error(path, 1, 0);
  }
				
  fcontent=(char*)malloc(finfo.st_size+1);
  memset(fcontent, 0, finfo.st_size+1);
  fread(fcontent,sizeof(char),finfo.st_size,f);
  fclose(f);

  global_par("CF", fcontent);
  free(fcontent);
  t_open(T_CTRL_FILE, 1);

}


/* 
 * Writes the changes made in the textarea to the proper control file.
 */ 
void modify_file()
{
 struct stat fstat;
 char *filenm=NULL, *tarea_data=NULL, *ptr=NULL, *rws=NULL, *cls=NULL;
 FILE *filer;
 char path[255];
 int i;

  memset(path, 0, 255);
  if(((filenm=cgi_is_var("file_name")) == NULL) || 
     ((tarea_data=cgi_is_var("tarea")) == NULL)) {
    global_error(": invalid input data", 1, 0);
  }

  for(ptr=filenm; *ptr; ptr++) {
    if((*ptr == '.') || (*ptr == '/')) {
      global_error(": invalid file name", 1, 0);
    }
  }
	
  snprintf(path, 254, "%s/control/%s", QMAILDIR, filenm);
	
  if(lstat(path, &fstat) == 0) {
    if(S_ISLNK(fstat.st_mode) != 0) {
      global_error(": not a file", 1, 0);
    }
	
    if((filer=fopen(path, "w")) == NULL) {
      snprintf(path, 254, ": could not find %s", filenm);
      global_error(path, 1, 0);
    }
  } else {
    if((filer=fopen(path, "w+")) == NULL) {
      snprintf(path, 254, ": could not find %s", filenm);
      global_error(path, 1, 0);
    }
  }

  for(i=0;tarea_data[i]!=0;++i) {
    if ((tarea_data[i] != '\013') && (tarea_data[i] != '\r')) {
      fputc(tarea_data[i], filer);
    }
  }

  /* make sure there is a new line at the end */
  if ( tarea_data[i-1]!='\n') fputc('\n', filer);
  fclose(filer);

  snprintf(path, 254, "%s updated successfully", filenm);	
  global_warning(path);

  if((rws=cgi_is_var("rws")) == NULL) rws="20";
  if((cls=cgi_is_var("cls")) == NULL) cls="80";

  global_par("CF", tarea_data);
  global_par("NM", filenm);
  global_par("RW", rws);
  global_par("CL", cls);

  show_controls();
}						

void show_controls()
{
 int i;
 char *tmpbuf;
 FILE *fs;

  tmpbuf = calloc(sizeof(char), 1000 );
  t_open("html/qmail_controls_top.html",0);

  for(i=0;ControlFiles[i]!=0;++i) {
    snprintf(tmpbuf, 1000, 
"<a href=/cgi-bin/vqadmin/vqadmin.cgi?nav=display_file&fname=%s>%s</a>\n",
      ControlFiles[i], ControlFiles[i]);
    global_par("a0", tmpbuf); 

    /* value */
    snprintf(tmpbuf,1000,"%s/control/%s", QMAILDIR, ControlFiles[i]);
    if ( (fs=fopen(tmpbuf, "r")) == NULL ) {
      global_par("a1", "default"); 
    } else {
      memset(tmpbuf,0,1000);
      fread(tmpbuf,sizeof(char), 1000, fs );
      fclose(fs);
      global_par("a1", tmpbuf); 
    }

    /* default */
    global_par("a2", ControlDefaults[i]); 
    t_open("html/qmail_controls_middle.html",0);
  }
  t_open("html/qmail_controls_bottom.html",1);
}

void delete_file()
{
 char *filenm=NULL;
 char path[255];

  if( (filenm=cgi_is_var("file_name")) == NULL) {
    global_error(": invalid input data", 1, 0);
  }

  snprintf(path,255, "%s/control/%s", QMAILDIR,filenm);
  unlink(path);
  show_controls();
}

