/**=======================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#pragma once

void kcrypt_fast_encrypt_block (const char *in,  
    char *out, int size, const char *key, char *alg_data);
void kcrypt_fast_decrypt_block (const char *in,  
    char *out, int size, const char *key, char *alg_data);
int kcrypt_fast_get_key_size (void);
int kcrypt_fast_get_block_size (void);
char *kcrypt_fast_init_encrypt (const char *key);
char *kcrypt_fast_init_decrypt (const char *key);

