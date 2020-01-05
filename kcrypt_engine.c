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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib.h" 
#include "kcrypt_fast.h" 
#include "kcrypt_idea.h" 
#include "kcrypt_engine.h" 

/*=======================================================================
  kcrypt_engine_get_algo_name
=========================================================================*/
const char *kcrypt_engine_get_algo_name (KCryptAlgorithm algo)
  {
  switch (algo)
    {
    case ALG_FAST: return "XOR"; break;
    case ALG_IDEA: return "IDEA"; break;
    default: return "unknown";
    }
  }

/*=======================================================================
  encrypt_block 
  // IMPORTANT -- 
  // Input and output buffers must be the same size, and a multiple of
  //  the encryption block size
=========================================================================*/
static void encrypt_block (KCryptAlgorithm algo, const char *in,  
    char *out, int size, const char *key, char *alg_data)
  {
  switch (algo)
    {
    case ALG_FAST: kcrypt_fast_encrypt_block (in, out, size, key, alg_data); 
      break;
    case ALG_IDEA: kcrypt_idea_encrypt_block (in, out, size, key, alg_data); 
      break;
    default:
       fprintf (stderr, "Bad algorithm code: %d\n", algo);
    }
  }

/*=======================================================================
  decrypt_block 
  // IMPORTANT -- 
  // Input and output buffers must be the same size, and a multiple of
  //  the encryption block size
=========================================================================*/
static void decrypt_block (KCryptAlgorithm algo, const char *in,  
    char *out, int size, const char *key, char *alg_data)
  {
  switch (algo)
    {
    case ALG_FAST: kcrypt_fast_decrypt_block (in, out, size, key, alg_data); 
      break;
    case ALG_IDEA: kcrypt_idea_decrypt_block (in, out, size, key, alg_data); 
      break;
    default:
       {
       fprintf (stderr, "Bad algorithm code: %d\n", algo);
       exit (-1);
       }
    }
  }

/*=======================================================================
  get_key_size
=========================================================================*/
static int get_key_size (KCryptAlgorithm algo)
  {
  switch (algo)
    {
    case ALG_FAST: return kcrypt_fast_get_key_size (); break;
    case ALG_IDEA: return kcrypt_idea_get_key_size (); break;
    default:
       {
       fprintf (stderr, "Bad algorithm code: %d\n", algo);
       exit (-1);
       }
    }
  return -1;
  }


/*=======================================================================
  get_block_size
=========================================================================*/
static int get_block_size (KCryptAlgorithm algo)
  {
  switch (algo)
    {
    case ALG_FAST: return kcrypt_fast_get_block_size (); break;
    case ALG_IDEA: return kcrypt_idea_get_block_size (); break;
    default:
       {
       fprintf (stderr, "Bad algorithm code: %d\n", algo);
       exit (-1);
       }
    }
  return -1;
  }


/*=======================================================================
  init_encrypt 
=========================================================================*/
static char *init_encrypt (KCryptAlgorithm algo, char *key)
  {
  switch (algo)
    {
    case ALG_FAST: return kcrypt_fast_init_encrypt (key);
    case ALG_IDEA: return kcrypt_idea_init_encrypt (key);
    default:
       {
       fprintf (stderr, "Bad algorithm code: %d\n", algo);
       exit (-1);
       }
    }
  }

/*=======================================================================
  init_decrypt 
=========================================================================*/
static char *init_decrypt (KCryptAlgorithm algo, char *key)
  {
  switch (algo)
    {
    case ALG_FAST: return kcrypt_fast_init_decrypt (key);
    case ALG_IDEA: 
      {
      char *data = kcrypt_idea_init_decrypt (key);
      return data;
      }
    default:
       {
       fprintf (stderr, "Bad algorithm code: %d\n", algo);
       exit (-1);
       }
    }
  }

