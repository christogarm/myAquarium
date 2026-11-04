/*
 * Author: Christogarm
 *
 * timeSim.c
 */

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "timeSim.h"

#define TIME_SIM_DAYS_PER_WEEK      (7)
#define TIME_SIM_MAX_WEEK           (52)
#define TIME_SIM_TM_YEAR_BASE       (1900)

/* -------------------------------------------------------------------------- */
static bool time_sim_config_is_valid(const time_sim_config_t* cfg)
{
	bool valid = true;

	if ((cfg->utc_offset_seconds < TIME_SIM_UTC_OFFSET_MIN_S) ||
		(cfg->utc_offset_seconds > TIME_SIM_UTC_OFFSET_MAX_S))
	{
		valid = false;
	}

	return valid;
}

/* -------------------------------------------------------------------------- */
time_status_t time_sim_init(void* instance_, const void* config_)
{
	time_status_t status = TIME_STATUS_OK;
	time_sim_t* self = NULL;
	const time_sim_config_t* cfg = NULL;
	time_t now;

	if ((instance_ == NULL) || (config_ == NULL))
	{
		status = TIME_STATUS_NULL_POINTER;
	}
	else
	{
		self = (time_sim_t*)instance_;
		cfg  = (const time_sim_config_t*)config_;

		if (!time_sim_config_is_valid(cfg))
		{
			status = TIME_STATUS_ERROR;
		}
		else
		{
			now = time(NULL);

			if (now == (time_t)(-1))
			{
				status = TIME_STATUS_ERROR;
			}
			else
			{
				self->config = *cfg;

				if (cfg->use_start_time)
				{
					self->state.start_delta_seconds =
						(int64_t)cfg->start_unix - (int64_t)now;
				}
				else
				{
					self->state.start_delta_seconds = 0;
				}

				self->state.initialized = true;
			}
		}
	}

	return status;
}

/* -------------------------------------------------------------------------- */
time_status_t time_sim_get_time(void* instance, timeData_t* timeData)
{
	time_status_t status = TIME_STATUS_OK;
	const time_sim_t* self = NULL;
	time_t now;
	int64_t utc_s;
	int64_t local_s;
	time_t local_t;
	const struct tm* tm_local = NULL;
	int32_t yday;
	int32_t week;

	if ((instance == NULL) || (timeData == NULL))
	{
		status = TIME_STATUS_NULL_POINTER;
	}
	else
	{
		self = (const time_sim_t*)instance;

		if (!self->state.initialized)
		{
			status = TIME_STATUS_ERROR;
		}
		else
		{
			now = time(NULL);

			if (now == (time_t)(-1))
			{
				status = TIME_STATUS_ERROR;
			}
			else
			{
				utc_s   = (int64_t)now + self->state.start_delta_seconds;
				local_s = utc_s + (int64_t)self->config.utc_offset_seconds;

				/* timeUNIX es uint32_t: el tiempo UTC debe caber en ese rango */
				if ((utc_s < 0) || (utc_s > (int64_t)UINT32_MAX) || (local_s < 0))
				{
					status = TIME_STATUS_ERROR;
				}
				else
				{
					local_t  = (time_t)local_s;
					tm_local = gmtime(&local_t);

					if (tm_local == NULL)
					{
						status = TIME_STATUS_ERROR;
					}
					else
					{
						yday = (int32_t)tm_local->tm_yday;	/* 0-365 */
						week = (yday / TIME_SIM_DAYS_PER_WEEK) + 1;

						if (week > TIME_SIM_MAX_WEEK)
						{
							week = TIME_SIM_MAX_WEEK;		/* Rango de la interfaz: 1-52 */
						}

						timeData->timeUNIX = (uint32_t)utc_s;
						timeData->year     = (uint8_t)((tm_local->tm_year + TIME_SIM_TM_YEAR_BASE) % 100);
						timeData->month    = (uint8_t)(tm_local->tm_mon + 1);
						timeData->week     = (uint8_t)week;
						timeData->weekDay  = (uint8_t)tm_local->tm_wday;	/* 0 = domingo */
						timeData->day      = (uint8_t)tm_local->tm_mday;
						timeData->hour     = (uint8_t)tm_local->tm_hour;
						timeData->minute   = (uint8_t)tm_local->tm_min;
						timeData->second   = (uint8_t)tm_local->tm_sec;
					}
				}
			}
		}
	}

	return status;
}
