#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>
#include "key.h"
#include "base.h"
#include "log.h"
#include <math.h>

#ifdef ULONG_MAX
    #undef ULONG_MAX
#endif
#define ULONG_MAX  0xffffffff

static Key Key_Max;
static Key Key_Half;

/* HELPER FUNCTIONS */

uint16_t read_u16_from_buffer (const char* read)
{
    uint16_t n_u16 = *((uint16_t*)read);
    return ntohs(n_u16);
}

uint32_t read_u32_from_buffer (const char *read)
{
    uint32_t n_u32 = *((uint32_t*)read);
    return ntohl(n_u32);
}

void write_u16_to_buffer (uint16_t h_u16, char* write)
{
    uint16_t n_u16 = htons (h_u16);
    memcpy (write, &n_u16, sizeof(uint16_t));
}

void write_u32_to_buffer (uint32_t h_u32, char* write)
{
    uint32_t n_u32 = htonl (h_u32);
    memcpy (write, &n_u32, sizeof(uint32_t));
}

Key read_key_from_buffer (const char* read)
{
    Key k;
    int i=0;
    const char *tmp = read;
    for (i=0; i < KEY_ARRAY_SIZE; i++) {
        k.t[i] = read_u32_from_buffer(tmp);
        tmp += sizeof(uint32_t);
    }
    return k;
}

void write_key_to_buffer (Key k, char* write)
{
    int i=0;
    char* tmp = write;
    for (i=0; i < KEY_ARRAY_SIZE; i++) {
        write_u32_to_buffer(k.t[i], tmp);
        tmp += sizeof (uint32_t);
    }
}

void key_print (Key k)
{
    char keystr[KEY_SIZE];      // this is big just to be safe
    key_to_cstr(&k,keystr,KEY_SIZE);
    printf ("%s\n", keystr);
}

/* HELPER FUNCTIONS END */

void key_to_cstr (const Key* k, char* s, size_t slen)
{
    int i;
    size_t klen = KEY_SIZE / BASE_B;
    if (slen > klen) {
        if (IS_BASE_16) {
            for (i=0; i < KEY_ARRAY_SIZE; ++i) {
                sprintf(s+(i*8), "%08x", k->t[KEY_ARRAY_SIZE-1 - i]);
            }
            s[klen] = '\0';
            return;
        }
        else if (IS_BASE_4) {
            char temp[KEY_SIZE];
            for (i=0; i < KEY_ARRAY_SIZE; ++i) {
                sprintf(temp+(i*8), "%08x", k->t[KEY_ARRAY_SIZE-1 - i]);
            }
            temp[BASE_16_KEYLENGTH] = '\0';
            hex_to_base4 (temp, s, slen);
            s[klen] = '\0';
            return;
        }
    }
    fprintf(stderr, "Convert key to cstring failed, size: %ld!\n", slen);
    s[0] = '\0';
}

// FIXME: Check, if this is still that expensive
void cstr_to_key (char *s, Key * k)
{
    int i, len;
    char key_str[KEY_SIZE / BASE_B + 1];
    char str[KEY_SIZE / BASE_B + 1];
    char tempString[KEY_SIZE];

    memset (key_str, '0', (KEY_SIZE / BASE_B + 1));
    key_str[KEY_SIZE / BASE_B] = '\0';
    memset (str, '\0', (KEY_SIZE / BASE_B + 1));

    if (strlen (s) < (KEY_SIZE / BASE_B + 1)) {
        strcpy (str, s);
    } else {
        strncpy (str, s, KEY_SIZE / BASE_B);
        str[KEY_SIZE / BASE_B] = '\0';
    }
    // Now, if str is in a different base than hex, replace the str contents with corresponding hex contents
    if (IS_BASE_4) {
        strcpy (tempString, str);
        memset (str, 0, strlen (tempString));
        base4_to_hex (tempString, str);
    }

    // By now, the string should be in base 16
    len = strlen (str);
    if (len == 0) {
        fprintf (stderr, "str_to_key: Warning:Empty input string\n");
    }
    else if (len > BASE_16_KEYLENGTH) {
        strncpy (key_str, str, BASE_16_KEYLENGTH);
    }
    else if (len <= BASE_16_KEYLENGTH) {
        for (i = 0; i < len; i++) {
            key_str[i + (BASE_16_KEYLENGTH) - len] = str[i];
        }
    }
    key_str[BASE_16_KEYLENGTH] = '\0';

    for (i = 0; i < KEY_ARRAY_SIZE; i++) {
        sscanf (key_str + (i*8), "%08x", &(k->t[(KEY_ARRAY_SIZE-1 - i)]));
    }
}

