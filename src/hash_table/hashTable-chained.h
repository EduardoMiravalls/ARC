/**
 * @file hashTable-chained.h
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2013-05-22 09:46
 *
 * @brief Hash Table
 */

#ifndef _HASHTABLE_CHAINED_H_
#define _HASHTABLE_CHAINED_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "types.h"

/*----------------------------------------------------------------------------
 * DEFINED CODES AND DATA TYPES
 *----------------------------------------------------------------------------*/

/** \addtogroup CHashTable Chained Hash Table
 * @brief
 * Hash table implementation which resolves collisions with chaining.
 * @{
 */

/**
 * @brief Chained Hash Table type
 */
typedef struct ChainedHashTable *CHashTable;


/**
 * @brief allocates a new hash table which resolves collisions by chaining.
 *
 * @param capacity table's initial capacity.
 * @param kcmp key comparator.
 *
 * @return a ChashTable.
 * @return NULL if an error occurred.
 */
CHashTable CHashTable_new(size_t capacity, cmpfunc kcmp);

/*----------------------------------------------------------------------------
 * GETTERS & SETTERS
 *----------------------------------------------------------------------------*/

/**
 * @details sets the maximum number of elements which will be rehashed on a
 *          single rehash operation. Remaining elements will be rehashed
 *          after future insertions, deletions, and lookups.
 *
 *          If set to 0, no rehash operation will be made.
 *
 *          It's equivalent to setting the maximum load factor to UINT_MAX and
 *          minimum load factor to 0.
 *
 * @param hTable hash table.
 * @param rehashes new limit.
 */
void CHashTable_setMaxRehashes(CHashTable hTable, uint rehashes);

/**
 * @brief sets upper rehash threshold.
 * @details when the load factor is greater than this threshold, the table will
 *          grow and rehash all its keys.
 *
 * @param hTable hash table.
 * @param percentage new threshold.
 */
void CHashTable_setMaxLoadFactor(CHashTable hTable, uint percentage);

/**
 * @brief sets lower rehash threshold.
 * @details when the load factor is less than this threshold, the table will grow
 *          and rehash all its keys.
 *
 * @param hTable hash table.
 * @param percentage new threshold.
 */
void CHashTable_setMinLoadFactor(CHashTable hTable, uint percentage);

/**
 * @brief returns the percentage of buckets which are not empty.
 *
 * @param hTable hash table.
 *
 * @return percentage.
 */
double CHashTable_getLoadFactor(CHashTable hTable);

/**
 * @brief sets the function to apply to keys when they are being removed.
 * @details if set to NULL (default), no operation will be performed on them.
 *
 * @param hTable hash table.
 * @param kfree pointer to a key's free function.
 */
void CHashTable_setKeyFree(CHashTable hTable, delfunc kfree);

/**
 * @brief sets the function to apply to values when they are being removed.
 * @details if set to NULL (default), no operation will be performed on them.
 *
 * @param hTable hash table.
 * @param vfree pointer to value's free function.
 */
void CHashTable_setValueFree(CHashTable hTable, delfunc vfree);

/**
 * @brief returns the number of key-value pairs present in the table.
 *
 * @param hTable hash table.
 *
 * @return number of key-value pairs.
 */
size_t CHashTable_getSize(CHashTable hTable);

/*----------------------------------------------------------------------------
 * OPERATIONS
 *----------------------------------------------------------------------------*/

/**
 * @brief inserts a key in the table.
 * @details if the key is present, insertion will fail.
 *
 * @param hTable hash table.
 * @param key key.
 * @param hashCode key's hash.
 * @param value key's associated value.
 *
 * @return   0 if insertion succeeded.
 * @return < 0 if the key is already present in the table.
 * @return > 0 if insertion succeeded but the table failed to start a rehash operation.
 */
int CHashTable_insert(CHashTable hTable, void *key, size_t hashCode, void *value);

/**
 * @brief inserts a key in the table.
 * @details if the key is already present, it replaces it's value.
 *
 * @param hTable hash table.
 * @param key key.
 * @param hashCode key's hash.
 * @param value key's associated value.
 *
 * @return OK if insertion succeeded.
 * @return ERR if an internal allocation failed.
 */
STATUS CHashTable_replace(CHashTable hTable, void *key, size_t hashCode, void *value);

/**
 * @brief looks for the key and removes it from the table.
 *
 * @param hTable hash table.
 * @param key key to look for.
 * @param hashCode key's hash.
 *
 * @return   0 if deletion succeeded.
 * @return < 0 if the key wasn't found in the table.
 * @return > 0 if deletion succeeded but the table failed to start a rehash operation.
 */
int CHashTable_remove(CHashTable hTable, void *key, size_t hashCode);

/**
 * @brief looks for a key in the table, returning it's value.
 *
 * @param hTable hash table.
 * @param key key to look for.
 * @param hashCode key's hash.
 *
 * @return NULL if key wasn't found.
 * @return key's value otherwise.
 */
void *CHashTable_lookup(CHashTable hTable, void *key, size_t hashCode);

/**
 * @brief empties a table.
 *
 * @param hTable hash table.
 */
void CHashTable_flush(CHashTable hTable);

/*----------------------------------------------------------------------------
 * DESTRUCTOR
 *----------------------------------------------------------------------------*/

/**
 * @brief frees the hash table and all its associated resources.
 * @details sets the pointer to NULL.
 *
 * @param hTable table to be freed.
 */
void CHashTable_free(CHashTable *hTable);

/**
 * @}
 */

#ifdef  __cplusplus
}
#endif

#endif /* _HASHTABLE_CHAINED_H_ */

