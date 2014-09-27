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
 * @file hashTable-chained.h
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2013-05-22 09:46
 *
 * @brief Hash Table
 */


#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "hashTable-chained.h"

/*----------------------------------------------------------------------------
 * DEFINED CODES AND DATA TYPES
 *----------------------------------------------------------------------------*/

struct bucket {
	void *key;                         /**< key */
	size_t hash;                       /**< key's hash */
	void *value;                       /**< value */
	struct bucket *next;               /**< next bucket in the linked list */
};

struct hashTable {
	size_t capacity;                   /**< number of buckets in the hashTable. It's a power of 2 */
	size_t bitmask;                    /**< minimum number of bits needed to represent capacity's value all set to 1's. */
	size_t nelems;                     /**< number of buckets used */
	struct bucket **buckets;           /**< hash table */
};

struct ChainedHashTable {
	cmpfunc kcmpf;                     /**< key comparing function */
	delfunc kfree;                     /**< key free function */
	delfunc vfree;                     /**< value free function */

	unsigned int maxRehashes;          /**< maximum number of rehashes performed on a single rehash operation */
	unsigned int rehashPoint;          /**< rehashing continues from this bucket */
	size_t minimumCapacity;            /**< table size won't be smaller than initial size */

	unsigned int maxLoadFactor;
	unsigned int minLoadFactor;

	unsigned int rehashCeilThreshold;  /**< calculated threshold using maximum load factor */
	unsigned int rehashFloorThreshold; /**< calculated threshold using minimum load factor */

	struct hashTable *table;           /**< here we have it! */
	struct hashTable *secondTable;     /**< rehashing can be done in several operations */
};

#define DEFAULT_MAX_REHASHES 5

#define DEFAULT_MIN_LOAD_FACTOR 10
#define DEFAULT_MAX_LOAD_FACTOR 75

/*----------------------------------------------------------------------------
 * PRIVATE FUNCTIONS PROTOTIPES
 *----------------------------------------------------------------------------*/
static struct hashTable *newTable(size_t maxcap);

static void flushTable(struct hashTable *table, delfunc kfree, delfunc vfree);

static void freeTable(struct hashTable *table, delfunc kfree, delfunc vfree);

static int checkRehashThresholds(struct ChainedHashTable *htable);

static void rehash(struct ChainedHashTable *htable);

static struct bucket *newBucket(void *key, void *value);

static void freeBucket(struct bucket *target, delfunc kfree, delfunc vfree);


static STATUS insert_private(struct hashTable *hTable,
                             cmpfunc kcmpf,
                             void *key,
                             size_t hashCode,
                             void *value);

static STATUS replace_private(struct hashTable *hTable,
                              cmpfunc kcmpf,
                              void *key,
                              size_t hash,
                              void *value);

static struct bucket *lookup_private(struct hashTable *table,
                                     cmpfunc kcmpf,
                                     void *key,
                                     size_t hashCode,
                                     struct bucket **previous);

static struct bucket *remove_private(struct bucket *previous,
                                     struct bucket **first);

static void updateThresholds(struct ChainedHashTable *htable);

static unsigned int round_up_to_the_next_highest_power_of_2(unsigned int num);
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 * PUBLIC FUNCTIONS
 *----------------------------------------------------------------------------*/

CHashTable CHashTable_new(size_t capacity, cmpfunc kcmp)
{
	struct ChainedHashTable *new;

	assert(kcmp != NULL);

	if ((new = malloc(sizeof(*new))) == NULL) {
		return NULL;
	}

	capacity = round_up_to_the_next_highest_power_of_2(capacity);

	if ((new->table = newTable(capacity)) == NULL) {
		free(new);
		return NULL;
	}

	new->kcmpf = kcmp;
	new->kfree = NULL;
	new->vfree = NULL;
	new->maxRehashes = DEFAULT_MAX_REHASHES;
	new->rehashPoint = 0;
	new->minimumCapacity = capacity;
	new->maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
	new->minLoadFactor = DEFAULT_MIN_LOAD_FACTOR;
	new->rehashCeilThreshold = capacity * DEFAULT_MAX_LOAD_FACTOR;
	new->rehashFloorThreshold = 0;
	new->secondTable = NULL;
	return new;
}

