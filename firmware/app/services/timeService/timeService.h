/*
 * Author: Christogarm
 */

#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

/* Includes */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "time_if.h"



typedef struct
{
	time_init_f init_f;
	time_getTime_f getTime_f;

	void* instance;
	void* configInstance;
}timeService_config_t;


typedef struct
{
	timeService_config_t config;
	timeData_t timeData;
	bool init;
}timeService_t;


/* Prototipos de las funciones Publicas */
time_status_t timeService_init(timeService_t* timeService_, const timeService_config_t* config_);
time_status_t timeService_getTime(timeService_t* timeService_);


#endif /* TIME_SERVICE_H */