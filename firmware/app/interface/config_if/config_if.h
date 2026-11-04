/*
 * Author: Christogarm
 */

#ifndef CONFIG_IF_H
#define CONFIG_IF_H

#include <stdint.h>
#include <stddef.h>

#define CONFIG_NUM_PARAMETERS                   20

 /* Macros para la primera programacion */
#define CONFIG_BK_SETPOINT                      (float) 26.0f
#define CONFIG_BK_DEADBAND                      (float) 0.3f
#define CONFIG_BK_RATELIMIT                     (float) 20.0f
#define CONFIG_BK_INITOUTPUT                    (float) 50.0f
#define CONFIG_BK_KP                            (float) 5.0f
#define CONFIG_BK_KI                            (float) 0.1f
#define CONFIG_BK_KD                            (float) 2.0f

#define CONFIG_BK_MIN_POWER_HEATER              (float) 0.0f
#define CONFIG_BK_MAX_POWER_HEATER              (float) 100.0f

#define CONFIG_BK_MAX_WATER_TEMP                (float) 30.0f
#define CONFIG_BK_MIN_WATER_TEMP                (float) 25.0f
#define CONFIG_BK_DIF_WATER_TEMP                (float) 0.5f
#define CONFIG_BK_TIME_SHOW_FAIL_WATER_TEMP     (float) 3.0f

#define CONFIG_BK_MAX_AMBIENT_TEMP              (float) 40.0f
#define CONFIG_BK_MIN_AMBIENT_TEMP              (float) 5.0f
#define CONFIG_BK_DIF_AMBIENT_TEMP              (float) 1.0f
#define CONFIG_BK_TIME_SHOW_FAIL_AMBIENT_TEMP   (float) 3.0f

#define CONFIG_BK_TIME_ADD_NEW_REG_LOG          (float) 1.0f
#define CONFIG_BK_ENABLE_EVENT_MASK             (config_event_mask_t) CONFIG_EVENT_MASK_ALL

#define CONFIG_BK_FLAGS                         (config_device_flag_t) (    CONFIG_DEVICE_FLAG_CONTROL +                    \
                                                                            CONFIG_DEVICE_FLAG_AMB_TEMP                     \
                                                                        )

 /* Enum para los tipos de eventos a loggear */
typedef enum {
	CONFIG_STATUS_OK,
	CONFIG_STATUS_NULL_POINTER,
	CONFIG_STATUS_ERROR,
}config_status_t;

/* Enum para las banderas de Configuracion */
typedef enum {
    CONFIG_DEVICE_FLAG_CONTROL = (1UL << 0),
    CONFIG_DEVICE_FLAG_AMB_TEMP = (1UL << 1),
    CONFIG_DEVICE_FLAG_WATER_TEMP = (1UL << 2),
} config_device_flag_t;

/* Enum para enmascarar los eventos a registrar */
typedef enum {
    CONFIG_EVENT_MASK_FAIL_AMBIENT_TEMP = (1UL << 0),
    CONFIG_EVENT_MASK_HIGH_AMBIENT_TEMP = (1UL << 1),
    CONFIG_EVENT_MASK_LOW_AMBIENT_TEMP = (1UL << 2),
    CONFIG_EVENT_MASK_FAIL_WATER_TEMP = (1UL << 3),
    CONFIG_EVENT_MASK_HIGH_WATER_TEMP = (1UL << 4),
    CONFIG_EVENT_MASK_LOW_WATER_TEMP = (1UL << 5),
    CONFIG_EVENT_MASK_FAIL_HEATER = (1UL << 6),
    CONFIG_EVENT_MASK_ALL = (0xFFUL),
} config_event_mask_t;

/* Tipos de Datos */
    /* Estructura de Configuracion para todo el Sistema */
typedef struct
{
    /* Configuracion del Control PID */
    float setpoint;                     // Temperatura del Agua Deseada °C
    float deadband;                     // Banda Muerta, Temperatura donde el control PID no realiza cambios de Potencia
    float rateLimit;                    // Cambios de Porcentaje de Porencia respecto al tiempo %/s
    float initOutput;                   // Al realizar un encendido del Control, el % de Potencia será determinado por este valor
    float kp, ki, kd;                   // Ganacias del control PID

    /* Configuracion del Calentador */
    float minPowerHeater;               // Potencia Mínima del Calentador
    float maxPowerHeater;               // Potencia Máxima del Calentador

    /* Configuracion del Sensor de Temperatura del Agua */
    float maxWaterTemperature;          // Temperatura máxima para levantar una Alarma de Alta Temperatura
    float minWaterTemperature;          // Temperatura mínima para levanta una Alarma de Baja Temperatura
    float difWaterTemperature;          // Diferencial de Temepratura para Apagar las Alarmas de Temepratura.
    float timeShowFailWaterTemp;        // Tiempo para mostrar la Falla del Sensor de Temperatura

    /* Configuracion del Sensor de Temperatura Ambiente */
    float maxAmbientTemperature;          // Temperatura máxima para levantar una Alarma de Alta Temperatura
    float minAmbientTemperature;          // Temperatura mínima para levanta una Alarma de Baja Temperatura
    float difAmbientTemperature;          // Diferencial de Temepratura para Apagar las Alarmas de Temepratura.
    float timeShowFailAmbientTemp;        // Tiempo para mostrar la Falla del Sensor de Temperatura

    /* Configuracion del Logger */
    float timeAddNewRegisterLogger;                 // Tiempo para Registrar un nuevo Registro de Logger
    config_event_mask_t enabled_events_mask;        // Mascara de que eventos se registraran

    /* Banderas de Configuracion */
    config_device_flag_t flags;                     // Banderas de Configuracion
} dataConfig_t;


 /* Callbacks */
typedef config_status_t(*config_init_f)(void* instance_, const void* config_);
typedef config_status_t(*config_write_f)(void* instance_, dataConfig_t* configData_);
typedef config_status_t(*config_read_f)(void* instance_, dataConfig_t* configData_);

#endif /* CONFIG_IF_H */