void CHashTable_setMaxRehashes(CHashTable hTable, unsigned int rehashes)
{
	assert(hTable != NULL);

	if (rehashes == 0) {
		hTable->maxLoadFactor = UINT_MAX;
		hTable->minLoadFactor = 0;
		updateThresholds(hTable);

	} else {
		hTable->maxRehashes = rehashes;
	}
}

void CHashTable_setMaxLoadFactor(CHashTable hTable, unsigned int percentage)
{
	assert(hTable != NULL);

	hTable->maxLoadFactor = percentage;
	updateThresholds(hTable);
}

void CHashTable_setMinLoadFactor(CHashTable hTable, unsigned int percentage)
{
	assert(hTable != NULL);

	hTable->minLoadFactor = percentage;
	updateThresholds(hTable);
}

double CHashTable_getLoadFactor(CHashTable hTable)
{
	assert(hTable != NULL);

	if (hTable->secondTable != NULL) {
		return ((hTable->secondTable->nelems + hTable->table->nelems) * 1.0)
		       / hTable->secondTable->capacity;
	}

	return (hTable->table->nelems * 1.0) / hTable->table->capacity;
}

void CHashTable_setKeyFree(CHashTable hTable, delfunc kfree)
{
	assert(hTable != NULL);

	hTable->kfree = kfree;
}

void CHashTable_setValueFree(CHashTable hTable, delfunc vfree)
{
	assert(hTable != NULL);

	hTable->vfree = vfree;
}

size_t CHashTable_getSize(CHashTable hTable)
{
	size_t nelems;

	assert(hTable != NULL);
	nelems = hTable->table->nelems;

	if (hTable->secondTable != NULL) {
		nelems += hTable->secondTable->nelems;
		rehash(hTable);
	}

	return nelems;
}

int CHashTable_insert(CHashTable hTable, void *key, size_t hashCode, void *value)
{
	int retvalue;

	assert(hTable != NULL);

	if (hTable->secondTable != NULL) {
		retvalue = insert_private(hTable->secondTable,
		                          hTable->kcmpf,
		                          key, hashCode, value);
		rehash(hTable);

	} else {
		if ((retvalue = insert_private(hTable->table, hTable->kcmpf, key, hashCode, value)) == OK) {
			retvalue = checkRehashThresholds(hTable);
		}
	}

	return retvalue;
}

STATUS CHashTable_replace(CHashTable hTable, void *key, size_t hashCode, void *value)
{
	STATUS retvalue;

	assert(hTable != NULL);

	retvalue = replace_private(hTable->table, hTable->kcmpf, key, hashCode, value);

	if (retvalue == OK) {
		if (hTable->secondTable != NULL) {
			rehash(hTable);
		}

	} else if (hTable->secondTable != NULL) {
		retvalue = replace_private(hTable->secondTable, hTable->kcmpf, key, hashCode, value);
		rehash(hTable);
	}

	return retvalue;
}

int CHashTable_remove(CHashTable hTable, void *key, size_t hashCode)
{
	int retval = -1;
	struct bucket *target, *parent;

	assert(hTable != NULL);

	target = lookup_private(hTable->table, hTable->kcmpf, key, hashCode, &parent);

	if (target != NULL) {
		remove_private(parent,
		               hTable->table->buckets + (target->hash & hTable->table->bitmask));
		freeBucket(target, hTable->kfree, hTable->vfree);
		hTable->table->nelems--;
		retval = 0;
	}

	if (hTable->secondTable != NULL) {
		if (target == NULL) {
			target = lookup_private(hTable->secondTable, hTable->kcmpf, key, hashCode, &parent);

			if (target != NULL) {
				remove_private(parent,
				               hTable->secondTable->buckets + (target->hash & hTable->secondTable->bitmask));
				freeBucket(target, hTable->kfree, hTable->vfree);
				hTable->secondTable->nelems--;
				retval = 0;
			}
		}

		rehash(hTable);

	} else {
		retval = checkRehashThresholds(hTable);
	}

	return retval;
}

