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


#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-type-define.h"
#include "vp-play-macro-define.h"

#include "vp-subtitle-select.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-file-util.h"
#include "vp-play-preference.h"

#define VP_SUBTITLE_SELECT_GENLIST_DATA_KEY		"vp.subtitle.select.genlist"


typedef struct _SubtitleSelectPopup {
	Evas_Object		*pParent;
	Evas_Object		*pPopup;
	Evas_Object		*pRadio;
	Evas_Object		*pButton;
	Evas_Object		*pBox;
	Evas_Object		*pGenList;
	Elm_Genlist_Item_Class 	*st_SubtitleSelect_Itc;
	Elm_Genlist_Item_Class 	*st_SubtitleSelect_Itc1;

	int			nAppendCount;

	void 			*pUserData;
	SubtitleCloseCbFunc	pCloseCb;
	SubtitleCloseCbFunc	pButtonCb;
	GList			*pItemList;
	int				nSubtitleIdx;
} SubtitleSelectPopup;



static void _vp_subtitle_select_destroy_handle(SubtitleSelectPopup *pSubtitleSelect);
static void  __vp_subtitle_select_genlist_item_selected_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo);

static void __vp_subtitle_select_genlist_realized(void *data, Evas_Object *obj, void *event_info)
{
	VP_GENLIST_HIDE_BOTTOMLINE(data, obj, event_info);
}

static void __vp_subtitle_select_popup_rotate_cb(void *data, Evas_Object *obj, void *event_info)
{
	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)data;
	if (!pSubtitleSelect) {
		VideoLogError("pSubtitleSelect IS null");
		return;
	}
	if (pSubtitleSelect->nAppendCount < VP_POPUP_MIN_ITEMS) {
		return;
	}
	vp_popup_set_popup_min_size(pSubtitleSelect->pParent, pSubtitleSelect->pBox, pSubtitleSelect->nAppendCount, VIDEO_POPUP_2_TEXT_1_ICON);
}


/* callback functions */
static char *__vp_subtitle_select_genlist_text_get_cb(const void *pUserData, Evas_Object *pObj, const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	const char *szTxt = (const char *)pUserData;

	if (!strcmp(pPart, "elm.text")) {
		const char *szFileName = vp_file_get(szTxt);
		if (szFileName) {
			return strdup(szFileName);
		}
	}

	return NULL;
}

static Evas_Object *__vp_subtitle_select_genlist_content_get_cb(const void *pUserData, Evas_Object *pObj, const char *pPart)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *)pUserData;

	if (!strcmp(pPart, "elm.swallow.end")) {
		Evas_Object *pRadioObj = NULL;

		SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)evas_object_data_get(pObj , VP_SUBTITLE_SELECT_GENLIST_DATA_KEY);
		if (pSubtitleSelect == NULL) {
			VideoLogWarning("evas_object_data_get is fail");
			return NULL;
		}

		if (pSubtitleSelect->pItemList == NULL) {
			// sub-title does not exist
			pRadioObj = elm_radio_add(pObj);
			return pRadioObj;
		} else {
			// sub-title exist
			int nIndex = 0;
			int nCount = 0;
			int i = 0;

			nCount = g_list_length(pSubtitleSelect->pItemList);
			for (i = 0; i < nCount ; i++) {
				char *szName = NULL;
				szName = (char *)g_list_nth_data(pSubtitleSelect->pItemList, i);

				if (szName == NULL) {
					continue;
				}
				if (strcmp(szName, szTxt) == 0) {
					break;
				}
				nIndex++;
			}

			/* get file name list */
			pRadioObj = elm_radio_add(pObj);
			elm_radio_state_value_set(pRadioObj, nIndex);
			elm_radio_group_add(pRadioObj, pSubtitleSelect->pRadio);
			elm_radio_value_set(pSubtitleSelect->pRadio, pSubtitleSelect->nSubtitleIdx);
			evas_object_smart_callback_add(pRadioObj, "changed", __vp_subtitle_select_genlist_item_selected_cb, pSubtitleSelect);
			evas_object_show(pRadioObj);
			return pRadioObj;
		}
	}

	return NULL;
}

