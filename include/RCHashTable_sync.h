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
 * @file RCHashTable_sync.h
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-11 23:05
 */


#ifndef _REFERENCE_COUNTING_HASH_TABLE_SYNC_
#define _REFERENCE_COUNTING_HASH_TABLE_SYNC_

#ifdef  __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include "types.h"

/** \addtogroup SRCHT Syncronized RCHashTable
 * @brief
 * Syncronized reference counting hash table.
 * @details
 * All operations EXCEPT _free are Thread Safe.
 * @{
 */

/** \typedef
 * @brief opaque type.
 */
typedef struct refcounting_hash_table_sync *RCHashTable_sync;

/**
 * @brief allocates a new RCHashTable_sync.
 *
 * @param inicap initial capacity. It'll grow automatically as needed.
 * @param key_cmp function to compare keys.
 * @param key_free function to free keys. If NULL, no key will be freed on deletions.
 *
 * @return NULL if allocation failed.
 * @return RCHashTable_sync.
 */
RCHashTable_sync RCHashTable_sync_new(uint32_t inicap, cmpfunc key_cmp,
                                      delfunc key_free);


/**
 * @brief sets the function to apply to keys when they are being removed.
 * @details if set to NULL, no operation will be performed on them.
 * 
 * @param rcht table.
 * @param key_free key's destructor.
 */
void RCHashTable_sync_setKeyFree(RCHashTable_sync rcht, delfunc key_free);

/**
 * @see CHashTable_setMaxLoadFactor()
 * 
 * @param rcht table.
 * @param percentage new bound.
 */
void RCHashTable_sync_setMaxLoadFactor(RCHashTable_sync rcht, unsigned int percentage);

/**
 * @see CHashTable_setMinLoadFactor()
 * 
 * @param rcht table.
 * @param percentage new bound.
 */
void RCHashTable_sync_setMinLoadFactor(RCHashTable_sync rcht, unsigned int percentage);

/**
 * @see CHashTable_setMaxRehashes()
 * 
 * @param rcht table.
 * @param maxRehashes new limit.
 */
void RCHashTable_sync_setMaxRehashes(RCHashTable_sync rcht, unsigned int maxrehashes);

/**
 * @brief inserts a new key-value pair that will reference counted.
 * @details reference count starts at 1.
 *
 * @param rcht table.
 * @param key key.
 * @param hash key's hash.
 * @param value value.
 * @param f value's destructor. Can be NULL.
 *
 * @return   0 if insertion succeeded.
 * @return < 0 if insertion failed because key is already present in the table.
 * @return > 0 if insertion succeeded but the table failed to start a rehash operation.
 */
int RCHashTable_insert_sync(RCHashTable_sync rcht,
                            void *key, uint32_t hash,
                            void *value, delfunc f);

/**
 * @brief removes a value from the table and returns it.
 * @details allows to remove a value from the table without destroying it.
 *
 * @param rcht table.
 * @param key object's key.
 * @param hash key's hash.
 *
 * @return NULL if the key wasn't found or couldn't be removed.
 * @return the object otherwise.
 */
void *RCHashTable_remove_sync(RCHashTable_sync rcht, void *key, uint32_t hash);

/**
 * @brief marks for deletion a key-value pair and decrements its reference count by 1.
 * @details future RCHashTable_refinc()'s will fail, and only RCHashTable_refdec()'s
 *          will succeed. When reference count reaches 0, the marked key-value pair
 *          will be freed.
 *
 * @param rcht table.
 * @param key key.
 * @param hash key's hash.
 *
 * @return   0 if deletion succeeded.
 * @return < 0 if deletion failed because key wasn't found in the table.
 * @return > 0 if deletion succeeded but the table failed to start a rehash operation.
 */
int RCHashTable_delete_sync(RCHashTable_sync rcht, void *key, uint32_t hash);

/**
 * @brief increments by 1 the reference count of a value and returns it.
 *
 * @param rcht table.
 * @param key key.
 * @param hash key's hash.
 *
 * @return NULL if object isn't available (either it wasn't found or it's been marked for removal).
 * @return the value otherwise.
 */
void *RCHashTable_refinc_sync(RCHashTable_sync rcht, void *key, uint32_t hash);

/**
 * @brief decrements by 1 the reference count of a value.
 *
 * @param rcht table.
 * @param key key.
 * @param hash key's hash.
 *
 * @return   0 if object's reference count reached 0.
 * @return < 0 if the object wasn't found in the table.
 * @return > 0 if deletion succeeded but the table failed to start a rehash operation.
 */
int RCHashTable_refdec_sync(RCHashTable_sync rcht, void *key, uint32_t hash);

/**
 * @brief frees a RCHashTable. All key-value pairs will be freed.
 *
 * @details this function is not thread safe. That's because any thread can be blocked
 * in the mutex after it's blocked, and what happens after the mutex is destroyed is undefined.
 *
 * Use this function when all threads have finished using it.
 *
 * @param rcht table.
 */
void RCHashTable_free_sync(RCHashTable_sync rcht);

/**
 * @}
 */

#ifdef  __cplusplus
}
#endif

#endif /* _REFERENCE_COUNTING_HASH_TABLE_SYNC_ */

