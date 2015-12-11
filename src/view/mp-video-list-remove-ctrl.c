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

#include <app.h>
#include <Ecore_Evas.h>
#include <Elementary.h>



#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-list-remove-ctrl.h"
#include "mp-util-move.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-util-preference.h"
#include "mp-util-config.h"

typedef struct _MpRemoveVideoCtrl {
	/*obj*/
	Evas_Object *pPopUpHandle;
	Evas_Object *pProgressbar;

	/*data*/
	Ecore_Idler *pDelIdler;
	Ecore_Timer *pStartDelTimer;
	Ecore_Timer *pFinishDelTimer;
	ListRemoveCtrlCbFunc fDelCtrlUserCbFunc;

	int nDelTotalItem;
	int nDelCompltedItem;
	int nDelEditListSize;
	bool *pDelEditList;
	bool bFolderDel;

} stMpRemoveCtrl;


static stMpRemoveCtrl *g_pRemoveCtrl = NULL;

/*//////////////////////////////////////////////////////////*/
/* Internal function*/
/*//////////////////////////////////////////////////////////*/
static void mp_remove_ctrl_button_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo);

void mp_remove_ctrl_delete_popup_handle(void)
{
	VideoLogInfo("");
	if (!g_pRemoveCtrl) {
		VideoLogWarning("");
		return;
	}


	MP_DEL_IDLER(g_pRemoveCtrl->pDelIdler);
	MP_DEL_TIMER(g_pRemoveCtrl->pStartDelTimer);
	MP_DEL_TIMER(g_pRemoveCtrl->pFinishDelTimer);

	if (g_pRemoveCtrl->pPopUpHandle) {
		eext_object_event_callback_del(g_pRemoveCtrl->pPopUpHandle,
					       EEXT_CALLBACK_BACK,
					       mp_remove_ctrl_button_popup_cb);
		MP_DEL_OBJ(g_pRemoveCtrl->pPopUpHandle);
	}
	MP_FREE_STRING(g_pRemoveCtrl);

}

static void mp_remove_ctrl_button_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	VideoLogInfo("");
	if (!g_pRemoveCtrl) {
		VideoLogWarning("");
		return;
	}
	mp_util_move_set_cancel_state(TRUE);
	if (g_pRemoveCtrl->fDelCtrlUserCbFunc) {
		g_pRemoveCtrl->fDelCtrlUserCbFunc();
	}
	mp_remove_ctrl_delete_popup_handle();
}

static Eina_Bool __mp_remove_ctrl_finish_delete_timer_cb(void *pUserData)
{
	if (!g_pRemoveCtrl) {
		VideoLogWarning("");
		return FALSE;
	}

	g_pRemoveCtrl->pFinishDelTimer = NULL;
	if (g_pRemoveCtrl->fDelCtrlUserCbFunc) {
		g_pRemoveCtrl->fDelCtrlUserCbFunc();
	}
	mp_remove_ctrl_delete_popup_handle();

	return FALSE;
}

