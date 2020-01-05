/**=======================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#pragma once

void kcrypt_idea_encrypt_block (const char *in,  
    char *out, int size, const char *key, char *alg_data);
void kcrypt_idea_decrypt_block (const char *in,  
    char *out, int size, const char *key, char *alg_data);
int kcrypt_idea_get_key_size (void);
int kcrypt_idea_get_block_size (void);
char *kcrypt_idea_init_encrypt (const char *key);
char *kcrypt_idea_init_decrypt (const char *key);

