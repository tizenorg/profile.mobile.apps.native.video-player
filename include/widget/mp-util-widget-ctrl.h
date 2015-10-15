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

#ifndef __MP_UTIL_WIDGET_CTRL__
#define __MP_UTIL_WIDGET_CTRL__

#include <Eina.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

typedef enum
{
	VIDEOS_POPUP_NONE = 0x00,
	VIDEOS_POPUP_114,
	VIDEOS_POPUP_141,
	VIDEOS_POPUP_MAX
}euPopUpStyle;


void mp_widget_ctrl_enable_navi_handle_focus(Evas_Object *pNaviHandle);
void mp_widget_ctrl_disable_navi_handle_focus_except_item(Evas_Object *pNaviHandle, Elm_Object_Item *pCurentNaviItem);
void mp_widget_ctrl_set_popup_min_size(Evas_Object *pBox, int nListCount, euPopUpStyle eStyle);
void mp_widget_ctrl_navi_item_layout_focus_out(Evas_Object *pFocusLayout);
void mp_widget_ctrl_navi_item_layout_focus_in(Evas_Object *pFocusLayout);
Evas_Object* mp_widget_ctrl_create_navi_btn(Evas_Object *pParent, const char *pIconPath, Evas_Smart_Cb pFunc, void *pUserData);

Evas_Object* mp_widget_ctrl_create_navi_left_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
	const char *pIconPath, Evas_Smart_Cb pFunc, void *pUserData);

Evas_Object* mp_widget_ctrl_create_navi_right_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
	const char *pIconPath, Evas_Smart_Cb pFunc, void *pUserData);

bool mp_widget_ctrl_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc,
	Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout);

Evas_Object * mp_widget_ctrl_create_progressbar(Evas_Object *pParent, double dRadio);

#endif
