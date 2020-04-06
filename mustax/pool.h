#ifndef __POOL_H__
#define __POOL_H__

/**
 * Generic pool structure for specified element type.
 * Meant to be used _inside_ struct braces.
 */

#define POOL_STRUCT(el_type)    \
                el_type *ptr;   \
                el_type *last;  \
                el_type *limit

#endif
