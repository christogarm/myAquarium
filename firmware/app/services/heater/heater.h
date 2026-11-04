/*
 * Author: Christogarm
 */

#ifndef HEATER_H
#define HEATER_H

/* Includes */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "heater_if.h"



typedef struct
{
	heater_init_f init_f;
	heater_setOutput_f setOutput_f;

	void* instance;
	void* configInstance;

	float rateLimit;
	float minOutput, maxOutput;
}heater_config_t;


typedef struct
{
	heater_config_t config;
	float power;
	bool init;
}heater_t;


/* Prototipos de las funciones Publicas */
heater_status_t heater_init(heater_t* heater_, const heater_config_t* config_);
heater_status_t heater_setOutput(heater_t* heater_, float output_, float time_s);


#endif /* HEATER_H */