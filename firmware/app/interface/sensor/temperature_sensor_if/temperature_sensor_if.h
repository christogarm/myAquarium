/*
 * Author: Christogarm
 */

#ifndef TEMPERATURE_SENSOR_IF_H
#define TEMPERATURE_SENSOR_IF_H 

 /* Macros */
#define TEMPERATURE_NUM_SAMPLES                  10
#define TEMPERATURE_ERROR_MEASUREMENT           -274.15f

/* Tipos de Datos */

    /* Status de las funciones */
typedef enum {
    TEMPERATURE_STATUS_OK,
    TEMPERATURE_STATUS_NULL_POINTER,
    TEMPERATURE_STATUS_ERROR,
}temperature_status_t;

    /* Estado del sensor */
typedef enum {
    TEMPERATURE_STATE_RESET,
    TEMPERATURE_STATE_OK,
    TEMPERATURE_STATE_FAIL,
    TEMPERATURE_STATE_WARNING,
}temperature_states_t;

    /* Banderas de Advertencias del sensor */
typedef enum {
    TEMPERATURE_WARNING_NONE = 0,
    TEMPERATURE_WARNING_HIGH = (0x1UL) << 0,
    TEMPERATURE_WARNING_LOW = (0x1UL) << 1,
}temperature_warnings_t;

    /* Callbacks */
typedef temperature_status_t(*temperature_init_f)(void* instance, void* config);
typedef temperature_states_t(*temperature_updateSample_f)(void* instance, float * temperature);
typedef temperature_status_t(*temperature_reset_f)(void* instance);

    /* Abstraccion del Sensor */
typedef struct 
{
	temperature_init_f init_f;
    temperature_updateSample_f updateSample_f;
    temperature_reset_f reset_f;

    void * instance;
    void * instanceConfig;
}temperature_device_t;


#endif /* TEMPERATURE_SENSOR_IF_H */