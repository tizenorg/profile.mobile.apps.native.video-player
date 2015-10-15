/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <device/display.h>
#include <device/power.h>


#include "vp-device.h"
#include "vp-play-macro-define.h"

/* check temp */
#include "vp-play-log.h"

bool vp_device_display_lcd_lock(void)
{
	int nRet = 0;
	nRet = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
	if (nRet != 0) {
		VideoLogError
		("device_power_request_lock(POWER_LOCK_DISPLAY, 0) set fail : %d",
		 nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_device_display_lcd_unlock(void)
{
	int nRet = 0;
	nRet = device_power_release_lock(POWER_LOCK_DISPLAY);
	if (nRet != 0) {
		VideoLogError
		("device_power_release_lock(POWER_LOCK_DISPLAY) set fail : %d",
		 nRet);
		return FALSE;
	}

	return TRUE;
}


bool vp_device_set_frame_rate(int nRate)
{
	return TRUE;
}

bool vp_device_set_brightness(int nVal)
{
	int nRet = 0;
	nRet = device_display_set_brightness(0, nVal);	// 0 is main display, brightness
	if (nRet != 0) {
		VideoLogError("device_display_set_brightness set fail : %d",
			      nRet);
		return FALSE;
	}

	return TRUE;
}

bool vp_device_get_brightness(int *nCurVal)
{
	int nVal = 0;
	int nRet = 0;
	nRet = device_display_get_brightness(0, &nVal);
	if (nVal < 0 || nRet != DEVICE_ERROR_NONE) {
		VideoLogError("device_display_get_brightness set fail : %d",
			      nVal);
		return FALSE;
	}

	*nCurVal = nVal;

	return TRUE;
}


bool vp_device_get_max_brightness(int *nMaxVal)
{
	int nVal = 0;
	int nRet = 0;
	nRet = device_display_get_max_brightness(0, &nVal);
	if (nVal < 0 || nRet != DEVICE_ERROR_NONE) {
		VideoLogError("device_display_get_max_brightness get fail : %d",
			      nVal);
		return FALSE;
	}

	*nMaxVal = nVal;

	return TRUE;
}

bool vp_device_get_min_brightness(int *nMinVal)
{
	int nVal = 0;

	*nMinVal = nVal;

	return TRUE;
}

bool vp_device_set_screen_off(bool bLock)
{
	if (bLock) {
		if (device_power_request_lock(POWER_LOCK_CPU, 0) !=
				DEVICE_ERROR_NONE) {
			VideoLogError("lock failed");
			return FALSE;
		}
	} else {
		if (device_power_release_lock(POWER_LOCK_CPU) !=
				DEVICE_ERROR_NONE) {
			VideoLogError("unlock failed");
			return FALSE;
		}
	}

	return TRUE;
}