static Eina_Bool mp_remove_ctrl_delete_item_idler_cb(void *pUserData)
{

	if (!g_pRemoveCtrl) {
		VideoLogWarning("");
		return FALSE;
	}

	if (!g_pRemoveCtrl->pDelEditList) {
		VideoLogError
		("The userdata and g_pRemoveCtrl->pDelEditList is null.");
		g_pRemoveCtrl->pDelIdler = NULL;
		return FALSE;
	}

	VideoLogInfo("");

	int nCounter = 0;
	char *szMediaUrlForPreview = mp_util_config_get_preview();
	VideoLogInfo("Media-Url For Preview : %s", szMediaUrlForPreview);
	int nResult = 0;

	for (nCounter = 0; nCounter < g_pRemoveCtrl->nDelEditListSize;
			nCounter++) {
		if (g_pRemoveCtrl->pDelEditList[nCounter]) {
			if (g_pRemoveCtrl->bFolderDel) {
				char *szFolderUrl =
					mp_util_svc_get_video_folder_url(nCounter);
				if (szFolderUrl) {

					nResult =
						mp_util_move_delete_video_folder(szFolderUrl,
								NULL);
					/*if (mp_util_check_empty_dir(szFolderUrl))
					   {
					   vp_file_unlink(szFolderUrl)
					   } */
					if (szMediaUrlForPreview
							&&
							!mp_util_check_local_file_exist
							(szMediaUrlForPreview)) {
						mp_util_config_set_preview(NULL);
						MP_FREE_STRING(szMediaUrlForPreview);
					}
					mp_util_svc_remove_folder_list_item_from_list
					(nCounter);
					MP_FREE_STRING(szFolderUrl);
					if (nResult == 1) {
						VideoLogInfo("cancel delete");
						return FALSE;
					}
				}
			} else {
				char *szFileUrl = mp_util_svc_get_video_url(nCounter);
				if (szFileUrl) {
					if (szMediaUrlForPreview) {
						if (!strcmp(szMediaUrlForPreview, szFileUrl)) {
							VideoLogInfo("Erase preview url.");
							mp_util_config_set_preview(NULL);
						}
					}
					VideoLogInfo("Delete file url : %s", szFileUrl);
					/* Now when the data is cloud data,Click delete button, Send remove request to cloud server,
					   then remove current item ,But Don't care of if this data is deleted from cloud server. */
					unlink(szFileUrl);
					MP_FREE_STRING(szFileUrl);

					mp_util_svc_remove_item_by_list_index(nCounter);
					/*mp_util_svc_move_from_db(szFileUrl);*/
				}

			}

			int nMoveItemCounter = 0;
			for (nMoveItemCounter = nCounter;
					nMoveItemCounter < g_pRemoveCtrl->nDelEditListSize - 1;
					nMoveItemCounter++) {
				g_pRemoveCtrl->pDelEditList[nMoveItemCounter] =
					g_pRemoveCtrl->pDelEditList[nMoveItemCounter + 1];
			}
			g_pRemoveCtrl->nDelEditListSize--;

			/*                      pRemoveCheckedItemsEditList[nCounter] = FALSE;*/
			++g_pRemoveCtrl->nDelCompltedItem;
			break;
		}
	}
	MP_FREE_STRING(szMediaUrlForPreview);

	VideoLogInfo("nCompltedDeleteItem : %d",
		     g_pRemoveCtrl->nDelCompltedItem);

	double nDeleteRate =
		((double) g_pRemoveCtrl->nDelCompltedItem) /
		((double) g_pRemoveCtrl->nDelTotalItem);
	VideoLogInfo
	("nDeleteRate:%f, nCompltedDeleteItem:%d, nTotalDeleteItem:%d",
	 nDeleteRate, g_pRemoveCtrl->nDelCompltedItem,
	 g_pRemoveCtrl->nDelTotalItem);
	if (g_pRemoveCtrl->pProgressbar) {
		elm_progressbar_value_set(g_pRemoveCtrl->pProgressbar,
					  nDeleteRate);
	} else {
		VideoLogInfo("No exist progress bar handle.");
	}

	if (g_pRemoveCtrl->nDelCompltedItem == g_pRemoveCtrl->nDelTotalItem) {
		VideoLogInfo("nCompltedDeleteItem == nTotalDeleteItem");
		MP_DEL_TIMER(g_pRemoveCtrl->pFinishDelTimer);
		g_pRemoveCtrl->pDelIdler = NULL;
		g_pRemoveCtrl->pFinishDelTimer =
			ecore_timer_add(0.1, __mp_remove_ctrl_finish_delete_timer_cb,
					pUserData);

		return FALSE;
	}

	return TRUE;
}

static Eina_Bool __mp_remove_ctrl_start_delete_timer_cb(void *pUserData)
{
	if (!g_pRemoveCtrl) {
		VideoLogWarning("");
		return FALSE;
	}

	g_pRemoveCtrl->pStartDelTimer = NULL;
	g_pRemoveCtrl->pDelIdler =
		ecore_idler_add(mp_remove_ctrl_delete_item_idler_cb, pUserData);
	return FALSE;
}


