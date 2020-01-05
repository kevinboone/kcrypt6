/**=======================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#ifdef WIN32

#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "klib.h" 
#include "kcrypt_win32.h" 
#include "kcrypt_ui.h" 

static HWND main_window = NULL;
BOOL request_close = FALSE;
BOOL use_colour = FALSE; // Currently not used

/*=======================================================================
  Types and constants 
=========================================================================*/
typedef struct _PromptOperationStruct
  {
  int num_files;
  Operation operation;
  } PromptOperationStruct;

typedef struct _PromptPasswordStruct
  {
  Operation operation;
  BOOL confirm_pw;
  char *password;
  } PromptPasswordStruct;

/*=======================================================================
  kcrypt_win32_show_error
=========================================================================*/
void kcrypt_win32_show_error (const char *s)
  {
  MessageBox (main_window, s, APPNAME, MB_OK | MB_ICONERROR);
  }

/*=======================================================================
  kcrypt_win32_okcancel
=========================================================================*/
BOOL kcrypt_win32_okcancel (const char *msg, BOOL defltOK)
  {
  int ret = MessageBox (main_window, msg, APPNAME, 
     MB_OKCANCEL | MB_ICONQUESTION | (defltOK ? MB_DEFBUTTON1 : MB_DEFBUTTON2)); 
  if (ret == IDOK) return TRUE;
  return FALSE;
  }


/*=======================================================================
  prompt_operation_proc 
=========================================================================*/
int prompt_operation_proc (HWND hDlg, UINT uMsg, WPARAM wParam,
      LPARAM lParam)
  {
  // Making this a simple static will fail if there is ever a chance
  //  that two of these dialogs will be on screen at the same time. However,
  //  since this is a modal dialog, that should never happen
  static PromptOperationStruct *pos = NULL; 
  switch (uMsg)
    {
    case WM_INITDIALOG: 
      {
      pos = (PromptOperationStruct *)lParam;
      int num_files = pos->num_files;
      klib_String *msg = klib_string_new_empty();
      if (num_files == 1)
        klib_string_printf (msg, "Which operation to apply to this file?");
      else 
        klib_string_printf (msg, "Which operation to apply to these %d files?",
          num_files);
      SetDlgItemText (hDlg, ID_MESSAGE, klib_string_cstr (msg));
      klib_string_free (msg);
      return TRUE;
      }
    
    case WM_COMMAND:
      {
      switch (LOWORD (wParam))
        {
        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          pos->operation = OP_NONE;
          return TRUE;
        case ID_LIST:
          EndDialog(hDlg, IDOK);
          pos->operation = OP_LIST;
          return TRUE;
        case ID_WIPE:
          EndDialog(hDlg, IDOK);
          pos->operation = OP_WIPE;
          return TRUE;
        case ID_ENCRYPTNOMANGLE:
          EndDialog(hDlg, IDOK);
          pos->operation = OP_ENCRYPTNOMANGLE;
          return TRUE;
        case ID_ENCRYPTMANGLE:
          EndDialog(hDlg, IDOK);
          pos->operation = OP_ENCRYPTMANGLE;
          return TRUE;
        case ID_DECRYPTUNMANGLE:
          EndDialog(hDlg, IDOK);
          pos->operation = OP_DECRYPTUNMANGLE;
          return TRUE;
        }
      break;
      }
    }
  return FALSE;
  }


