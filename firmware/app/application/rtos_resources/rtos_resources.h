/*
 * Author: Christogarm
 */

#ifndef RTOS_RESOURCES_H
#define RTOS_RESOURCES_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"              /* Para Queues (Colas) */
#include "semphr.h"             /* Para Mutexes y Semáforos */
#include "event_groups.h"       /* Para Event Groups */
#include "task.h"               /* Para Task Notifications */

#include "config.h"
/* Macros */
#define RTOS_TIMEOUT_MAX        portMAX_DELAY
#define RTOS_OK                 pdPASS
#define RTOS_ERROR              pdFALSE

/* Enums */
typedef enum {
    CONFIG_TASK_INIT_BIT_NONE = 0,
    CONFIG_TASK_INIT_BIT_TEMP_WATER = (1UL << 0),
    CONFIG_TASK_INIT_BIT_TEMP_AMB = (1UL << 1),
    CONFIG_TASK_INIT_BIT_HEATER_POWER = (1UL << 2),
    CONFIG_TASK_INIT_BIT_TEMP_CTRL = (1UL << 3),
    CONFIG_TASK_INIT_BIT_TIME_LOG = (1UL << 4),
    CONFIG_TASK_INIT_BIT_EVENT_LOG = (1UL << 5),
    CONFIG_TASK_INIT_BIT_ALL = (CONFIG_TASK_INIT_BIT_TEMP_WATER | CONFIG_TASK_INIT_BIT_TEMP_AMB | CONFIG_TASK_INIT_BIT_HEATER_POWER | CONFIG_TASK_INIT_BIT_TEMP_CTRL | CONFIG_TASK_INIT_BIT_TIME_LOG | CONFIG_TASK_INIT_BIT_EVENT_LOG),
}configTaskInit_bit_t;

typedef enum {
    TRANS_BIT_NONE = 0,
    TRANS_BIT_TEMP_WATER = (1UL << 0),
    TRANS_BIT_TEMP_AMB = (1UL << 1),
    TRANS_BIT_HEATER_POWER = (1UL << 2),
    TRANS_BIT_ALL = (TRANS_BIT_TEMP_WATER | TRANS_BIT_TEMP_AMB | TRANS_BIT_HEATER_POWER),
}transductor_bit_t;

typedef enum {
    WARNING_BIT_NONE = 0,
    WARNING_BIT_TEMP_WATER_HIGH = (1UL << 0),
    WARNING_BIT_TEMP_WATER_LOW = (1UL << 1),
    WARNING_BIT_TEMP_AMB_HIGH = (1UL << 2),
    WARNING_BIT_TEMP_AMB_LOW = (1UL << 3),
    WARNING_BIT_HEATER_POWER = (1UL << 4),
    WARNING_BIT_ALL = (WARNING_BIT_TEMP_WATER_HIGH | WARNING_BIT_TEMP_WATER_LOW | WARNING_BIT_TEMP_AMB_HIGH | WARNING_BIT_TEMP_AMB_LOW | WARNING_BIT_HEATER_POWER),
}warning_bit_t;

typedef enum {
    QUEUE_ID_TEMP_WATER,
    QUEUE_ID_TEMP_AMB_CONFIG,
    QUEUE_ID_HEATER_POWER,
    QUEUE_ID_HEATER_POWER_CONFIG,
    QUEUE_ID_TEMP_CTRL_DATA,
    QUEUE_ID_TEMP_CTRL_CONFIG,
    QUEUE_ID_LOGGER_EVENT,
    QUEUE_ID_LOGGER_TIME,
}queueID_t;

typedef enum {
    MUTEX_ID_CONFIGURATION,
    MUTEX_ID_SYS_DATA,
}mutexID_t;

typedef enum {
    SYS_EVENT_GROUP_POWER_ON = (1UL << 0),           // Eventos de Power On
    SYS_EVENT_GROUP_RECONFIG_CONTROL = (1UL << 1),           // Eventos de Reconfiguración del control
    SYS_EVENT_GROUP_WATER_TEMP_CHANGE_STATE = (1UL << 2),           // Evento de Falla del Sensor de Temperatura del Agua
    SYS_EVENT_GROUP_AMB_TEMP_CHANGE_STATE = (1UL << 3),           // Evento de Falla del Sensor de Temperatura Ambiente
    SYS_EVENT_GROUP_HEATER_CHANGE_STATE = (1UL << 4),           // Evento de Falla del Calentador (No hay cambios de Temperatura)
    SYS_EVENT_GROUP_ALL = (0b11111UL),
} sysGroupEvents_t;

/* Tipos de Datos */
typedef struct
{
    float value;
    float timeStamp;
    bool failure;
    bool enable;
}queueData_t;

typedef struct
{
	float highTemperature;
	float lowTemperature;
	float difTemperature;
	float failedReadingTime_s;
	bool flagEnable;
}configTempSensor_t;

typedef struct
{
    float rateLimit;
    float difPowerHeater;
    float minPowerHeater;
    float maxPowerHeater;
    bool flagEnable;
}configHeater_t;

typedef struct
{
	float setPoint;
	float deadband;
	float kp, ki, kd;
	float initOutput;
    config_device_flag_t enableFlags;
}configCtrlTemper_t;

typedef struct
{
	float ambTemp;
	float waterTemp;
    float time_s;
    transductor_bit_t enable;
    transductor_bit_t failures;
}dataSensor_CtrlTemper_t;

typedef struct
{
	float tempAmb;
	float tempWater;
	float heaterPower;
    configTaskInit_bit_t configTaskInit;
    transductor_bit_t readyData;
    transductor_bit_t enable;
    transductor_bit_t failures;
    warning_bit_t warnings;
}sysData_t;


/* Datos Publicos */
extern sysData_t sysData;


/* Prototipos de las Funciones Publicas */
void rtos_resources_init(void);

    /* Manejo de Grupo de Eventos */
void rtos_setEvent(sysGroupEvents_t event_);
_Bool rtos_pollEvent(sysGroupEvents_t event_);
sysGroupEvents_t rtos_waitEvent(sysGroupEvents_t event_);
void rtos_clearEvent(sysGroupEvents_t event_);

    /* Manejo para compartir/enviar datos (Colas) */
void queueData_reset(queueData_t* dataSensor_);
uint32_t rxDataQueue(void * dataSensor_, queueID_t id_, uint32_t waitTime_);
uint32_t txDataQueue(void * dataSensor_, queueID_t id_, uint32_t waitTime_);
uint32_t txDataQueue_overWrite(void * dataSensor_, queueID_t id_);

    /* Mutex */
uint32_t mutexTake(mutexID_t id_,  uint32_t waitTime_);
uint32_t mutexGive(mutexID_t id_);

    /* Time with Ticks */
float getTimeSeconds(void);
float getTimeMiliSeconds(void);

    /* Delay */
void rtos_delayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement);
void rtos_delay(TickType_t delay_);


#endif /* RTOS_RESOURCES_H */
