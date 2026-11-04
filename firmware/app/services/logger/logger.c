/*
 * Author: Christogarm
 */

#include "logger.h"


logger_status_t logger_init(logger_t* logger_, const logger_config_t* config_){
	if (logger_ == NULL || config_ == NULL || config_->init_f == NULL ||
		config_->addRegisterTime_f == NULL || config_->readTimeLogger_f == NULL ||
		config_->addRegisterEvent_f == NULL  || config_->readEventLogger_f == NULL ||
		config_->instance == NULL || config_->configInstanceTime == NULL || 
		config_->configInstanceEvent == NULL)
		return LOGGER_STATUS_NULL_POINTER;

	if(config_->init_f(config_->instance, config_->configInstanceEvent, config_->configInstanceTime) != LOGGER_STATUS_OK)
		return LOGGER_STATUS_ERROR;

	logger_->config = *config_;
	logger_->init = true;

	return LOGGER_STATUS_OK;
}

logger_status_t logger_addEventRegister(logger_t* logger_) {
	if (logger_ == NULL)
		return LOGGER_STATUS_NULL_POINTER;
	return logger_->config.addRegisterEvent_f(logger_->config.instance, &logger_->eventLog);
}

logger_status_t logger_addTimeRegister(logger_t* logger_) {
	if (logger_ == NULL)
		return LOGGER_STATUS_NULL_POINTER;
	return logger_->config.addRegisterTime_f(logger_->config.instance, &logger_->timeLog);
}

void* logger_readTimeLogger(logger_t* logger_, uint32_t numRegister_) {
	if (logger_ == NULL)
		return NULL;
	return logger_->config.readTimeLogger_f(logger_->config.instance, numRegister_);
}

void* logger_readEventLogger(logger_t* logger_, uint32_t numRegister_) {
	if (logger_ == NULL)
		return NULL;

	return logger_->config.readEventLogger_f(logger_->config.instance, numRegister_);
}

bool logger_isInit(const logger_t* logger_) {
	if (logger_ == NULL)
		return false;
	return logger_->init;
}