void *CHashTable_lookup(CHashTable hTable, void *key, size_t hashCode)
{
	struct bucket *temp, *parent;

	assert(hTable != NULL);

	temp = lookup_private(hTable->table, hTable->kcmpf, key, hashCode, &parent);

	if (hTable->secondTable != NULL) {
		if (temp == NULL) {
			temp = lookup_private(hTable->secondTable, hTable->kcmpf, key, hashCode, &parent);
		}

		rehash(hTable);
	}

	if (temp == NULL) {
		return NULL;
	}

	return temp->value;
}

void CHashTable_flush(CHashTable hTable)
{
	assert(hTable != NULL);

	if (hTable->secondTable != NULL) {
		flushTable(hTable->secondTable, hTable->kfree, hTable->vfree);
		freeTable(hTable->table, hTable->kfree, hTable->vfree);
		hTable->table = hTable->secondTable;
		hTable->secondTable = NULL;

	} else {
		flushTable(hTable->table, hTable->kfree, hTable->vfree);
	}
}

void CHashTable_free(CHashTable *hTable)
{
	assert((*hTable) != NULL);

	freeTable((*hTable)->table, (*hTable)->kfree, (*hTable)->vfree);

	if ((*hTable)->secondTable != NULL) {
		freeTable((*hTable)->secondTable, (*hTable)->kfree, (*hTable)->vfree);
	}

	free((*hTable));
	*hTable = NULL;
}

/*----------------------------------------------------------------------------
 * PRIVATE FUNCTIONS
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Name: newTable
 * Description: allocates a new struct hashTable.
 *
 * Parameters:
 *    1. maxcap, number of buckets.
 *
 * Return:
 * pointer to a hashTable.
 * NULL if an error occurred.
----------------------------------------------------------------------------*/
static struct hashTable *newTable(size_t maxcap)
{
	struct hashTable *newTable;

	if ((newTable = malloc(sizeof(newTable[0]))) == NULL) {
		return NULL;

	} else if ((newTable->buckets = calloc(maxcap, sizeof(newTable->buckets[0]))) == NULL) {
		free(newTable);
		return NULL;
	}

	newTable->capacity = maxcap;
	newTable->nelems = 0;
	newTable->bitmask = maxcap - 1;
	return newTable;
}

/*----------------------------------------------------------------------------
 * Name: flushTable
 * Description: clear a table. All buckets will be freed.
 *
 * Parameters:
 *    1.table,  table to be freed.
 *    2. kfree, key free function.
 *    3. vfree, value free function.
----------------------------------------------------------------------------*/
static void flushTable(struct hashTable *table, delfunc kfree, delfunc vfree)
{
	register struct bucket *target;
	register unsigned int i;

	for (i = 0; i < table->capacity;) {
		if (table->buckets[i] == NULL) {
			i++;

		} else {
			target = remove_private(NULL, table->buckets + i);
			freeBucket(target, kfree, vfree);
		}
	}
}

/*----------------------------------------------------------------------------
 * Name: freeTable
 * Description: frees a struct hashTable.
 *
 * Parameters:
 *    1.table,  table to be freed.
 *    2. kfree, key free function.
 *    3. vfree, value free function.
----------------------------------------------------------------------------*/
static void freeTable(struct hashTable *table, delfunc kfree, delfunc vfree)
{
	flushTable(table, kfree, vfree);
	free(table->buckets);
	free(table);
}

/*----------------------------------------------------------------------------
 * Name: checkRehashThresholds
 * Description: check if the table needs to start a rehash operation. If that's
 *              the case, allocate a secondary table and starts rehashing.
 *
 * Parameters:
 *    1. hTable, hash table.
 *
 * Return:
 * 1 if table has to be rehashed but new table's allocation failed.
 * 0 otherwise.
----------------------------------------------------------------------------*/
static int checkRehashThresholds(struct ChainedHashTable *hTable)
{
	if (hTable->rehashCeilThreshold < hTable->table->nelems) {
		if ((hTable->secondTable = newTable(hTable->table->capacity * 2)) == NULL) {
			return 1;
		}

		updateThresholds(hTable);
		hTable->rehashPoint = 0;
		rehash(hTable);

	} else if (hTable->table->nelems < hTable->rehashFloorThreshold) {
		if ((hTable->secondTable = newTable(hTable->table->capacity / 2)) == NULL) {
			return 1;
		}

		updateThresholds(hTable);
		hTable->rehashPoint = 0;
		rehash(hTable);
	}

	return 0;
}

