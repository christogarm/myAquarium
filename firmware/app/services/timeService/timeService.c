/*
 * Author: Christogarm
 */

#include "timeService.h"

time_status_t timeService_init(timeService_t* timeService_, const timeService_config_t* config_) {
	if (timeService_ == NULL || config_ == NULL ||
		config_->init_f == NULL || config_->getTime_f == NULL)
		return TIME_STATUS_NULL_POINTER;
	
	if (config_->init_f(config_->instance, config_->configInstance) != TIME_STATUS_OK)
		return TIME_STATUS_ERROR;

	timeService_->config = *config_;
	timeService_->init = true;

	return TIME_STATUS_OK;
}


time_status_t timeService_getTime(timeService_t* timeService_) {
	if (timeService_ == NULL)
		return TIME_STATUS_NULL_POINTER;

	if(!timeService_->init)
		return TIME_STATUS_ERROR;

	time_status_t status = timeService_->config.getTime_f(timeService_->config.instance, &timeService_->timeData);
	return status;
}
