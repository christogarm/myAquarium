/*
 * API para interpolacion y busqueda binaria
 */

#ifndef INTERPOLATION_H_
#define INTERPOLATION_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
	INTERPOLATION_STATUS_OK,
	INTERPOLATION_STATUS_NULL_POINTER,
	INTERPOLATION_STATUS_ERROR,
}interpolation_status_t;

typedef struct
{
    float* tabX,* tabY;      // Apuntadores de la Tabla
	uint32_t sizeTab;		// Tamaño de la tabla
	uint32_t attempts;		// Numero de intentos con Cache
} interpolation_config_t;

typedef struct
{
	interpolation_config_t config;
	uint32_t indexCache;
	bool init;
} interpolation_t;


interpolation_status_t interpolation_init(interpolation_t* instance_, const interpolation_config_t* config_);
float interpolation_calculate(interpolation_t* instance_, const float input_);



#endif /* INTERPOLATION_H_ */