/*=======================================================================
  password_to_key 
=========================================================================*/
char *password_to_key (const char *password, KCryptAlgorithm algo)
  {
  int l = get_key_size (algo);
  int pwl = strlen (password);
  char *ret = malloc (l + 1);
  int i, j = 0;
  for (i = 0; i < l; i++)
    {
    ret [i] = password [j];
    j++;
    if (j >= pwl) j = 0;
    }
  // We null-terminate the key for convenience
  ret [l] = 0; 
  return ret;
  }


/*=======================================================================
  kcrypt_engine_get_file_info
=========================================================================*/
void kcrypt_engine_get_file_info (const char *path, KCryptFileInfo *info, 
    const char *password, klib_Error **error)
  {
  info->enc_status = ENC_UNENCRYPTED;
  FILE *f = fopen (path, "rb");
  if (f)
    {
    KCryptFileHeader header;
    if (fread (&header, sizeof (header), 1, f) == 1)
      {
      // This file is long enough to have a kcrypt header
      if (memcmp (&header.magic, KCRYPT_MAGIC, sizeof (header.magic)) == 0)
         {
         // TODO -- check algo
         // This is definitely a kcrypt file, but can we decrypt it?
         char dec_magic [sizeof (header.magic)];
         char *key = password_to_key (password, header.algorithm);
         char *alg_data = init_decrypt (header.algorithm, key);
         decrypt_block (header.algorithm, header.enc_magic, dec_magic, 
           sizeof (dec_magic), key, alg_data);
         if (memcmp (dec_magic, KCRYPT_MAGIC, sizeof (dec_magic)) == 0)
           {
           // Yes, this is a valid file and our password decrypts it
           char dec_name [KCRYPT_MAX_FILENAME];
           decrypt_block (header.algorithm, header.orig_name, dec_name, 
             sizeof (dec_name), key, alg_data); 
           strcpy (info->orig_name, dec_name);
           info->major_version = header.major_version;
           info->minor_version = header.minor_version;
           info->file_size = header.file_size;
           info->algorithm = header.algorithm;
           // Compare the stored name against the name we were passed,
           //  to see if the name is mangled.
           char *shortname = _klib_path_get_shortname (path); 
           if (strcmp (info->orig_name, shortname) == 0)
             info->enc_status = ENC_ENCRYPTEDNOTMANGLED;
           else
             info->enc_status = ENC_ENCRYPTEDANDMANGLED;
           free (shortname);
           }
         else
           {
           // It is a valid file, but our password does not decrypt it
           // This is not an error, but the caller needs to be aware
           //  that nothing is known about this file apart from that
           //  it has at some time been encrypted with this program
           info->enc_status = ENC_ENCRYPTEDBADPASSWD;
           }
         memset (key, 0, strlen (key)); 
         free (key);
         if (alg_data) free (alg_data);
         }
      else
         {
         // No magic number -- probably not encrypted
         info->enc_status = ENC_UNENCRYPTED;
         }
      }
    else
      {
      // Do nothing -- file is short, but that's not an error -- it's 
      //  probably not encrypted
      info->enc_status = ENC_UNENCRYPTED;
      }

    fclose (f);
    }
  else
    *error = klib_error_new (errno, "Can't open %s for reading", path);
  }


/*=======================================================================
  write_header 
=========================================================================*/
static BOOL write_header (FILE *f, const char *shortname, int64_t size, 
    const char *password, KCryptAlgorithm algo)
  {
  char *key = password_to_key (password, algo);
  char *alg_data = init_encrypt (algo, key);

  KCryptFileHeader header;
  memset (&header, 0, sizeof (header));
  memcpy (header.magic, KCRYPT_MAGIC, sizeof (header.magic));
  encrypt_block (algo, header.magic, 
   header.enc_magic, sizeof (header.magic), key, alg_data);
  header.major_version = atoi (MAJOR_VERSION);
  header.minor_version = atoi (MINOR_VERSION);
  header.file_size = size;
  header.algorithm = algo;
  char padded_shortname [KCRYPT_MAX_FILENAME];
  memset (padded_shortname, 0, sizeof (padded_shortname));
  strcpy (padded_shortname, shortname);
  encrypt_block (algo, padded_shortname, 
   header.orig_name, sizeof (header.orig_name), key, alg_data);

  if (alg_data) free (alg_data);
  free (key);

  if (fwrite (&header, sizeof (header), 1, f) == 1)
    return TRUE;
  else
    return FALSE;
  }


