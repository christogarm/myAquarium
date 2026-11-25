/*
 * Author: Christogarm
 */

#include "tempCtrlTask.h"
#include "FreeRTOS.h"
#include "config.h"
#include "task.h"
#include "PID.h"

/* Macros */
#define HEATER_POWER_ERROR              0               // Potencia con la que notifica si hay una falla 
#define HEATER_POWER_DISABLE            0               // Potencia con la que notifica si esta desactivado el control


/* Prototipos de las Funciones Privadas*/
static void tempAquariumControl_init(tempCtrlTask_t* tempCtrlTask_);
static void obtainTemperatures(tempCtrlTask_t* tempCtrlTask_);

static void tempCtrl_stateEvents(tempCtrlTask_t* tempCtrlTask_);
static void detectEvent_powerOn(tempCtrlTask_t* tempCtrlTask_);
static void detectEvent_reconfig(tempCtrlTask_t* tempCtrlTask_);
static void sendHaterPower(tempCtrlTask_t* tempCtrlTask_, float heaterPower_);

/* Funciones Publicas */
void vTempControlTask(void *pvParameters){
    
    if (pvParameters == NULL) {
        vTaskDelete(NULL);
        return;
    }

    tempCtrlTask_t *tempCtrlTask_ = (tempCtrlTask_t *)pvParameters;
    ctrlTemper_t* ctrlTemper = &tempCtrlTask_->ctrlTemper;

    /* Inicializacion de Variables */
    tempAquariumControl_init(tempCtrlTask_);          // Control de Temperatura
    
    for (;;)
    {   
        /* Detección de eventos */
            /* Detección de eventos sin dependencia del estado */
        detectEvent_powerOn(tempCtrlTask_);                      // Evento del Power On
        detectEvent_reconfig(tempCtrlTask_);                     // Evento de Reconfiguracion

            /* Detección de eventos dependiendo del estado, asi como las acciones a realizar por estado */
        tempCtrl_stateEvents(tempCtrlTask_);

        /* Maquina de estados del control de Temperatura */
        ctrlTemper_MachineState(ctrlTemper, &tempCtrlTask_->ctrlData);

        /* Compartirmos el valor de la potencia del Calentador, en caso de tener un nuevo Valor */
        if(ctrlTemper_isAvailablePowerHeater(ctrlTemper))
            sendHaterPower(tempCtrlTask_, ctrlTemper_getHeaterPower(ctrlTemper));
    }
    
}

/* Funciones Privadas */
static void tempAquariumControl_init(tempCtrlTask_t* tempCtrlTask_){

    ctrlTemper_t* ctrlTemper = &tempCtrlTask_->ctrlTemper;
    PID_Control* temperPID = &tempCtrlTask_->temperPID;
    configCtrlTemper_t* config = &tempCtrlTask_->config;
    bool* ctrlEvents = tempCtrlTask_->ctrlEvents;

	temperPID->param.setPoint = config->setPoint;
    temperPID->param.deadband = config->deadband;
    temperPID->param.Kp = config->kp;
    temperPID->param.Ki = config->ki;
    temperPID->param.Kd = config->kd;
	temperPID->param.initOutput = config->initOutput;
    temperPID->param.clampIntMin = config->ki != 0 ? -100 / config->ki : -1;;
    temperPID->param.clampIntMax = config->ki != 0 ? 100 / config->ki : 1;

    ctrlEvents[CTRL_TEMPER_EVENT_ENABLE_CONTROL] = (config->enableFlags & CONFIG_DEVICE_FLAG_CONTROL) != 0;
    ctrlEvents[CTRL_TEMPER_EVENT_ENABLE_AMB_TEMP] = (config->enableFlags & CONFIG_DEVICE_FLAG_AMB_TEMP) != 0;

    ctrlEvents[CTRL_TEMPER_EVENT_CONFIG_FAIL] = 0;
    if(ctrlTemper_Init(&ctrlTemper, &ctrlTemper->config) != CTRL_TEMPER_STATUS_OK){
        ctrlEvents[CTRL_TEMPER_EVENT_CONFIG_FAIL] = 1;
    }
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_RECONFIGURATION] = 1;
}

static void obtainTemperatures(tempCtrlTask_t* tempCtrlTask_){
    /* Esperamos las Muestras de Temperaturas */
    rxDataQueue(&tempCtrlTask_->dataSensor, QUEUE_ID_TEMP_CTRL_DATA, RTOS_TIMEOUT_MAX);

    /* Copiamos los datos de las Temperaturas */
	tempCtrlTask_->ctrlData.ambTemp = tempCtrlTask_->dataSensor.ambTemp;
    tempCtrlTask_->ctrlData.waterTemp = tempCtrlTask_->dataSensor.waterTemp;
    tempCtrlTask_->ctrlData.dt_s = tempCtrlTask_->dataSensor.time_s - tempCtrlTask_->lastDataSensor.time_s;
    
    tempCtrlTask_->lastDataSensor = tempCtrlTask_->dataSensor;

    /* Guardamos en caso de Fallas de los sensores */
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_AMB_TEMP_FAIL] = (tempCtrlTask_->dataSensor.failures & TRANS_BIT_TEMP_AMB ) == TRANS_BIT_TEMP_AMB;
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL] = (tempCtrlTask_->dataSensor.failures & TRANS_BIT_TEMP_WATER) == TRANS_BIT_TEMP_WATER;
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_HEATER_FAIL] = (tempCtrlTask_->dataSensor.failures & TRANS_BIT_HEATER_POWER) == TRANS_BIT_HEATER_POWER;
}

