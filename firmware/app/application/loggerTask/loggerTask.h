/*
 * Author: Christogarm
 */

#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include <stdint.h>

#include "rtos_resources.h"
#include "logger.h"
#include "config.h"


typedef struct
{
	queueID_t queueID;
}loggerTask_config_t;

typedef struct
{
	loggerTask_config_t config;	
	logger_t logger;
}loggerTask_t;

void vLoggerTask(void *pvParameters);

#endif /* LOGGER_TASK_H */