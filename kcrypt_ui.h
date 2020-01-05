#pragma once

typedef enum _Operation 
  {
  OP_NONE = 0,
  OP_ENCRYPTMANGLE = 1,
  OP_ENCRYPTNOMANGLE = 2,
  OP_DECRYPTUNMANGLE = 3,
  OP_WIPE = 4,
  OP_LIST = 5, 
  OP_CAT = 6 
  } Operation;

typedef enum _OpResult
  {
  RES_SUCCESS = 0,
  RES_SKIP = 1,
  RES_ERROR = -1,
  } OpResult;

/* Gets the name of an operation from the enum */
const char *kcrypt_ui_get_opname (Operation operation);

#define OK_TEXT "[OK] "
#define SKIPPED_TEXT "[SKIPPED] "
#define ERROR_TEXT "[ERROR] "
#define WARNING_TEXT "[WARNING] "

extern BOOL use_colour;

void show_version (FILE *f, const char *argv0);
Operation prompt_operation (int n);
char *prompt_password (Operation operation, BOOL confirm_pw);
BOOL prompt_process_files (Operation operation, int num_files);
void progress (int percent, void *user_data);
void show_starting_file (const char *shortname, Operation operation, 
  int n, int num_files);
void show_finished_file (const char *shortname, OpResult ret);
void report_final_results (Operation operation, 
    int skips, int errors, int successes);
void wait_for_ui_close (void);
void popup_error (const char *format,...);
void prompt_quit_interrupt (void);
void init_ui (void);