static void tempCtrl_stateEvents(tempCtrlTask_t* tempCtrlTask_){

    ctrlTemper_t* ctrlTemper_ = &tempCtrlTask_->ctrlTemper;
    /* Borramos las banderas de Eventos */
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_FIRST_SAMPLE] = 0;
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_SAMPLE_UPDATE] = 0;

    /* Acciones que se ejecutan en cada estado */
    switch (ctrlTemper_->state){
    case CTRL_TEMPER_STATE_RESET:
        /* Obtenemos nuevos valores de Temperatura */
        obtainTemperatures(tempCtrlTask_);
        /* Levantamos bandera de Primera Muestra */
        tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_FIRST_SAMPLE] = 1;
        break;

    case CTRL_TEMPER_STATE_WAIT:
        /* Obtenemos nuevos valores de Temperatura */
        obtainTemperatures(tempCtrlTask_);
        /* Levantamos bandera de Primera Muestra */
        tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_SAMPLE_UPDATE] = 1;
        break;

    case CTRL_TEMPER_STATE_CONTROL:
        
        break;

    case CTRL_TEMPER_STATE_FAULT:
        /* Notificamos al calentador que tenemos fallas */
        sendHaterPower(tempCtrlTask_, (float) HEATER_POWER_ERROR);

        /* Esperamos que las fallas se resuelvan */
		bool flagWait = flagWait = (tempCtrlTask_->dataSensor.failures & TRANS_BIT_TEMP_WATER) == TRANS_BIT_TEMP_WATER ||
            (tempCtrlTask_->dataSensor.failures & TRANS_BIT_HEATER_POWER) == TRANS_BIT_HEATER_POWER;
        while (flagWait) {
            rxDataQueue(&tempCtrlTask_->dataSensor, QUEUE_ID_TEMP_CTRL_DATA, RTOS_TIMEOUT_MAX);
			flagWait = (tempCtrlTask_->dataSensor.failures & TRANS_BIT_TEMP_WATER) == TRANS_BIT_TEMP_WATER || 
                (tempCtrlTask_->dataSensor.failures & TRANS_BIT_HEATER_POWER) == TRANS_BIT_HEATER_POWER;
        }
        tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL] = 0;
        tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_HEATER_FAIL] = 0;

        /* Falla de Configuracion */
		while (tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_CONFIG_FAIL]) {
            rxDataQueue(&tempCtrlTask_->config, QUEUE_ID_TEMP_CTRL_CONFIG, RTOS_TIMEOUT_MAX);
            tempAquariumControl_init(tempCtrlTask_);
		}   
        
        break;

    case CTRL_TEMPER_STATE_DISABLED:
        /* Avisamos que el control de Temperatura esta desactivado */
        sendHaterPower(tempCtrlTask_, (float) HEATER_POWER_DISABLE);

        ctrlTemper_t* ctrlTemper = &tempCtrlTask_->ctrlTemper;
        PID_Control* temperPID = &tempCtrlTask_->temperPID;
        configCtrlTemper_t* config = &tempCtrlTask_->config;

        /* Esperamos que se reconfigure el control */
        while (!tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_ENABLE_CONTROL]) {
            rxDataQueue(&tempCtrlTask_->config, QUEUE_ID_TEMP_CTRL_CONFIG, RTOS_TIMEOUT_MAX);
            tempAquariumControl_init(tempCtrlTask_);
        }

        break;

    default:
        ctrlTemper_setState(ctrlTemper_, CTRL_TEMPER_STATE_RESET);
        break;
    }
}

static void detectEvent_powerOn(tempCtrlTask_t* tempCtrlTask_){
    tempCtrlTask_->ctrlEvents[CTRL_TEMPER_EVENT_POWER_ON] = rtos_pollEvent(SYS_EVENT_GROUP_POWER_ON);
}

static void detectEvent_reconfig(tempCtrlTask_t* tempCtrlTask_){
    if (rxDataQueue(&tempCtrlTask_->config, QUEUE_ID_TEMP_CTRL_CONFIG, 0) == RTOS_OK) {
        tempAquariumControl_init(tempCtrlTask_);
    }
}

static void sendHaterPower(tempCtrlTask_t* tempCtrlTask_, float heaterPower_){
    queueData_t dataSensor_ = {
        .failure = ctrlTemper_getState(&tempCtrlTask_->ctrlTemper) == CTRL_TEMPER_STATE_FAULT? 1 : 0,
        .enable = ctrlTemper_getState(&tempCtrlTask_->ctrlTemper) == CTRL_TEMPER_STATE_DISABLED? 0 : 1,
        .timeStamp = getTimeSeconds(),
        .value = heaterPower_,
    };

    txDataQueue_overWrite(&dataSensor_, QUEUE_ID_HEATER_POWER);
}