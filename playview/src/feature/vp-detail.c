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

#include "vp-util.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"

#include "vp-detail.h"

/* check temp */
#include "vp-play-log.h"

#define VP_DETAIL_GENLIST_DATA_KEY		"vp.detail.genlist"


typedef struct _DetailPopup {
	Evas_Object *pParent;
	Evas_Object *pPopup;
	Evas_Object *pButton;
	Evas_Object *pBox;
	Evas_Object *pGenList;
	Elm_Genlist_Item_Class *st_Detail_Itc;
	vp_detail_info *pDetailInfo;
	int nListCount;

	void *pUserData;
	PopupCloseCbFunc pCloseCb;

} DetailPopup;


static void _vp_detail_destroy_handle(DetailPopup *pDetailPopup);


/* callback functions */

static void __vp_detail_genlist_realized(void *data, Evas_Object *obj,
                                        void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_detail_popup_rotate_cb(void *data, Evas_Object *obj,
                                        void *event_info)
{
	DetailPopup *pDetailPopup = (DetailPopup *) data;
	if (!pDetailPopup) {
		VideoLogError("pDetailPopup IS null");
		return;
	}
	if (pDetailPopup->nListCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pDetailPopup->pParent, pDetailPopup->pBox,
	                            pDetailPopup->nListCount,
	                            VIDEO_POPUP_2_TEXT);
	elm_popup_orient_set(pDetailPopup->pPopup, ELM_POPUP_ORIENT_CENTER);
}

static char *__vp_detail_genlist_text_get_cb(const void *pUserData,
                                Evas_Object *pObj,
                                const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	VideoLogWarning("%s : %s", pPart, szTxt);

	if (!strcmp(pPart, "elm.text")) {
		return strdup(szTxt);
	} else if (!strcmp(pPart, "elm.text.sub")) {

		DetailPopup *pDetailPopup =
		    (DetailPopup *) evas_object_data_get(pObj,
		            VP_DETAIL_GENLIST_DATA_KEY);
		if (pDetailPopup == NULL) {
			return NULL;
		}

		vp_detail_info *pDetailInfo = pDetailPopup->pDetailInfo;
		if (pDetailInfo == NULL) {
			return NULL;
		}

		if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_TITLE)) {
			if (pDetailInfo->szTitle) {
				return elm_entry_utf8_to_markup(pDetailInfo->szTitle);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_FORMAT)) {
			if (pDetailInfo->szFormat) {
				return elm_entry_utf8_to_markup(pDetailInfo->szFormat);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_DATE)) {
			if (pDetailInfo->szDate) {
				return elm_entry_utf8_to_markup(pDetailInfo->szDate);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_SIZE)) {
			if (pDetailInfo->szSize) {
				return elm_entry_utf8_to_markup(pDetailInfo->szSize);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_LAST_MODIFIED)) {
			if (pDetailInfo->szLastModified)
				return elm_entry_utf8_to_markup(pDetailInfo->
				                                szLastModified);
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_RESOLUTION)) {
			if (pDetailInfo->szResolution)
				return elm_entry_utf8_to_markup(pDetailInfo->
				                                szResolution);
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_LOCATION)) {
			if (pDetailInfo->szLocation) {
				return elm_entry_utf8_to_markup(pDetailInfo->szLocation);
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_LATITUDE)) {
			if (pDetailInfo->szLatitude) {
				if (atof(pDetailInfo->szLatitude) !=
				        VP_DETAIL_VALUE_GPS_DEFAULT)
					return elm_entry_utf8_to_markup(pDetailInfo->
					                                szLatitude);
				else {
					return g_strdup(VP_PLAY_STRING_DETAIL_VALUE_UNKNOWN);
				}
			}
		} else if (!strcmp(szTxt, VP_PLAY_STRING_DETAIL_LONGITUDE)) {
			if (pDetailInfo->szLongitude) {
				if (atof(pDetailInfo->szLongitude) !=
				        VP_DETAIL_VALUE_GPS_DEFAULT)
					return elm_entry_utf8_to_markup(pDetailInfo->
					                                szLongitude);
				else {
					return g_strdup(VP_PLAY_STRING_DETAIL_VALUE_UNKNOWN);
				}
			}
		}
	}

	return NULL;
}

