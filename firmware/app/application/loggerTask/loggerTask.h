/*
 * Author: Christogarm
 */

#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include <stdint.h>

#include "rtos_resources.h"
#include "logger.h"
#include "config.h"
#include "timeService.h"


typedef struct
{
	uint32_t timeAddRegisterTime_ms;									// Retraso de la tarea en segundos
	sysGroupEvents_t typesEvents_enable;
	eventLogger_t* buffEvents;
}loggerTask_config_t;

typedef struct
{
	loggerTask_config_t config;	
	logger_t logger;
	timeService_t eventLog_time;
	timeService_t timeLog_time;
	sysGroupEvents_t initEvents;
}loggerTask_log_t;

void vTimeLoggerTask(void *pvParameters);
void vEventLoggerTask(void* pvParameters);

#endif /* LOGGER_TASK_H */