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
#include <efl_extension.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef void (*PopupCloseCbFunc)(int nType, bool bPause, void* pUserData);

typedef enum {
	POPUP_STYLE_DEFAULT_NO_CANCEL_BTN = 0x00,
	POPUP_STYLE_DEFAULT_WITH_CANCEL_BTN,
	POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN,
	POPUP_STYLE_MENU_LIST_WITH_CANCEL_BTN,
	POPUP_STYLE_EXPAND_NO_CANCEL_BTN,
	POPUP_STYLE_EXPAND_WITH_CANCEL_BTN,
	POPUP_STYLE_PROGRESS_WITH_NO_CANCEL_BTN,
	POPUP_STYLE_FULLSCREEN_NO_CANCEL_BTN,
	POPUP_STYLE_FULLSCREEN_NO_TITLE_NO_CANCEL_BTN
}video_popup_style_t;

typedef enum
{
	VIDEOS_POPUP_NONE = 0x00,
	VIDEO_POPUP_DEFAULT,
	VIDEO_POPUP_2_TEXT,
	VIDEO_POPUP_2_TEXT_1_ICON,
	VIDEO_POPUP_LOADING_LIST,
	VIDEOS_POPUP_MAX
}video_list_popup_style_t;

Evas_Object *vp_popup_create(Evas_Object *pParent, video_popup_style_t nStyle, char *szTitle, char *szContent, double dTimeOut, Evas_Smart_Cb pTimeoutFunc, Eext_Event_Cb pCancelKeyCb, Evas_Object_Event_Cb pCancelMouseCb, void *pUserData);
Evas_Object *vp_two_button_popup_create( Evas_Object *pParent, char *szTitle, char *szContent, char *pLeftButtonText, Evas_Smart_Cb leftButtonCb, char *pRightButtonText, Evas_Smart_Cb rightButtonCb, const void *pUserData );
#ifdef _SUBTITLE_MULTI_LANGUAGE
Evas_Object *vp_title_two_button_popup_create( Evas_Object *pParent, char *szTitle, char *szContent, char *pLeftButtonText, Evas_Smart_Cb leftButtonCb, char *pRightButtonText, Evas_Smart_Cb rightButtonCb, const void *pUserData );
#endif
bool vp_popup_check_landspace_by_win(Evas_Object *pWin);
void vp_popup_set_popup_min_size(Evas_Object *pWin, Evas_Object *pBox, int nListCount, video_list_popup_style_t eStyle);

