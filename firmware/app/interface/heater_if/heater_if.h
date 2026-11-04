/*
 * Author: Christogarm
 */

#ifndef HEATER_IF_H
#define HEATER_IF_H

 /* Enum para los tipos de eventos a loggear */
typedef enum {
	HEATER_STATUS_OK,
	HEATER_STATUS_NULL_POINTER,
	HEATER_STATUS_ERROR,
}	heater_status_t;

 /* Callbacks */
typedef heater_status_t(*heater_init_f)(void* instance_, const void* config_);
typedef heater_status_t(*heater_setOutput_f)(void* instance_, float output_);


#endif /* HEATER_IF_H */