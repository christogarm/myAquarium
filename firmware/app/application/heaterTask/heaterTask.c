/*
 * Author: Christogarm
 */

#include "heaterTask.h"

/* Macros */


/* Variable Privadas */


/* Prototipos de las Funciones Privadas*/

/* Funciones Publicas */
void vHeaterTask(void* pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
    heaterTask_log_t* heaterLogTask_Heater = (heaterTask_log_t*)pvParameters;
	heater_t* Heater = (heater_t*)&heaterLogTask_Heater->heater;

    if(heater_init(Heater, &Heater->config) != HEATER_STATUS_OK) {
        vTaskDelete(NULL);
        return;   
    }
    
    float initTime_s, endTime_s;
	float time_s;
    for (;;)
    {   
        if(getDataQueue(&heaterLogTask_Heater->heaterDataPower, heaterLogTask_Heater->config.queueID, 0) != RTOS_OK) {
            if (heaterLogTask_Heater->heaterDataPower.value == heaterLogTask_Heater->heater.power)
                getDataQueue(&heaterLogTask_Heater->heaterDataPower, heaterLogTask_Heater->config.queueID, RTOS_TIMEOUT_MAX);
        }  

        /* Tiempo de Espera para actualizar la Salida en el Calentador*/
        initTime_s = getTimeSeconds();
        rtos_delay(pdMS_TO_TICKS(heaterLogTask_Heater->config.timeUpdateHeater_ms));
        endTime_s = getTimeSeconds();
        time_s = endTime_s - initTime_s;

        /* Se output del Calentador */
        heater_setOutput(Heater, heaterLogTask_Heater->heaterDataPower.value, time_s);
        
        /* Send Heater Power Data to Logger */
        mutexTake(MUTEX_ID_LOGGER, RTOS_TIMEOUT_MAX);
        dataLogging.readyData |= heaterLogTask_Heater->config.readyData;
        dataLogging.heaterPower = heaterLogTask_Heater->heater.power;
        mutexGive(MUTEX_ID_LOGGER);

    }
    
}

/* Funciones Privadas */
