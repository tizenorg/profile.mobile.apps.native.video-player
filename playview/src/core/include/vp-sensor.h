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

#pragma once

#include <stdbool.h>
#include <Elementary.h>


typedef enum {
	VP_SENSOR_TYPE_NONE= 0x00,
	VP_SENSOR_TYPE_FACEDOWN,
	VP_SENSOR_TYPE_DOUBLETAP,
}vp_sensor_type_t;


typedef void (*SensorEventCbFunc)(vp_sensor_type_t nType, unsigned long long timestamp, void* pUserData);


typedef void *sensor_handle;


sensor_handle vp_sensor_create(SensorEventCbFunc pEventCb);
void vp_sensor_destroy(sensor_handle pSensorHandle);
bool vp_sensor_realize(sensor_handle pSensorHandle);
bool vp_sensor_unrealize(sensor_handle pSensorHandle);
bool vp_sensor_set_user_data(sensor_handle pSensorHandle, void *pUserData);

#endif
