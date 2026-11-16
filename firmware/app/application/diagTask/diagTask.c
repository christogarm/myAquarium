/*
 * Author: Christogarm
 */

#include "diagTask.h"

#include "FreeRTOS.h"
#include "config.h"
#include "task.h"

/* Macros */


/* Variable Privadas */
static void updateSystemData(diagTask_t* diagTask);
static void sendDataLoggerTime(diagTask_t* diagTask);

/* Prototipos de las Funciones Privadas*/


/* Funciones Publicas */
void vDiagTask(void *pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    diagTask_t* diagTask = (diagTask_t*)pvParameters; 


    /* Inicializacion de Variables */
    if(timeService_init(&diagTask->timeService, &diagTask->timeService.config) != TIME_STATUS_OK)
    {
        vTaskDelete(NULL);
        return;
    }

	float startTime, endTime = 0, timeSeconds;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    diagTask->timeLogger_s = 0;

    for (;;)
    {   
        /* Tiempos de la Tarea */
        startTime = getTimeSeconds();
        timeService_getTime(&diagTask->timeService);
        timeSeconds = startTime - endTime;
        diagTask->timeLogger_s += timeSeconds;

        /* Actualizamos los datos del sistema */
        updateSystemData(diagTask);

		/* Enviamos los datos al Logger */
        sendDataLoggerTime(diagTask);
        
        endTime = getTimeSeconds();
        rtos_delayUntil(&xLastWakeTime, pdMS_TO_TICKS(diagTask->config.timeDelayTask_ms));
    }
}

/* Funciones Privadas */
static void updateSystemData(diagTask_t* diagTask) {
    mutexTake(MUTEX_ID_SYS_DATA, RTOS_TIMEOUT_MAX);
    diagTask->data = sysData;
    mutexGive(MUTEX_ID_SYS_DATA);

}

static void sendDataLoggerTime(diagTask_t* diagTask) {
    if ((diagTask->data.readyData &
        (TRANS_BIT_TEMP_WATER | TRANS_BIT_TEMP_AMB | TRANS_BIT_HEATER_POWER)) !=
        (TRANS_BIT_TEMP_WATER | TRANS_BIT_TEMP_AMB | TRANS_BIT_HEATER_POWER)) {
        return;
    }
    
    if(diagTask->timeLogger_s >= diagTask->config.timeLogger_addNewReg_s) {
		diagTask->timeLogger_s = 0;

        diagTask->timeLoggerData.timeUNIX = diagTask->timeService.timeData.timeUNIX;
		diagTask->timeLoggerData.tempAmb = diagTask->data.tempAmb;
        diagTask->timeLoggerData.tempWater = diagTask->data.tempWater;
        diagTask->timeLoggerData.heaterPower = diagTask->data.heaterPower;

        txDataQueue(&diagTask->timeLoggerData, QUEUE_ID_LOGGER_TIME, pdMS_TO_TICKS(diagTask->config.timeDelayTask_ms));
    }
}