/*=======================================================================
  prompt_password_proc 
=========================================================================*/
int prompt_password_proc (HWND hDlg, UINT uMsg, WPARAM wParam,
      LPARAM lParam)
  {
  // Making this a simple static will fail if there is ever a chance
  //  that two of these dialogs will be on screen at the same time. However,
  //  since this is a modal dialog, that should never happen
  static PromptPasswordStruct *pos = NULL; 
  switch (uMsg)
    {
    case WM_INITDIALOG: 
      {
      pos = (PromptPasswordStruct *)lParam;
      klib_String *s = klib_string_new_empty();
      const char *opname = kcrypt_ui_get_opname (pos->operation);
      klib_string_printf (s, "Enter password for %s operation:",
        opname);
      SetDlgItemText (hDlg, ID_MESSAGE, klib_string_cstr (s));
      klib_string_free (s);
      if (!pos->confirm_pw)
        {
        EnableWindow (GetDlgItem (hDlg, ID_PASSWORD2), FALSE);
        EnableWindow (GetDlgItem (hDlg, ID_PWLABEL2), FALSE);
        }
      return TRUE;
      }
    
    case WM_COMMAND:
      {
      switch (LOWORD (wParam))
        {
        case IDCANCEL:
          EndDialog(hDlg, IDCANCEL);
          pos->password = NULL;
          return TRUE;
        case IDOK:
          {
          char p1[64];
          char p2[64];
          GetDlgItemText (hDlg, ID_PASSWORD1, p1, sizeof (p1));
          GetDlgItemText (hDlg, ID_PASSWORD2, p2, sizeof (p2));
          if (strlen (p1) == 0)
            MessageBox (hDlg, "Password cannot be empty", APPNAME, 
             MB_OK|MB_ICONHAND);
          else
            {
            if (strcmp (p1, p2) == 0 || !pos->confirm_pw)
              {
              EndDialog(hDlg, IDOK);
              pos->password = strdup (p1);
              }
            else
              {
              MessageBox (hDlg, "Passwords do not match", APPNAME, 
                 MB_OK|MB_ICONHAND);
              }
            }
          }
          return TRUE;
        }
      break;
      }
    }
  return FALSE;
  }

/*=======================================================================
  main_window_proc 
=========================================================================*/
int main_window_proc (HWND hDlg, UINT uMsg, WPARAM wParam,
      LPARAM lParam)
  {
  switch (uMsg)
    {
    case WM_INITDIALOG: 
      {
      EnableWindow (GetDlgItem (hDlg, ID_CLOSE), FALSE);
      // Ugly way to set the scroll bar range :/
      SendDlgItemMessage (hDlg, ID_LOG, LB_SETHORIZONTALEXTENT, 600, 0);
      return TRUE;
      }
    case WM_COMMAND:
      {
      switch (LOWORD (wParam))
        {
        case ID_CLOSE:
          request_close = TRUE;
          return TRUE;
        }
      } 
    }
  return FALSE;
  }


/*=======================================================================
  prompt_operation
=========================================================================*/
Operation prompt_operation (int num_files)
  {
  PromptOperationStruct pos;
  pos.num_files = num_files;
  pos.operation = OP_NONE;
  int ret = DialogBoxParam (GetModuleHandle(NULL), "prompt_operation", 
    main_window, (DLGPROC)prompt_operation_proc, (LPARAM)&pos);
  if (ret != IDOK) return OP_NONE;
  return pos.operation;
  }

/*=======================================================================
  prompt_password
=========================================================================*/
char *prompt_password (Operation operation, BOOL confirm_pw)
  {
  PromptPasswordStruct pos;
  pos.confirm_pw = confirm_pw;
  pos.password = NULL;
  pos.operation = operation;
  int ret = DialogBoxParam (GetModuleHandle(NULL), "prompt_password", 
    main_window, (DLGPROC)prompt_password_proc, (LPARAM)&pos);
  if (ret != IDOK) return strdup ("###cancel");
  return pos.password;
  }

/*=======================================================================
  yield
=========================================================================*/
static void yield (void)
  {
  MSG msg;
  while (PeekMessage (&msg, main_window, 0, 0, PM_REMOVE))
    {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
    }
  }

/*=======================================================================
  main_window_logger
=========================================================================*/
static void main_window_logger (int level, const char *message)
  {
  if (main_window)
    {
    SendDlgItemMessage (main_window, ID_LOG, LB_ADDSTRING, 0,
      (LPARAM) message);
    int l = SendDlgItemMessage (main_window, ID_LOG, LB_GETCOUNT, 0, 0);
    SendDlgItemMessage (main_window, ID_LOG, LB_SETTOPINDEX, l-1, 0);
    }
  else
    printf ("%s\n", message);
  }

