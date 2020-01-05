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
#include <stdlib.h>
#include "klib.h"
#include "kcrypt_ui.h"
#include "kcrypt_engine.h"
#include "mygetpass.h"

BOOL use_colour = TRUE;

/*========================================================================
  show_version
=========================================================================*/
void show_version (FILE *f, const char *argv0)
  {
  fprintf (f, "This is " APPNAME " version %s\n", VERSION); 
  fprintf (f, "Copyright (c)1998-2013 Kevin Boone\n");
  }


/*========================================================================
  getchar_or_default 
=========================================================================*/
static int getchar_or_default (int def)
  {
  char s[4];
  fgets (s, sizeof(s), stdin);
  if (strlen (s) == 0) return def;
  return s[0];
  }


/*========================================================================
  prompt_operation 
=========================================================================*/
Operation prompt_operation (int n)
  {
  int operation = 0;
  printf ("Which operation to apply to %s file%s --\n",
   (n == 1 ? "this" : "these"), (n == 1 ? "" : "s"));
  printf ("[E]ncrypt and retain names\n");
  printf ("Encrypt and [m]angle names\n");
  printf ("[D]ecrypt and/or unmangle names\n");
  printf ("[W]ipe files\n");
  printf ("[L]ist files and status\n");

  int c = getchar_or_default(0);
  switch (c)
      {
      case 'e': case 'E':
        operation = OP_ENCRYPTNOMANGLE; 
        break;
      case 'm': case 'M':
        operation = OP_ENCRYPTMANGLE;
        break;
      case 'd': case 'D':
        operation = OP_DECRYPTUNMANGLE; 
        break;
      case 'w': case 'W':
        operation = OP_WIPE; 
        break;
      case 'l': case 'L':
        operation = OP_LIST; 
        break;
      }
  return operation;
  }


/*========================================================================
  prompt_password 
=========================================================================*/
char *prompt_password (Operation operation, BOOL confirm_pw)
  {
  //return strdup ("###cancel");
  char *result = NULL;
  BOOL done = FALSE;
  while (!done)
      {
      char *p1 = mygetpass ("Password: ");
      if (confirm_pw)
         {
         //printf ("\n");
         char *p2 = mygetpass ("Confirm Password: ");
         if (strcmp (p1, p2) == 0)
           {
           done = TRUE;
           result = p1;
           free (p2);
           //printf ("\n");
           } 
         else
           {
           free (p1);
           free (p2);
           fprintf (stderr, "Passwords do not match.\n");
           }
         }
      else
         {
         //printf ("\n");
         done = TRUE;
         result = p1;
         } 
      }
  return result; 
  }


/*========================================================================
  prompt_process_files
=========================================================================*/
BOOL prompt_process_files (Operation operation, int num_files)
  {
  int l = num_files;
  printf (APPNAME ": %s %s %d file%s ? [y/N] ",
    kcrypt_ui_get_opname (operation),
    (l == 1 ? "this" : "these"), l, (l == 1 ? "" : "s"));
  int c = getchar_or_default('n');
  if (c == 'y' || c == 'Y') return TRUE;
  return FALSE;
  }

/*========================================================================
  progress
=========================================================================*/
void progress (int percent, void *user_data)
  {
#ifdef WIN32
#else
  if (isatty (1))
    {
    printf ("[%03d%%]", percent);
    printf ("\x1B[6D");
    fflush (stdout);
    }
#endif
  }


/*========================================================================
  show_starting_file
=========================================================================*/
void show_starting_file (const char *shortname, Operation operation, 
    int n, int num_files)
  {
  }


/*========================================================================
  show_finished_file
=========================================================================*/
void show_finished_file (const char *shortname, OpResult ret)
  {
  } 

  
/*========================================================================
  report_final_results 
=========================================================================*/
void report_final_results (Operation operation, 
    int skips, int errors, int successes)
  {
  // It looks ugly to give the totals when doing a list operation
  if (operation != OP_LIST && operation != OP_CAT)
    {
    klib_log_info ("Files successfully processed: %d", successes);
    klib_log_info ("Files skipped: %d", skips);
    klib_log_info ("Errors: %d", errors);
    }
  }


/*========================================================================
  wait_for_ui_close 
=========================================================================*/
void wait_for_ui_close (void)
  {
  }


/*=======================================================================
  popup_error
=========================================================================*/
void popup_error (const char *format,...)
  {
  va_list ap;
  va_start (ap, format);
  char *s = klib_string_format_args (format, ap); 
  printf ("kcrypt: %s\n", s); 
  free (s);
  va_end (ap);
  }


/*=======================================================================
  prompt_quit_interrupt 
=========================================================================*/
void prompt_quit_interrupt (void)
  {
  int c;
  do
    {
    printf 
      ("Proper clean-up cannot be ensured if program exits at this point.\n"
         "Exit anyway (y/N): ");
    c = getchar_or_default('n');
    if (c == 'y' || c == 'Y') exit (-1); 
    } while (c != 'n' && c != 'N');
  }


/*======================================================================
   colour
   Sets the current foreground colour by sending ANSI colour codes
   directly
======================================================================*/
static void colour(int c)
  {
#ifdef WIN32
#else
  if (use_colour && isatty (2) && isatty (1))
    {
    if (c == -1)
      printf ("\x1b\x5b\x6d");
    else
      printf ("\x1b\x5b\x33%c\x6d", (char)(c + '0'));
    }
#endif
  }


/*=======================================================================
  cmdui_logger
=========================================================================*/
static void cmdui_logger (int level, const char *message)
  {
  const char *actual_message = message;

  if (strstr (message, OK_TEXT))
    {
    colour (2); // GREEN 
    printf (OK_TEXT);
    actual_message = message + strlen (OK_TEXT);
    }
  else if (strstr (message, SKIPPED_TEXT))
    {
    colour (3);  // ORANGE-YELLOW
    printf (SKIPPED_TEXT);
    actual_message = message + strlen (SKIPPED_TEXT);
    }
  else if (strstr (message, ERROR_TEXT))
    {
    colour (1); // REF 
    printf (ERROR_TEXT);
    actual_message = message + strlen (ERROR_TEXT);
    }
  colour (-1);

  printf ("%s\n", actual_message);
  }


/*=======================================================================
  init_ui
=========================================================================*/
void init_ui (void)
  {
  klib_log_set_handler (cmdui_logger);
  }