/*----------------------------------------------------------------------------
 * Name: rehash
 * Description: rehashes at most maxRehashes keys from the main table into the
 * secondary table.
 *
 * Parameters:
 *    1. hTable, hash table main structure.
----------------------------------------------------------------------------*/
static void rehash(struct ChainedHashTable *hTable)
{
	register struct bucket **curr;
	register struct bucket *temp, **insertionPoint;
	register unsigned int i, position;

	curr = hTable->table->buckets + hTable->rehashPoint;

	for (i = 0; i < hTable->maxRehashes; ++i) {
		if (*curr != NULL) {
			temp = remove_private(NULL, curr);
			position = temp->hash;
			position &= hTable->secondTable->bitmask;

			/* insertion at the beginning of the chain */
			insertionPoint = hTable->secondTable->buckets + position;
			temp->next = *insertionPoint;
			*insertionPoint = temp;

			hTable->secondTable->nelems++;
			hTable->table->nelems--;

		} else if (hTable->rehashPoint != (hTable->table->capacity - 1)) {
			hTable->rehashPoint++;
			curr++;

		} else {
			freeTable(hTable->table, NULL, NULL);
			hTable->table = hTable->secondTable;
			hTable->secondTable = NULL;
			break;
		}
	}
}

/*----------------------------------------------------------------------------
 * Name: newBucket
 * Description: allocates a new bucket.
 *
 * Parameters:
 *    1. key,       bucket's key.
 *    2. value,     bucket's value.
 *
 * Return:
 * pointer to the bucket.
 * NULL if and error occurred.
----------------------------------------------------------------------------*/
static struct bucket *newBucket(void *key, void *value)
{
	register struct bucket *new;

	if ((new = malloc(sizeof(*new))) == NULL) {
		return NULL;
	}

	new->key = key;
	new->next = NULL;
	new->value = value;
	return new;
}

/*----------------------------------------------------------------------------
 * Name: freeBucket
 * Description: deallocates a new bucket.
 *
 * Parameters:
 *    1. target,    pointer to bucket to be deallocated.
 *    2. kfree,     pointer to a key free function.         (Can be NULL)
 *    3. vfree,     pointer to a value free function.       (Can be NULL)
----------------------------------------------------------------------------*/
static void freeBucket(struct bucket *target, delfunc kfree, delfunc vfree)
{
	if (kfree != NULL) {
		(*kfree)(target->key);
	}

	if (vfree != NULL) {
		(*vfree)(target->value);
	}

	free(target);
}

/*----------------------------------------------------------------------------
 * Name: insert_private
 * Description: inserts a key-value if the key it's not already present in the table.
 *
 * Parameters:
 *    1. hTable,    hash table to be modified.
 *    2. kcmpf,     key comparing function.
 *    3. key,       key to look for in retrieval operation.
 *    4. hash,      key's hash.
 *    5. value,     key's associated value.
 *
 * Return:
 * OK if success.
 * ERR if key is already present, or bucket's allocation failed.
----------------------------------------------------------------------------*/
static STATUS insert_private(struct hashTable *hTable,
                             cmpfunc kcmpf,
                             void *key,
                             size_t hash,
                             void *value)
{
	register struct bucket **temp;
	unsigned int position;

	position = hash & hTable->bitmask;
	temp = hTable->buckets + position;

	while (*temp != NULL) {
		if ((*kcmpf)(key, (*temp)->key) == 0) {
			return ERR;

		} else {
			temp = &((*temp)->next);
		}
	}

	/*
	 * Key not present: insert it
	 */
	if ((*temp = newBucket(key, value)) == NULL) {
		return ERR;
	}

	(*temp)->hash = hash;
	hTable->nelems++;
	return OK;
}