static void __vp_detail_popup_key_event_cb(void *pUserData,
                                Evas_Object *pObj,
                                void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	DetailPopup *pDetailPopup = (DetailPopup *) pUserData;

	if (pDetailPopup->pCloseCb) {
		pDetailPopup->pCloseCb(-1, FALSE,
		                       (void *) pDetailPopup->pUserData);
	}
}

static void __vp_detail_popup_mouse_event_cb(void *pUserData,
        Evas *pEvas,
        Evas_Object *pObj,
        void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	if (pEventInfo == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Evas_Event_Mouse_Up *ev = pEventInfo;

	if (ev->button == 3) {
		DetailPopup *pDetailPopup = (DetailPopup *) pUserData;

		if (pDetailPopup->pCloseCb) {
			pDetailPopup->pCloseCb(-1, FALSE,
			                       (void *) pDetailPopup->pUserData);
		}
	}
}


/* internal functions */

static void _vp_detail_destroy_handle(DetailPopup *pDetailPopup)
{
	if (pDetailPopup == NULL) {
		VideoLogError("pDetailPopup is NULL");
		return;
	}
	evas_object_smart_callback_del(pDetailPopup->pGenList, "realized",
	                               __vp_detail_genlist_realized);
	evas_object_smart_callback_del(pDetailPopup->pParent,
	                               "rotation,changed",
	                               __vp_detail_popup_rotate_cb);


	VP_EVAS_DEL(pDetailPopup->pGenList);
	VP_EVAS_DEL(pDetailPopup->pButton);
	VP_EVAS_DEL(pDetailPopup->pBox);

	VP_FREE(pDetailPopup->pDetailInfo->szTitle);
	VP_FREE(pDetailPopup->pDetailInfo->szDate);
	VP_FREE(pDetailPopup->pDetailInfo->szSize);
	VP_FREE(pDetailPopup->pDetailInfo->szFormat);
	VP_FREE(pDetailPopup->pDetailInfo->szResolution);
	VP_FREE(pDetailPopup->pDetailInfo->szLatitude);
	VP_FREE(pDetailPopup->pDetailInfo->szLongitude);
	VP_FREE(pDetailPopup->pDetailInfo->szLocation);

	VP_FREE(pDetailPopup->pDetailInfo);

	if (pDetailPopup->st_Detail_Itc) {
		elm_genlist_item_class_free(pDetailPopup->st_Detail_Itc);
		pDetailPopup->st_Detail_Itc = NULL;
	}

	VP_EVAS_DEL(pDetailPopup->pPopup);

	VP_FREE(pDetailPopup);
}

static Evas_Object *_vp_detail_create_genlist(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_genlist_add(pParent);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(pObj);
	return pObj;
}


static bool _vp_detail_add_genlist_item(Evas_Object *pObj,
                                        void *pUserData,
                                        int *added_item_cnt)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return FALSE;
	}

	DetailPopup *pDetailPopup = (DetailPopup *) pUserData;
	if (pDetailPopup->st_Detail_Itc) {
		elm_genlist_item_class_free(pDetailPopup->st_Detail_Itc);
		pDetailPopup->st_Detail_Itc = NULL;
	}

	int item_cnt = 0;

	pDetailPopup->st_Detail_Itc = elm_genlist_item_class_new();
	if (pDetailPopup->st_Detail_Itc == NULL) {
		VideoLogError("failed to create genlist class");
		return FALSE;
	}

	pDetailPopup->st_Detail_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
	pDetailPopup->st_Detail_Itc->item_style = "type1";
	pDetailPopup->st_Detail_Itc->func.text_get =
	    (void *) __vp_detail_genlist_text_get_cb;
	pDetailPopup->st_Detail_Itc->func.content_get = NULL;
	pDetailPopup->st_Detail_Itc->func.state_get = NULL;
	pDetailPopup->st_Detail_Itc->func.del = NULL;

	Elm_Object_Item *pItem = NULL;

	pItem =
	    elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc,
	                            (void *) VP_PLAY_STRING_DETAIL_TITLE,
	                            NULL, ELM_GENLIST_ITEM_NONE, NULL,
	                            (void *) pDetailPopup);
	elm_genlist_item_select_mode_set(pItem,
	                                 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	item_cnt++;

	pItem =
	    elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc,
	                            (void *) VP_PLAY_STRING_DETAIL_SIZE, NULL,
	                            ELM_GENLIST_ITEM_NONE, NULL,
	                            (void *) pDetailPopup);
	elm_genlist_item_select_mode_set(pItem,
	                                 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	item_cnt++;

	if (pDetailPopup->pDetailInfo->bPlayready == FALSE) {
		pItem =
		    elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc,
		                            (void *) VP_PLAY_STRING_DETAIL_FORMAT,
		                            NULL, ELM_GENLIST_ITEM_NONE, NULL,
		                            (void *) pDetailPopup);
		elm_genlist_item_select_mode_set(pItem,
		                                 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		item_cnt++;
	}

	/*pItem = elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc, (void *)VP_PLAY_STRING_DETAIL_DATE, NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)pDetailPopup);
	   elm_genlist_item_select_mode_set(pItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	   item_cnt++; */

	if (pDetailPopup->pDetailInfo->bPlayready == FALSE) {
		pItem =
		    elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc,
		                            (void *)
		                            VP_PLAY_STRING_DETAIL_RESOLUTION,
		                            NULL, ELM_GENLIST_ITEM_NONE, NULL,
		                            (void *) pDetailPopup);
		elm_genlist_item_select_mode_set(pItem,
		                                 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		item_cnt++;
	}

	pItem =
	    elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc,
	                            (void *)
	                            VP_PLAY_STRING_DETAIL_LAST_MODIFIED, NULL,
	                            ELM_GENLIST_ITEM_NONE, NULL,
	                            (void *) pDetailPopup);
	elm_genlist_item_select_mode_set(pItem,
	                                 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	item_cnt++;

	if (pDetailPopup->pDetailInfo->bPlayready == FALSE) {
		pItem =
		    elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc,
		                            (void *)
		                            VP_PLAY_STRING_DETAIL_LOCATION, NULL,
		                            ELM_GENLIST_ITEM_NONE, NULL,
		                            (void *) pDetailPopup);
		elm_genlist_item_select_mode_set(pItem,
		                                 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		item_cnt++;

		/*pItem = elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc, (void *)VP_PLAY_STRING_DETAIL_LATITUDE, NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)pDetailPopup);
		   elm_genlist_item_select_mode_set(pItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		   item_cnt++;

		   pItem = elm_genlist_item_append(pObj, pDetailPopup->st_Detail_Itc, (void *)VP_PLAY_STRING_DETAIL_LONGITUDE, NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)pDetailPopup);
		   elm_genlist_item_select_mode_set(pItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		   item_cnt++; */
	}

	*added_item_cnt = item_cnt;


	return TRUE;
}