/*=======================================================================
  kcrypt_engine_encrypt_to_new_file
=========================================================================*/
void kcrypt_engine_encrypt_to_new_file (const char *source_filename, 
        const char *target_filename, KCryptAlgorithm algo, 
        const char *password, KCryptEngineProgressProc progress, 
        void *user_data, klib_Error **error)
  {
  klib_Path *source_path = klib_path_new (source_filename);
  char *shortname = klib_path_get_shortname (source_path);
  struct stat sb;
  stat (source_filename, &sb);
  // Don't worry about stat failing -- the read will fail as well
  int64_t size = sb.st_size;
  FILE *fin = fopen (source_filename, "rb");
  if (fin)
    {
    FILE *fout = fopen (target_filename, "wb");
    if (fout)
      {
      if (write_header (fout, shortname, size, password, algo))
        {
        int block_size = get_block_size(algo);
        char *block = malloc (block_size);
        char *enc_block = malloc (block_size);
        char *key = password_to_key (password, algo);
        char *alg_data = init_encrypt (algo, key);

        int n = 0;
        int percent = 0;
        int64_t total_written = 0LL;
        do 
          {
          memset (block, 0, block_size);
          n = fread (block, 1, block_size, fin);
          if (n > 0)
            {
            encrypt_block (algo, block, enc_block, 
             block_size, key, alg_data); 
            fwrite (enc_block, block_size, 1, fout); 
            total_written += block_size;
            int new_percent = (int) (total_written * 100LL / sb.st_size);
            if (new_percent > percent)
             {
             percent = new_percent; 
             progress (percent, user_data);
             } 
            }
          } while (n > 0);

        if (alg_data)
          free (alg_data);
        free (key);
        free (block);
        free (enc_block);
        }
      else
        {
        *error = klib_error_new (errno, "Can't write file %s", 
          target_filename);
        }
      fclose (fout);
      }
    else
      {
      *error = klib_error_new (errno, "Can't open %s for writing", 
        target_filename);
      } 
    fclose (fin);
    }
  else
    {
    *error = klib_error_new (errno, "Can't open %s for reading", 
      source_filename);
    }

  free (shortname);
  klib_path_free (source_path);
  }


/*=======================================================================
  kcrypt_engine_decrypt_to_new_file
=========================================================================*/
void kcrypt_engine_decrypt_to_new_file (const char *source_filename, 
        const char *target_filename, 
        const char *password, 
        KCryptEngineProgressProc progress, 
        void *user_data, klib_Error **error)
  {
  klib_Path *source_path = klib_path_new (source_filename);
  char *shortname = klib_path_get_shortname (source_path);

  KCryptFileInfo info;
  kcrypt_engine_get_file_info (source_filename, &info, password, error);

  if (!(*error))
    {
    FILE *fin = fopen (source_filename, "rb");
    if (fin)
      {
      FILE *fout = fopen (target_filename, "wb");
      if (fout)
        {
        // Skip and discard header -- we already know all we need
        //   to from calling get_file_info
        KCryptFileHeader header;
        fread (&header, sizeof (header), 1, fin);    

        int block_size = get_block_size(info.algorithm);
        char *block = malloc (block_size);
        char *dec_block = malloc (block_size);
        char *key = password_to_key (password, info.algorithm);
        char *alg_data = init_decrypt (info.algorithm, key);

        int n = 0;
        int64_t total_written = 0;
        int percent = 0;
        do 
          {
          memset (block, 0, block_size);
          n = fread (block, 1, block_size, fin);
          if (n > 0)
            {
            decrypt_block (info.algorithm, block, dec_block, 
             block_size, key, alg_data); 
            int r = block_size;
            if (info.file_size - total_written < block_size)
              r = info.file_size - total_written; 
            fwrite (dec_block, r, 1, fout); 
            total_written += block_size;
            int new_percent = (int) (total_written * 100LL / info.file_size);
            if (new_percent > percent)
             {
             percent = new_percent; 
             progress (percent, user_data);
             } 
            }
          } while (n > 0);

        if (alg_data) free (alg_data);
        free (key);
        free (block);
        free (dec_block);


        fclose (fout);
        }
      else
        {
        *error = klib_error_new (errno, "Can't open %s for writing", 
          target_filename);
        }
      fclose (fin);
      }
    else
      {
      *error = klib_error_new (errno, "Can't open %s for reading", 
        source_filename);
      }
    }


  free (shortname);
  klib_path_free (source_path);
  }


