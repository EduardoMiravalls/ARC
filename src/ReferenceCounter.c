/**
 * @file RC.c
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-05 19:50
 *
 * @brief Reference Counter
 */


#include <stdlib.h>
#include <assert.h>

#include "types.h"

/*----------------------------------------------------------------------------
 * DEFINED CODES AND DATA TYPES
 *----------------------------------------------------------------------------*/

/**
 * @brief real RC definition.
 */
struct refcounter {
	void         *obj;     /**< object's reference */
	delfunc      obj_free; /**< object's destructor */
	unsigned int count;    /**< object's reference count */
};

/*----------------------------------------------------------------------------
 *                              PUBLIC FUNCTIONS
 *----------------------------------------------------------------------------*/

void RC_ini(struct refcounter *r, void *obj, delfunc obj_free)
{
	assert(r != NULL);

	r->obj = obj;
	r->obj_free = obj_free;
	r->count = 1;
}

struct refcounter *RC_new(void *obj, delfunc obj_free)
{
	struct refcounter *new_rc;

	if ((new_rc = malloc(sizeof(*new_rc))) == NULL) {
		return NULL;
	}

	RC_ini(new_rc, obj, obj_free);
	return new_rc;
}

void RC_freeObj(struct refcounter *r)
{
	assert(r != NULL);

	if (r->obj_free != NULL) {
		(*r->obj_free)(r->obj);
	}

	r->obj = NULL;
}

int RC_refinc(struct refcounter *r)
{
	assert(r != NULL);

	/*
	 * Can't increment the counter when it has already reached 0
	 * because the object has already been freed.
	 */
	if (r->count == 0) {
		return -1;
	}

	r->count++;
	return 0;
}

int RC_refdec(struct refcounter *r)
{
	assert(r != NULL);

	/*
	 * Check the count is not 0 or else it will underflow
	 */
	if (r->count == 0) {
		return 0;
	}

	r->count--;

	if (r->count == 0) {
		RC_freeObj(r);
		return 0;
	}

	return 1;
}

void *RC_getObj(struct refcounter *r)
{
	assert(r != NULL);

	return r->obj;
}

unsigned int RC_getCount(struct refcounter *r)
{
	assert(r != NULL);

	return r->count;
}

void RC_setObjFree(struct refcounter *r, delfunc obj_free)
{
	assert(r != NULL);

	r->obj_free = obj_free;
}

void RC_free(struct refcounter *r)
{
	assert(r != NULL);

	RC_freeObj(r);
	free(r);
}
