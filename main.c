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
#include <signal.h>
#include "klib.h"
#include "kcrypt_ui.h"
#include "kcrypt_engine.h"
#include "mygetpass.h"

/*========================================================================
  show_short_usage 
=========================================================================*/
void show_short_usage (FILE *f, const char *argv0)
  {
  fprintf (f, "Usage: %s [options...] [paths...]\n", argv0);
  fprintf (f, "'%s --longhelp' for more details.\n", argv0); 
  }

/*========================================================================
  show_long_usage 
=========================================================================*/
void show_long_usage (FILE *f, const char *argv0)
  {
  fprintf (f, "Usage: %s [options...] [paths...]\n", argv0);
  fprintf (f, "  -c,--cat                  decrypt files to standard out\n");
  fprintf (f, "  -d,--decrypt              decrypt files and/or unmangle names\n");
  fprintf (f, "  -e,--encrypt              encrypt files without mangling names\n");
  fprintf (f, "  -f,--max-files {nubmer}   process this number of files at most\n");
  fprintf (f, "  -h,--help                 brief usage\n");
  fprintf (f, "  -l,--list                 list files\n");
  fprintf (f, "  --longhelp                detailed usage\n");
  fprintf (f, "  -m,--encryptmangle        encrypt files and mangle names\n");
  fprintf (f, "  -n,--no-colour            do not use colour in output\n");
  fprintf (f, "  -p,--password {text}      specify password -- no prompt\n");
  fprintf (f, "  -r,--recurse              recurse into subdirectories\n");
  fprintf (f, "  -v,--verbose              show more information\n");
  fprintf (f, "  --version                 show version\n");
  fprintf (f, "  -w,--wipe                 wipe files\n");
  fprintf (f, "  -x,--xor                  use fast, week XOR algorithm\n");
  fprintf (f, "  -y,--yes                  answer 'yes' to all prompts\n");
  }

/*========================================================================
  build_file_list 
=========================================================================*/
BOOL build_file_list (const char *pathname, klib_List *list, int max_files,
    BOOL recurse)
  {
  klib_Path *path = klib_path_new (pathname);

  if (klib_path_is_file (path))
    {
    klib_log_debug ("'%s' is a file", pathname);
    klib_list_append (list, (klib_Object *)path); 
    }
  else if (klib_path_is_dir (path))
    {
    klib_log_debug ("'%s' is a directory", pathname);
    if (recurse)
      {
      DIR *d = opendir (pathname);
      if (d)
        {
        struct dirent *de = readdir (d);
        BOOL stop = FALSE;
        while (de && !stop)
          {
          if (de->d_name[0] != '.')
            {
            klib_Path *p = klib_path_new (pathname);
            klib_path_append (p, de->d_name);
            if (!build_file_list (klib_path_cstr (p), list, max_files, recurse))
              stop = TRUE;
            klib_path_free (p);
            }
          de = readdir (d);
          }
        }
      else
        klib_log_warning ("Can't open directory '%s'", pathname);
      }
    }
  else
    {
    klib_log_warning ("'%s' is neither a file nor a directory", pathname);
    }
  klib_path_free (path);

  if (klib_list_length (list) >= max_files && recurse) return FALSE;

  return TRUE;
  }


/*========================================================================
  get_random_path_alongside
  Gets a random filename in the same directory as the specified path
=========================================================================*/
char *get_random_path_alongside (const char *_path)
  {
  char *ret = NULL;
  klib_Path *path = klib_path_new (_path);
  klib_Path *dir = klib_path_get_dir (path);
  do
    {
    klib_Path *test = klib_path_new (klib_path_cstr (dir));
    char s[20];
    sprintf (s, "%08X", (int)rand());
    klib_path_append (test, s);
    if (access (klib_path_cstr (test), R_OK) != 0)
      {
      ret = strdup (klib_path_cstr (test));
      }
    klib_path_free (test);
    } while (!ret);
  klib_path_free (path);
  klib_path_free (dir);
  return ret; 
  }


/*========================================================================
  signal_handler 
=========================================================================*/
void interrupt_handler (int dummy)
  {
  prompt_quit_interrupt ();
  }


