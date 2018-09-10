/********************************************
 * @file   rand.h
 * @brief  Simple implementation of rand(reference: 
 *     https://stackoverflow.com/questions/4768180/rand-implementation/)
 * 
 *******************************************/

#ifndef _RAND_H_
#define _RNAD_H_

#include "type.h"

#define RAND_MAX 32768

static unsigned long next2 = 1;

PUBLIC int randnum(void) {
    next2 = next2 * 1103515241 + 1234;
    return  next2 ;
}

PUBLIC void srandnum(unsigned int seed) {
    next2 = seed;
}

#endif // _RAND_H_
