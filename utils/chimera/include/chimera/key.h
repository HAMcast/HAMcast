/*
** $Id: key.h,v 1.16 2006/06/07 09:21:28 krishnap Exp $
**
** Matthew Allen
** description: 
*/

#ifndef _CHIMERA_KEY_H_
#define _CHIMERA_KEY_H_

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <string.h>

#ifndef ULONG_MAX
    #define ULONG_MAX  0xffffffff
#endif

#define KEY_ARRAY_SIZE 3
#define KEY_SIZE (KEY_ARRAY_SIZE * sizeof(uint32_t) * 8)

// Changed this to 2 for base4 and 4 to have keys in base 16; Only these two are supported right now
#define BASE_B 2		/* Base representation of key digits */

#define BASE_2 2
#define BASE_4 4
#define BASE_16 16

#define KEYLENGTH (KEY_SIZE / BASE_B)
#define BASE_16_KEYLENGTH (KEY_SIZE / BASE_4)

#define IS_BASE_2  (BASE_B == 1)
#define IS_BASE_4  (BASE_B == 2)
#define IS_BASE_16 (BASE_B == 4)

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint32_t t[KEY_ARRAY_SIZE];
    short int valid;
} Key;

typedef struct Keylist_t
{
    Key                 *value;
    struct Keylist_t    *next;
} Keylist;

/* HELPER FUNCTIONS */
/**
 * @brief read_u16_from_buffer
 * @param read
 * @return
 */
uint16_t read_u16_from_buffer (const char *read);

/**
 * @brief read_u32_from_buffer
 * @param read
 * @return
 */
uint32_t read_u32_from_buffer (const char* read);

/**
 * @brief write_u16_to_buffer
 * @param u16
 * @param write
 */
void write_u16_to_buffer(uint16_t u16, char* write);

/**
 * @brief write_u32_to_buffer
 * @param u32
 * @param write
 */
void write_u32_to_buffer(uint32_t u32, char* write);

/**
 * @brief read_key_from_buffer
 * @param read
 * @return
 */
Key read_key_from_buffer (const char *read);

/**
 * @brief write_key_to_buffer
 * @param k
 * @param write
 */
void write_key_to_buffer (Key k, char* write);

/* END HELPER FUNCTIONS */

/* global variables!! that are set in key_init function */
// moved to key.c
//static Key Key_Max;
//static Key Key_Half;

/* key_makehash: hashed, s
** assign sha1 hash of the string #s# to #hashed# */

void key_makehash (void *logs, Key * hashed, const char *s);


/* key_make_hash */
void key_make_hash (Key * hashed, const char *s, size_t size);

/* key_init: 
** initializes Key_Max and Key_Half */

void key_init ();

/* key_distance:k1,k2
** calculate the distance between k1 and k2 in the keyspace and assign that to #diff# */

void key_distance (void *logs, Key * diff, const Key * const k1,
		   const Key * const k2);


/* key_between: test, left, right
** check to see if the value in #test# falls in the range from #left# clockwise
** around the ring to #right#. */

int key_between (void *logs, const Key * const test, const Key * const left,
		 const Key * const right);


/* key_midpoint: mid, key
** calculates the midpoint of the namespace from the #key#  */

void key_midpoint (void *logs, Key * mid, Key key);


/* key_index: mykey, key
** returns the lenght of the longest prefix match between #mykey# and #k# */

int key_index (void *logs, Key mykey, Key k);

/* key_index: mykey, key
** returns the lenght of the longest prefix match between #mykey# and #k# */
int key_index_int(Key mykey, Key k);

int get_col(const Key k, int i);

void key_print (Key k);

void key_to_cstr (const Key *k, char* s, size_t slen);

char *get_key_string (Key* key);

void cstr_to_key (char* s, Key* k);

void hash_to_key (char* h, Key* k);

/* key_assign: k1, k2
** copies value of #k2# to #k1# */

void key_assign (Key * k1, Key k2);

/* key_assign_ui: k1, ul
** copies #ul# to the least significant 32 bits of #k# */

void key_assign_ui (Key * k, uint32_t ul);

/* key_equal:k1, k2 
** return 1 if #k1#==#k2# 0 otherwise*/

int key_equal (Key k1, Key k2);

/* key_equal_ui:k1, ul
** return 1 if the least significat 32 bits of #k1#==#ul# 0 otherwise */

int key_equal_ui (Key k, uint32_t ul);

/*key_comp: k1, k2
** returns >0 if k1>k2, <0 if k1<k2, and 0 if k1==k2 */

int key_comp (const Key * const k1, const Key * const k2);


#ifdef __cplusplus
}
#endif

#endif /* _CHIMERA_KEY_H_ */
