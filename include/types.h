/**
 * @file types.h
 *
 * @author Eduardo Miravalls Sierra          <edu.miravalls@hotmail.es>
 *
 * @date 2014-03-11 00:13
 */


#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef HAVE_CMPFUNC
#define HAVE_CMPFUNC
/**
 * @brief pointer to comparator function.
 *
 * @param e1 element.
 * @param e2 element.
 *
 * @return < 0 if e1 < e2
 * @return   0 if e1 == e2
 * @return > 0 if e1 > e2
 */
typedef int  (*cmpfunc) (const void *, const void *);
#endif

#ifndef HAVE_DELFUNC
#define HAVE_DELFUNC
/**
 * @brief pointer to delete function.
 * @details custom free function.
 *
 * @param v value to be freed.
 */
typedef void (*delfunc) (void *);
#endif

#ifndef HAVE_STATUS
#define HAVE_STATUS
typedef enum {OK = 0, ERR = -1} STATUS;
#endif

#ifdef  __cplusplus
}
#endif

#endif /* _TYPES_H_ */

