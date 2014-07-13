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
 * @file RCHashTable_sync.c
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-11 23:05
 */


#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

#include "hash_table/hashTable-chained.h"
#include "ReferenceCounter.h"
#include "RCHashTable.h"
#include "RCHashTable_sync.h"

/*----------------------------------------------------------------------------
 * DEFINED CODES AND DATA TYPES
 *----------------------------------------------------------------------------*/

struct refcounting_hash_table_sync {
	CHashTable ht;
	pthread_mutex_t mutex;
};

/*----------------------------------------------------------------------------
 * PUBLIC FUNCTIONS
 *----------------------------------------------------------------------------*/

RCHashTable_sync RCHashTable_sync_new(uint32_t inicap, cmpfunc key_cmp,
                                      delfunc key_free)
{
	RCHashTable_sync rcht_sync;

	if ((rcht_sync = malloc(sizeof(*rcht_sync))) == NULL) {
		return NULL;
	}

	if ((rcht_sync->ht = CHashTable_new(inicap, key_cmp)) == NULL) {
		free(rcht_sync);
		return NULL;
	}

	CHashTable_setKeyFree(rcht_sync->ht, key_free);
	CHashTable_setValueFree(rcht_sync->ht, (void (*)(void *))RC_free);

	if ((pthread_mutex_init(&(rcht_sync->mutex), NULL))) {
		RCHashTable_free((RCHashTable)rcht_sync);
		return NULL;
	}

	return rcht_sync;
}

void RCHashTable_sync_setKeyFree(RCHashTable_sync rcht, delfunc key_free)
{
	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		CHashTable_setKeyFree(rcht->ht, key_free);
	}
	pthread_mutex_unlock(&(rcht->mutex));
}

void RCHashTable_sync_setMaxLoadFactor(RCHashTable_sync rcht, unsigned int percentage)
{
	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		CHashTable_setMaxLoadFactor(rcht->ht, percentage);
	}
	pthread_mutex_unlock(&(rcht->mutex));
}

void RCHashTable_sync_setMinLoadFactor(RCHashTable_sync rcht, unsigned int percentage)
{
	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		CHashTable_setMinLoadFactor(rcht->ht, percentage);
	}
	pthread_mutex_unlock(&(rcht->mutex));
}

void RCHashTable_sync_setMaxRehashes(RCHashTable_sync rcht, unsigned int maxrehashes)
{
	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		CHashTable_setMaxRehashes(rcht->ht, maxrehashes);
	}
	pthread_mutex_unlock(&(rcht->mutex));
}

int RCHashTable_insert_sync(RCHashTable_sync rcht,
                            void *key, uint32_t hash,
                            void *value, delfunc f)
{
	int retval;

	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		retval = RCHashTable_insert((RCHashTable)rcht, key, hash, value, f);
	}
	pthread_mutex_unlock(&(rcht->mutex));

	return retval;
}

void *RCHashTable_remove_sync(RCHashTable_sync rcht, void *key, uint32_t hash)
{
	void *retval;

	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		retval = RCHashTable_remove((RCHashTable)rcht, key, hash);
	}
	pthread_mutex_unlock(&(rcht->mutex));

	return retval;
}

int RCHashTable_delete_sync(RCHashTable_sync rcht, void *key, uint32_t hash)
{
	int retval;

	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		retval = RCHashTable_delete((RCHashTable)rcht, key, hash);
	}
	pthread_mutex_unlock(&(rcht->mutex));

	return retval;
}

void *RCHashTable_refinc_sync(RCHashTable_sync rcht, void *key, uint32_t hash)
{
	void *retval;

	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		retval = RCHashTable_refinc((RCHashTable)rcht, key, hash);
	}
	pthread_mutex_unlock(&(rcht->mutex));

	return retval;
}

int RCHashTable_refdec_sync(RCHashTable_sync rcht, void *key, uint32_t hash)
{
	int retval;

	assert(rcht != NULL);

	pthread_mutex_lock(&(rcht->mutex));
	{
		retval = RCHashTable_refdec((RCHashTable)rcht, key, hash);
	}
	pthread_mutex_unlock(&(rcht->mutex));

	return retval;
}

void RCHashTable_free_sync(RCHashTable_sync rcht)
{
	assert(rcht != NULL);

	pthread_mutex_destroy(&(rcht->mutex));
	RCHashTable_free((RCHashTable)rcht);
}

