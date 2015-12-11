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

#include <glib.h>
#include <stdbool.h>
#include "mp-util-widget-ctrl.h"
#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-info-ctrl.h"
#include "mp-rotate-ctrl.h"


/*Focus UI*/
void mp_widget_ctrl_navi_item_layout_focus_out(Evas_Object *pFocusLayout)
{
	if (pFocusLayout == NULL) {
		VideoLogError("pFocusLayout is NULL");
		return;
	}

	elm_object_focus_set(pFocusLayout, EINA_FALSE);
	elm_object_tree_focus_allow_set(pFocusLayout, EINA_FALSE);
	elm_object_focus_allow_set(pFocusLayout, EINA_FALSE);
}

void mp_widget_ctrl_navi_item_layout_focus_in(Evas_Object *pFocusLayout)
{
	if (pFocusLayout == NULL) {
		VideoLogError("pFocusLayout is NULL");
		return;
	}

	elm_object_focus_allow_set(pFocusLayout, EINA_TRUE);
	elm_object_tree_focus_allow_set(pFocusLayout, EINA_TRUE);
	elm_object_focus_set(pFocusLayout, EINA_TRUE);
}

void mp_widget_ctrl_enable_navi_handle_focus(Evas_Object *pNaviHandle)
{
	return;
}


void mp_widget_ctrl_disable_navi_handle_focus_except_item(Evas_Object *
		pNaviHandle,
		Elm_Object_Item
		*
		pCurentNaviItem)
{
	return;
}

void mp_widget_ctrl_set_popup_min_size(Evas_Object *pBox, int nListCount,
				       euPopUpStyle eStyle)
{
	if (pBox) {
		int nMinHeight = 0;
		switch (eStyle) {
		case VIDEOS_POPUP_114:
			if (mp_rotate_ctrl_check_landspace_by_win
					((Evas_Object *) mp_util_get_main_window_handle())) {
				nMinHeight = VIDEO_POPUP_LIST_HEIGHT_L(nListCount);
			} else {
				nMinHeight = VIDEO_POPUP_LIST_HEIGHT(nListCount);
			}
			break;
		case VIDEOS_POPUP_141:
			if (mp_rotate_ctrl_check_landspace_by_win
					((Evas_Object *) mp_util_get_main_window_handle())) {
				nMinHeight = VIDEO_POPUP_2_TEXT_HEIGHT_L(nListCount);
			} else {
				nMinHeight = VIDEO_POPUP_2_TEXT_HEIGHT(nListCount);
			}
			break;

		default:
			VideoLogError("invalid style =%d.", eStyle);
			break;
		}

		evas_object_size_hint_min_set(pBox,
					      VIDEO_POPUP_ITEM_W *
					      elm_config_scale_get(),
					      nMinHeight *
					      elm_config_scale_get());
	}
}

Evas_Object *mp_widget_ctrl_create_navi_btn(Evas_Object *pParent,
		const char *pIconPath,
		Evas_Smart_Cb pFunc,
		void *pUserData)
{
	if (!pParent) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (!pIconPath) {
		VideoLogError("pIconPath is NULL");
		return NULL;
	}

	VideoLogInfo("");
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_IMAGE_NAME_EDJ);
	free(path);
	Evas_Object *pImage = elm_image_add(pParent);
	elm_image_file_set(pImage, edj_path, pIconPath);
	elm_image_resizable_set(pImage, EINA_TRUE, EINA_TRUE);
	evas_object_show(pImage);

	Evas_Object *pTitleBtn = elm_button_add(pParent);
	elm_object_style_set(pTitleBtn, "naviframe/title_icon");
	elm_object_part_content_set(pTitleBtn, "icon", pImage);
	evas_object_smart_callback_add(pTitleBtn, "clicked", pFunc,
				       pUserData);

	return pTitleBtn;
}

