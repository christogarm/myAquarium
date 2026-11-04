/*
 * Author: Christogarm
 */

#include "tempCtrlTask.h"
#include "ctrlTemper.h"
#include "FreeRTOS.h"
#include "config.h"
#include "rtos_resources.h"
#include "task.h"
#include "PID.h"

/* Macros */
#define HEATER_POWER_ERROR              0               // Potencia con la que notifica si hay una falla 
#define HEATER_POWER_DISABLE            0               // Potencia con la que notifica si esta desactivado el control

/* Variable Privadas */
static ctrlTemper_t controlTemp = {0};
static PID_Control heaterPID = {0};
static _Bool ctrlEvents[numEvents] = {0};
static uint32_t ctrlNotif = 0;

static queueData_t waterTemp = {0};
static queueData_t ambTemp = {0};

static float lastTimeStamp = 0;

static EventBits_t receivedEvents;

/* Prototipos de las Funciones Privadas*/
static void tempAquariumControl_init(void);
static void obtainTemperatures(void);

static void tempCtrl_stateEvents(ctrlTemper_t * ctrlTemper_);
static void detectEvent_powerOn(void);
static void detectEvent_reconfig(void);
static void detectEvent_heaterFail(void);
static void sendHaterPower(float heaterPower_);

/* Funciones Publicas */
void vTempControlTask(void *pvParameters){

    /* Inicializacion de Variables */
    tempAquariumControl_init();          // Control de Temperatura
    
    for (;;)
    {   
        /* Detección de eventos */
            /* Detección de eventos sin dependencia del estado */
        detectEvent_powerOn();                      // Evento del Power On
        detectEvent_reconfig();                     // Evento de Reconfiguracion
        detectEvent_heaterFail();                   // Evento de falla del calentador

            /* Detección de eventos dependiendo del estado, asi como las acciones a realizar por estado */
        tempCtrl_stateEvents(&controlTemp);

        /* Maquina de estados del control de Temperatura */
        ctrlTemper_MachineState(&controlTemp, ctrlEvents);

        /* Compartirmos el valor de la potencia del Calentador, en caso de tener un nuevo Valor */
        if(ctrlTemper_isAvailablePowerHeater(&controlTemp))
            sendHaterPower(ctrlTemper_getHeaterPower(&controlTemp));
    }
    
}


/* Funciones Privadas */
static void tempAquariumControl_init(void){

    /* Proteccion de memoria compartida entre tareas, la configuracion es un recurso compartido */
    mutexTake(MUTEX_ID_CONFIGURATION, RTOS_TIMEOUT_MAX);
    uint32_t configFlags = configData_RAM.flags;
	
    controlTemp.config.instance = &heaterPID;
	controlTemp.config.init_f = PID_Init;
    controlTemp.config.update_f = PID_Update;
    controlTemp.config.isInit_f = PID_IsInit;
    controlTemp.config.reset_f = PID_Reset;
    controlTemp.config.getOutput_f = PID_GetOutput;
    controlTemp.config.maxOutput = 100;
    controlTemp.config.minOutput = 0;
    controlTemp.config.configInstance = &heaterPID.param;
    
	heaterPID.param.setPoint = configData_RAM.setpoint;
    heaterPID.param.deadband = configData_RAM.deadband;
    heaterPID.param.Kp = configData_RAM.kp;
    heaterPID.param.Ki = configData_RAM.ki;
    heaterPID.param.Kd = configData_RAM.kd;
	heaterPID.param.alpha = 0.8;
	heaterPID.param.minOutput = 0;
    heaterPID.param.maxOutput = 100;
	heaterPID.param.initOutput = configData_RAM.initOutput;
    heaterPID.param.initInput = 0;
    heaterPID.param.clampIntMin = configData_RAM.ki != 0 ? -100 / configData_RAM.ki : -1;
    heaterPID.param.clampIntMax = configData_RAM.ki != 0 ? 100 / configData_RAM.ki : 1;

    mutexGive(MUTEX_ID_CONFIGURATION);

    ctrlEvents[CTRL_TEMPER_EVENT_ENABLE_CONTROL] = (configFlags & CONFIG_DEVICE_FLAG_CONTROL) != 0;
    ctrlEvents[CTRL_TEMPER_EVENT_ENABLE_AMB_TEMP] = (configFlags & CONFIG_DEVICE_FLAG_AMB_TEMP) != 0;

    ctrlEvents[CTRL_TEMPER_EVENT_CONFIG_FAIL] = 0;
    if(ctrlTemper_Init(&controlTemp, &controlTemp.config) != CTRL_TEMPER_STATUS_OK){
        ctrlEvents[CTRL_TEMPER_EVENT_CONFIG_FAIL] = 1;
    }

    rtos_clearEvent(CTRL_TEMPER_EVENT_RECONFIGURATION);
}

