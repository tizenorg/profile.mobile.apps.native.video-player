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

#include "vp-play-value-define.h"

#include "vp-play-popup.h"
#include "vp-play-macro-define.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-util.h"

Evas_Object *vp_popup_create(Evas_Object *pParent,
                             video_popup_style_t nStyle, char *szTitle,
                             char *szContent, double dTimeOut,
                             Evas_Smart_Cb pTimeoutFunc,
                             Eext_Event_Cb pCancelKeyCb,
                             Evas_Object_Event_Cb pCancelMouseCb,
                             void *pUserData)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}
	Evas_Object *pObj = NULL;

	pObj = elm_popup_add(pParent);

	if (pObj == NULL) {
		VideoLogError("elm_popup_add is fail");
		return NULL;
	}

	/*if (nStyle == POPUP_STYLE_DEFAULT) {
	   //elm_object_style_set(pObj, "popup/default");
	   }
	   else if (nStyle == POPUP_STYLE_MENU_LIST) {
	   elm_object_style_set(pObj, "content_no_vhpad");
	   }
	   else if (nStyle == POPUP_STYLE_EXPAND) {
	   //elm_object_style_set(pObj, "content_expand");
	   elm_object_style_set(pObj, "content_no_vhpad");
	   } */

	if (nStyle == POPUP_STYLE_FULLSCREEN_NO_CANCEL_BTN) {
		elm_layout_theme_set(pObj, "popup", "fullsrceen", "default");
	}

	if (nStyle == POPUP_STYLE_FULLSCREEN_NO_TITLE_NO_CANCEL_BTN) {
		elm_layout_theme_set(pObj, "popup", "fullsrceen/notitle",
		                     "default");
	}

	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);

	if (szTitle) {
		elm_object_part_text_set(pObj, "title,text", szTitle);
	}

	if (szContent) {
		elm_object_text_set(pObj, szContent);
	}

	if (dTimeOut > 0.0) {
		elm_popup_timeout_set(pObj, dTimeOut);
		if (pTimeoutFunc) {
			evas_object_smart_callback_add(pObj, "timeout", pTimeoutFunc,
			                               (void *) pUserData);
		}
	}

	if (pCancelKeyCb) {
		eext_object_event_callback_add(pObj, EEXT_CALLBACK_BACK,
		                               pCancelKeyCb, (void *) pUserData);
	}

	if (nStyle == POPUP_STYLE_DEFAULT_NO_CANCEL_BTN
	        || nStyle == POPUP_STYLE_MENU_LIST_NO_CANCEL_BTN
	        || nStyle == POPUP_STYLE_EXPAND_NO_CANCEL_BTN
	        || nStyle == POPUP_STYLE_PROGRESS_WITH_NO_CANCEL_BTN)
		evas_object_smart_callback_add(pObj, "block,clicked",
		                               pCancelKeyCb, (void *) pUserData);

	if (pCancelMouseCb) {
		evas_object_event_callback_add(pObj, EVAS_CALLBACK_MOUSE_UP,
		                               pCancelMouseCb,
		                               (void *) pUserData);
	}

	return pObj;
}


