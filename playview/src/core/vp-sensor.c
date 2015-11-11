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

#ifdef OLD_SENSOR_API

#include <sensor.h>

#include "vp-play-macro-define.h"

#include "vp-sensor.h"

/* check temp */
#include "vp-play-log.h"


typedef struct _SensorHandle {
	sensor_h pSensorH;
	bool bIsRealize;
	bool bFaceDownEnable;
	bool bDoubleTabEnable;

	void *pUserData;
	SensorEventCbFunc pEventCb;

} SensorHandle;

static void _vp_sensor_destroy_handle(SensorHandle *pSensorHandle);


/* callback functions */
static void __vp_sensor_motion_doubletap_event_cb(unsigned long long
        timestamp,
        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SensorHandle *pSensor = (SensorHandle *) pUserData;
	if (pSensor->pEventCb) {
		pSensor->pEventCb(VP_SENSOR_TYPE_DOUBLETAP, timestamp,
		                  pSensor->pUserData);
	}

}

static void __vp_sensor_motion_facedown_event_cb(unsigned long long
        timestamp,
        void *pUserData)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SensorHandle *pSensor = (SensorHandle *) pUserData;

	if (pSensor->pEventCb) {
		pSensor->pEventCb(VP_SENSOR_TYPE_FACEDOWN, timestamp,
		                  pSensor->pUserData);
	}
}




/* internal functions */
static void _vp_sensor_destroy_handle(SensorHandle *pSensorHandle)
{
	if (pSensorHandle == NULL) {
		VideoLogError("pSensorHandle is NULL");
		return;
	}

	int nRet = sensor_destroy(pSensorHandle->pSensorH);
	if (nRet != SENSOR_ERROR_NONE) {
		VideoLogInfo("sensor_destroy is fail : 0x%x", nRet);
	}


	VP_FREE(pSensorHandle);
}



/* external functions */
sensor_handle vp_sensor_create(SensorEventCbFunc pEventCb)
{
	SensorHandle *pSensor = NULL;

	pSensor = calloc(1, sizeof(SensorHandle));

	if (pSensor == NULL) {
		VideoLogError("pSensorHandle alloc fail");
		return NULL;
	}

	pSensor->pEventCb = pEventCb;

	VideoLogInfo(">> sensor create");

	int nRet = sensor_create(&pSensor->pSensorH);
	if (nRet != SENSOR_ERROR_NONE) {
		VideoLogInfo("sensor_create is fail : 0x%x", nRet);
		_vp_sensor_destroy_handle(pSensor);
		return NULL;
	}
	VideoLogInfo("<< sensor create");

	if (pSensor->pSensorH == NULL) {
		VideoLogInfo("pSensorHandle->pSensorH is NULL");
		_vp_sensor_destroy_handle(pSensor);
		return NULL;
	}

	return (sensor_handle) pSensor;
}

void vp_sensor_destroy(sensor_handle pSensorHandle)
{
	if (pSensorHandle == NULL) {
		VideoLogError("pSensorHandle is NULL");
		return;
	}

	SensorHandle *pSensor = (SensorHandle *) pSensorHandle;

	vp_sensor_unrealize(pSensorHandle);

	_vp_sensor_destroy_handle(pSensor);
}

