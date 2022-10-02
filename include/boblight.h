/*
 * boblight.h
 *
 *  Created on: Apr 4, 2015
 *  Imported 2022-08-22
 *      Author: c3ma
 */

#ifndef BOBLIGHT_H_
#define BOBLIGHT_H_

#define BOBLIGHT_VERSION 	"1.1"

void boblight_init(void);

/**
 *@fn int boblight_loop(void)
 */
int boblight_loop(void);

/** 
 * Receive statistic information
 */
long getCountRecevied(void);
long getCountDuplicate(void);
long getCountValid(void);
long getCountSuperBright(void);

#endif /* BOBLIGHT_H_ */
