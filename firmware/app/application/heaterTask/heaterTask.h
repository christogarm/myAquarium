/*
 * Author: Christogarm
 */

#ifndef HEATER_TASK_H
#define HEATER_TASK_H

#include <stdint.h>

#include "rtos_resources.h"
#include "config.h"
#include "heater.h"


typedef struct
{
	uint32_t timeUpdateHeater_ms;									// Retraso para actualizar la potencia del calefactor
	queueID_t queueID;										// Cola para enviar los datos de temperatura
	readyData_t readyData;											// Bandera para indicar que la data esta lista

}heaterTask_config_t;

typedef struct
{
	heaterTask_config_t config;	
	heater_t heater;
	queueData_t heaterDataPower;
}heaterTask_log_t;

void vHeaterTask(void *pvParameters);

#endif /* HEATER_TASK_H */