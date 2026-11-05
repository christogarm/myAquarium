/*
 * Author: Christogarm
 */

/* includes */
#include "temperatureSim.h"
#include <stdlib.h> // Necesaria para rand() y srand()
#include <time.h>   // Necesaria para time()
#include <errno.h>
#include <math.h>

/* Prototipos de las funciones Privadas */
static bool parseFloat(const char* text, float* value);
static int parseCSV(char* line, char fields[][TEMP_SIM_MAX_FIELD_LEN], size_t maxFields);
static char* trim(char* str);

/* Funciones Públicas */
temperature_status_t temperatureSim_init(void* instance, void* config) {
	if (instance == NULL || config == NULL) 
		return TEMPERATURE_STATUS_NULL_POINTER;

	temperatureSim_t* tempSim = (temperatureSim_t*)instance;
	temperatureSim_config_t* tempSim_config = (temperatureSim_config_t*)config;

	tempSim->time.config.utc_offset_seconds = 0;
	tempSim->time.config.use_start_time = false;
	tempSim->time.config.start_unix = 0;
	if (time_sim_init(&tempSim->time, &tempSim->time.config) != TIME_STATUS_OK)
		return TEMPERATURE_STATUS_ERROR;

	time_sim_get_time(&tempSim->time, &tempSim->time_init);

	tempSim->file = fopen(tempSim->config.filename, "r");
	if (tempSim->file == NULL)
		return TEMPERATURE_STATUS_ERROR;

	char line[TEMP_SIM_READ_MAX_LINE_CHAR];
	char values[2][TEMP_SIM_MAX_FIELD_LEN];
	int count = 0;
	for (size_t i = 0; i < 24; i++)
	{
		if (fgets(line, sizeof(line), tempSim->file) == NULL) {
			fclose(tempSim->file);
			return TEMPERATURE_STATUS_ERROR;
		}
		parseCSV(line, values, 2);
		parseFloat(values[0], &tempSim->tabTimeUnix[i] );
		parseFloat(values[1], &tempSim->tabTemperature[i]);
	}
	fclose(tempSim->file);

	tempSim->interpolation.config.tabX = tempSim->tabTimeUnix;
	tempSim->interpolation.config.tabY = tempSim->tabTemperature;
	tempSim->interpolation.config.sizeTab = 24;
	tempSim->interpolation.config.attempts = 4;

	if (interpolation_init(&tempSim->interpolation, &tempSim->interpolation.config) != INTERPOLATION_STATUS_OK)
		return TEMPERATURE_STATUS_ERROR;

	tempSim->config = *tempSim_config;
	tempSim->init = true;

	return TEMPERATURE_STATUS_OK;
}

temperature_states_t temperatureSim_updateSample(void* instance, float* temperature) {
	if (instance == NULL || temperature == NULL)
		return TEMPERATURE_STATUS_NULL_POINTER;

	temperatureSim_t* tempSim = (temperatureSim_t*)instance;

	if(!tempSim->init)
		return TEMPERATURE_STATUS_ERROR;

	time_sim_get_time(&tempSim->time, &tempSim->timeData);
	tempSim->timeUNIX = tempSim->timeData.timeUNIX - tempSim->time_init.timeUNIX;
	
	tempSim->temperature = interpolation_calculate(&tempSim->interpolation, tempSim->timeUNIX);

	*temperature = tempSim->temperature;
	
	return TEMPERATURE_STATE_OK;
}

temperature_status_t temperatureSim_reset(void* instance) {
	if (instance == NULL)
		return TEMPERATURE_STATUS_NULL_POINTER;
	temperatureSim_t* tempSim = (temperatureSim_t*)instance;

	if (!tempSim->init)
		return TEMPERATURE_STATUS_ERROR;

	tempSim->temperature = tempSim->tabTemperature[0];

	return TEMPERATURE_STATUS_OK;
}


/* Funciones Privadas */
static bool parseFloat(const char* text, float* value)
{
	char* end;
	errno = 0;

	float result = strtof(text, &end);

	if (text == end || *end != '\0' ||
		errno == ERANGE || !isfinite(result)) {
		return false;
	}

	*value = result;
	return true;
}

static int parseCSV(char* line, char fields[][TEMP_SIM_MAX_FIELD_LEN], size_t maxFields)
{
	size_t count = 0;
	char* start = line;

	while (true) {
		char* comma = strchr(start, ',');

		if (count >= maxFields)
			return -1;

		if (comma != NULL)
			*comma = '\0';

		char* field = trim(start);
		size_t len = strlen(field);

		if (len >= TEMP_SIM_MAX_FIELD_LEN)
			return -1;

		memcpy(fields[count], field, len + 1);
		count++;

		if (comma == NULL)
			break;

		start = comma + 1;
	}

	return (int)count;
}

static char* trim(char* str)
{
	while (*str == ' ' || *str == '\t')
		str++;

	size_t len = strlen(str);

	while (len > 0 &&
		(str[len - 1] == ' ' ||
			str[len - 1] == '\t' ||
			str[len - 1] == '\r' ||
			str[len - 1] == '\n')) {
		str[--len] = '\0';
	}

	return str;
}