/*========================================================================
  enable_interrupt
=========================================================================*/
void enable_interrupt (void)
  {
  signal (SIGTERM, SIG_DFL);
  signal (SIGINT, SIG_DFL);
#ifndef WIN32
  signal (SIGHUP, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
#endif
  }


/*========================================================================
  disable_interrupt
=========================================================================*/
void disable_interrupt (void)
  {
  signal (SIGTERM, interrupt_handler);
  signal (SIGINT, interrupt_handler);
#ifndef WIN32
  signal (SIGHUP, interrupt_handler);
  signal (SIGQUIT, interrupt_handler);
#endif
  }


/*========================================================================
  do_operation
  Carry out the specified operation on the specified file,
  reporting progress and errors
=========================================================================*/
OpResult do_operation (Operation operation, const char *path, 
    int n, int num_files, const char *password, KCryptAlgorithm algo)
  {
  OpResult ret = RES_SKIP;
  BOOL mangle = FALSE;

  char *shortname = _klib_path_get_shortname (path);
  
  show_starting_file (shortname, operation, n, num_files);

  klib_log_debug ("%s file %s, %d of %d... ", kcrypt_ui_get_opname (operation),
         shortname, n + 1, num_files);

  switch (operation)
    {
    case OP_LIST:
      {    
      KCryptFileInfo info;
      klib_Error *error = NULL;
      kcrypt_engine_get_file_info (path, &info, password, &error);
      if (error)
        {
        klib_log_error (ERROR_TEXT " %s: %s", shortname, 
          klib_error_cstr(error));
        klib_error_free (error);
        ret = RES_ERROR;
        }
      else
        {
        if (info.enc_status == ENC_UNENCRYPTED)
          {
          klib_log_info (SKIPPED_TEXT "%s: Not encrypted with " 
            APPNAME, shortname);
          ret = RES_SKIP;
          }
        else if (info.enc_status == ENC_ENCRYPTEDBADPASSWD) 
          {
          // This is not an error
          klib_log_info (SKIPPED_TEXT 
            "%s: Encrypted, but password does not decrypt the file", 
            shortname); 
          ret = RES_SKIP;
          }
        else
          {
          klib_log_info 
             (OK_TEXT 
               "%s: '%s', encrypted with version %d.%d, "
               "algorithm %s, size %lld",
            shortname, info.orig_name, info.major_version, 
            info.minor_version, kcrypt_engine_get_algo_name (info.algorithm), 
            info.file_size);
          ret = RES_SUCCESS;
          }
        }
      }
      break;

    case OP_DECRYPTUNMANGLE:
      {
      KCryptFileInfo info;
      klib_Error *error = NULL;
      kcrypt_engine_get_file_info (path, &info, password, &error);
      if (error)
        {
        klib_log_error (ERROR_TEXT "%s: %s", shortname, klib_error_cstr(error));
        klib_error_free (error);
        ret = RES_ERROR;
        }
      else
        {
        if (info.enc_status == ENC_ENCRYPTEDBADPASSWD)
          {
          klib_log_info (SKIPPED_TEXT 
            "%s: password does not decrypt the file", shortname);
          ret = RES_SKIP;
          }
        else if (info.enc_status == ENC_UNENCRYPTED)
          {
          klib_log_info (SKIPPED_TEXT "%s: file is not encrypted", 
            shortname);
          ret = RES_SKIP;
          }
        else 
          {
          // Encrypted
          klib_Error *error = NULL;
          char *newpath = get_random_path_alongside (path);
          disable_interrupt();
          kcrypt_engine_decrypt_to_new_file (path, newpath, password, 
            progress, NULL, &error);
          if (error)
            {
            // If decryption fails, should we leave what might amount
            //  to a partial plaintext file? Probably not.
            klib_log_error (klib_error_cstr (error));
            klib_error_free (error);
            klib_log_debug (ERROR_TEXT "Decryption failed: deleting %s", 
              newpath);
            kcrypt_engine_wipe (newpath, progress, NULL, &error); 
            // Hope for the best :)
            ret = RES_ERROR;
            }
          else
            {
            // Plaintext file is is newpath. Whether name was mangled
            //  or not, we need to delete the ciphertext and then
            //  rename to the plaintext to the old ciphertext
            klib_log_debug ("Deleting encrypted file %s", path);
            unlink (path); 
            klib_Path *_path = klib_path_new (path);
            klib_Path *origpath = klib_path_get_dir (_path);
            klib_path_append (origpath, info.orig_name); 
            klib_log_debug ("Renaming %s to %s", newpath, 
             klib_path_cstr (origpath));
            rename (newpath, klib_path_cstr (origpath));
            klib_path_free (_path);
            klib_path_free (origpath);
            ret = RES_SUCCESS;
            if (strcmp (shortname, info.orig_name) == 0)
              klib_log_info (OK_TEXT "'%s' successfully decrypted in place", 
                  shortname);
            else
              klib_log_info (OK_TEXT "'%s' successfully decrypted to '%s'", 
                  shortname, info.orig_name);
            }
          if (newpath) free (newpath);        
          enable_interrupt();
          }
        }
      }
      break;

    case OP_CAT:
      {
      KCryptFileInfo info;
      klib_Error *error = NULL;
      kcrypt_engine_get_file_info (path, &info, password, &error);
      if (error)
        {
        klib_log_error (ERROR_TEXT "%s: %s", shortname, klib_error_cstr(error));
        klib_error_free (error);
        ret = RES_ERROR;
        }
      else
        {
        if (info.enc_status == ENC_ENCRYPTEDBADPASSWD)
          {
          klib_log_info (SKIPPED_TEXT 
            "%s: password does not decrypt the file", shortname);
          ret = RES_SKIP;
          }
        else if (info.enc_status == ENC_UNENCRYPTED)
          {
          klib_log_info (SKIPPED_TEXT "%s: file is not encrypted", 
            shortname);
          ret = RES_SKIP;
          }
        else 
          {
          // Encrypted
          klib_Error *error = NULL;
          klib_log_debug ("Password OK, decrypting %s\n", path);
          kcrypt_engine_decrypt_to_stream (path, stdout, password, 
            NULL, NULL, &error); // progress callback must be null, as
                                 // progress listing would corrupt stdout
          if (error)
            {
            klib_log_error (ERROR_TEXT "%s: %s", shortname, 
              klib_error_cstr(error));
            klib_error_free (error);
            ret = RES_ERROR;
            }
          else
            {
            ret = RES_SUCCESS;
            }
          }
        }
      }
      break;

    case OP_ENCRYPTMANGLE:
      {
      mangle = TRUE;
      }
      // FALL THROUGH
    case OP_ENCRYPTNOMANGLE:
      {
      KCryptFileInfo info;
      klib_Error *error = NULL;
      kcrypt_engine_get_file_info (path, &info, password, &error);
      if (error)
        {
        klib_log_error (ERROR_TEXT "%s: %s", shortname, klib_error_cstr(error));
        klib_error_free (error);
        ret = RES_ERROR;
        }
      else
        {
        if (info.enc_status == ENC_UNENCRYPTED)
          {
          klib_Error *error = NULL;
          disable_interrupt();
          char *newpath = get_random_path_alongside (path);
          klib_log_debug ("Encrypting %s to %s...", path, newpath);
          kcrypt_engine_encrypt_to_new_file (path, newpath, algo, password, 
            progress, NULL, &error);
          if (error)
            {
            klib_log_error (klib_error_cstr (error));
            klib_error_free (error);
            klib_log_debug (ERROR_TEXT 
              "Encryption failed: deleting %s", newpath);
            kcrypt_engine_wipe (newpath, progress, NULL, 
               &error); // Hope for the best :)
            ret = RES_ERROR;
            }
          else
            {
            if (mangle)
              {
              // We're mangling names. Just delete the old plaintext file
              //  The file indicated by newpath already has a mangled
              //  name
              klib_log_debug ("Deleting old plaintext file %s", path);
              klib_Error *error = NULL;
              kcrypt_engine_wipe (path, progress, NULL, &error);
              if (error)
                {
                klib_log_error (ERROR_TEXT "%s", klib_error_cstr (error));
                klib_error_free (error);
                }
              else
                {
                klib_Path *np = klib_path_new (newpath);
                klib_log_info (OK_TEXT "'%s' successfully encypted to '%s'", 
                  shortname, klib_path_get_shortname (np));
                klib_path_free (np);
                ret = RES_SUCCESS;
                }
              }
            else
              {
              // Not mangling -- rename newpath to path, and remove
              //  path
              klib_log_debug ("Wiping plaintext file %s", path);
              kcrypt_engine_wipe (path, progress, NULL, &error);
              if (error)
                {
                klib_log_error (klib_error_cstr (error));
                klib_error_free (error);
                }
     
              klib_log_debug ("Renaming %s to %s", path, newpath);
              rename (newpath, path);
              klib_log_info (OK_TEXT "'%s' successfully encypted in place");
              }
            ret = RES_SUCCESS;
            }
          if (newpath) free (newpath);        
          enable_interrupt();
          }
        else
          {
          klib_log_info (SKIPPED_TEXT 
            "%s: file is already encrypted",
            shortname);
          ret = RES_SKIP;
          }
        }
      }
      break;

    case OP_WIPE:
      {
      klib_Error *error = NULL;
      disable_interrupt();
      kcrypt_engine_wipe (path, progress, NULL, &error);
      if (error)
        {
        klib_log_error ("%s: %s", shortname, klib_error_cstr (error));
        klib_error_free (error);
        ret = RES_ERROR;
        }
      else
        {
        klib_log_info (OK_TEXT 
          "%s: file wiped", shortname, shortname);
        ret = RES_SUCCESS;
        }
      enable_interrupt();
      }
      break;

    case OP_NONE: break; // Should not happen
    }

  show_finished_file (shortname, ret);

  return ret;
  }


/*========================================================================
  main
=========================================================================*/
int main (int argc, char **argv)
  {
  srand (time (NULL));
  klib_log_set_level (KLIB_LOG_INFO);

  klib_GetOpt *getopt = klib_getopt_new ();
  klib_getopt_add_spec (getopt, "help", "help", 'h', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "cat", "cat", 'c', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "xor", "xor", 'x', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "version", "version", 0, KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "longhelp", "longhelp", 0, KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "gui", "gui", 0, KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "yes", "yes", 'y', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "verbose", "verbose", 'v', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "decrypt", "decrypt", 'd', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "encrypt", "encrypt", 'e', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "list", "list", 'l', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "wipe", "wipe", 'w', KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "no-colour", "no-colour", 'n', 
    KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "encryptmangle", "encryptmangle", 'm', 
    KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "recurse", "recurse", 'r', 
    KLIB_GETOPT_NOARG);
  klib_getopt_add_spec (getopt, "password", "password", 'p', 
    KLIB_GETOPT_COMPARG);
  klib_getopt_add_spec (getopt, "max-files", "max-files", 'f', 
    KLIB_GETOPT_COMPARG);

  int ret = 0;
  int max_files = 20;

  klib_Error *error = NULL;

  klib_getopt_parse (getopt, argc, (const char **)argv, &error);

  if (!error)
    {
    if (klib_getopt_arg_set (getopt, "verbose"))
      klib_log_set_level (KLIB_LOG_DEBUG);

    int n;
    if (klib_getopt_arg_set (getopt, "max-files"))
      {
      if (sscanf (klib_getopt_get_arg (getopt, "max-files"), "%d", &n) == 1)
        max_files = n;
      }
  
    if (FALSE)
      {
      }
    else
      {
      init_ui();
      BOOL yes = FALSE;
      if (klib_getopt_arg_set (getopt, "yes"))
        yes = TRUE; 

      if (klib_getopt_arg_set (getopt, "no-colour"))
        use_colour = FALSE;

      BOOL recurse = FALSE;
      if (klib_getopt_arg_set (getopt, "recurse"))
        recurse = TRUE; 

      if (klib_getopt_arg_set (getopt, "help"))
        show_short_usage (stdout, argv[0]);
      else if (klib_getopt_arg_set (getopt, "longhelp"))
        show_long_usage (stdout, argv[0]);
      else if (klib_getopt_arg_set (getopt, "version"))
        show_version (stdout, argv[0]);
      else
        {
        int i, argc = klib_getopt_argc (getopt); 
        if (argc == 0)
          {
          show_short_usage (stdout, argv[0]);
          }
        else
          {
          klib_List *list = klib_list_new ();
          BOOL stop = FALSE;
          for (i = 0; i < argc && !stop; i++)
            {
            const char *argv = klib_getopt_argv (getopt, i);
            if (build_file_list (argv, list, max_files, recurse) == FALSE)
              stop = TRUE;
            }

          if (stop)
           klib_log_warning (WARNING_TEXT 
            "File scanning stopped after %d files",
             klib_list_length (list));          

          int l = klib_list_length (list);
          if (l > 0)
            {
            Operation operation = OP_NONE;
            BOOL prompted = FALSE;

            if (klib_getopt_arg_set (getopt, "encrypt"))
              operation = OP_ENCRYPTNOMANGLE;
            if (klib_getopt_arg_set (getopt, "decrypt"))
              operation = OP_DECRYPTUNMANGLE;
            if (klib_getopt_arg_set (getopt, "wipe"))
              operation = OP_WIPE;
            if (klib_getopt_arg_set (getopt, "list"))
              operation = OP_LIST;
            if (klib_getopt_arg_set (getopt, "encryptmangle"))
              operation = OP_ENCRYPTMANGLE;
            if (klib_getopt_arg_set (getopt, "cat"))
              operation = OP_CAT;

            if (operation == 0)
               {
               // prompt for operation
               operation = prompt_operation (l);
               prompted = TRUE;
               } 

            if (operation != OP_NONE)
              {
              if (FALSE)
                {
                }
              else
                {
                BOOL proceed = FALSE; // FRIG
                // If we've already been prompted, don't prompt for further
                //  confirmation. User has already been given a chance to
                //  to change his mind
                if (prompted) proceed = TRUE;
                // Don't prompt for just one file -- just do it
                if (l == 1) proceed = TRUE;
                if (yes) proceed = TRUE;
                if (operation == OP_LIST) proceed = TRUE;

                if (!proceed)
                   {
                   // We stil need to check whether we need GUI here. It's
                   //  just possible that the user is using --gui with
                   //  specific -e, etc
                   if (prompt_process_files (operation, l))
                     proceed = TRUE;
                   }

                if (proceed)
                  {
                  //printf ("operation=%d\n", operation);
                  char *password = NULL;
                  const char *cmd_password = klib_getopt_get_arg 
                    (getopt, "password");
                  if (cmd_password)
                    password = strdup (cmd_password);

                  // We need a password for all operations except WIPE
                  if (operation != OP_WIPE)
                    {
                    // .. but only encryption requires the safeguard of
                    //     double passwords

                    BOOL confirm_pw = FALSE;
                    if (operation == OP_ENCRYPTNOMANGLE ||
                        operation == OP_ENCRYPTMANGLE)
                      confirm_pw = TRUE;
      
                    if (!password)
                       password = prompt_password (operation, confirm_pw); 
                     }

                  //printf ("password = %s\n", password);
                  BOOL cancel = FALSE;
                  if (password)
                    if (strcmp (password, "###cancel") == 0)
                       cancel = TRUE;

                  if (!cancel)
                    {
                    int skips = 0, errors = 0, successes = 0;
                    for (i = 0; i < l; i++)
                      {
                      const klib_Path *path = (klib_Path *) 
                        klib_list_get (list, i); 
  
                      KCryptAlgorithm algo = ALG_IDEA;
                      if (klib_getopt_arg_set (getopt, "xor"))
                        algo = ALG_FAST; 

                      OpResult res = do_operation (operation, 
                         klib_path_cstr (path), i, l, password, algo);
                      switch (res)
                        {
                        case RES_SUCCESS: successes++; break;
                        case RES_SKIP: skips++; break;
                        case RES_ERROR: errors++; break;
                        }
                      }
                    report_final_results (operation, 
                      skips, errors, successes);
                    wait_for_ui_close ();
                    }

                  if (password)
                    {
                    // Don't leave password lurking in memory
                    memset (password, 0, strlen(password));
                    free (password);
                    }
                  }
                }
              }
            else
              {
              // User cancelled
              }
            }
          else
            {
            popup_error ("No files selected to process."); 
            ret = 0; // Not really an error
            }
          klib_list_free (list);
          }
        }
      }
    }
  else
    {
    popup_error (klib_error_cstr(error)); 
    klib_error_free (error);
    ret = -1;
    }
  return ret;
  }

