/*
 * bits.h
 *
 *  Created on: Jun 3, 2024
 *      Author: abolinaga
 */

#ifndef BITS_H_
#define BITS_H_

#define BITS_PER_LONG     32

#define BIT(nr)			(1UL << (nr))
#define GENMASK(h, l) \
	    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#endif /* BITS_H_ */