/*//////////////////////////////////////////////////////////*/
/* External function*/
/*//////////////////////////////////////////////////////////*/
bool mp_remove_ctrl_show(bool *pCheckedItemsEditList,
			 int pCheckedItemsEditListSize,
			 ListRemoveCtrlCbFunc pListRemoveCtrlCb,
			 bool bFolderDel, void *pUserData)
{

	if (!pCheckedItemsEditList) {
		VideoLogError("No have popup message is null.");
		return FALSE;
	}

	VideoLogInfo("pCheckedItemsEditListSize : %d",
		     pCheckedItemsEditListSize);

	mp_remove_ctrl_delete_popup_handle();
	g_pRemoveCtrl = (stMpRemoveCtrl *) calloc(1, sizeof(stMpRemoveCtrl));
	if (!g_pRemoveCtrl) {
		VideoLogError("g_pRemoveCtrl is NULL");
		return FALSE;
	}


	g_pRemoveCtrl->fDelCtrlUserCbFunc = pListRemoveCtrlCb;
	g_pRemoveCtrl->nDelEditListSize = pCheckedItemsEditListSize;
	g_pRemoveCtrl->pDelEditList = pCheckedItemsEditList;
	g_pRemoveCtrl->bFolderDel = bFolderDel;

	int nCounter = 0;
	for (nCounter = 0; nCounter < g_pRemoveCtrl->nDelEditListSize;
			nCounter++) {
		if (g_pRemoveCtrl->pDelEditList[nCounter]) {
			char *szTitle = mp_util_svc_get_video_title(nCounter);
			VideoLogInfo(" Delete index - %d ; %s", nCounter, szTitle);
			MP_FREE_STRING(szTitle);

			++g_pRemoveCtrl->nDelTotalItem;
		}
	}

	VideoLogInfo("nTotalDeleteItem : %d", g_pRemoveCtrl->nDelTotalItem);

	g_pRemoveCtrl->pPopUpHandle =
		elm_popup_add(mp_util_get_main_window_handle());
	/*elm_object_part_text_set(g_pRemoveCtrl->pPopUpHandle, "title,text", VIDEOS_DELETEPOP_MSG_DELETING);*/
	evas_object_size_hint_weight_set(g_pRemoveCtrl->pPopUpHandle,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	eext_object_event_callback_add(g_pRemoveCtrl->pPopUpHandle,
				       EEXT_CALLBACK_BACK,
				       mp_remove_ctrl_button_popup_cb, NULL);

	/*Evas_Object *pButton = elm_button_add(g_pRemoveCtrl->pPopUpHandle);
	   elm_object_style_set (pButton, "popup");
	   elm_object_text_set(pButton, VIDEOS_COMMON_BUTTON_CANCEL);
	   elm_object_part_content_set(g_pRemoveCtrl->pPopUpHandle, "button1", pButton);
	   evas_object_smart_callback_add(pButton, "clicked", mp_remove_ctrl_button_popup_cb, pUserData);
	   evas_object_show(pButton); */
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s/%s/%s", path , "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);

	Evas_Object *pPopupLayout =
		elm_layout_add(g_pRemoveCtrl->pPopUpHandle);
	elm_layout_file_set(pPopupLayout, edj_path,
			    "popup_text_progressbar_view_layout");
	elm_object_part_text_set(pPopupLayout, "elm.text.description",
				 VIDEOS_DELETEPOP_MSG_DELETING);
	elm_object_part_text_set(pPopupLayout, "progress_text", "00/00");
	evas_object_size_hint_weight_set(pPopupLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	evas_object_show(pPopupLayout);

	Evas_Object *pProgressbar =
		elm_progressbar_add(g_pRemoveCtrl->pPopUpHandle);
	elm_object_style_set(pProgressbar, "wheel");
	evas_object_size_hint_align_set(pProgressbar, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pProgressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_pulse(pProgressbar, EINA_TRUE);
	elm_object_signal_emit(pProgressbar, "elm,units,show", "elm");
	evas_object_show(pProgressbar);

	elm_object_part_content_set(pPopupLayout, "progressbar",
				    pProgressbar);

	g_pRemoveCtrl->pProgressbar = pProgressbar;

	elm_object_content_set(g_pRemoveCtrl->pPopUpHandle, pPopupLayout);
	evas_object_data_set(g_pRemoveCtrl->pPopUpHandle, "progressbar",
			     pProgressbar);

	evas_object_show(g_pRemoveCtrl->pPopUpHandle);

	MP_DEL_TIMER(g_pRemoveCtrl->pStartDelTimer);
	g_pRemoveCtrl->pStartDelTimer =
		ecore_timer_add(0.3, __mp_remove_ctrl_start_delete_timer_cb,
				pUserData);

	return TRUE;
}
