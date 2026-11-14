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
	transductor_bit_t heaterBit;											// Bandera para indicar que la data esta lista
	bool flagEnable;														// Bandera para indicar si el calefactor esta habilitado

}heaterTask_config_t;

typedef struct
{
	heaterTask_config_t config;	
	
	bool configFailed;													// Bandera para indicar si la configuracion de la tarea fallo

	heater_t heater;
	configHeater_t configHeater;
	queueData_t heaterDataPower;
}heaterTask_t;

void vHeaterTask(void *pvParameters);

#endif /* HEATER_TASK_H */