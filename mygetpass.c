/*========================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#endif
#include "klib.h"
#include "kcrypt_ui.h"
#include "mygetpass.h"

/*========================================================================
  mygetpass
=========================================================================*/
char *mygetpass (const char *prompt)
  {
  char password[64];
  memset (password, 0, sizeof (password));

#ifdef WIN32

  size_t i = 0;
  int c;

  fputs (prompt, stderr);
  fflush (stderr);

  for (;;)
    {
    c = _getch ();
    if (c == '\r')
	{
	password[i] = '\0';
	break;
	}
      else if (i < sizeof (password) - 1)
	{
	password[i++] = c;
	}

      if (i >= sizeof (password) - 1)
	{
	password[i] = '\0';
	break;
	}
    }

  fputs ("\r\n", stderr);
  fflush (stderr);

#else

  struct termios oflags, nflags;

  tcgetattr(fileno(stdin), &oflags);
  nflags = oflags;
  nflags.c_lflag &= ~ECHO;
//  nflags.c_lflag |= ECHONL;
  tcsetattr(fileno(stdin), TCSANOW, &nflags); 

  fprintf(stderr, prompt);
  fflush (stdout);
  fgets(password, sizeof(password), stdin);
   
  if (strlen (password) > 0)
    password[strlen(password) - 1] = 0; // remove \n

  fseek (stderr, 0, SEEK_CUR);

  tcsetattr(fileno(stdin), TCSANOW, &oflags); 

  printf ("\n");

#endif

  return strdup (password);
  }


