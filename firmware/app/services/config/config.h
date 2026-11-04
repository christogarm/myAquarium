/*
 * Author: Christogarm
 */

#ifndef CONFIG_H
#define CONFIG_H

/* Includes */
#include "config_if.h"
#include <stdbool.h>

/* Tipos de Datos */
typedef struct {
	config_init_f init_f;
	config_write_f write_f;
	config_read_f read_f;

	void* instance;
	void* instanceConfig;
}config_config_t;

typedef struct
{
	config_config_t config;
	dataConfig_t dataConfig;

	bool init;
}config_t;

/* Variables Publicas */
extern config_t config_BK;
extern config_t config_SYS;
extern dataConfig_t configData_RAM;

/* Prototipos de las funciones Publicas */
config_status_t config_init(config_t * configInst_, config_config_t * config_);
config_status_t config_write(config_t* configInst_);
config_status_t config_read(config_t* configInst_);



#endif /* CONFIG_H */