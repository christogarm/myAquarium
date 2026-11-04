/*
 * Logger tiene la funcion de Grabar Eventos y Datos en dos archivos distintos
 * DataLog.csv      ->  Se graban datos de interes
 * EventosLog.csv   ->  Se graban Eventos como errores, advertencias o informacion
 */

#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "config_if.h"

#define CONFIG_READ_MAX_LINEA_CHAR		400
#define MAX_FIELD_LEN   400

typedef struct
{
    const char* filename;      // Nombre del Archivo
	char** pEncabezado;        // Apuntador para escribir el encabezado de la tabla de Datos
} config_file_config_t;

typedef struct
{
	config_file_config_t config;
	dataConfig_t data;
    FILE* file;          // Archivo
	char header[CONFIG_NUM_PARAMETERS][MAX_FIELD_LEN];
	char values[CONFIG_NUM_PARAMETERS][MAX_FIELD_LEN];
	bool init;
} config_file_t;


config_status_t config_file_init(void* instance_, const void* config_);
config_status_t config_file_write(void* instance_, dataConfig_t* configData_);
config_status_t config_file_read(void* instance_, dataConfig_t* configData_);


#endif /* CONFIG_FILE_H_ */