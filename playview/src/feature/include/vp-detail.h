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
#pragma once

#include <stdbool.h>
#include <Elementary.h>

#include <vp-play-popup.h>

typedef void *detail_handle;

typedef struct _vp_detail_info{
	char *szTitle;
	char *szFormat;
	char *szDate;
	char *szSize;
	char *szLastModified;
	char *szResolution;
	char *szLocation;
	char *szLatitude;
	char *szLongitude;

	bool bDrm;
	bool bForwardLock;
	bool bPlayready;
}vp_detail_info;


detail_handle vp_detail_create(Evas_Object *pParent, PopupCloseCbFunc pCloseCb, vp_detail_info *pDetailInfo);
void vp_detail_destroy(detail_handle pDetailHandle);
bool vp_detail_realize(detail_handle pDetailHandle);
bool vp_detail_unrealize(detail_handle pDetailHandle);
bool vp_detail_set_user_data(detail_handle pDetailHandle, void *pUserData);

