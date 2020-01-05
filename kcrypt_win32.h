/**=======================================================================


kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#pragma once

#ifdef WIN32

#include "kcrypt_ui.h"

#define ID_MESSAGE         100
#define ID_ENCRYPTMANGLE   101
#define ID_ENCRYPTNOMANGLE 102
#define ID_DECRYPTUNMANGLE 103
#define ID_LIST            104
#define ID_WIPE            105
#define ID_PASSWORD1       106
#define ID_PASSWORD2       107
#define ID_PWLABEL2        108
#define ID_OPERATION       109
#define ID_FILE            110
#define ID_COUNT           111
#define ID_PROGRESS        112
#define ID_LOG             113
#define ID_CLOSE           114

void kcrypt_win32_show_error (const char *error);
BOOL kcrypt_win32_okcancel (const char *msg, BOOL defltOK);
int kcrypt_win32_prompt_operation (int num_files);
char *kcrypt_win32_prompt_password (Operation operation, BOOL confirm_pw);
void kcrypt_win32_show_finished_file (const char *shortname, Operation opres);
void kcrypt_win32_show_starting_file (const char *shortname, 
    Operation operation, int n, int num_files);
void kcrypt_win32_report_final_results (int skips, int errors, int successes);
void kcrypt_win32_wait_for_close (void);
void kcrypt_win32_file_progress (int percent);

#endif

