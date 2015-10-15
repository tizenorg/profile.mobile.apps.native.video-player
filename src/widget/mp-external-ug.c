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


#include <app.h>
#include <Elementary.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-external-ug.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"

void mp_external_service_share_panel(const char *pUrl)
{
	if (!pUrl) {
		VideoLogInfo("pUrl is null.");
		return;
	}

	int nErr = 0;
	app_control_h pService = NULL;
	char *pOperation = VIDEO_SHARE_OPERATION_SINGLE;

	VideoSecureLogInfo("share pUrl = %s", pUrl);

	nErr = app_control_create(&pService);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogError("Fail to create ug service handle - [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}

	nErr = app_control_set_uri(pService, pUrl);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogError("Fail to set uri into ug service handle - [0x%x]",
			      nErr);
		goto ERROR_EXCEPTION;
	}

	nErr = app_control_set_operation(pService, pOperation);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogError("app_control_set_operation.. [0x%x]", nErr);
		goto ERROR_EXCEPTION;
	}

	nErr = app_control_send_launch_request(pService, NULL, NULL);
	if (nErr != APP_CONTROL_ERROR_NONE) {
		VideoLogWarning("[WARNING] rvice_send_launch_request().. [0x%x]",
				nErr);
		goto ERROR_EXCEPTION;
	}

	app_control_destroy(pService);
	pService = NULL;


ERROR_EXCEPTION:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return;
}

