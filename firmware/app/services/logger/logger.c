/*
 * Author: Christogarm
 */

#include "logger.h"


logger_status_t logger_init(logger_t* logger_, const logger_config_t* config_){
	if (logger_ == NULL || config_ == NULL || config_->init_f == NULL ||
		config_->addRegister_f == NULL || config_->readRegister_f == NULL ||
		config_->instance == NULL || config_->instanceConfig == NULL)
		return LOGGER_STATUS_NULL_POINTER;
	logger_->init = false;
	if(config_->init_f(config_->instance, config_->instanceConfig) != LOGGER_STATUS_OK)
		return LOGGER_STATUS_ERROR;

	if(config_->type == LOGGER_TYPE_EVENT){
		logger_->log = &logger_->eventLog;
	}
	else if (config_->type == LOGGER_TYPE_TIME){
		logger_->log = &logger_->timeLog;
	}
	else
		return LOGGER_STATUS_ERROR;

	logger_->config = *config_;
	logger_->init = true;

	return LOGGER_STATUS_OK;
}

logger_status_t logger_addRegister(logger_t* logger_) {
	if (logger_ == NULL)
		return LOGGER_STATUS_NULL_POINTER;
	return logger_->config.addRegister_f(logger_->config.instance, logger_->log);
}

logger_status_t logger_readRegister(logger_t* logger_, uint32_t numRegister_) {
	if (logger_ == NULL)
		return LOGGER_STATUS_NULL_POINTER;
	return logger_->config.readRegister_f(logger_->config.instance, logger_->log, numRegister_);
}
