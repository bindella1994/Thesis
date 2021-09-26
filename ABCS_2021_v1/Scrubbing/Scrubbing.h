/*
 * scrubbing.h
 *
 *  Created on: 22 ago 2021
 *      Author: bindella
 */

#ifndef SCRUBBING_H_
#define SCRUBBING_H_
#include "abacus.h"
#include "satellite/configuration.h"
#include "PersistentRam/PersistentRam.h"


uint8_t scrubbingRoutine();


uint8_t periodicScrubPersistentRAM();


#endif /* SCRUBBING_H_ */