/* external functions */
detail_handle vp_detail_create(Evas_Object *pParent,
                               PopupCloseCbFunc pCloseCb,
                               vp_detail_info *pDetailInfo)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	if (pDetailInfo == NULL) {
		VideoLogError("pDetailInfo is NULL");
		return NULL;
	}

	DetailPopup *pDetailPopup = NULL;

	pDetailPopup = calloc(1, sizeof(DetailPopup));

	if (pDetailPopup == NULL) {
		VideoLogError("pDetailPopup alloc fail");
		return NULL;
	}

	pDetailPopup->pDetailInfo = calloc(1, sizeof(vp_detail_info));
	if (pDetailPopup->pDetailInfo == NULL) {
		VideoLogError("pDetailPopup alloc fail");
		_vp_detail_destroy_handle(pDetailPopup);
		return NULL;
	}
	VP_STRDUP(pDetailPopup->pDetailInfo->szTitle, pDetailInfo->szTitle);
	VP_STRDUP(pDetailPopup->pDetailInfo->szDate, pDetailInfo->szDate);
	VP_STRDUP(pDetailPopup->pDetailInfo->szFormat, pDetailInfo->szFormat);
	VP_STRDUP(pDetailPopup->pDetailInfo->szSize, pDetailInfo->szSize);
	VP_STRDUP(pDetailPopup->pDetailInfo->szLastModified,
	          pDetailInfo->szLastModified);
	VP_STRDUP(pDetailPopup->pDetailInfo->szResolution,
	          pDetailInfo->szResolution);
	VP_STRDUP(pDetailPopup->pDetailInfo->szLatitude,
	          pDetailInfo->szLatitude);
	VP_STRDUP(pDetailPopup->pDetailInfo->szLongitude,
	          pDetailInfo->szLongitude);
	pDetailPopup->pDetailInfo->szLocation =
	    vp_util_convert_file_location(pDetailInfo->szLocation);

	pDetailPopup->pDetailInfo->bDrm = pDetailInfo->bDrm;
	pDetailPopup->pDetailInfo->bForwardLock = pDetailInfo->bForwardLock;
	pDetailPopup->pDetailInfo->bPlayready = pDetailInfo->bPlayready;


	pDetailPopup->pParent = pParent;
	pDetailPopup->pCloseCb = pCloseCb;

	pDetailPopup->pPopup =
	    vp_popup_create(pParent, POPUP_STYLE_DEFAULT_NO_CANCEL_BTN,
	                    VP_PLAY_STRING_COM_DETAILS, NULL, 0.0, NULL,
	                    __vp_detail_popup_key_event_cb,
	                    __vp_detail_popup_mouse_event_cb,
	                    (void *) pDetailPopup);

	if (pDetailPopup->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_detail_destroy_handle(pDetailPopup);
		return NULL;
	}
	//elm_popup_content_text_wrap_type_set(pDetailPopup->pPopup, ELM_WRAP_MIXED);
	pDetailPopup->pGenList =
	    _vp_detail_create_genlist(pDetailPopup->pPopup);
	if (pDetailPopup->pGenList == NULL) {
		VideoLogError("_vp_detail_create_genlist fail");
		_vp_detail_destroy_handle(pDetailPopup);
		return NULL;
	}

	evas_object_data_set(pDetailPopup->pGenList,
	                     VP_DETAIL_GENLIST_DATA_KEY,
	                     (void *) pDetailPopup);
	elm_genlist_mode_set(pDetailPopup->pGenList, ELM_LIST_COMPRESS);

	int added_item_cnt = 0;
	if (!_vp_detail_add_genlist_item
	        (pDetailPopup->pGenList, (void *) pDetailPopup,
	         &added_item_cnt)) {
		VideoLogError("_vp_detail_add_genlist_item fail");
		_vp_detail_destroy_handle(pDetailPopup);
		return NULL;
	}
	pDetailPopup->nListCount = added_item_cnt;
	evas_object_smart_callback_add(pDetailPopup->pGenList, "realized",
	                               __vp_detail_genlist_realized, NULL);
	evas_object_smart_callback_add(pDetailPopup->pParent,
	                               "rotation,changed",
	                               __vp_detail_popup_rotate_cb,
	                               pDetailPopup);

	pDetailPopup->pBox = elm_box_add(pDetailPopup->pPopup);

	VideoLogInfo("item cnt = %d", added_item_cnt);

	vp_popup_set_popup_min_size(pDetailPopup->pParent, pDetailPopup->pBox,
	                            pDetailPopup->nListCount,
	                            VIDEO_POPUP_2_TEXT);
	elm_popup_orient_set(pDetailPopup->pPopup, ELM_POPUP_ORIENT_CENTER);
	elm_box_pack_end(pDetailPopup->pBox, pDetailPopup->pGenList);

	elm_object_content_set(pDetailPopup->pPopup, pDetailPopup->pBox);

	return pDetailPopup;
}