char *get_key_string (Key * key)
{
    size_t klen = KEY_SIZE / BASE_B + 1;
    char* keystr = (char *) malloc (klen);
    key_to_cstr(key,keystr,klen);
    return keystr;
}

void key_assign (Key * k1, Key k2)
{
    int i;
    for (i = 0; i < KEY_ARRAY_SIZE; i++) {
        k1->t[i] = k2.t[i];
    }
    k1->valid = k2.valid;
}

void key_assign_ui (Key * k, uint32_t ul)
{
    int i;
    for (i = 1; i < KEY_ARRAY_SIZE; i++)
        k->t[i] = 0;
    k->t[0] = ul;
}

int key_equal (Key k1, Key k2)
{
    int i;
    for (i = 0; i < KEY_ARRAY_SIZE; i++) {
        if (k1.t[i] != k2.t[i])
            return (0);
    }
    return (1);
}

int key_equal_ui (Key k, uint32_t ul)
{
    int i;
    if (k.t[0] != ul)
        return (0);

    for (i = 1; i < KEY_ARRAY_SIZE; i++) {
        if (k.t[i] != 0)
            return (0);
    }
    return (1);
}

int key_comp (const Key * const k1, const Key * const k2)
{
    if ((k1 == NULL) && (k2 == NULL))
        return (0);
    if (k1 == NULL)
        return (-1);
    if (k2 == NULL)
        return (1);

    int i;
    for (i = KEY_ARRAY_SIZE-1; i >= 0; i--) {
        if (k1->t[i] > k2->t[i])
            return (1);
        else if (k1->t[i] < k2->t[i])
            return (-1);
    }
    return (0);
}

void key_add (Key * result, const Key * const op1, const Key * const op2)
{
    double tmp, a, b;
    int i;
    a = b = tmp = 0;

    for (i = 0; i < KEY_ARRAY_SIZE; i++) {
	    a = op1->t[i];
	    b = op2->t[i];
	    tmp += a + b;

        if (tmp > ULONG_MAX) {
		    result->t[i] = (uint32_t) tmp;
		    tmp = 1;
		}
        else {
		    result->t[i] = (uint32_t) tmp;
		    tmp = 0;
		}
	}
    result->valid = 0;
}

void key_sub (void *logs, Key * result, const Key * const op1,
	      const Key * const op2)
{
    int i;
    double tmp, a, b, carry;

    carry = 0;
    if (key_comp (op1, op2) < 0)
        return;

    for (i = 0; i < KEY_ARRAY_SIZE; i++) {
	    a = op1->t[i] - carry;
	    b = op2->t[i];

        if (b <= a) {
		    tmp = a - b;
		    carry = 0;
		}
        else {
		    a = a + ULONG_MAX + 1;
		    tmp = a - b;
		    carry = 1;
		}
	    result->t[i] = (uint32_t) tmp;
	}

    result->valid = 0;
}

