/*
 * File:    testHashTable.c
 * Author:  Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * Date:    2014-07-12 01:14
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "hash_table/hashTable-chained.h"

/*
 * Test for new and free. (Check with Valgrind that memory is managed properly)
 */
void test1(void)
{
	CHashTable ht;

	printf("%s: ", __func__);

	ht = CHashTable_new(0, (cmpfunc)strcmp);

	CHashTable_free(&ht);
	printf("success!\n");
}

/*
 * Check that double insertion fails and that the inserted key can be found.
 */
void test2(void)
{
	CHashTable ht;
	char *ptr = (void *)0xC0DEDBAD;

	printf("%s: ", __func__);

	ht = CHashTable_new(0, (cmpfunc)strcmp);

	assert(CHashTable_insert(ht, "Hello World", 1, ptr) == 0);
	assert(CHashTable_insert(ht, "Hello World", 1, ptr) < 0);

	assert(CHashTable_lookup(ht, "Hello World", 1) == ptr);

	CHashTable_free(&ht);
	printf("success!\n");
}

/* Bob Jenkins' one at a time hash */
unsigned int BJ_OAT_hash(const char *key, size_t sizeofkey)
{
	register unsigned int hash, i;

	for (hash = i = 0; i < sizeofkey; ++i) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

#define KEYS 1000
#define BUFF_SIZE 128

int random_numbers[KEYS];

/*
 * checks that the table grows and that no key is lost.
 */
void test3(void)
{
	CHashTable ht;
	char *keys[KEYS];
	char buff[BUFF_SIZE];
	int i;

	printf("%s: ", __func__);

	ht = CHashTable_new(1, (cmpfunc)strcmp);

	CHashTable_setKeyFree(ht, free); /* on table destruction, all keys should be freed */

	/*
	 * Randomly generate keys & insert them in the Hash Table.
	 */
	for (i = 0; i < KEYS; i++) {
		snprintf(buff, BUFF_SIZE, "string-%d", random_numbers[i]);
		keys[i] = strdup(buff);
		assert(CHashTable_insert(ht, keys[i], BJ_OAT_hash(keys[i], strlen(keys[i])), keys[i]) == 0);
	}

	/*
	 * Test that every one of them can be found.
	 */
	for (i = 0; i < KEYS; i++) {
		assert(CHashTable_lookup(ht, keys[i], BJ_OAT_hash(keys[i], strlen(keys[i]))) == keys[i]);
	}

	CHashTable_free(&ht);
	printf("success!\n");
}

/*
 * Modified version of previous test. Remove 4 keys from the table,
 * check that those can't be found but the rest still can.
 */
void test4(void)
{
	CHashTable ht;
	char *keys[KEYS];
	char buff[BUFF_SIZE];
	int i, j, k;
	int removed[] = {10, 25, 67, 901};

	printf("%s: ", __func__);

	ht = CHashTable_new(1, (cmpfunc)strcmp);


	/*
	 * Randomly generate keys & insert them in the Hash Table.
	 */
	for (i = 0; i < KEYS; i++) {
		snprintf(buff, BUFF_SIZE, "string-%d", random_numbers[i]);
		keys[i] = strdup(buff);
		assert(CHashTable_insert(ht, keys[i], BJ_OAT_hash(keys[i], strlen(keys[i])), keys[i]) == 0);
	}

	for (i = 0; i < 4; ++i) {
		assert(CHashTable_remove(ht, keys[removed[i]], BJ_OAT_hash(keys[removed[i]], strlen(keys[removed[i]]))) == 0);
	}

 	/* 
 	 * Removed keys haven't been freed yet because I haven't set a destructor for them.
 	 */
	CHashTable_setKeyFree(ht, free);

	/*
	 * Test that every one of them can be found, except for those which have been removed.
	 */
	for (i = 0, j = removed[0], k = 0; i < KEYS; i++) {
		if (i == j && k < 4) {
			assert(CHashTable_lookup(ht, keys[i], BJ_OAT_hash(keys[i], strlen(keys[i]))) == NULL);
			free(keys[removed[k]]);
			k++;
			j = removed[k];

		} else {
			assert(CHashTable_lookup(ht, keys[i], BJ_OAT_hash(keys[i], strlen(keys[i]))) == keys[i]);
		}
	}

	CHashTable_free(&ht);
	printf("success!\n");
}


void test(void)
{
	CHashTable ht;
	printf("%s: ", __func__);

	ht = CHashTable_new(0, (cmpfunc)strcmp);

	CHashTable_free(&ht);
	printf("success!\n");
}

/*
 * There's a low probability of collision because I'm not taking a lot of samples,
 * but I still wanna make sure that all numbers are different.
 */
void generate_random_vector(void)
{
	int i, j, x;

	srand(time(NULL));

	for (i = 0; i < KEYS;) {
		x = rand();

		for (j = 0; j < i; j++) { /* check that x is a unique new random number, naive (slow) way */
			if (random_numbers[j] == x) {
				break;
			}
		}

		if (j == i) {
			random_numbers[i] = x;
			i++;
		}
	}
}

int main()
{
	generate_random_vector();
	test1();
	test2();
	test3();
	test4();
	printf("All tests passed!\n");
	return 0;
}
