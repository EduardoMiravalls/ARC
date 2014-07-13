/*
 * File:    testRC.c
 * Author:  Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * Date:    2014-07-11 21:11
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "ReferenceCounter.h"

/*
 * Check that object gets freed.
 */
void test1(void)
{
	RC r;
	char *str = strdup("Hello World!");

	printf("%s: ", __func__);

	r = RC_new(str, free);

	assert(RC_getObj(r) != NULL && RC_getCount(r) == 1);
	assert(RC_refdec(r) == 0);
	assert(RC_getObj(r) == NULL && RC_getCount(r) == 0);

	RC_free(r);
	printf("success!\n");
}

/*
 * check if obj gets freed only after second refdec
 */
void test2(void)
{
	RC r;
	char *str = strdup("Hello World!");

	printf("%s: ", __func__);

	r = RC_new(str, free);

	assert(RC_getObj(r) != NULL);
	assert(RC_refinc(r) == 0);
	assert(RC_getObj(r) != NULL);
	assert(RC_refdec(r) != 0);
	assert(RC_getObj(r) != NULL);
	assert(RC_refdec(r) == 0);
	assert(RC_getObj(r) == NULL);

	RC_free(r);
	printf("success!\n");
}

/*
 * Testing double refdec doesn't crash.
 */
void test3(void)
{
	RC r;
	char *str = strdup("Hello World!");

	printf("%s: ", __func__);

	r = RC_new(str, free);

	assert(RC_getObj(r) != NULL && RC_getCount(r) == 1);
	assert(RC_refdec(r) == 0);
	assert(RC_getObj(r) == NULL && RC_getCount(r) == 0);
	assert(RC_refdec(r) == 0);
	assert(RC_getObj(r) == NULL && RC_getCount(r) == 0);

	RC_free(r);
	printf("success!\n");
}

/*
 * Testing RC_free() frees the object no matter what the reference count is.
 */
void test4(void)
{
	RC r;
	char *str = strdup("Hello World!");

	printf("%s: ", __func__);

	r = RC_new(str, free);

	assert(RC_getObj(r) != NULL && RC_getCount(r) == 1);
	assert(RC_refinc(r) == 0);
	assert(RC_refinc(r) == 0);

	RC_free(r);
	printf("success!\n");
}

/*
 * Testing RC_freeObj() frees the object no matter what the reference count is.
 */
void test5(void)
{
	struct refcounter r;
	char *str = strdup("Hello World!");

	printf("%s: ", __func__);

	RC_ini(&r, str, free);

	assert(RC_getObj(&r) != NULL && RC_getCount(&r) == 1);
	assert(RC_refinc(&r) == 0);
	assert(RC_refinc(&r) == 0);

	RC_freeObj(&r);
	printf("success!\n");
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	printf("All tests passed!\n");
	return 0;
}