static void __vp_subtitle_select_genlist_item_selected_cb(void *pUserData, Evas_Object *pObject, void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}
	if (pEventInfo == NULL) {
		VideoLogError("pEventInfo is NULL");
		return;
	}

	Elm_Object_Item *pItem = (Elm_Object_Item *) pEventInfo;
	Elm_Object_Item *pSelectedItem = elm_genlist_selected_item_get(pObject);
	if (pSelectedItem) {
		elm_genlist_item_selected_set(pSelectedItem, EINA_FALSE);
	}
	char *szTxt = (char *)elm_object_item_data_get(pItem);

	VideoSecureLogInfo("Selected subtitle path=%s", szTxt);
	if (szTxt == NULL) {
		VideoLogError("szTxt is NULL");
		return;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pUserData;
	int nIndex = 0;
	int nCount = 0;
	int i = 0;

	nCount = g_list_length(pSubtitleSelect->pItemList);
	for (i = 0; i < nCount ; i++) {
		char *szName = NULL;
		szName = (char *)g_list_nth_data(pSubtitleSelect->pItemList, i);

		if (szName == NULL) {
			continue;
		}
		if (strcmp(szName, szTxt) == 0) {
			break;
		}
		nIndex++;
	}

	pSubtitleSelect->nSubtitleIdx = nIndex;
	if (szTxt) {
		char *szName = NULL;
		VP_STRDUP(szName, szTxt);
		if (pSubtitleSelect->pCloseCb) {
			pSubtitleSelect->pCloseCb(szName, (void *)pSubtitleSelect->pUserData);
		}
		VP_FREE(szName);
	}
}

/*static void __vp_subtitle_select_popup_key_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pUserData;

	if (pSubtitleSelect->pCloseCb) {
		pSubtitleSelect->pCloseCb(NULL, (void *)pSubtitleSelect->pUserData);
	}
}*/

static void __vp_subtitle_select_popup_cancel_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pUserData;

	if (pSubtitleSelect->pButtonCb) {
		pSubtitleSelect->pButtonCb("cancel", (void *)pSubtitleSelect->pUserData);
	}
}

