/**
 * @file RCHashTable.c
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-08 00:25
 */

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

#include "RCHashTable.h"

#include "hash_table/hashTable-chained.h"
#include "ReferenceCounter.h"

/*----------------------------------------------------------------------------
 * DEFINED CODES AND DATA TYPES
 *----------------------------------------------------------------------------*/

struct refcounting_hash_table {
	CHashTable ht;
};

struct entry {
	struct refcounter rc;    /**< the reference counting structure */
	bool marked_for_removal; /**< if set to true, this entry should be ignored in future RCHashTable_refinc() */
};


/*----------------------------------------------------------------------------
 * PUBLIC FUNCTIONS
 *----------------------------------------------------------------------------*/

RCHashTable RCHashTable_new(uint32_t inicap, cmpfunc key_cmp, delfunc key_free)
{
	RCHashTable rcht;

	if ((rcht = malloc(sizeof(*rcht))) == NULL) {
		return NULL;
	}

	if ((rcht->ht = CHashTable_new(inicap, key_cmp)) == NULL) {
		free(rcht);
		return NULL;
	}

	CHashTable_setKeyFree(rcht->ht, key_free);
	CHashTable_setValueFree(rcht->ht, (void (*)(void *))RC_free);

	return rcht;
}

void RCHashTable_setKeyFree(RCHashTable rcht, delfunc key_free)
{
	assert(rcht != NULL);

	CHashTable_setKeyFree(rcht->ht, key_free);
}

void RCHashTable_setMaxLoadFactor(RCHashTable rcht, unsigned int percentage)
{
	assert(rcht != NULL);

	CHashTable_setMaxLoadFactor(rcht->ht, percentage);
}

void RCHashTable_setMinLoadFactor(RCHashTable rcht, unsigned int percentage)
{
	assert(rcht != NULL);

	CHashTable_setMinLoadFactor(rcht->ht, percentage);
}

void RCHashTable_setMaxRehashes(RCHashTable rcht, unsigned int maxrehashes)
{
	assert(rcht != NULL);

	CHashTable_setMaxRehashes(rcht->ht, maxrehashes);
}

int RCHashTable_insert(RCHashTable rcht,
                       void *key, uint32_t hash,
                       void *value, delfunc f)
{
	int retval;
	struct entry *e;

	assert(rcht != NULL);

	if ((e = malloc(sizeof(*e))) == NULL) {
		return 1;
	}

	RC_ini(&e->rc, value, f);
	e->marked_for_removal = false;

	/*
	 * Negative return values mean insertion error occurred
	 */
	if ((retval = CHashTable_insert(rcht->ht, key, hash, e)) < 0) {
		free(e);
	}

	return retval;
}

void *RCHashTable_remove(RCHashTable rcht, void *key, uint32_t hash)
{
	struct entry *e;
	void *value;

	assert(rcht != NULL);

	if ((e = CHashTable_lookup(rcht->ht, key, hash)) == NULL) {
		return NULL;
	}

	value = RC_getObj(&e->rc);
	/*
	 * Setting object's destructor to NULL will prevent object's destruction
	 * when it's removed from the table.
	 */
	RC_setObjFree(&e->rc, NULL);

	if (CHashTable_remove(rcht->ht, key, hash) != OK) {
		return NULL;
	}

	return value;
}

int RCHashTable_delete(RCHashTable rcht, void *key, uint32_t hash)
{
	int retval = -1;
	struct entry *e;

	assert(rcht != NULL);

	if ((e = CHashTable_lookup(rcht->ht, key, hash)) != NULL) {

		if (RC_getCount(&e->rc) == 1) {
			/*
			 * value will be freed when RC_free() is called
			 * in the hash table
			 */
			retval = CHashTable_remove(rcht->ht, key, hash);

		} else {
			retval = RC_refdec(&e->rc);
			e->marked_for_removal = true;
		}
	}

	return retval;
}


void *RCHashTable_refinc(RCHashTable rcht, void *key, uint32_t hash)
{
	void *retval = NULL;

	assert(rcht != NULL);

	struct entry *e;

	if ((e = CHashTable_lookup(rcht->ht, key, hash)) != NULL &&
	        e->marked_for_removal == false) {

		RC_refinc(&e->rc);

		retval = RC_getObj(&e->rc);
	}

	return retval;
}

int RCHashTable_refdec(RCHashTable rcht, void *key, uint32_t hash)
{
	RC tmp;
	int retval = -1;

	assert(rcht != NULL);

	if ((tmp = CHashTable_lookup(rcht->ht, key, hash)) != NULL) {

		if (RC_getCount(tmp) == 1) {
			/*
			 * value will be freed when RC_free() is called
			 * in the hash table
			 */
			retval = CHashTable_remove(rcht->ht, key, hash);

		} else {
			retval = RC_refdec(tmp);
		}
	}

	return retval;
}

void RCHashTable_free(RCHashTable rcht)
{
	assert(rcht != NULL);

	CHashTable_free(&rcht->ht);
	free(rcht);
}
