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
#include "klib.h"
#include "kcrypt_ui.h"

/*========================================================================
  kcrypt_ui_get_opname 
=========================================================================*/
const char *kcrypt_ui_get_opname (Operation operation)
  {
  const char *opname = "";
  switch (operation)
    {
    case OP_WIPE: opname = "wipe"; break;
    case OP_LIST: opname = "list"; break;
    case OP_ENCRYPTMANGLE: opname = "encrypt-mangle"; break;
    case OP_ENCRYPTNOMANGLE: opname = "encrypt-nomangle"; break;
    case OP_DECRYPTUNMANGLE: opname = "decrypt-unmangle"; break;
    case OP_CAT: opname = "cat"; break;
    case OP_NONE: break;
    }
  return opname;
  }



