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
	logger_addEventRegister_f addRegisterEvent_f;
	logger_addTimeRegister_f addRegisterTime_f;
	logger_readTimeLogger_f readTimeLogger_f;
	logger_readEventLogger_f readEventLogger_f;

	void* instance;
	void* configInstanceEvent;
	void* configInstanceTime;
}logger_config_t;


typedef struct
{
	logger_config_t config;

	eventLogger_t eventLog;
	timeLogger_t timeLog;

	bool init;
}logger_t;


/* Prototipos de las funciones Publicas */
logger_status_t logger_init(logger_t* logger_, const logger_config_t* config_);
logger_status_t logger_addEventRegister(logger_t* logger_);
logger_status_t logger_addTimeRegister(logger_t* logger_);
void* logger_readTimeLogger(logger_t* logger_, uint32_t numRegister_);
void* logger_readEventLogger(logger_t* logger_, uint32_t numRegister_);
bool logger_isInit(const logger_t* logger_);


#endif /* LOGGER_H */