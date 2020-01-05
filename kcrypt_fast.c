/**=======================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib.h" 
#include "kcrypt_engine.h" 
#include "kcrypt_fast.h" 

#define FAST_MAGIC "\xB9\xC2\x45\x37\x14\xE2\xE1\x09\x01\x24\x99\x37\x22\x09\xF2\x8A"
#define KCRYPT_FAST_BLOCK_SIZE 16
#define KCRYPT_FAST_KEY_SIZE 16



/*=======================================================================
  encrypt_block 
  // IMPORTANT -- 
  // Input and output buffers must be the same size, and a multiple of
  //  the encryption block size
=========================================================================*/
void kcrypt_fast_encrypt_block (const char *in,  
    char *out, int size, const char *key, char *alg_data)
  {
  int i, j = 0;
  for (i = 0; i < size; i++)
    {
    char xor = key[j] ^ FAST_MAGIC[j]; 
    out[i] = in[i] ^ xor;
    j++;
    if (j == KCRYPT_FAST_KEY_SIZE) j = 0; 
    }
  }


/*=======================================================================
  decrypt_block 
  // IMPORTANT -- 
  // Input and output buffers must be the same size, and a multiple of
  //  the encryption block size
=========================================================================*/
void kcrypt_fast_decrypt_block (const char *in,  
    char *out, int size, const char *key, char *alg_data)
  {
  kcrypt_fast_encrypt_block (in, out, size, key, alg_data);
  }


/*=======================================================================
  kcrypt_fast_get_key_size()
=========================================================================*/
int kcrypt_fast_get_key_size (void)
  {
  return KCRYPT_FAST_KEY_SIZE;
  }


/*=======================================================================
  kcrypt_fast_get_block_size()
=========================================================================*/
int kcrypt_fast_get_block_size (void)
  {
  return KCRYPT_FAST_BLOCK_SIZE;
  }


/*=======================================================================
  kcrypt_fast_init_encrypt ()
=========================================================================*/
char *kcrypt_fast_init_encrypt (const char *key)
  {
  // This algorithm requires no context data
  return strdup ("Hello world");
  }


/*=======================================================================
  kcrypt_fast_init_decrypt ()
=========================================================================*/
char *kcrypt_fast_init_decrypt (const char *key)
  {
  // This algorithm requires no context data
  return strdup ("Hello world");
  }


