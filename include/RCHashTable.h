/**
 * @file RCHashTable.h
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-08 00:25
 */

#ifndef _REFERENCE_COUNTING_HASH_TABLE_
#define _REFERENCE_COUNTING_HASH_TABLE_

#ifdef  __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include "types.h"

/** \addtogroup RCHT RCHashTable
 * @brief
 * Reference counting hash table.
 * @{
 */

/** \typedef
 * @brief opaque data type.
 */
typedef struct refcounting_hash_table *RCHashTable;

/**
 * @brief allocates a new RCHashTable.
 *
 * @param inicap initial capacity. It'll grow automatically as needed.
 * @param key_cmp function to compare keys.
 * @param key_free function to free keys. If set to NULL, no key will be freed
 *                 on deletions.
 *
 * @return NULL if allocation failed.
 * @return a table otherwise.
 */
RCHashTable RCHashTable_new(uint32_t inicap, cmpfunc key_cmp, delfunc key_free);

/**
 * @brief sets the function to apply to keys when they are being removed.
 * @details if set to NULL, no operation will be performed on them.
 * 
 * @param rcht table.
 * @param key_free key's destructor.
 */
void RCHashTable_setKeyFree(RCHashTable rcht, delfunc key_free);

/**
 * @see CHashTable_setMaxLoadFactor()
 * 
 * @param rcht table.
 * @param percentage new bound.
 */
void RCHashTable_setMaxLoadFactor(RCHashTable rcht, unsigned int percentage);

/**
 * @see CHashTable_setMinLoadFactor()
 * 
 * @param rcht table.
 * @param percentage new bound.
 */
void RCHashTable_setMinLoadFactor(RCHashTable rcht, unsigned int percentage);

/**
 * @see CHashTable_setMaxRehashes()
 * 
 * @param rcht table.
 * @param maxRehashes new limit.
 */
void RCHashTable_setMaxRehashes(RCHashTable rcht, unsigned int maxrehashes);

/**
 * @brief inserts a new key-value pair that will be reference counted.
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
int RCHashTable_insert(RCHashTable rcht,
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
void *RCHashTable_remove(RCHashTable rcht, void *key, uint32_t hash);

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
int RCHashTable_delete(RCHashTable rcht, void *key, uint32_t hash);

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
void *RCHashTable_refinc(RCHashTable rcht, void *key, uint32_t hash);

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
int RCHashTable_refdec(RCHashTable rcht, void *key, uint32_t hash);

/**
 * @brief frees a RCHashTable. All key-value pairs will be freed.
 *
 * @param rcht table.
 */
void RCHashTable_free(RCHashTable rcht);

/**
 * @}
 */

#ifdef  __cplusplus
}
#endif

#endif /* _REFERENCE_COUNTING_HASH_TABLE_ */

