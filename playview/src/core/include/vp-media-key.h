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


typedef enum {
	VP_MEDIA_KEY_PLAY = 0x00,
	VP_MEDIA_KEY_PAUSE,
	VP_MEDIA_KEY_PLAYPAUSE,
	VP_MEDIA_KEY_PREVIOUS,
	VP_MEDIA_KEY_REWIND,
	VP_MEDIA_KEY_NEXT,
	VP_MEDIA_KEY_FASTFORWARD,
	VP_MEDIA_KEY_STOP,
}vp_media_key_event_t;


typedef void (*MediaKeyEventCbFunc)(vp_media_key_event_t nKey, bool bRelease, void* pUserData);


typedef void *media_key_handle;


media_key_handle vp_media_key_create(Evas_Object *pWin, MediaKeyEventCbFunc pEventCb);
void vp_media_key_destroy(media_key_handle pMediaKeyHandle);
bool vp_media_key_realize(media_key_handle pMediaKeyHandle);
bool vp_media_key_unrealize(media_key_handle pMediaKeyHandle);
bool vp_media_key_set_user_data(media_key_handle pMediaKeyHandle, void *pUserData);

