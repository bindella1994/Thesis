/*
 * abcs_status.h
 *
 *  Created on: 14 gen 2021
 *      Author: Augusto Nascetti
 */

#ifndef ABCS_STATUS_H_
#define ABCS_STATUS_H_

#include "configuration.h"

#define NORMALMODE          0x00
#define EXPERIMENTMODE      0x01
#define SAFEMODE            0x02

#define EXECUTED            0xEE
#define REQUESTED           0x11

#define ENTERINGSTATE           0
#define LOWBATTERY              1
#define GOODBATTERY             2
#define EXPERIMENTSTARTED       3
#define HIGHTEMP                4
#define LOWTEMP                 5
#define CMDCHGCONFIGURATION     6

#define MINVOLTAGENORMALMODE    6200
#define MINVOLTAGEEXPERIMENT    6500
#define MIN_NORMAL_TEMPERATURE  1U /*10 degree celsius*/
#define MAX_NORMAL_TEMPERATURE  30U /*60 degree celsius*/
#define MARIE_EXP_FINISHED      255U

void bootFlightStatus();
void checkStatus();
void report_status();

#endif /* INCLUDE_ABCS_STATUS_H_ */