static void __vp_subtitle_select_popup_settings_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
{
	if (pUserData == NULL) {
		VideoLogError("pUserData is NULL");
		return;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pUserData;

	if (pSubtitleSelect->pButtonCb) {
		pSubtitleSelect->pButtonCb("settings", (void *)pSubtitleSelect->pUserData);
	}
}

/*static void __vp_subtitle_select_popup_mouse_event_cb(void *pUserData, Evas_Object *pObj, void *pEventInfo)
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
		SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pUserData;

		if (pSubtitleSelect->pCloseCb) {
			pSubtitleSelect->pCloseCb(NULL, (void *)pSubtitleSelect->pUserData);
		}
	}
}
*/

/* internal functions */
static void _vp_subtitle_select_destroy_handle(SubtitleSelectPopup *pSubtitleSelect)
{
	if (pSubtitleSelect == NULL) {
		VideoLogError("pSubtitleSelect is NULL");
		return;
	}
	int nCount = 0;
	int i = 0;

	evas_object_smart_callback_del(pSubtitleSelect->pParent, "rotation,changed", __vp_subtitle_select_popup_rotate_cb);
	evas_object_smart_callback_del(pSubtitleSelect->pGenList, "realized", __vp_subtitle_select_genlist_realized);


	VP_EVAS_DEL(pSubtitleSelect->pRadio);
	VP_EVAS_DEL(pSubtitleSelect->pGenList);
	VP_EVAS_DEL(pSubtitleSelect->pButton);
	VP_EVAS_DEL(pSubtitleSelect->pBox);

	if (pSubtitleSelect->st_SubtitleSelect_Itc) {
		elm_genlist_item_class_free(pSubtitleSelect->st_SubtitleSelect_Itc);
		pSubtitleSelect->st_SubtitleSelect_Itc = NULL;
	}

	nCount = g_list_length(pSubtitleSelect->pItemList);
	for (i = 0; i < nCount ; i++) {
		char *szName = NULL;
		szName = (char *)g_list_nth_data(pSubtitleSelect->pItemList, i);
		VP_FREE(szName);
	}

	g_list_free(pSubtitleSelect->pItemList);
	pSubtitleSelect->pItemList = NULL;
	VP_EVAS_DEL(pSubtitleSelect->pPopup);

	VP_FREE(pSubtitleSelect);
}


static Evas_Object *_vp_subtitle_select_create_genlist(Evas_Object *pParent)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	Evas_Object *pObj = NULL;

	pObj = elm_genlist_add(pParent);
	evas_object_size_hint_weight_set(pObj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pObj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(pObj);
	return pObj;
}


static bool _vp_subtitle_select_add_genlist_item(Evas_Object *pObj, void *pUserData)
{
	if (pObj == NULL) {
		VideoLogError("pObj is NULL");
		return FALSE;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pUserData;

	if (pSubtitleSelect->st_SubtitleSelect_Itc) {
		elm_genlist_item_class_free(pSubtitleSelect->st_SubtitleSelect_Itc);
		pSubtitleSelect->st_SubtitleSelect_Itc = NULL;
	}

	pSubtitleSelect->st_SubtitleSelect_Itc = elm_genlist_item_class_new();

	if (pSubtitleSelect->st_SubtitleSelect_Itc != NULL) {
		pSubtitleSelect->st_SubtitleSelect_Itc->version = ELM_GENLIST_ITEM_CLASS_VERSION;
		pSubtitleSelect->st_SubtitleSelect_Itc->item_style = "type1";
		pSubtitleSelect->st_SubtitleSelect_Itc->func.text_get = (void *)__vp_subtitle_select_genlist_text_get_cb;
		pSubtitleSelect->st_SubtitleSelect_Itc->func.content_get = (void *)__vp_subtitle_select_genlist_content_get_cb;
		pSubtitleSelect->st_SubtitleSelect_Itc->func.state_get = NULL;
		pSubtitleSelect->st_SubtitleSelect_Itc->func.del = NULL;
	}

	int nCount = 0;
	int i = 0;
	pSubtitleSelect->nAppendCount = 0;
	Elm_Object_Item *it = NULL;
	nCount = g_list_length(pSubtitleSelect->pItemList);
	for (i = 0; i < nCount ; i++) {
		char *szName = NULL;
		szName = (char *)g_list_nth_data(pSubtitleSelect->pItemList, i);
		if (szName == NULL) {
			continue;
		}
		it = elm_genlist_item_append(pObj, pSubtitleSelect->st_SubtitleSelect_Itc, (void *)szName, NULL, ELM_GENLIST_ITEM_NONE, __vp_subtitle_select_genlist_item_selected_cb, pUserData);
		pSubtitleSelect->nAppendCount++;
	}
	if (pSubtitleSelect->nAppendCount == 1) {
		elm_object_item_disabled_set(it, EINA_TRUE);
	}
	VideoLogError("pSubtitleSelect->nAppendCount= %d", pSubtitleSelect->nAppendCount);

	return TRUE;
}


/* external functions */
subtitle_select_handle vp_subtitle_select_create(Evas_Object *pParent, SubtitleCloseCbFunc pCloseCb, SubtitleCloseCbFunc pButtonCb, GList *pFileList, char *szCurrentPath, bool isSubtitleOff)
{
	if (pParent == NULL) {
		VideoLogError("pParent is NULL");
		return NULL;
	}

	SubtitleSelectPopup *pSubtitleSelect = NULL;

	pSubtitleSelect = calloc(1, sizeof(SubtitleSelectPopup));

	if (pSubtitleSelect == NULL) {
		VideoLogError("pSubtitleSelect alloc fail");
		return NULL;
	}

	pSubtitleSelect->pParent = pParent;
	pSubtitleSelect->pCloseCb = pCloseCb;
	pSubtitleSelect->pButtonCb = pButtonCb;
	int nItemCount = 0;
	if (pFileList) {
		nItemCount = g_list_length(pFileList);
	}
	if (nItemCount > 0)
		pSubtitleSelect->pPopup = vp_two_button_popup_create(pParent, VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES,
		                          NULL, VP_PLAY_STRING_COM_CANCEL,
		                          __vp_subtitle_select_popup_cancel_cb, VP_PLAY_STRING_SETTINGS,
		                          __vp_subtitle_select_popup_settings_cb,
		                          (void *)pSubtitleSelect);
	else {
		pSubtitleSelect->pPopup = vp_two_button_popup_create(pParent, VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES,
		                          NULL, VP_PLAY_STRING_COM_CANCEL,
		                          __vp_subtitle_select_popup_cancel_cb, VP_PLAY_STRING_SETTINGS,
		                          NULL,
		                          (void *)pSubtitleSelect);
		Elm_Object_Item *it = NULL;
		it = (Evas_Object *)elm_object_part_content_get(pSubtitleSelect->pPopup, "button2");
		if (it) {
			elm_object_disabled_set(it, EINA_TRUE);
		}
	}
	eext_object_event_callback_add(pSubtitleSelect->pPopup, EEXT_CALLBACK_BACK, __vp_subtitle_select_popup_cancel_cb, (void *)pSubtitleSelect);
	if (pSubtitleSelect->pPopup == NULL) {
		VideoLogError("vp_popup_create fail");
		_vp_subtitle_select_destroy_handle(pSubtitleSelect);
		return NULL;
	}

	pSubtitleSelect->pGenList = _vp_subtitle_select_create_genlist(pSubtitleSelect->pPopup);
	if (pSubtitleSelect->pGenList == NULL) {
		VideoLogError("_vp_subtitle_select_create_genlist fail");
		_vp_subtitle_select_destroy_handle(pSubtitleSelect);
		return NULL;
	}
	evas_object_data_set(pSubtitleSelect->pGenList , VP_SUBTITLE_SELECT_GENLIST_DATA_KEY, (void *)pSubtitleSelect);
	elm_genlist_mode_set(pSubtitleSelect->pGenList, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(pSubtitleSelect->pGenList, "realized", __vp_subtitle_select_genlist_realized, NULL);

	pSubtitleSelect->pRadio = elm_radio_add(pSubtitleSelect->pGenList);

	int i = 0;
	int nCount = 0;

	if (pFileList) {
		for (i = 0; i < nItemCount; i++) {
			char *szName = NULL;
			char *szItem = NULL;
			szName = (char *)g_list_nth_data(pFileList, i);
			if (szName == NULL) {
				continue;
			}
			if (szCurrentPath) {
				if (strcmp(szName, szCurrentPath) == 0) {
					pSubtitleSelect->nSubtitleIdx = nCount;
					elm_radio_value_set(pSubtitleSelect->pRadio, pSubtitleSelect->nSubtitleIdx);
				}
			}
			VP_STRDUP(szItem, szName);
			pSubtitleSelect->pItemList = g_list_append(pSubtitleSelect->pItemList, (void *)szItem);
			nCount++;
		}
	}

	//char *szName = VP_PLAY_STRING_SUBTITLE_OFF;
	char *szItem = NULL;
	VP_STRDUP(szItem, VP_PLAY_STRING_SUBTITLE_OFF);
	pSubtitleSelect->pItemList = g_list_append(pSubtitleSelect->pItemList, (void *)szItem);
	nCount++;
	bool bOn = FALSE;
	vp_play_preference_get_subtitle_show_key(&bOn);
	if (!_vp_subtitle_select_add_genlist_item(pSubtitleSelect->pGenList, (void *)pSubtitleSelect)) {
		VideoLogError("_vp_subtitle_select_add_genlist_item fail");
		return FALSE;
	}
	if (isSubtitleOff || !bOn) {
		pSubtitleSelect->nSubtitleIdx = (nCount - 1);
		elm_radio_value_set(pSubtitleSelect->pRadio, pSubtitleSelect->nSubtitleIdx);
	}
	evas_object_smart_callback_add(pSubtitleSelect->pParent, "rotation,changed", __vp_subtitle_select_popup_rotate_cb, pSubtitleSelect);

	pSubtitleSelect->pBox = elm_box_add(pSubtitleSelect->pPopup);
	vp_popup_set_popup_min_size(pSubtitleSelect->pParent, pSubtitleSelect->pBox, pSubtitleSelect->nAppendCount, VIDEO_POPUP_DEFAULT);

	elm_box_pack_end(pSubtitleSelect->pBox, pSubtitleSelect->pGenList);
	elm_object_content_set(pSubtitleSelect->pPopup, pSubtitleSelect->pBox);

	return pSubtitleSelect;
}

void vp_subtitle_select_destroy(subtitle_select_handle pSubtitleSelectHandle)
{
	if (pSubtitleSelectHandle == NULL) {
		VideoLogError("pSubtitleSelectHandle is NULL");
		return;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pSubtitleSelectHandle;

	_vp_subtitle_select_destroy_handle(pSubtitleSelect);
}

bool vp_subtitle_select_realize(subtitle_select_handle pSubtitleSelectHandle)
{
	if (pSubtitleSelectHandle == NULL) {
		VideoLogError("pSubtitleSelectHandle is NULL");
		return FALSE;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pSubtitleSelectHandle;

	evas_object_show(pSubtitleSelect->pPopup);

	return TRUE;
}

bool vp_subtitle_select_unrealize(subtitle_select_handle pSubtitleSelectHandle)
{
	if (pSubtitleSelectHandle == NULL) {
		VideoLogError("pSubtitleSelectHandle is NULL");
		return FALSE;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pSubtitleSelectHandle;

	evas_object_hide(pSubtitleSelect->pPopup);

	return TRUE;
}

bool vp_subtitle_select_set_user_data(subtitle_select_handle pSubtitleSelectHandle, void *pUserData)
{
	if (pSubtitleSelectHandle == NULL) {
		VideoLogError("pSubtitleSelectHandle is NULL");
		return FALSE;
	}

	SubtitleSelectPopup *pSubtitleSelect = (SubtitleSelectPopup *)pSubtitleSelectHandle;

	pSubtitleSelect->pUserData = pUserData;

	return TRUE;
}

