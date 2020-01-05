/**=======================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#pragma once

#include "klib.h"

// Longest filename we can process. This is distinct from the longest
//  _path_, because we store the filename in the header when
//  mangling 
#define KCRYPT_MAX_FILENAME 256 
#define KCRYPT_MAGIC_SIZE 16

#define KCRYPT_MAGIC "\x01\x24\x99\x37\x22\x09\xF2\x8A\xB9\xC2\x45\x37\x14\xE2\xE1\x09"

typedef enum _KCryptEncStatus 
  {
  ENC_UNENCRYPTED = 0,
  ENC_ENCRYPTEDNOTMANGLED = 1,
  ENC_ENCRYPTEDANDMANGLED = 2,
  ENC_ENCRYPTEDBADPASSWD = 3 
  } KCryptEncStatus;

typedef enum _KCryptAlforithm
  {
  ALG_IDEA = 0,
  ALG_FAST = 1
  } KCryptAlgorithm;

typedef struct _KCryptFileInfo
  {
  KCryptEncStatus enc_status;
  char orig_name [KCRYPT_MAX_FILENAME];
  int8_t major_version;
  int8_t minor_version;
  int64_t file_size;
  int8_t algorithm;
  } KCryptFileInfo;

// VITAL: File header must be a multiple of the block size for all
//  algorithms we intend to use. Adding 24 bytes padding makes this
// 21 * 16 which should suit most algos, but we have to ensure that
// the compiler doesn't add any extra padding
// MingW C compiler ignores the packed attribute, but still produces
//  a structure that appears to have the right size
typedef struct _KCryptFileHeader
  {
  char magic [KCRYPT_MAGIC_SIZE] __attribute__ ((packed));
  char enc_magic [KCRYPT_MAGIC_SIZE] __attribute__ ((packed));
  char orig_name [KCRYPT_MAX_FILENAME] __attribute__ ((packed));
  int8_t major_version __attribute__ ((packed));
  int8_t minor_version __attribute__ ((packed));
  int64_t file_size __attribute__ ((packed));
  int8_t algorithm __attribute__ ((packed));
  char pad[21] __attribute__ ((packed));
  } KCryptFileHeader __attribute__ ((packed));

typedef void (*KCryptEngineProgressProc) (int percent, void *user_data);

void kcrypt_engine_get_file_info (const char *path, KCryptFileInfo *info, 
  const char *password, klib_Error **error);

void kcrypt_engine_encrypt_to_new_file (const char *source_filename, 
        const char *target_filename, KCryptAlgorithm algo, 
        const char *password, KCryptEngineProgressProc progress, 
       void *user_data, klib_Error **error);

void kcrypt_engine_decrypt_to_new_file (const char *source_filename, 
        const char *target_filename, 
        const char *password, KCryptEngineProgressProc progress, 
        void *user_data, klib_Error **error);

void kcrypt_engine_wipe (const char *filename,  
   KCryptEngineProgressProc progress, void *user_data, klib_Error **error);

const char *kcrypt_engine_get_algo_name (KCryptAlgorithm algo);

void kcrypt_engine_decrypt_to_stream (const char *path, FILE *f, 
    const char *password, KCryptEngineProgressProc progress, 
    void *user_data, klib_Error **error);

