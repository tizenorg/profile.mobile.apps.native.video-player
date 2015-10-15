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
//#include <Ecore_X.h>

#include "vp-play-type-define.h"

typedef void *play_view_handle;

typedef void (*vp_play_view_realize_cb) (void *pUserData);
typedef void (*vp_play_view_destroy_cb) (void *pUserData);
typedef Eina_Bool (*vp_play_view_event_key_down_cb) (void *pUserData, int nType, void *pEvent);
typedef Eina_Bool (*vp_play_view_event_key_up_cb) (void *pUserData, int nType, void *pEvent);


play_view_handle vp_play_view_create(Evas_Object *pParent, Ecore_X_Window nParentXID, video_play_mode_t nMode, video_play_launching_type_t nLaunchingType);
void vp_play_view_destroy(play_view_handle pViewHandle);
#ifdef _PERF_TEST_
void vp_play_view_reset(play_view_handle pViewHandle, int nMode);
#endif
bool vp_play_view_realize(play_view_handle pViewHandle);
bool vp_play_view_live_stream_realize(play_view_handle pViewHandle);
bool vp_play_view_unrealize(play_view_handle pViewHandle);
bool vp_play_view_is_realize(play_view_handle pViewHandle, bool *bIsRealize);
bool vp_play_view_set_user_data(play_view_handle pViewHandle, void *pUserData);
bool vp_play_view_set_url(play_view_handle pViewHandle, const char *szMediaURL);
bool vp_play_view_get_url(play_view_handle pViewHandle, char **szMediaURL);
bool vp_play_view_get_visible_status(play_view_handle pViewHandle, bool *bVisible);
bool vp_play_view_set_device_id(play_view_handle pViewHandle, const char *szDeviceID);
bool vp_play_view_set_dmr_id(play_view_handle pViewHandle, const char *szDMRID);
bool vp_play_view_set_media_title(play_view_handle pViewHandle, const char *szMediaTitle);
bool vp_play_view_set_play_mode(play_view_handle pViewHandle, video_play_mode_t nMode);
bool vp_play_view_set_start_position(play_view_handle pViewHandle, int nPosition);
bool vp_play_view_set_rotate(play_view_handle pViewHandle, video_play_rotate_t nRotate);
bool vp_play_view_get_popup_mode(play_view_handle pViewHandle, bool *bPopupMode);
bool vp_play_view_set_relaunch_caller(play_view_handle pViewHandle, bool bRelaunch);
bool vp_play_view_set_launching_mode(play_view_handle pViewHandle, video_play_launching_type_t nLaunchingType);
bool vp_play_view_set_list_mode(play_view_handle pViewHandle, video_play_list_type_t nListType);
bool vp_play_view_set_cookie(play_view_handle pViewHandle, const char *szCookie);
bool vp_play_view_set_proxy(play_view_handle pViewHandle, const char *szProxy);
bool vp_play_view_set_store_download_mode(play_view_handle pViewHandle, bool bStoreDownload);
bool vp_play_view_set_stop_after_repeat_mode(play_view_handle pViewHandle, bool bStopAfterRepeat);
bool vp_play_view_set_store_download_filesize(play_view_handle pViewHandle, double dStoreDL_FileSize);
bool vp_play_view_set_store_order_info(play_view_handle pViewHandle, char *pStoreOrderId, char *pStoreServerId, char *pStoreAppId, char *pStoreMvId);

bool vp_play_view_add_multi_path(play_view_handle pViewHandle, const char *szURL, char *szTitle, char *szSubTitle, int nPosition, int nDuration, bool bIsSameAP);

bool vp_play_view_update(play_view_handle pViewHandle);

bool vp_play_view_set_callback(play_view_handle pViewHandle, video_play_callback_type_t nCbType, void *pFunc);
bool vp_play_view_unset_callback(play_view_handle pViewHandle, video_play_callback_type_t nCbType);

Evas_Object* vp_play_view_get_main_window(play_view_handle pViewHandle);
bool vp_play_view_change_style_minimode_to_fullmode(play_view_handle pViewHandle, video_play_launching_type_t nLaunchingType);

