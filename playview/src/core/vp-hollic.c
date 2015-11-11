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

#include "vp-hollic.h"
#include "vp-play-macro-define.h"
#include "vp-play-log.h"

#define OBJECT_PATH    			"/Org/Tizen/System/DeviceD"
#define DEVICED_PATH_HALL		OBJECT_PATH"/Hall"
#define SIGNAL_HALL_STATE		"ChangeState"

typedef struct _HollIC_handle {
	void *pDbus_connection;
	void *pSignal_handler;

	void *pUserData;
	HollICEventCbFunc pEventCb;
} HollIC_handle;

hollic_handle vp_hollic_create_handle(HollICEventCbFunc pEventCb,
                                      void *pUserData)
{
	return NULL;
}

void vp_hollic_destroy_handle(hollic_handle pHollICHandle)
{
	if (pHollICHandle == NULL) {
		VideoLogError("pHollICHandle is NULL");
		return;
	}

	return;
}