/*=======================================================================
  kcrypt_engine_decrypt_to_stream
=========================================================================*/
void kcrypt_engine_decrypt_to_stream (const char *source_filename, FILE *f, 
    const char *password, KCryptEngineProgressProc progress, 
    void *user_data, klib_Error **error)
  {
  klib_Path *source_path = klib_path_new (source_filename);
  char *shortname = klib_path_get_shortname (source_path);

  KCryptFileInfo info;
  kcrypt_engine_get_file_info (source_filename, &info, password, error);

  if (!(*error))
    {
    FILE *fin = fopen (source_filename, "rb");
    if (fin)
      {
      FILE *fout = f; 
      if (fout)
        {
        // Skip and discard header -- we already know all we need
        //   to from calling get_file_info
        KCryptFileHeader header;
        fread (&header, sizeof (header), 1, fin);    

        int block_size = get_block_size(info.algorithm);
        char *block = malloc (block_size);
        char *dec_block = malloc (block_size);
        char *key = password_to_key (password, info.algorithm);
        char *alg_data = init_decrypt (info.algorithm, key);

        int n = 0;
        int64_t total_written = 0;
        int percent = 0;
        do 
          {
          memset (block, 0, block_size);
          n = fread (block, 1, block_size, fin);
          if (n > 0)
            {
            decrypt_block (info.algorithm, block, dec_block, 
             block_size, key, alg_data); 
            int r = block_size;
            if (info.file_size - total_written < block_size)
              r = info.file_size - total_written; 
            fwrite (dec_block, r, 1, fout); 
            total_written += block_size;
            if (progress)
              {
              int new_percent = (int) (total_written * 100LL / info.file_size);
              if (new_percent > percent)
                {
                percent = new_percent; 
                progress (percent, user_data);
                } 
              }
            }
          } while (n > 0);

        if (alg_data) free (alg_data);
        free (key);
        free (block);
        free (dec_block);
        }
      else
        {
        // This can't happen
        }
      fclose (fin);
      }
    else
      {
      *error = klib_error_new (errno, "Can't open %s for reading", 
        source_filename);
      }
    }


  free (shortname);
  klib_path_free (source_path);

  }


/*=======================================================================
  kcrypt_engine_wipe
=========================================================================*/
void kcrypt_engine_wipe (const char *filename, 
       KCryptEngineProgressProc progress, void *user_data, 
       klib_Error **error)
  {
  struct stat sb;
  if (stat (filename, &sb) == 0)
    { 
    // Don't worry about stat failing -- the open will fail as well
    FILE *f = fopen (filename, "wb");
    int64_t total_written = 0;
    char buff[4096];
    memset (buff, 0, sizeof(buff));
    if (f)
      {
      do
        {
        fwrite (buff, sizeof (buff), 1, f);
        total_written += sizeof (buff);
        } while (total_written < sb.st_size); 
      fclose (f); 
      unlink (filename); 
      }
    }
  else
    *error = klib_error_new (errno, "stat() call failed for %s: %s",
       filename, strerror (errno));
  }


