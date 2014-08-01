/*
 *  Automatic Reference Counting Hash Table library.
 *  Copyright (C) 2014  Eduardo Miravalls Sierra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

struct record {
	struct refcounter rc;    /**< the reference counting structure */
	bool marked_for_removal; /**< if set to true, this record should be ignored in future RCHashTable_refinc() */
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
	CHashTable_setValueFree(rcht->ht, (delfunc)RC_free);

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
	struct record *r;

	assert(rcht != NULL);

	if ((r = malloc(sizeof(*r))) == NULL) {
		return 1;
	}

	RC_ini(&r->rc, value, f);
	r->marked_for_removal = false;

	/*
	 * Negative return values mean insertion error occurred
	 */
	if ((retval = CHashTable_insert(rcht->ht, key, hash, r)) < 0) {
		free(r);
	}

	return retval;
}

void *RCHashTable_remove(RCHashTable rcht, void *key, uint32_t hash)
{
	struct record *r;
	void *value;

	assert(rcht != NULL);

	if ((r = CHashTable_lookup(rcht->ht, key, hash)) == NULL) {
		return NULL;
	}

	value = RC_getObj(&r->rc);
	/*
	 * Setting object's destructor to NULL will prevent object's destruction
	 * when it's removed from the table.
	 */
	RC_setObjFree(&r->rc, NULL);

	if (CHashTable_remove(rcht->ht, key, hash) < 0) {
		return NULL;
	}

	return value;
}

int RCHashTable_delete(RCHashTable rcht, void *key, uint32_t hash)
{
	int retval = -1;
	struct record *r;

	assert(rcht != NULL);

	if ((r = CHashTable_lookup(rcht->ht, key, hash)) != NULL) {

		retval = RC_refdec(&r->rc);

		if (retval == 0) {  /* obj was freed */
			retval = CHashTable_remove(rcht->ht, key, hash);

		} else {
			r->marked_for_removal = true;
		}
	}

	return retval;
}


void *RCHashTable_refinc(RCHashTable rcht, void *key, uint32_t hash)
{
	void *retval = NULL;

	assert(rcht != NULL);

	struct record *r;

	if ((r = CHashTable_lookup(rcht->ht, key, hash)) != NULL &&
	        r->marked_for_removal == false) {

		RC_refinc(&r->rc);
		retval = RC_getObj(&r->rc);
	}

	return retval;
}

int RCHashTable_refdec(RCHashTable rcht, void *key, uint32_t hash)
{
	struct record *r;
	int retval = -1;

	assert(rcht != NULL);

	if ((r = CHashTable_lookup(rcht->ht, key, hash)) != NULL) {

		retval = RC_refdec(&r->rc);

		if (retval == 0) {  /* obj was freed */
			retval = CHashTable_remove(rcht->ht, key, hash);
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
