/*
 * Author: Christogarm
 */

#include "heater.h"

/* Prototipos de las Funciones Privadas */
static void heater_applyRateLimit(heater_t* heater_, float output_, float time_s);

/* Funciones Públicas */
heater_status_t heater_init(heater_t* heater_, const heater_config_t* config_) {
	if (heater_ == NULL || config_ == NULL ||
		config_->init_f == NULL || config_->setOutput_f == NULL)
		return HEATER_STATUS_NULL_POINTER;
	
	if(config_->rateLimit < 0 || config_->minOutput > config_->maxOutput)
		return HEATER_STATUS_ERROR;

	if (config_->init_f(config_->instance, config_->configInstance) != HEATER_STATUS_OK)
		return HEATER_STATUS_ERROR;

	heater_->config = *config_;
	heater_->power = 0.0f;
	heater_->init = true;

	return HEATER_STATUS_OK;
}


heater_status_t heater_setOutput(heater_t* heater_, float output_, float time_s) {
	if (heater_ == NULL)
		return HEATER_STATUS_NULL_POINTER;
	if (heater_->init == false)
		return HEATER_STATUS_ERROR;
	if (time_s < 0)
		return HEATER_STATUS_ERROR;

	heater_applyRateLimit(heater_, output_, time_s);
	heater_->config.setOutput_f(heater_->config.instance, heater_->power);
	
	return HEATER_STATUS_OK;
}

/* Funciones Privadas */
static void heater_applyRateLimit(heater_t* heater_, float output_, float time_s) {
	float maxChange = heater_->config.rateLimit * time_s;

	if (output_ > heater_->power)
	{
		heater_->power += maxChange;

		if (heater_->power > heater_->config.maxOutput)
			heater_->power = heater_->config.maxOutput;
		
	}
	else if (output_ < heater_->power)
	{
		heater_->power -= maxChange;

		if (heater_->power < heater_->config.minOutput)
			heater_->power = heater_->config.minOutput;
	}
}