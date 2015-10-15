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

typedef void *share_handle;

typedef enum {
	VP_SHARE_NONE = 0x00,
	VP_SHARE_MESSAGE,
	VP_SHARE_EMAIL,
	VP_SHARE_BLUETOOTH,
	VP_SHARE_FTM,
	VP_SHARE_FACEBOOK,
	VP_SHARE_YOUTUBE,
	VP_SHARE_NFC,
	VP_SHARE_FILE_TRANSFER,
	VP_SHARE_DROP_BOX,
}video_share_type_t;

share_handle vp_share_create(Evas_Object *pParent, char *pMediaUrl, PopupCloseCbFunc pCloseCb);
void vp_share_destroy(share_handle pShareHandle);
bool vp_share_realize(share_handle pShareHandle);
bool vp_share_unrealize(share_handle pShareHandle);
bool vp_share_set_user_data(share_handle pShareHandle, void *pUserData);


