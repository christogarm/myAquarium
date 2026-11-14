/*
 * Author: Christogarm
 */

#include "heaterTask.h"

/* Macros */


/* Variable Privadas */


/* Prototipos de las Funciones Privadas*/
static void updateDataSystemData(heaterTask_t* heaterTask_Heater);
static void checkReconfig(heaterTask_t* heaterTask_Heater);
static void setReconfiguration(heaterTask_t* heaterTask_Heater);

/* Funciones Publicas */
void vHeaterTask(void* pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
    heaterTask_t* heaterTask_Heater = (heaterTask_t*)pvParameters;
	heater_t* Heater = (heater_t*)&heaterTask_Heater->heater;

    heaterTask_Heater->configFailed = heater_init(Heater, &Heater->config) != HEATER_STATUS_OK;
    
    float initTime_s, endTime_s = 0;
	float time_s;
    for (;;)
    {
        /* Tiempo de Espera para actualizar la Salida en el Calentador*/
        initTime_s = getTimeSeconds();
        time_s = initTime_s - endTime_s;

        /* Revisamos si tenenmos una nueva configuracion */
        checkReconfig(heaterTask_Heater);

        /* Tenemos un nuevo valor para la potencia del calentador? */
        rxDataQueue(&heaterTask_Heater->heaterDataPower, QUEUE_ID_HEATER_POWER, 0);

        /* Aplicamos la salida al calentador */
        heater_setOutput(Heater, heaterTask_Heater->heaterDataPower.value, time_s);

        /* Compartimos los datos al sistema */
        updateDataSystemData(heaterTask_Heater);

        endTime_s = getTimeSeconds();
        /* Verificamos si el nuevo valor es diferente al actual */
        if (heaterTask_Heater->configFailed || (!heaterTask_Heater->config.flagEnable)) {
            heater_setOutput(Heater, 0, 100.0f);
            rxDataQueue(&heaterTask_Heater->configHeater, QUEUE_ID_HEATER_POWER_CONFIG, RTOS_TIMEOUT_MAX);
            setReconfiguration(heaterTask_Heater);
            endTime_s = getTimeSeconds();
        }
        else if (heaterTask_Heater->heaterDataPower.value != heaterTask_Heater->heater.power) {
            rtos_delay(pdMS_TO_TICKS(heaterTask_Heater->config.timeUpdateHeater_ms));
        }
        else
        {
            rxDataQueue(&heaterTask_Heater->heaterDataPower, QUEUE_ID_HEATER_POWER, RTOS_TIMEOUT_MAX);
            endTime_s = getTimeSeconds();
        }
    }
    
}

/* Funciones Privadas */
static void updateDataSystemData(heaterTask_t* heaterTask_Heater) {
    heater_t* Heater = (heater_t*)&heaterTask_Heater->heater;

	mutexTake(MUTEX_ID_SYS_DATA, RTOS_TIMEOUT_MAX);
    sysData.heaterPower = heaterTask_Heater->heater.power;
    sysData.configTaskInit = heaterTask_Heater->configFailed ?
        (sysData.configTaskInit & (~CONFIG_TASK_INIT_BIT_HEATER_POWER)) :
        (sysData.configTaskInit | (CONFIG_TASK_INIT_BIT_HEATER_POWER));
	sysData.readyData |= heaterTask_Heater->config.heaterBit;
    sysData.enable = heaterTask_Heater->config.flagEnable ?
        (TRANS_BIT_HEATER_POWER | sysData.enable) :
        ((~TRANS_BIT_HEATER_POWER) & sysData.enable);
    sysData.warnings = heater_getMaxPowerAlarm(Heater) ?
        (sysData.warnings | (WARNING_BIT_HEATER_POWER)):
        (sysData.warnings & (~WARNING_BIT_HEATER_POWER));
	mutexGive(MUTEX_ID_SYS_DATA);
}

static void checkReconfig(heaterTask_t* heaterTask_Heater) {
    if (rxDataQueue(&heaterTask_Heater->configHeater, QUEUE_ID_HEATER_POWER_CONFIG, 0) == RTOS_OK) {
        setReconfiguration(heaterTask_Heater);
    }
}

static void setReconfiguration(heaterTask_t* heaterTask_Heater) {
    heater_t* heater_ = &heaterTask_Heater->heater;

    heaterTask_Heater->config.flagEnable = heaterTask_Heater->configHeater.flagEnable;

    heater_->config.maxOutput = heaterTask_Heater->configHeater.maxPowerHeater; 
    heater_->config.minOutput = heaterTask_Heater->configHeater.minPowerHeater; 
    heater_->config.rateLimit = heaterTask_Heater->configHeater.rateLimit; 

    heaterTask_Heater->configFailed = heater_init(heater_, &heater_->config) != HEATER_STATUS_OK;
}