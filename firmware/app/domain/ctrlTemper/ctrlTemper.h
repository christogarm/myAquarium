/*
 * Author: Christogarm
 */

#ifndef CTRL_TEMPER_H
#define CTRL_TEMPER_H

#include <stdint.h>
#include <stddef.h>

/* Macros */
#define numEvents                                   11

/* Tipos de Datos */
    /* Callbacks */
typedef void* (*ctrlTemper_init)(void* instance, void *config);
typedef void* (*ctrlTemper_update)(void* instance, float input, float dt_);
typedef _Bool (*ctrlTemper_isInit)(void* instance);
typedef void* (*ctrlTemper_reset)(void* instance);
typedef float (*ctrlTemper_getOutput)(void* instance);

    /* Status de las funciones*/
typedef enum{
    CTRL_TEMPER_STATUS_OK,
    CTRL_TEMPER_STATUS_NULL_POINTER,
    CTRL_TEMPER_STATUS_ERROR,
}ctrlTemper_status_t;

    /* Estados del control de Temperatura */
typedef enum {
    CTRL_TEMPER_STATE_RESET,
    CTRL_TEMPER_STATE_WAIT,
    CTRL_TEMPER_STATE_CONTROL,
    CTRL_TEMPER_STATE_DISABLED,
    CTRL_TEMPER_STATE_FAULT,
}ctrlTemper_state_t;

    /* Eventos del control de Temperatura*/
typedef enum{
    CTRL_TEMPER_EVENT_POWER_ON = 0,                         // Evento de Encendido
    CTRL_TEMPER_EVENT_RECONFIGURATION,                      // Evento de Reconfiguracion del control
    CTRL_TEMPER_EVENT_WATER_TEMP_FAIL,                      // Evento de Falla del sensor de Temperatura del Agua
    CTRL_TEMPER_EVENT_AMB_TEMP_FAIL,                        // Evento de Falla del sensor de Temperatura Ambiente
    CTRL_TEMPER_EVENT_FIRST_SAMPLE,                         // Evento de primera Muestra
    CTRL_TEMPER_EVENT_SAMPLE_UPDATE,                        // Evento de una nueva Muestra (excluyendo la primera muestra)
    CTRL_TEMPER_EVENT_ENABLE_CONTROL,                       // Evento habilitacion del control de Temperatura
    CTRL_TEMPER_EVENT_ENABLE_AMB_TEMP,                      // Evento de habilitacion del sensor de Temperatura Ambiente
    CTRL_TEMPER_EVENT_HEATER_FAIL,                          // Evento de Falla del Calentador
    CTRL_TEMPER_EVENT_CONFIG_FAIL,                          // Evento de Falla de Configuracion
	CTRL_TEMPER_EVENT_NUMBER_OF_EVENTS                      // Numero total de eventos
}ctrlTemper_event_t;

    /* Configuracion del control de Temperatura */
typedef struct
{
	void* instance;                                 // Instancia del Controlador de Temperatura, este define el tipo de controlador que se utilizr[a
	void* configInstance;                           // Instancia de la Configuracion del Controlador de Temperatura, este define el tipo de controlador que se utilizar
	ctrlTemper_init init_f;                         // Funcion de Inicializacion del Controlador de Temperatura, este define el tipo de controlador que se utilizar
    ctrlTemper_update update_f;                     // Funcion de Actualizacion del Controlador de Temperatura, este define el tipo de controlador que se utilizat
    ctrlTemper_isInit isInit_f;                     // Funcion para verificar si el Controlador de Temperatura está inicializado
	ctrlTemper_reset reset_f;                       // Funcion para reiniciar el Controlador de Temperatura
    ctrlTemper_getOutput getOutput_f;               // Funcion para obtener la salida del Controlador de Temperatura

	float maxOutput, minOutput;                             // Valor Maximo y Minimo de Salida del Controlador de Temperatura

}ctrlTemper_config_t;                               // Estructura de Configuracion del Control de Temperatura


    /* Estructura de Datos para el manejo del control de Temperatura */
typedef struct
{
    ctrlTemper_config_t config;
    ctrlTemper_state_t state;
    float ambTemp, waterTemp;
    float heaterPower;
    _Bool newDataPower;
    float dt;
    _Bool init;
}ctrlTemper_t;


/* Prototipos de las funciones Publicas */
ctrlTemper_status_t ctrlTemper_Init(ctrlTemper_t * ctrlTemper_, ctrlTemper_config_t * config_);
ctrlTemper_status_t ctrlTemper_MachineState(ctrlTemper_t * ctrlTemper_, _Bool * eventFlags_);
ctrlTemper_status_t ctrlTemper_setTemperatures(ctrlTemper_t * ctrlTemper_, float ambTemp_, float waterTemp_);
ctrlTemper_status_t ctrlTemper_setDtime(ctrlTemper_t * ctrlTemper_, float dt_);
ctrlTemper_status_t ctrlTemper_setState(ctrlTemper_t * ctrlTemper_, ctrlTemper_state_t state_);
ctrlTemper_state_t ctrlTemper_getState(ctrlTemper_t * ctrlTemper_);
float ctrlTemper_getHeaterPower(ctrlTemper_t * ctrlTemper_);
_Bool ctrlTemper_isAvailablePowerHeater(ctrlTemper_t * ctrlTemper_);


#endif /* CTRL_TEMPER_H */
