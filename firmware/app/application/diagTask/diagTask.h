/*
 * Author: Christogarm
 */

#ifndef DIAG_TASK_H_
#define DIAG_TASK_H_

#include <stdint.h>
#include <stdbool.h>

#include "rtos_resources.h"
#include "timeService.h"
#include "logger.h"

typedef struct
{
	uint32_t timeDelayTask_ms;										// Retraso de la tarea en milisegundos
	float timeLogger_addNewReg_s;
}diagTask_config_t;

typedef struct
{
	diagTask_config_t config;									// Configuracion de la tarea
	
	float timeLogger_s;											// Tiempo acumulado de la tarea
	sysData_t data;

	timeService_t timeService;										// Servicio de tiempo
	timeLogger_t timeLoggerData;

}diagTask_t;

void vDiagTask(void *pvParameters);

#endif /* DIAG_TASK_H_ */