/*----------------------------------------------------------------------------
 * Name: replace_private
 * Description: inserts a key-value in the table. If the key is found in the table,
 *              replaces it's value.
 *
 * Parameters:
 *    1. hTable,    hash table to be modified.
 *    2. kcmpf,     key comparing function.
 *    3. key,       key to look for in retrieval operation.
 *    4. hash,      key's hash.
 *    5. value,     key's associated value.
 *
 * Return:
 * OK if success.
 * ERR if bucket's allocation failed.
----------------------------------------------------------------------------*/
static STATUS replace_private(struct hashTable *hTable,
                              cmpfunc kcmpf,
                              void *key,
                              size_t hash,
                              void *value)
{
	struct bucket **temp;
	unsigned int position;

	assert(hTable != NULL);

	position = hash & hTable->bitmask;
	temp = hTable->buckets + position;

	while (*temp != NULL) {
		if ((*kcmpf)(key, (*temp)->key) == 0) {
			(*temp)->value = value;
			return OK;

		} else {
			temp = &((*temp)->next);
		}
	}

	/*
	 * Key not present: insert it
	 */
	if ((*temp = newBucket(key, value)) == NULL) {
		return ERR;
	}

	(*temp)->hash = hash;
	hTable->nelems++;
	return OK;
}

/*----------------------------------------------------------------------------
 * Name: lookup_private
 *
 * Description: looks for the key in the table.
 * Stores the key's bucket's previous node in the (*previous)
 * parameter pointer.
 *
 * Parameters:
 *    1. table,     table to be searched.
 *    2. kcmpf,     key comparing function.
 *    3. key,       key to be searched.
 *    4. hash,      key's hash.
 *    5. previous,  to store the previous bucket to the key's bucket.
 *
 * Return:
 * pointer to the bucket which contains the key.
 * NULL if it couldn't find it.
----------------------------------------------------------------------------*/
static struct bucket *lookup_private(struct hashTable *table,
                                     cmpfunc kcmpf,
                                     void *key,
                                     size_t hash,
                                     struct bucket **previous)
{
	register struct bucket *current;

	hash &= table->bitmask;
	current = table->buckets[hash];
	*previous = NULL;

	while (current != NULL) {
		if ((*kcmpf)(key, current->key) == 0) {
			return current;

		} else {
			*previous = current;
			current = current->next;
		}
	}

	return NULL;
}

/*----------------------------------------------------------------------------
 * Name: remove_private
 * Description: removes a bucket from the chain.
 *
 * Parameters:
 *    1. previous,  previous bucket to target.
 *    2. first,     pointer to pointer to first bucket of the chain.
 *                  (*first) can't be NULL.
 * Return:
 * pointer to target bucket.
----------------------------------------------------------------------------*/
static struct bucket *remove_private(struct bucket *previous,
                                     struct bucket **first)
{
	struct bucket *target;

	if (previous == NULL) { /* target is (*first) */
		target = *first;
		*first = (*first)->next;

	} else {
		target = previous->next;
		previous->next = target->next;
	}

	return target;
}

/*----------------------------------------------------------------------------
 * Name: updateThresholds
 * Description: updates table's rehash thresholds.
 *
 * Parameters:
 *    1. htable,    hash table.
----------------------------------------------------------------------------*/
static void updateThresholds(struct ChainedHashTable *htable)
{
	size_t cap;

	if (htable->secondTable != NULL) {
		cap = htable->secondTable->capacity;

	} else {
		cap = htable->table->capacity;
	}

	htable->rehashCeilThreshold = (cap * htable->maxLoadFactor) / 100;

	if (cap != htable->minimumCapacity) {
		htable->rehashFloorThreshold = (cap * htable->minLoadFactor) / 100;

	} else {
		htable->rehashFloorThreshold = 0;
	}
}

/* from http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup */
static unsigned int round_up_to_the_next_highest_power_of_2(unsigned int num)
{
	num--;
	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;
	num++;
	return num += (num == 0);
}