/*=======================================================================
  show_main_window 
=========================================================================*/
static void show_main_window (void)
  {
  if (!main_window)
    {
    main_window = CreateDialog (GetModuleHandle (NULL),
     "main_window", NULL, (DLGPROC)main_window_proc);
    klib_log_set_handler (main_window_logger);
    }
 
  yield();
  ShowWindow (main_window, SW_SHOW);
  }

/*=======================================================================
  show_starting_file 
=========================================================================*/
void show_starting_file (const char *shortname, 
    Operation operation, int n, int num_files)
  {
  show_main_window();
  SetDlgItemText (main_window, ID_OPERATION,
    kcrypt_ui_get_opname (operation));
  SetDlgItemText (main_window, ID_FILE,
    shortname);
  char s[20];
  snprintf (s, sizeof (s), "%d of %d", n + 1, num_files);
  SetDlgItemText (main_window, ID_COUNT,
    s);
  SetDlgItemText (main_window, ID_PROGRESS, "");
  }


/*=======================================================================
  show_finished_file 
=========================================================================*/
void show_finished_file (const char *shortname, OpResult opres)
  {
  show_main_window();
  // TODO record success/failure
  SetDlgItemText (main_window, ID_OPERATION, "");
  SetDlgItemText (main_window, ID_FILE, "");
  }


/*=======================================================================
  report_final_results
=========================================================================*/
void report_final_results (Operation operation, int skips, 
     int errors, int successes)
  {
  // Don't really need to prompt -- the results have already been written
  //  to the log window
  // But we'll use this opportunity to enable to Close button
  EnableWindow (GetDlgItem (main_window, ID_CLOSE), TRUE);
  SetFocus (GetDlgItem (main_window, ID_CLOSE));

  SetDlgItemText (main_window, ID_OPERATION, "Done");  
  SetDlgItemText (main_window, ID_FILE, "N/A");  
  SetDlgItemText (main_window, ID_COUNT, "N/A");  

  if (operation != OP_LIST)
    {
    klib_log_info ("Files successfully processed: %d", successes);
    klib_log_info ("Files skipped: %d", skips);
    klib_log_info ("Errors: %d\n", errors);
    }
  }


/*=======================================================================
  wait_for_ui_close
=========================================================================*/
void wait_for_ui_close (void)
  {
  while (!request_close)
    {
    yield();
    usleep (100000);
    }
  }


/*=======================================================================
  kcrypt_win32_file_progress
=========================================================================*/
void progress (int percent, void *user_data)
  {
  if (main_window)
    {
    char s[20];
    sprintf (s, "%d%%", percent);
    SetDlgItemText (main_window, ID_PROGRESS, s);
    yield();
    }
  }


/*=======================================================================
  show_version 
=========================================================================*/
void show_version (FILE *f, const char *argv0)
  {
  MessageBox (main_window,
    APPNAME " version " VERSION ",\n"
    "Copyright (c)1998-2013 Kevin Boone.",
  APPNAME, MB_OK|MB_ICONINFORMATION);
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
  MessageBox (main_window, s, APPNAME, 
    MB_OK|MB_ICONERROR);
  free (s);
  va_end (ap);
  }

/*========================================================================
  prompt_process_files
=========================================================================*/
BOOL prompt_process_files (Operation operation, int num_files)
  {
  int l = num_files;
  char s[200];
  snprintf (s, sizeof (s), APPNAME ": %s %s %d file%s ? [y/N] ",
    kcrypt_ui_get_opname (operation),
    (l == 1 ? "this" : "these"), l, (l == 1 ? "" : "s"));
  if (MessageBox (main_window, s, APPNAME, 
       MB_OK|MB_ICONERROR) == IDOK) 
    return TRUE;
  else
    return FALSE;
  }


/*=======================================================================
  prompt_quit_interrupt 
=========================================================================*/
void prompt_quit_interrupt (void)
  {
  if (MessageBox (main_window,
    "Proper clean-up cannot be ensured if program exits at this point.\n"
       "Exit anyway?", APPNAME, MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
    exit (-1);
  }


/*=======================================================================
  init_ui 
=========================================================================*/
void init_ui (void)
  {
  }


#endif