static void obtainTemperatures(void){
    /* Esperamos las Muestras de Temperaturas */
    getDataQueue(&waterTemp, QUEUE_ID_TEMP_WATER, RTOS_TIMEOUT_MAX);
    
    if(ctrlEvents[CTRL_TEMPER_EVENT_ENABLE_AMB_TEMP])
        getDataQueue(&ambTemp, QUEUE_ID_TEMP_AMB, RTOS_TIMEOUT_MAX);

    /* Copiamos los datos de las Temperaturas */
    ctrlTemper_setTemperatures(&controlTemp, ambTemp.value, waterTemp.value);

    /* Copiamos el diferencial de Tiempo */
    ctrlTemper_setDtime(&controlTemp, waterTemp.timeStamp - lastTimeStamp);
    lastTimeStamp = waterTemp.timeStamp;

    /* Guardamos en caso de Fallas de los sensores */
    ctrlEvents[CTRL_TEMPER_EVENT_AMB_TEMP_FAIL] = ambTemp.failure; 
    ctrlEvents[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL] = waterTemp.failure;
}

static void tempCtrl_stateEvents(ctrlTemper_t * ctrlTemper_){
    /* Borramos las banderas de Eventos */
    ctrlEvents[CTRL_TEMPER_EVENT_FIRST_SAMPLE] = 0;
    ctrlEvents[CTRL_TEMPER_EVENT_SAMPLE_UPDATE] = 0;

    /* Acciones que se ejecutan en cada estado */
    switch (ctrlTemper_->state){
    case CTRL_TEMPER_STATE_RESET:
        /* Obtenemos nuevos valores de Temperatura */
        obtainTemperatures();
        /* Levantamos bandera de Primera Muestra */
        ctrlEvents[CTRL_TEMPER_EVENT_FIRST_SAMPLE] = 1;
        break;

    case CTRL_TEMPER_STATE_WAIT:
        /* Obtenemos nuevos valores de Temperatura */
        obtainTemperatures();
        /* Levantamos bandera de Primera Muestra */
        ctrlEvents[CTRL_TEMPER_EVENT_SAMPLE_UPDATE] = 1;
        break;

    case CTRL_TEMPER_STATE_CONTROL:
        
        break;

    case CTRL_TEMPER_STATE_FAULT:
        /* Notificamos al calentador que tenemos fallas */
        sendHaterPower((float) HEATER_POWER_ERROR);

        /* Falla del sensor de Temperatura del Agua */
        if(ctrlEvents[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL]){
            /* Esperamos que el sensor este funcionando */
            rtos_waitEvent(SYS_EVENT_GROUP_WATER_TEMP_CHANGE_STATE);
            /* Desactivamos la falla del sensor de Temperatura del Agua */
            ctrlEvents[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL] = 0;
        }

        /* Falla del Calentador */
        if(ctrlEvents[CTRL_TEMPER_EVENT_HEATER_FAIL]){
            /* Esperamos que el sensor este funcionando */
            rtos_waitEvent(SYS_EVENT_GROUP_HEATER_CHANGE_STATE);
            /* Desactivamos la falla del sensor de Temperatura del Agua */
            ctrlEvents[CTRL_TEMPER_EVENT_HEATER_FAIL] = 0;
        }
        
        /* Falla de Configuracion */
        if(ctrlEvents[CTRL_TEMPER_EVENT_CONFIG_FAIL]){
            rtos_waitEvent(SYS_EVENT_GROUP_RECONFIG_CONTROL);
        }
        break;

    case CTRL_TEMPER_STATE_DISABLED:
        /* Avisamos que el control de Temperatura esta desactivado */
        sendHaterPower((float) HEATER_POWER_DISABLE);
        
        /* Esperamos que se reconfigure el control */
        rtos_waitEvent(SYS_EVENT_GROUP_RECONFIG_CONTROL);
        break;

    default:
        ctrlTemper_setState(ctrlTemper_, CTRL_TEMPER_STATE_RESET);
        break;
    }
}

static void detectEvent_powerOn(void){
    ctrlEvents[CTRL_TEMPER_EVENT_POWER_ON] = rtos_pollEvent(SYS_EVENT_GROUP_POWER_ON);
}

static void detectEvent_reconfig(void){    
    ctrlEvents[CTRL_TEMPER_EVENT_RECONFIGURATION] = rtos_pollEvent(SYS_EVENT_GROUP_RECONFIG_CONTROL);

    if(ctrlEvents[CTRL_TEMPER_EVENT_RECONFIGURATION])
        tempAquariumControl_init();
}

static void detectEvent_heaterFail(void){
    ctrlEvents[CTRL_TEMPER_EVENT_HEATER_FAIL] = rtos_pollEvent(SYS_EVENT_GROUP_HEATER_CHANGE_STATE);
}

static void sendHaterPower(float heaterPower_){
    queueData_t dataSensor_ = {
        .failure = ctrlTemper_getState(&controlTemp) == CTRL_TEMPER_STATE_FAULT? 1 : 0,
        .enable = ctrlTemper_getState(&controlTemp) == CTRL_TEMPER_STATE_DISABLED? 0 : 1,
        .timeStamp = getTimeSeconds(),
        .value = heaterPower_,
    };

    setDataQueue_overWrite(&dataSensor_, QUEUE_ID_HEATER_POWER);
}