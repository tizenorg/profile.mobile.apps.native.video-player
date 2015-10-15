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

#include "mp-video-list-view-common.h"

void mp_list_common_set_callabcks(void *viewHandle,
				  Evas_Smart_Cb
				  pGenlistItemtLanguageChangedCb,
				  Evas_Smart_Cb pGenlistItemtRealizeCb,
				  Evas_Smart_Cb pGenlistItemtPressCb,
				  Evas_Smart_Cb pGenlistItemtLongPressCb)
{
	st_VideoListViewMainViewHandle *handle =
		(st_VideoListViewMainViewHandle *) viewHandle;
	if (handle) {
		handle->pGenlistItemtLanguageChangedCb =
			pGenlistItemtLanguageChangedCb;
		handle->pGenlistItemtRealizeCb = pGenlistItemtRealizeCb;
		handle->pGenlistItemtPressCb = pGenlistItemtPressCb;
		handle->pGenlistItemtLongPressCb = pGenlistItemtLongPressCb;
	}
}

void mp_list_common_unset_callabcks(void *viewHandle)
{
	st_VideoListViewMainViewHandle *handle =
		(st_VideoListViewMainViewHandle *) viewHandle;
	if (handle) {
		handle->pGenlistItemtLanguageChangedCb = NULL;
		handle->pGenlistItemtRealizeCb = NULL;
		handle->pGenlistItemtPressCb = NULL;
		handle->pGenlistItemtLongPressCb = NULL;
	}
}
