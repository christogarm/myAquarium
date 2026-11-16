/*
 * Author: Christogarm
 */

#ifndef LOGGER_H
#define LOGGER_H

/* Includes */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "logger_if.h"



typedef struct
{
	logger_init_f init_f;
	logger_addRegister_f addRegister_f;
	logger_readRegister_f readRegister_f;

	void* instance;
	void* instanceConfig;

	logger_type_t type;
}logger_config_t;


typedef struct
{
	logger_config_t config;

	eventLogger_t eventLog;
	timeLogger_t timeLog;

	void* log;

	bool init;
}logger_t;


/* Prototipos de las funciones Publicas */
logger_status_t logger_init(logger_t* logger_, const logger_config_t* config_);
logger_status_t logger_addRegister(logger_t* logger_);
logger_status_t logger_readRegister(logger_t* logger_, uint32_t numRegister_);



#endif /* LOGGER_H */