Evas_Object *vp_two_button_popup_create(Evas_Object *pParent,
                                        char *szTitle,
                                        char *szContent,
                                        char *pLeftButtonText,
                                        Evas_Smart_Cb leftButtonCb,
                                        char *pRightButtonText,
                                        Evas_Smart_Cb rightButtonCb,
                                        const void *pUserData)
{
	Evas_Object *pPopup = NULL;
	Evas_Object *pBtn1 = NULL;
	Evas_Object *pBtn2 = NULL;

	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	pPopup = elm_popup_add(pParent);

	if (pPopup == NULL) {
		VideoLogError("elm_popup_add is fail");
		return NULL;
	}

	if (szTitle) {
		elm_object_part_text_set(pPopup, "title,text", szTitle);
	}

	evas_object_size_hint_weight_set(pPopup, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	if (szContent) {
		elm_object_part_text_set(pPopup, "default", szContent);
	}

	pBtn1 = elm_button_add(pPopup);
	elm_object_style_set(pBtn1, "popup");
	elm_object_text_set(pBtn1, pLeftButtonText);
	elm_object_part_content_set(pPopup, "button1", pBtn1);
	if (leftButtonCb)
		evas_object_smart_callback_add(pBtn1, "clicked", leftButtonCb,
		                               (const void *) pUserData);

	pBtn2 = elm_button_add(pPopup);
	elm_object_style_set(pBtn2, "popup");
	elm_object_text_set(pBtn2, pRightButtonText);
	elm_object_part_content_set(pPopup, "button2", pBtn2);

	if (rightButtonCb)
		evas_object_smart_callback_add(pBtn2, "clicked", rightButtonCb,
		                               (const void *) pUserData);

	evas_object_show(pPopup);

	return pPopup;
}

#ifdef _SUBTITLE_MULTI_LANGUAGE
Evas_Object *vp_title_two_button_popup_create(Evas_Object *pParent,
        char *szTitle,
        char *szContent,
        char *pLeftButtonText,
        Evas_Smart_Cb leftButtonCb,
        char *pRightButtonText,
        Evas_Smart_Cb rightButtonCb,
        const void *pUserData)
{
	Evas_Object *pPopup = NULL;
	pPopup =
	    vp_two_button_popup_create(pParent, NULL, szContent,
	                               pLeftButtonText, leftButtonCb,
	                               pRightButtonText, rightButtonCb,
	                               pUserData);
	if (szTitle && pPopup) {
		elm_object_part_text_set(pPopup, "title,text", szTitle);
	}

	return pPopup;
}
#endif
bool vp_popup_check_landspace_by_win(Evas_Object *pWin)
{
	if (!pWin) {
		VideoLogError("win is NULL.");
		return FALSE;
	}
	bool bLandSpace = FALSE;
	int bROtation = elm_win_rotation_get(pWin);
	if (bROtation == 90 || bROtation == 270) {
		bLandSpace = TRUE;
	}
	return bLandSpace;
}

void vp_popup_set_popup_min_size(Evas_Object *pWin, Evas_Object *pBox,
                                 int nListCount,
                                 video_list_popup_style_t eStyle)
{
	if (!pWin) {
		VideoLogError("win is NULL.");
		return;
	}
	if (pBox) {
		int nMinHeight = 0;
		switch (eStyle) {
		case VIDEO_POPUP_DEFAULT:
			if (vp_popup_check_landspace_by_win(pWin)) {
				nMinHeight = VP_POPUP_LIST_HEIGHT_L(nListCount);
			} else {
				nMinHeight = VP_POPUP_LIST_HEIGHT(nListCount);
			}
			break;
		case VIDEO_POPUP_2_TEXT:
			if (vp_popup_check_landspace_by_win(pWin)) {
				nMinHeight = VP_POPUP_LIST_2_TEXT_HEIGHT_L(nListCount);
			} else {
				nMinHeight = VP_POPUP_LIST_2_TEXT_HEIGHT(nListCount);
			}
			break;
		case VIDEO_POPUP_2_TEXT_1_ICON:
			if (vp_popup_check_landspace_by_win(pWin)) {
				nMinHeight =
				    VP_POPUP_LIST_2_TEXT_1_ICON_HEIGHT_L(nListCount);
			} else {
				nMinHeight =
				    VP_POPUP_LIST_2_TEXT_1_ICON_HEIGHT(nListCount);
			}
			break;
		case VIDEO_POPUP_LOADING_LIST:
			if (vp_popup_check_landspace_by_win(pWin)) {
				nMinHeight = VP_POPUP_LOADING_LIST_HEIGHT_L(nListCount);
			} else {
				nMinHeight = VP_POPUP_LOADING_LIST_HEIGHT(nListCount);
			}
			break;

		default:
			VideoLogError("invalid style =%d.", eStyle);
			break;
		}

		evas_object_size_hint_min_set(pBox,
		                              VP_POPUP_LIST_WIDTH * VP_SCALE,
		                              nMinHeight * VP_SCALE);
	}
}
