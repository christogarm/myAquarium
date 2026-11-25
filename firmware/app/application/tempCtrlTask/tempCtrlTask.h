/*
 * Author: Christogarm
 */

#ifndef TEMP_CTRL_TASK_H
#define TEMP_CTRL_TASK_H

#include <stdint.h>
#include <stdbool.h>

#include "rtos_resources.h"
#include "ctrlTemper.h"
#include "PID.h"


typedef struct {
	ctrlTemper_t ctrlTemper;
	PID_Control temperPID;
	ctrlTemper_data_t ctrlData;
	configCtrlTemper_t config;

	dataSensor_CtrlTemper_t dataSensor;
	dataSensor_CtrlTemper_t lastDataSensor;
	bool ctrlEvents[numEvents];
} tempCtrlTask_t;

void vTempControlTask(void *pvParameters);

#endif /* TEMP_CTRL_TASK_H */