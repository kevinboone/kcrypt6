/*=======================================================================

kcrypt 6.0
Copyright (c)1998-2013 Kevin Boone
Distributed according to the terms of the GNU Public Licence, v2.0

=========================================================================*/

#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "klib.h" 
#include "kcrypt_engine.h" 
#include "kcrypt_idea.h" 

#define BLOCK_SIZE 8
// KSL is 52 16-bit integers
#define KEY_SCHEDULE_LENGTH 52
#define low16(x) ((x) & 0xFFFF)
#define IDEAROUNDS 8
#define IDEAKEYLEN (6*IDEAROUNDS+4)

typedef uint16_t uint16_t;
typedef uint32_t uint32_t;

//static char cfbdata[BLOCK_SIZE];

typedef struct _AlgData
  {
  char cfbdata[BLOCK_SIZE];
  uint16_t ks [KEY_SCHEDULE_LENGTH];
  } AlgData;

/*======================================================================
    MUL
========================================================================*/
#define MUL(x,y) \
    ((t16 = (y)) ? \
        (x=low16(x)) ? \
            t32 = (uint32_t)x*t16, \
            x = low16(t32), \
            t16 = t32>>16, \
            x = (x-t16)+(x<t16) \
        : \
            (x = 1-t16) \
    : \
        (x = 1-x))

/*======================================================================
	encrypt_block
======================================================================*/
static void encrypt_block (void *outbuf, const void *inbuf, AlgData *alg_data)
  {
  uint16_t *key = alg_data->ks;
  register uint16_t x1, x2, x3, x4, s2, s3;
  uint16_t *in, *out;
#ifndef SMALL_CACHE
  register uint16_t t16;	
  register uint32_t t32;
#endif
  int r = IDEAROUNDS;

  in = (uint16_t *) inbuf;
  x1 = *in++;
  x2 = *in++;
  x3 = *in++;
  x4 = *in;

  x1 = (x1 >> 8) | (x1 << 8);
  x2 = (x2 >> 8) | (x2 << 8);
  x3 = (x3 >> 8) | (x3 << 8);
  x4 = (x4 >> 8) | (x4 << 8);

  do {
	MUL(x1, *key++);
	x2 += *key++;
	x3 += *key++;
	MUL(x4, *key++);

	s3 = x3;
	x3 ^= x1;
	MUL(x3, *key++);
	s2 = x2;
	x2 ^= x4;
	x2 += x3;
	MUL(x2, *key++);
	x3 += x2;

	x1 ^= x2;
	x4 ^= x3;

	x2 ^= s3;
	x3 ^= s2;
    } while (--r);
  MUL(x1, *key++);
  x3 += *key++;
  x2 += *key++;
  MUL(x4, *key);

  out = (uint16_t *) outbuf;

  *out++ = (x1 >> 8) | (x1 << 8);
  *out++ = (x3 >> 8) | (x3 << 8);
  *out++ = (x2 >> 8) | (x2 << 8);
  *out = (x4 >> 8) | (x4 << 8);
  }


/*======================================================================
  makeKeySchedule
  Horrible things will happen if the supplied key is not the
  correct size for the algorithm 
  Returns a new array of 52 16-bit integers
======================================================================*/
static void make_key_schedule (const char *key, uint16_t *ks)
  {
  const char *userkey = key; 

  ks[0] = (short)((userkey[ 0] & 0xFF) << 8 | (userkey[ 1] & 0xFF));
  ks[1] = (short)((userkey[ 2] & 0xFF) << 8 | (userkey[ 3] & 0xFF));
  ks[2] = (short)((userkey[ 4] & 0xFF) << 8 | (userkey[ 5] & 0xFF));
  ks[3] = (short)((userkey[ 6] & 0xFF) << 8 | (userkey[ 7] & 0xFF));
  ks[4] = (short)((userkey[ 8] & 0xFF) << 8 | (userkey[ 9] & 0xFF));
  ks[5] = (short)((userkey[10] & 0xFF) << 8 | (userkey[11] & 0xFF));
  ks[6] = (short)((userkey[12] & 0xFF) << 8 | (userkey[13] & 0xFF));
  ks[7] = (short)((userkey[14] & 0xFF) << 8 | (userkey[15] & 0xFF));

  int i, zoff, j; 
  for (i = 0, zoff = 0, j = 8; j < KEY_SCHEDULE_LENGTH; i &= 7, j++)
    {
    i++;
    int zz = i + 7 + zoff;
    ks[zz] = (short)((ks[(i & 7) + zoff] << 9) |
        ((ks[((i + 1) & 7) + zoff] >> 7) & 0x1FF));
    zoff += i & 8;
    }
  }


/*=======================================================================
  xor_block
  This only works on the 8-byte blocks that IDEA uses
=========================================================================*/
void xor_block (char *inout, const char *xor)
  {
  // In the expectation that we'll probably be running on at least
  //  a 32-bit machine, do the XOR as two 32-bit operations, rather than
  //  8 8-bit operations
  int32_t *_inout1 = (int32_t *)inout;
  int32_t *_inout2 = (int32_t *)(inout + 4);
  int32_t *_xor1 = (int32_t *)xor;
  int32_t *_xor2 = (int32_t *)(xor + 4);

  (*_inout1) ^= *_xor1;
  (*_inout2) ^= *_xor2;

/*
  register int j; 
  for (j = 0; j < BLOCK_SIZE; j++)
    {
    inout [j] = inout [j] ^ xor[j]; 
    }
*/
  }