bool vp_sensor_realize(sensor_handle pSensorHandle)
{
	if (pSensorHandle == NULL) {
		VideoLogError("pSensorHandle is NULL");
		return FALSE;
	}

	VideoLogInfo(">> Sensor realize");

	SensorHandle *pSensor = (SensorHandle *) pSensorHandle;
	bool bReturn = TRUE;
	int nRet = SENSOR_ERROR_NONE;
	bool bEnableSensor = FALSE;
	if (pSensor->bIsRealize) {
		VideoLogWarning("Already Realize");
		return TRUE;
	}

	nRet = sensor_is_supported(SENSOR_MOTION_DOUBLETAP, &bEnableSensor);
	if (nRet != SENSOR_ERROR_NONE) {
		VideoLogInfo("sensor_is_supported is fail : 0x%x", nRet);
		bReturn = FALSE;
	}

	if (bEnableSensor) {
		nRet =
		    sensor_motion_doubletap_set_cb(pSensor->pSensorH,
		                                   __vp_sensor_motion_doubletap_event_cb,
		                                   (void *) pSensor);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_motion_doubletap_set_cb is fail : 0x%x",
			             nRet);
			bReturn = FALSE;
		}

		nRet = sensor_start(pSensor->pSensorH, SENSOR_MOTION_DOUBLETAP);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_start is fail : 0x%x", nRet);
			bReturn = FALSE;
		}
	}
	pSensor->bFaceDownEnable = bEnableSensor;

	nRet = sensor_is_supported(SENSOR_MOTION_FACEDOWN, &bEnableSensor);
	if (nRet != SENSOR_ERROR_NONE) {
		VideoLogInfo("sensor_is_supported is fail : 0x%x", nRet);
		bReturn = FALSE;
	}
	if (bEnableSensor) {
		nRet =
		    sensor_motion_facedown_set_cb(pSensor->pSensorH,
		                                  __vp_sensor_motion_facedown_event_cb,
		                                  (void *) pSensor);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_motion_facedown_set_cb is fail : 0x%x",
			             nRet);
			bReturn = FALSE;
		}

		nRet = sensor_start(pSensor->pSensorH, SENSOR_MOTION_FACEDOWN);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_start is fail : 0x%x", nRet);
			bReturn = FALSE;
		}
	}
	pSensor->bDoubleTabEnable = bEnableSensor;

	pSensor->bIsRealize = TRUE;

	VideoLogInfo("<< Sensor realize");

	return bReturn;
}

bool vp_sensor_unrealize(sensor_handle pSensorHandle)
{
	if (pSensorHandle == NULL) {
		VideoLogError("pSensorHandle is NULL");
		return FALSE;
	}

	SensorHandle *pSensor = (SensorHandle *) pSensorHandle;
	bool bReturn = TRUE;
	int nRet = SENSOR_ERROR_NONE;

	if (pSensor->bIsRealize == FALSE) {
		VideoLogWarning("Not yet Realize");
		return TRUE;
	}

	if (pSensor->bDoubleTabEnable) {
		nRet = sensor_motion_doubletap_unset_cb(pSensor->pSensorH);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo
			("sensor_motion_doubletap_unset_cb is fail : 0x%x", nRet);
			bReturn = FALSE;
		}

		nRet = sensor_stop(pSensor->pSensorH, SENSOR_MOTION_DOUBLETAP);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_stop is fail : 0x%x", nRet);
			bReturn = FALSE;
		} else {
			pSensor->bDoubleTabEnable = FALSE;
		}

	}
	if (pSensor->bFaceDownEnable) {
		nRet = sensor_motion_facedown_unset_cb(pSensor->pSensorH);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_motion_facedown_unset_cb is fail : 0x%x",
			             nRet);
			bReturn = FALSE;
		}

		nRet = sensor_stop(pSensor->pSensorH, SENSOR_MOTION_FACEDOWN);
		if (nRet != SENSOR_ERROR_NONE) {
			VideoLogInfo("sensor_stop is fail : 0x%x", nRet);
			bReturn = FALSE;
		} else {
			pSensor->bFaceDownEnable = FALSE;
		}

	}

	pSensor->bIsRealize = FALSE;

	return bReturn;
}

bool vp_sensor_set_user_data(sensor_handle pSensorHandle, void *pUserData)
{
	if (pSensorHandle == NULL) {
		VideoLogError("pSensorHandle is NULL");
		return FALSE;
	}

	SensorHandle *pSensor = (SensorHandle *) pSensorHandle;

	pSensor->pUserData = pUserData;

	return TRUE;
}

#endif
