/*
 * Author: Christogarm
 */

#ifndef TEMP_CTRL_TASK_H
#define TEMP_CTRL_TASK_H

#include <stdint.h>
#include <stdbool.h>

#include "rtos_resources.h"
#include "ctrlTemper.h"


typedef struct {
	ctrlTemper_t ctrlTemper;
	dataSensor_CtrlTemper_t dataSensor;
	dataSensor_CtrlTemper_t lastDataSensor;
	ctrlTemper_data_t ctrlData;
	bool ctrlEvents[numEvents];
} tempCtrlTask_t;

void vTempControlTask(void *pvParameters);

#endif /* TEMP_CTRL_TASK_H */