/*
 * Author: Christogarm
 */

/* includes */
#include "temperature.h"
/* Prototipos de las funciones Privadas */
static float promediarMuestras(float* buff_, uint8_t numIterations_);
static void temperature_evaluateWarnings(temperatureSensor_t* sensor_);

/* Funciones Publicas */
temperature_status_t temperature_init(temperatureSensor_t * sensor_, const temperature_config_t * config_){
    if(sensor_ == NULL || config_ == NULL || config_->device.init_f == NULL ||
        config_->device.updateSample_f == NULL || config_->device.reset_f == NULL ||
        config_->device.instance == NULL || config_->device.instanceConfig == NULL)
        return TEMPERATURE_STATUS_NULL_POINTER; 

    if (config_->difTemperature < 0 || config_->failedReadingTime_s <= 0)
        return TEMPERATURE_STATUS_ERROR;

    if((config_->highTemperature - config_->difTemperature) <= (config_-> lowTemperature + config_->difTemperature))
        return TEMPERATURE_STATUS_ERROR;
    
    if(config_->device.init_f(config_->device.instance,config_->device.instanceConfig) != TEMPERATURE_STATUS_OK)
        return TEMPERATURE_STATUS_ERROR;

    sensor_->config = *config_;
    sensor_->init = true;

    temperature_reset(sensor_);

    return TEMPERATURE_STATUS_OK;
}

temperature_status_t temperature_updateSample(temperatureSensor_t * sensor_, float timeSeconds_){
    if(sensor_ == NULL)
        return TEMPERATURE_STATUS_NULL_POINTER;

    if(timeSeconds_ <= 0)
        return TEMPERATURE_STATUS_ERROR;

    if (!sensor_->init)
        return TEMPERATURE_STATUS_ERROR;
    
    if (sensor_->countSamples >= TEMPERATURE_NUM_SAMPLES)
        sensor_->countSamples = 0;

    if(sensor_->config.device.updateSample_f(sensor_->config.device.instance, 
        &sensor_->tempSamples[sensor_->countSamples]) == TEMPERATURE_STATE_FAIL) {
        sensor_->accumulatedFailedTime_s += timeSeconds_;
        if (sensor_->accumulatedFailedTime_s >= sensor_->config.failedReadingTime_s) {
            temperature_reset(sensor_);

            sensor_->state = TEMPERATURE_STATE_FAIL;

            sensor_->accumulatedFailedTime_s = sensor_->config.failedReadingTime_s;
            sensor_->temperature = sensor_->config.errorTemperature;
        }
    }
    else{
        sensor_->countSamples++;
        sensor_->accumulatedFailedTime_s = 0.0f;

        uint8_t numSamples = sensor_->firstFullBuffer ? TEMPERATURE_NUM_SAMPLES : sensor_->countSamples;
        sensor_->temperature = promediarMuestras(sensor_->tempSamples, numSamples);
        temperature_evaluateWarnings(sensor_);

        if (sensor_->countSamples >= TEMPERATURE_NUM_SAMPLES) {
            sensor_->firstFullBuffer = true;
        }
    }

    return TEMPERATURE_STATUS_OK;
}

float temperature_getTemperature(const temperatureSensor_t * sensor_){
    if(sensor_ == NULL)
        return TEMPERATURE_ERROR_MEASUREMENT;

    if (!sensor_->init)
        return sensor_->config.errorTemperature;
    
    return sensor_->temperature;
}

temperature_states_t temperature_getState(const temperatureSensor_t* sensor_) {
    if(sensor_ == NULL)
        return TEMPERATURE_STATE_FAIL;
    if (!sensor_->init)
        return TEMPERATURE_STATE_RESET;
    
    return sensor_->state;
}

temperature_warnings_t temperature_getWarnings(const temperatureSensor_t * sensor_){
    if(sensor_ == NULL)
        return TEMPERATURE_WARNING_NONE;

    if (!sensor_->init)
        return TEMPERATURE_WARNING_NONE;
    
    return sensor_->warnings;
}

temperature_status_t temperature_reset(temperatureSensor_t * sensor_){
    if(sensor_ == NULL)
        return TEMPERATURE_STATUS_NULL_POINTER; 
    if (!sensor_->init)
        return TEMPERATURE_STATUS_ERROR;
 
    sensor_->temperature = 0;
    
    for (uint8_t i = 0; i < TEMPERATURE_NUM_SAMPLES; i++)
        sensor_->tempSamples[i] = 0;
    
    sensor_->countSamples = 0;
    sensor_->firstFullBuffer = false;
    
    sensor_->state = TEMPERATURE_STATE_RESET;
    sensor_->warnings = TEMPERATURE_WARNING_NONE;
    sensor_->accumulatedFailedTime_s = 0.0f;

    sensor_->config.device.reset_f(sensor_->config.device.instance);

    return TEMPERATURE_STATUS_OK; 
}


/*
 * Funciones Privadas
 */
static float promediarMuestras(float* buff_, uint8_t numIterations_) {
    if(numIterations_ == 0)
        return 0;

    float result = 0;
	for (uint8_t i = 0; i < numIterations_; i++)
        result += buff_[i];
	return result / numIterations_;
}

static void temperature_evaluateWarnings(temperatureSensor_t* sensor_) {

    if ((sensor_->warnings & TEMPERATURE_WARNING_HIGH) != 0U) {
        if (sensor_->temperature < (sensor_->config.highTemperature - sensor_->config.difTemperature))
            sensor_->warnings &= ~TEMPERATURE_WARNING_HIGH;
    }
    if ((sensor_->warnings & TEMPERATURE_WARNING_LOW) != 0U) {
        if (sensor_->temperature > (sensor_->config.lowTemperature + sensor_->config.difTemperature))
            sensor_->warnings &= ~TEMPERATURE_WARNING_LOW;
    }

    if (sensor_->temperature > sensor_->config.highTemperature) /* Estamos en el rango de temperatura alta? */
        sensor_->warnings |= TEMPERATURE_WARNING_HIGH;
    else if (sensor_->temperature < sensor_->config.lowTemperature)/* Estamos en el rango de temperatura baja? */
        sensor_->warnings |= TEMPERATURE_WARNING_LOW;

    sensor_->state = sensor_->warnings == TEMPERATURE_WARNING_NONE ? 
        TEMPERATURE_STATE_OK : TEMPERATURE_STATE_WARNING;
}