/*=======================================================================
  encrypt_block 
  // IMPORTANT -- 
  // Input and output buffers must be the same size, and a multiple of
  //  the encryption block size
=========================================================================*/
void kcrypt_idea_encrypt_block (const char *in,  
    char *out, int size, const char *key, char *_alg_data)
  {
  AlgData *alg_data = (AlgData *)_alg_data;
  int i, blocks = size / BLOCK_SIZE;
  const char *inp = in;
  char *outp = out;
  for (i = 0; i < blocks; i++)
    {
    char temp_block [BLOCK_SIZE];
    memcpy (temp_block, inp, BLOCK_SIZE);
    xor_block (temp_block, alg_data->cfbdata);
    encrypt_block (outp, temp_block, alg_data);
    memcpy (alg_data->cfbdata, outp, BLOCK_SIZE); 
    inp += BLOCK_SIZE;
    outp += BLOCK_SIZE;
    } 
  }


/*=======================================================================
  decrypt_block 
  // IMPORTANT -- 
  // Input and output buffers must be the same size, and a multiple of
  //  the encryption block size
=========================================================================*/
void kcrypt_idea_decrypt_block (const char *in,  
    char *out, int size, const char *key, char *_alg_data)
  {
  AlgData *alg_data = (AlgData *)_alg_data;
  int i, blocks = size / BLOCK_SIZE;
  const char *inp = in;
  char *outp = out;
  for (i = 0; i < blocks; i++)
    {
    char temp_block [BLOCK_SIZE];
    encrypt_block (temp_block, inp, alg_data);
    xor_block (temp_block, alg_data->cfbdata);
    memcpy (outp, temp_block, BLOCK_SIZE);
    memcpy (alg_data->cfbdata, inp, BLOCK_SIZE);
    inp += BLOCK_SIZE;
    outp += BLOCK_SIZE;
    }
  }

/*=======================================================================
  kcrypt_idea_get_get_key_size()
=========================================================================*/
int kcrypt_idea_get_key_size (void)
  {
  return IDEAKEYLEN;
  }

/*=======================================================================
  kcrypt_idea_get_get_block_size()
=========================================================================*/
int kcrypt_idea_get_block_size (void)
  {
  return BLOCK_SIZE;
  }

/*=======================================================================
  inv
=========================================================================*/
static short inv (uint16_t xx)
{
  int x = xx & 0xFFFF; 
  if (x <= 1)
    return (uint16_t)x;    

  int t1 = 0x10001 / x;  
  int y = 0x10001 % x;
  if (y == 1)
     return (uint16_t)(1 - t1);

  int t0 = 1;
  int q;
  do 
    {
    q = x / y;
    x = x % y;
    t0 += q * t1;
    if (x == 1)
      return (uint16_t)t0;
    q = y / x;
    y %= x;
    t1 += q * t0;
    } while (y != 1);
  return (uint16_t)(1 - t1);
  }

/*=======================================================================
  invert_key_schedule 
=========================================================================*/
static void invert_key_schedule (uint16_t *ks)
  {
  int i, j = 4, k = KEY_SCHEDULE_LENGTH - 1;
  uint16_t temp[KEY_SCHEDULE_LENGTH];
  temp[k--] = inv(ks[3]);
  temp[k--] = (uint16_t) -ks[2];
  temp[k--] = (uint16_t) -ks[1];
  temp[k--] = inv(ks[0]);
  for (i = 1; i < IDEAROUNDS; i++, j += 6) 
    {
    temp[k--] = ks[j + 1];
    temp[k--] = ks[j];
    temp[k--] = inv(ks[j + 5]);
    temp[k--] = (uint16_t) -ks[j + 3];
    temp[k--] = (uint16_t) -ks[j + 4];
    temp[k--] = inv(ks[j + 2]);
    }
  temp[k--] = ks[j + 1];
  temp[k--] = ks[j];
  temp[k--] = inv(ks[j + 5]);
  temp[k--] = (uint16_t) -ks[j + 4];
  temp[k--] = (uint16_t) -ks[j + 3];
  temp[k--] = inv(ks[j + 2]);
  memcpy (ks, &temp, KEY_SCHEDULE_LENGTH * sizeof(uint16_t));
  }

/*=======================================================================
  kcrypt_idea_init_encrypt ()
=========================================================================*/
char *kcrypt_idea_init_encrypt (const char *key)
  {
  AlgData *algData = malloc (sizeof (AlgData));
  make_key_schedule (key, algData->ks);
  memset (algData->cfbdata, 0, BLOCK_SIZE);
  return (char *)algData;
  }


/*=======================================================================
  kcrypt_idea_init_decrypt ()
=========================================================================*/
char *kcrypt_idea_init_decrypt (const char *key)
  {
  AlgData *algData = malloc (sizeof (AlgData));
  make_key_schedule (key, algData->ks);
  memset (algData->cfbdata, 0, BLOCK_SIZE);
  invert_key_schedule (algData->ks);
  return (char *)algData;
  }