void vp_detail_destroy(detail_handle pDetailHandle)
{
	if (pDetailHandle == NULL) {
		VideoLogError("pDetailHandle is NULL");
		return;
	}

	DetailPopup *pDetailPopup = (DetailPopup *) pDetailHandle;

	_vp_detail_destroy_handle(pDetailPopup);

}

bool vp_detail_realize(detail_handle pDetailHandle)
{
	if (pDetailHandle == NULL) {
		VideoLogError("pDetailHandle is NULL");
		return FALSE;
	}

	DetailPopup *pDetailPopup = (DetailPopup *) pDetailHandle;

	evas_object_show(pDetailPopup->pPopup);

	return TRUE;
}

bool vp_detail_unrealize(detail_handle pDetailHandle)
{
	if (pDetailHandle == NULL) {
		VideoLogError("pDetailHandle is NULL");
		return FALSE;
	}

	DetailPopup *pDetailPopup = (DetailPopup *) pDetailHandle;

	evas_object_hide(pDetailPopup->pPopup);

	return TRUE;
}

bool vp_detail_set_user_data(detail_handle pDetailHandle, void *pUserData)
{
	if (pDetailHandle == NULL) {
		VideoLogError("pDetailHandle is NULL");
		return FALSE;
	}

	DetailPopup *pDetailPopup = (DetailPopup *) pDetailHandle;

	pDetailPopup->pUserData = pUserData;

	return TRUE;
}