Evas_Object *mp_widget_ctrl_create_navi_left_btn(Evas_Object *pParent,
		Elm_Object_Item *
		pNaviItem,
		const char *pIconPath,
		Evas_Smart_Cb pFunc,
		void *pUserData)
{

	if (!pParent || !pNaviItem) {
		VideoLogError("parent is NULL.");
		return NULL;
	}

	VideoLogInfo("");

	Evas_Object *pLeftbtn = NULL;
	if (pIconPath) {
		pLeftbtn =
			mp_widget_ctrl_create_navi_btn(pParent, pIconPath, pFunc,
						       pUserData);
	} else {
		pLeftbtn = elm_button_add(pParent);
		elm_object_style_set(pLeftbtn, "naviframe/title_cancel");
		evas_object_smart_callback_add(pLeftbtn, "clicked", pFunc,
					       pUserData);
	}

	if (!pLeftbtn) {
		VideoLogError("[ERR] Fail to create pLeftbtn");
		return NULL;
	}

	elm_object_item_part_content_set(pNaviItem, "title_left_btn",
					 pLeftbtn);

	evas_object_show(pLeftbtn);

	return pLeftbtn;
}


Evas_Object *mp_widget_ctrl_create_navi_right_btn(Evas_Object *pParent,
		Elm_Object_Item *
		pNaviItem,
		const char *pIconPath,
		Evas_Smart_Cb pFunc,
		void *pUserData)
{

	if (!pParent || !pNaviItem) {
		VideoLogError("parent is NULL.");
		return NULL;
	}


	VideoLogInfo("");
	Evas_Object *pRightbtn = NULL;
	if (pIconPath) {
		pRightbtn =
			mp_widget_ctrl_create_navi_btn(pParent, pIconPath, pFunc,
						       pUserData);
	} else {
		pRightbtn = elm_button_add(pParent);
		elm_object_style_set(pRightbtn, "naviframe/title_done");
		evas_object_smart_callback_add(pRightbtn, "clicked", pFunc,
					       pUserData);
	}

	if (!pRightbtn) {
		VideoLogError("[ERR] Fail to create pRightbtn");
		return NULL;
	}

	elm_object_item_part_content_set(pNaviItem, "title_right_btn",
					 pRightbtn);

	evas_object_show(pRightbtn);

	return pRightbtn;

}

bool mp_widget_ctrl_create_select_all_layout(Evas_Object *pParent,
		Evas_Smart_Cb pChangeFunc,
		Evas_Object_Event_Cb
		pMouseDownFunc,
		void *pUserData,
		Evas_Object **pCheckBox,
		Evas_Object **pSelectLayout)
{
	if (!pParent) {
		VideoLogError("parent is NULL.");
		return FALSE;
	}

	Evas_Object *pSelectAllLayout = elm_layout_add(pParent);
	elm_layout_theme_set(pSelectAllLayout, "genlist", "item",
			     "select_all/default");
	/*elm_layout_file_set(pSelectAllLayout, VIDEO_CUSTOM_THEME, "select.all.layout");*/
	evas_object_size_hint_weight_set(pSelectAllLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_FILL);
	evas_object_size_hint_align_set(pSelectAllLayout, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_event_callback_add(pSelectAllLayout,
				       EVAS_CALLBACK_MOUSE_DOWN,
				       pMouseDownFunc, NULL);
	*pSelectLayout = pSelectAllLayout;

	Evas_Object *pSelectAllCheckbox = elm_check_add(pSelectAllLayout);
	/*elm_check_state_pointer_set(pSelectAllCheckbox, (void *)FALSE);*/
	evas_object_smart_callback_add(pSelectAllCheckbox, "changed",
				       pChangeFunc, NULL);
	evas_object_propagate_events_set(pSelectAllCheckbox, EINA_FALSE);
	elm_object_part_content_set(pSelectAllLayout, "elm.icon",
				    pSelectAllCheckbox);
	elm_object_domain_translatable_part_text_set(pSelectAllLayout,
			"elm.text.main",
			VIDEOS_STRING,
			VIDEOS_SELECTVIEW_BODY_SELECT_ALL_IDS);
	evas_object_show(pSelectAllLayout);
	*pCheckBox = pSelectAllCheckbox;
	return TRUE;

}

Evas_Object *mp_widget_ctrl_create_progressbar(Evas_Object *pParent,
		double dRadio)
{
	if (!pParent) {
		return NULL;
	}
	Evas_Object *pProgressBar = elm_progressbar_add(pParent);
	elm_object_style_set(pProgressBar,
			     "elm/progressbar/horizontal/default");
	elm_progressbar_horizontal_set(pProgressBar, EINA_TRUE);
	elm_progressbar_span_size_set(pProgressBar,
				      VIDEO_LIST_PROGRESSBAR_LENGTH);
	evas_object_size_hint_align_set(pProgressBar, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pProgressBar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_value_set(pProgressBar, dRadio);
	return pProgressBar;
}
