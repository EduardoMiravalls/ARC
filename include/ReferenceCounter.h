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
 * @file RC.h
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-05 19:50
 */


#ifndef _REFERENCE_COUNTER_H_
#define _REFERENCE_COUNTER_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "types.h"

/** \addtogroup RC ReferenceCounter
 *
 * @brief
 * Original idea and interface credits go to:
 * http://stoneship.org/essays/c-reference-counting-and-you/
 *
 * @details
 * This is just my implementation of that solution.
 *
 * Keep in mind this is NOT a threadsafe reference counter.
 *
 * If the reference count is embedded in the object, in order to check or modify
 * the reference count you have to access the object, causing a SIGSEGV if the
 * object has already been freed.
 *
 * This solution only displaces the problem to when to free the reference counter.
 * @{
 */

/*
 * Dummy reference counter definition.
 * This definition is provided so the RC can be embedded in
 * another structure or allocated on the stack.
 */
/** @cond HIDDEN_SYMBOL */
struct refcounter {
	void *dummy1;
	void *dummy2;
	int   dummy3;
};
/** @endcond*/

typedef struct refcounter *RC;

/**
 * @brief initialize a RC. The the reference count will be initialized to 1.
 *
 * @param r RC
 * @param obj object's reference.
 * @param obj_free object's destructor. As free(), should handle a NULL pointer.
 *                 Can be NULL.
 */
void RC_ini(RC r, void *obj, delfunc obj_free);

/**
 * @brief allocates a new RC.
 *
 * @param obj object to be reference counted.
 * @param obj_free funtion to apply to the object when the reference count reaches 0.
 *                 If it's NULL, nothing will be done.
 *
 * @return a RC with the reference count initialized to 1.
 * @return NULL if allocation failed.
 */
RC RC_new(void *obj, delfunc obj_free);

/**
 * @brief increments object's reference count.
 * @details if reference count has reached 0, returns immediately.
 *
 * @param r RC.
 *
 * @return 0 if operation succeeded.
 * @return != 0 if reference count is 0.
 */
int RC_refinc(RC r);

/**
 * @brief decrements object's reference count.
 * @details if reference count reaches 0, the object is freed with obj_free().
 *
 * @param r RC.
 *
 * @return 0 if the reference count reached 0.
 * @return != 0 if the reference count is still greater than 0.
 */
int RC_refdec(RC r);

/**
 * @param r RC.
 *
 * @return the object.
 */
void *RC_getObj(RC r);

/**
 * @param r RC.
 *
 * @return current reference count.
 */
unsigned int RC_getCount(RC r);

/**
 * @brief set object's free function to apply when reference count reaches 0.
 * 
 * @param r RC.
 * @param obj_free new object's destructor.
 */
void RC_setObjFree(RC r, delfunc obj_free);

/**
 * @brief forces object destruction no matter what reference count it has.
 * @details sets object's reference to NULL afterwards to prevent future accesses
 *          to invalid memory.
 *
 * @param r RC.
 */
void RC_freeObj(RC r);

/**
 * @brief calls RC_freeObj(), then frees the RC.
 *
 * @param r RC.
 */
void RC_free(RC r);

/**
 * @}
 */

#ifdef  __cplusplus
}
#endif

#endif /* _REFERENCE_COUNTER_H_ */