char *sha1_keygen (const char *key, size_t digest_size, char *digest)
{
    EVP_MD_CTX mdctx;
    const EVP_MD *md;
    unsigned char *md_value;
    int i;
    unsigned int md_len;
    char digit[10];
    char *tmp;

    md_value = (unsigned char *) malloc (EVP_MAX_MD_SIZE);

    OpenSSL_add_all_digests ();

    md = EVP_get_digestbyname ("sha1");

    EVP_MD_CTX_init (&mdctx);
    EVP_DigestInit_ex (&mdctx, md, NULL);
    EVP_DigestUpdate (&mdctx, key, digest_size);
    EVP_DigestFinal_ex (&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup (&mdctx);
    size_t dlen = 256;
    digest = (char *) malloc (digest_size > dlen ? digest_size : dlen);

    tmp = digest;
    tmp[0] = '\0';
    for (i = 0; i < md_len; i++) {
        if (IS_BASE_16) {
            convert_base16 (md_value[i], digit);
        }
        else if (IS_BASE_4) {
            convert_base4 (md_value[i], digit);
        }
        else if (IS_BASE_2) {
            convert_base2 (md_value[i], digit);
        }
        
        strcat (tmp, digit);
        tmp = tmp + strlen (digit);
    }

    free (md_value);

    tmp[0] = '\0';
    return (digest);
}

void key_makehash (void *logs, Key * hashed, const char *s)
{
    key_make_hash (hashed, s, strlen (s));
}

void hash_to_key (char* h, Key* k)
{
    cstr_to_key(h,k);
}

void key_make_hash (Key * hashed, const char *s, size_t size)
{
    char *digest;
    digest = sha1_keygen (s, size, NULL);
    hash_to_key (digest, hashed);
    //str_to_key (digest, hashed);
    free (digest);
}

void key_init ()
{
    int i;
    for (i = 0; i < KEY_ARRAY_SIZE; i++) {
        Key_Max.t[i] = ULONG_MAX;
        Key_Half.t[i] = ULONG_MAX;
    }
    Key_Half.t[KEY_ARRAY_SIZE-1] = Key_Half.t[KEY_ARRAY_SIZE-1] / 2;
}

void key_distance (void *logs, Key * diff, const Key * const k1,
                   const Key * const k2)
{
    int comp;
    comp = key_comp (k1, k2);

    /* k1 > k2 */
    if (comp > 0) {
        key_sub (logs, diff, k1, k2);
    }
    else {
        key_sub (logs, diff, k2, k1);
    }

    comp = key_comp (diff, &Key_Half);

    /* diff > Key_Half */
    if (comp > 0)
	    key_sub (logs, diff, &Key_Max, diff);

    diff->valid = 0;
}


int key_between (void *logs, const Key * const test, const Key * const left,
                 const Key * const right)
{
    int complr = key_comp (left, right);
    int complt = key_comp (left, test);
    int comptr = key_comp (test, right);

    /* it's on one of the edges */
    if (complt == 0 || comptr == 0)
	return (1);

    if (complr < 0) {
	    if (complt < 0 && comptr < 0)
		return (1);
	    return (0);
	}
    else if (complr == 0) {
	    return (0);
	}
    else {
	    if (complt < 0 || comptr < 0)
		return (1);
	    return (0);

	}
}

void key_midpoint (void *logs, Key * mid, Key key)
{

    if (key_comp (&key, &Key_Half) < 0)
	    key_add (mid, &key, &Key_Half);
    else
	    key_sub (logs, mid, &key, &Key_Half);

    mid->valid = 0;
}

int key_index (void *logs, Key mykey, Key k)
{
    return key_index_int(mykey,k);
//    int max_len, i;
//    size_t klen = (KEY_SIZE / BASE_B) + 1;
//    char mystr[klen];
//    char kstr[klen];

//    key_to_cstr(&mykey,mystr,klen);
//    key_to_cstr(&k,kstr,klen);

//    max_len = KEY_SIZE / BASE_B;
//    for (i = 0; (mystr[i] == kstr[i]) && (i < max_len); i++);

//    if (i == max_len)
//        i = max_len - 1;

//    return (i);
}

int key_index_int(Key mykey, Key k){
    int i =0;
    int m_number_of_digits = (sizeof(uint32_t)*8)/BASE_B;
    int jump = 0;
    int j;
    int max_len = KEY_SIZE / BASE_B;
    for(j = KEY_ARRAY_SIZE-1; j >=0;j--){
        if(mykey.t[j] == k.t[j]){
            i += m_number_of_digits;
            if (i == max_len){
             return  max_len - 1;
            }
        }
        else{
            jump = j;
            break;
        }
    }
    uint32_t a = mykey.t[jump];
    uint32_t b = k.t[jump];
    int tmp =m_number_of_digits;
    while(a != b){
       tmp = tmp-1;
       a = a >> BASE_B;
       b = b >> BASE_B;
    }
    i = i + tmp;
    return i;

}

int get_col(const Key k, int i){
    int max_len = KEY_SIZE / BASE_B;
    int m_number_of_digits = (sizeof(uint32_t)*8)/BASE_B;
    if(i > (max_len-1)){
        return 0;
    }
    i = i;
    int jump = i / m_number_of_digits;
    int sub = jump * m_number_of_digits;
    i = i-sub;
    int shift_left =0;
    shift_left = i*BASE_B;
    int shift_right = 0;
    shift_right = (m_number_of_digits-1)*BASE_B;
    int go_to = KEY_ARRAY_SIZE - 1 - jump;
    uint32_t g = k.t[go_to];
    g = g << shift_left;
    g = g >> shift_right;
    return g;
}
