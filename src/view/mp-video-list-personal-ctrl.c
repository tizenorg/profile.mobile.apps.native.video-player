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

#include <glib.h>

#include "mp-util-move.h"
#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-util-media-service.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-list-personal-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "vp-util.h"
#include "vp-file-util.h"
#include "mp-util-preference.h"
#include "mp-util-config.h"


typedef struct _PersonalPage {
	/*obj*/
	Evas_Object *pProgressbar;
	Evas_Object *pPopUpHandle;
	Evas_Object *pPopupLayout;
	/*data*/
	Ecore_Idler *pMoveItemIdlerHandle;
	Ecore_Timer *pStartMoveTimer;
	Ecore_Idler *pFinishMoveIdler;
	Ecore_Idler *pMoveFailedIdler;
	Ecore_Thread *pMoveItemThreadHandle;

	PersonalCtrlCbFunc PersonalCtrlUserCbFunc;

	int nTotalMoveItem;
	int nCompltedMoveItem;
	int nCompltedFolderMoveItem;
	int nPersonalViewStyle;
	int nCheckedEditListSize;
	unsigned long long nMovingSize;
	unsigned long long nTotalSize;

	bool *pCheckedItemsEditList;
	bool bCancelMove;
	bool bFolderMove;

} stPersonalPage;

static stPersonalPage *g_mpPersonalPage = NULL;


/*//////////////////////////////////////////////////////////*/
/* Internal function*/
/*//////////////////////////////////////////////////////////*/
static void mp_personal_ctrl_button_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo);

void mp_personal_ctrl_move_popup_handle(void)
{
	VideoLogInfo("");
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}
	g_mpPersonalPage->nTotalMoveItem = 0;
	g_mpPersonalPage->nCompltedMoveItem = 0;
	g_mpPersonalPage->nCheckedEditListSize = 0;

	g_mpPersonalPage->pProgressbar = NULL;
	g_mpPersonalPage->pCheckedItemsEditList = NULL;
	g_mpPersonalPage->pMoveItemThreadHandle = NULL;

	if (g_mpPersonalPage->pPopUpHandle) {
		eext_object_event_callback_del(g_mpPersonalPage->pPopUpHandle,
					       EEXT_CALLBACK_BACK,
					       mp_personal_ctrl_button_popup_cb);
	}
	MP_DEL_IDLER(g_mpPersonalPage->pMoveItemIdlerHandle);
	MP_DEL_TIMER(g_mpPersonalPage->pStartMoveTimer);
	MP_DEL_IDLER(g_mpPersonalPage->pFinishMoveIdler);
	MP_DEL_IDLER(g_mpPersonalPage->pMoveFailedIdler);
	/*MP_DEL_TIMER(pProgressbarTimer);*/
	MP_DEL_OBJ(g_mpPersonalPage->pPopUpHandle);
	if (g_mpPersonalPage->PersonalCtrlUserCbFunc) {
		g_mpPersonalPage->PersonalCtrlUserCbFunc();
	}
	MP_FREE_STRING(g_mpPersonalPage);


}

static void mp_personal_ctrl_button_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}
	g_mpPersonalPage->bCancelMove = TRUE;
	mp_util_move_set_cancel_state(g_mpPersonalPage->bCancelMove);

	/*if (g_mpPersonalPage->pMoveItemThreadHandle)
	   {
	   ecore_thread_cancel(g_mpPersonalPage->pMoveItemThreadHandle);
	   } */
}

static void __mp_personal_ctrl_update_percent(bool bUpdateFile,
		bool bUpdateSize)
{
	if (!g_mpPersonalPage || !g_mpPersonalPage->pPopupLayout) {
		VideoLogError("");
		return;
	}
	char *szSisePercent = NULL;
	char *szFilePercent = NULL;
	if (bUpdateSize) {
		double nPercent = 0.0;
		if (g_mpPersonalPage->nTotalSize < 1) {
			nPercent = 0.0;
		} else {
			nPercent =
				(double) g_mpPersonalPage->nMovingSize /
				(double) g_mpPersonalPage->nTotalSize;
		}
		nPercent = nPercent * 100;
		if (nPercent > (100 - 1)) {
			nPercent = 100.0;
		}
		szSisePercent = g_strdup_printf("%d%s", (int) nPercent, "%");
		elm_object_part_text_set(g_mpPersonalPage->pPopupLayout,
					 "elm.text.left", szSisePercent);
	}
	if (bUpdateFile) {
		if (g_mpPersonalPage->nCompltedMoveItem + 1 >
				g_mpPersonalPage->nTotalMoveItem) {
			g_mpPersonalPage->nCompltedMoveItem =
				g_mpPersonalPage->nTotalMoveItem - 1;
		}
		szFilePercent =
			g_strdup_printf("%d/%d",
					(g_mpPersonalPage->nCompltedMoveItem + 1),
					g_mpPersonalPage->nTotalMoveItem);
		elm_object_part_text_set(g_mpPersonalPage->pPopupLayout,
					 "elm.text.right", szFilePercent);
	}
	MP_FREE_STRING(szSisePercent);
	MP_FREE_STRING(szFilePercent);

}

static Eina_Bool __mp_personal_ctrl_finish_move_idler_cb(void *pUserData)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return FALSE;
	}

	g_mpPersonalPage->pFinishMoveIdler = NULL;


	mp_personal_ctrl_move_popup_handle();

	return FALSE;
}

static void __mp_personal_ctrl_warning_popup_cb(void *pUserData,
		Evas_Object *pObject,
		void *pEventInfo)
{
	VideoLogInfo("");
	mp_util_delete_popup_handle();

	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}
	mp_personal_ctrl_move_popup_handle();

}

static Eina_Bool __mp_personal_ctrl_move_failed_idler_cb(void *pUserData)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return FALSE;
	}

	g_mpPersonalPage->pMoveFailedIdler = NULL;
	if (g_mpPersonalPage->nPersonalViewStyle == 1) {
		mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR,
				     VIDEOS_NOTIPOP_MSG_MOVE_TO_NORMAL_STORAGE_FAILED,
				     __mp_personal_ctrl_warning_popup_cb);
	} else {
		mp_util_notify_popup(NULL, VIDEOS_COMMON_HEADER_ERROR,
				     VIDEOS_NOTIPOP_MSG_MOVE_TO_PERSONAL_STORAGE_FAILED,
				     __mp_personal_ctrl_warning_popup_cb);
	}

	return FALSE;
}

static void __mp_personal_ctrl_move_cb(euThreadFeedback msg_type,
				       unsigned long long size,
				       int nErrorCode, void *userData)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}
	g_mpPersonalPage->nMovingSize += size;

	if (nErrorCode == -1 || msg_type == MP_VIDEO_UPDATE_FAILED) {
		ecore_thread_feedback(g_mpPersonalPage->pMoveItemThreadHandle,
				      (void *) MP_VIDEO_UPDATE_FAILED);
	} else if (nErrorCode == 1 || msg_type == MP_VIDEO_UPDATE_CANCEL) {
		ecore_thread_cancel(g_mpPersonalPage->pMoveItemThreadHandle);
	} else if (nErrorCode == 0 || msg_type == MP_VIDEO_UPDATE) {
		ecore_thread_feedback(g_mpPersonalPage->pMoveItemThreadHandle,
				      (void *) MP_VIDEO_UPDATE);
	}
}

static bool __mp_personal_ctrl_move_get_total_size(unsigned long long
		*nTotalSize)
{

	if (!g_mpPersonalPage) {
		VideoLogError("");
		return FALSE;
	}

	if (!g_mpPersonalPage->pCheckedItemsEditList) {
		VideoLogError("The userdata and pCheckedItemsEditList is null.");

		return FALSE;
	}
	int nCounter = 0;
	unsigned long long nSize = 0;
	unsigned long long nSizeTmp = 0;
	char *szTmpFolderUrl = NULL;
	char *szSubTitle = NULL;
	char *szFileUrl = NULL;
	if (g_mpPersonalPage->bFolderMove) {
		for (nCounter = 0;
				nCounter < g_mpPersonalPage->nCheckedEditListSize;
				nCounter++) {
			if (g_mpPersonalPage->pCheckedItemsEditList[nCounter]) {
				szTmpFolderUrl =
					mp_util_svc_get_video_folder_url(nCounter);
				mp_util_move_get_total_dir_size(szTmpFolderUrl,
								&nSizeTmp);
				MP_FREE_STRING(szTmpFolderUrl);
				nSize += nSizeTmp;
			}
		}
	} else {
		for (nCounter = 0;
				nCounter < g_mpPersonalPage->nCheckedEditListSize;
				nCounter++) {
			if (g_mpPersonalPage->pCheckedItemsEditList[nCounter]) {
				nSizeTmp = mp_util_svc_get_video_item_size(nCounter);
				nSize += nSizeTmp;

				/*Support subtitle size.*/
				nSizeTmp = 0;
				szFileUrl = mp_util_svc_get_video_url(nCounter);
				mp_util_get_subtitle_path(szFileUrl, &szSubTitle);
				if (szSubTitle) {
					mp_util_get_file_size_by_path(szSubTitle, &nSizeTmp);
					nSize += nSizeTmp;
				}
				MP_FREE_STRING(szFileUrl);
				MP_FREE_STRING(szSubTitle);
			}
		}

	}

	*nTotalSize = nSize;
	return TRUE;
}

static void __mp_personal_ctrl_move_item_thread_main_cb(void *pUserData,
		Ecore_Thread *
		thread)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}

	if (!g_mpPersonalPage->pCheckedItemsEditList) {
		VideoLogError("The userdata and pCheckedItemsEditList is null.");

		ecore_thread_feedback(g_mpPersonalPage->pMoveItemThreadHandle,
				      (void *) MP_VIDEO_UPDATE_ERROR);
		return;
	}
	unsigned long long nSize = 0;
	__mp_personal_ctrl_move_get_total_size(&nSize);
	g_mpPersonalPage->nTotalSize = nSize;
	g_mpPersonalPage->nMovingSize = 0;
	VideoLogInfo("nMovefolderTotal:%lld", g_mpPersonalPage->nTotalSize);

	/*mp_util_svc_print_list_info();*/
	int nCounter = 0;
	char *pDestFile = NULL;
	char *szFileUrl = NULL;
	char *szMediaUrlForPreview = NULL;
	char *szTmpFolderUrl = NULL;
	char *szSubTitle = NULL;
	bool bPersonal = FALSE;
	int nRet = FALSE;
	while ((g_mpPersonalPage->nCompltedMoveItem <
			g_mpPersonalPage->nTotalMoveItem)
			&& (!g_mpPersonalPage->bCancelMove)) {

		nCounter = 0;
		for (nCounter = 0;
				nCounter < g_mpPersonalPage->nCheckedEditListSize;
				nCounter++) {
			if (g_mpPersonalPage->pCheckedItemsEditList[nCounter]) {
				__mp_personal_ctrl_update_percent(TRUE, FALSE);
				if (g_mpPersonalPage->bFolderMove) {
					szTmpFolderUrl =
						mp_util_svc_get_video_folder_url(nCounter);
					VideoSecureLogInfo
					("pRemoveCheckedItemsEditListmove=%s,%d",
					 szTmpFolderUrl, nCounter);

					bPersonal =
						mp_util_check_video_personal_status
						(szTmpFolderUrl);

					if (bPersonal) {
						pDestFile = g_strdup(VIDEO_DOWNLOAD_FILE_DIR);
					} else {
						pDestFile =
							g_strdup(VIDEO_UTIL_PERSONAL_HEAD_STR);
					}
					nRet =
						mp_util_move_push(szTmpFolderUrl, pDestFile,
								  __mp_personal_ctrl_move_cb,
								  pUserData,
								  g_mpPersonalPage->nTotalSize);
					if (nRet == -1) {
						VideoLogInfo("move failed");
						MP_FREE_STRING(pDestFile);
						MP_FREE_STRING(szTmpFolderUrl);
						ecore_thread_feedback(g_mpPersonalPage->
								      pMoveItemThreadHandle,
								      (void *)
								      MP_VIDEO_UPDATE_FAILED);
						return;
						/*usleep(100000);*/
					} else if (nRet == 0) {
						mp_util_svc_remove_folder_list_item_from_list
						(nCounter);
						vp_file_recursive_rm(szTmpFolderUrl);
					} else if (nRet == 1) {
						VideoLogInfo("move cancel");
						MP_FREE_STRING(pDestFile);
						MP_FREE_STRING(szTmpFolderUrl);
						ecore_thread_cancel(g_mpPersonalPage->
								    pMoveItemThreadHandle);
						return;
					}
					MP_FREE_STRING(pDestFile);
					MP_FREE_STRING(szTmpFolderUrl);

				} else {
					szFileUrl = mp_util_svc_get_video_url(nCounter);
					VideoSecureLogInfo
					("pRemoveCheckedItemsEditListmove=%s,%d",
					 szFileUrl, nCounter);
					if (szFileUrl) {
						szMediaUrlForPreview =
							mp_util_config_get_preview();
						if (szMediaUrlForPreview) {
							if (!strcmp(szMediaUrlForPreview, szFileUrl)
									&& !bPersonal) {
								VideoLogInfo("Erase preview url.");
								mp_util_config_set_preview(NULL);
							}
						}
						MP_FREE_STRING(szMediaUrlForPreview);

						bPersonal =
							mp_util_check_video_personal_status
							(szFileUrl);

						if (bPersonal) {
							pDestFile = g_strdup(VIDEO_DOWNLOAD_FILE_DIR);
						} else {
							pDestFile =
								g_strdup(VIDEO_UTIL_PERSONAL_HEAD_STR);
						}

						nRet =
							mp_util_move_push(szFileUrl, pDestFile,
									  __mp_personal_ctrl_move_cb,
									  pUserData,
									  g_mpPersonalPage->
									  nTotalSize);
						if (nRet == -1) {
							VideoLogInfo("move failed");
							MP_FREE_STRING(pDestFile);
							MP_FREE_STRING(szFileUrl);
							ecore_thread_feedback(g_mpPersonalPage->
									      pMoveItemThreadHandle,
									      (void *)
									      MP_VIDEO_UPDATE_FAILED);
							return;
							/*usleep(100000);*/
						} else if (nRet == 0) {
							mp_util_svc_remove_list_item_from_list
							(nCounter);
							unlink(szFileUrl);
						} else if (nRet == 1) {
							VideoLogInfo("move cancel");
							MP_FREE_STRING(pDestFile);
							MP_FREE_STRING(szFileUrl);
							ecore_thread_cancel(g_mpPersonalPage->
									    pMoveItemThreadHandle);
							return;
						}

						VideoSecureLogInfo("Move file url : %s:%s",
								   szFileUrl, pDestFile);
						/*Support subtitle moving, But only for file, folder has been implemented in move.c.*/
						/*begin*/
						mp_util_get_subtitle_path(szFileUrl, &szSubTitle);
						if (szSubTitle) {
							nRet =
								mp_util_move_push(szSubTitle, pDestFile,
										  __mp_personal_ctrl_move_cb,
										  pUserData,
										  g_mpPersonalPage->
										  nTotalSize);
							VideoLogInfo("szSubTitle move result = %d",
								     nRet);
						}
						MP_FREE_STRING(szSubTitle);
						/*end*/
						MP_FREE_STRING(pDestFile);
						MP_FREE_STRING(szFileUrl);
					}
				}

				int nMoveItemCounter = 0;
				for (nMoveItemCounter = nCounter;
						nMoveItemCounter <
						g_mpPersonalPage->nCheckedEditListSize - 1;
						nMoveItemCounter++) {
					g_mpPersonalPage->
					pCheckedItemsEditList[nMoveItemCounter] =
						g_mpPersonalPage->
						pCheckedItemsEditList[nMoveItemCounter + 1];
				}
				g_mpPersonalPage->nCheckedEditListSize--;
				++g_mpPersonalPage->nCompltedMoveItem;
				break;
			}
		}

		if (g_mpPersonalPage->nCompltedMoveItem >=
				g_mpPersonalPage->nTotalMoveItem) {
			VideoLogInfo("nCompltedMoveItem == nTotalMoveItem");
			ecore_thread_feedback(g_mpPersonalPage->pMoveItemThreadHandle,
					      (void *) MP_VIDEO_UPDATE_FINISH);
			return;
		}
	}


}

static void __mp_personal_ctrl_move_thread_cancel(void *data,
		Ecore_Thread *thread)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}
	VideoLogInfo("");
	/*ecore_thread_feedback(g_mpPersonalPage->pMoveItemThreadHandle, (void *)MP_VIDEO_PERSONAL_UPDATE_CANCEL);*/
	/*g_mpPersonalPage->bCancelMove = TRUE;*/
	mp_personal_ctrl_move_popup_handle();
}

static void __mp_personal_ctrl_move_thread_finish(void *pUserData,
		Ecore_Thread *thread)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}
	VideoLogInfo("");
	/*sleep 0.2s to wait for cancel and failed signal.*/
	/*MP_DEL_IDLER(g_mpPersonalPage->pFinishMoveIdler);*/
	/*g_mpPersonalPage->pFinishMoveIdler = ecore_idler_add(__mp_personal_ctrl_finish_move_idler_cb, pUserData);*/

}

static void __mp_personal_ctrl_thread_notify_cb(void *pUserData,
		Ecore_Thread *thread,
		void *msg_data)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return;
	}

	double nMoveRate = 0.0;
	euThreadFeedback feedback = (euThreadFeedback) msg_data;

	switch (feedback) {
	case MP_VIDEO_UPDATE_CANCEL:
		VideoLogInfo("MP_VIDEO_PERSONAL_UPDATE_CANCEL");
		mp_personal_ctrl_move_popup_handle();
		break;

	case MP_VIDEO_UPDATE:
		if (g_mpPersonalPage->nTotalSize > 0) {
			nMoveRate =
				((double) g_mpPersonalPage->nMovingSize) /
				((double) g_mpPersonalPage->nTotalSize);
			__mp_personal_ctrl_update_percent(FALSE, TRUE);
			/*VideoLogInfo("nMoveRate:%f:%lld:%lld", nMoveRate, g_mpPersonalPage->nMovingSize,  g_mpPersonalPage->nTotalSize);*/
			if (g_mpPersonalPage->pProgressbar) {
				elm_progressbar_value_set(g_mpPersonalPage->pProgressbar,
							  nMoveRate);
			}
		}

		break;

	case MP_VIDEO_UPDATE_FINISH:
		MP_DEL_IDLER(g_mpPersonalPage->pFinishMoveIdler);
		g_mpPersonalPage->pFinishMoveIdler =
			ecore_idler_add(__mp_personal_ctrl_finish_move_idler_cb,
					pUserData);

		break;

	case MP_VIDEO_UPDATE_FAILED:
		VideoLogInfo("MP_VIDEO_UPDATE_FAILED");
		MP_DEL_IDLER(g_mpPersonalPage->pMoveFailedIdler);
		MP_DEL_OBJ(g_mpPersonalPage->pPopUpHandle);
		g_mpPersonalPage->pMoveFailedIdler =
			ecore_idler_add(__mp_personal_ctrl_move_failed_idler_cb,
					pUserData);
		break;

	case MP_VIDEO_UPDATE_ERROR:
		mp_personal_ctrl_move_popup_handle();
		break;

	default:
		VideoLogInfo("invalid feedback");
		break;
	}


}

static Eina_Bool __mp_personal_ctrl_start_move_timer_cb(void *pUserData)
{
	if (!g_mpPersonalPage) {
		VideoLogError("");
		return FALSE;
	}
	if (mp_util_set_dest_file(VIDEO_DOWNLOAD_FILE_DIR) == -1) {
		VideoLogError("mp_util_set_dest_file failed");
		return FALSE;
	}

	g_mpPersonalPage->pStartMoveTimer = NULL;
	/*pProgressbarTimer = ecore_timer_add(0.05, MpVideoPersonalProgressbarTimerCb, pUserData);*/
	/*pMoveItemIdlerHandle = ecore_idler_add(MpVideoPersonalMoveItemIdlerCb, pUserData);*/

	g_mpPersonalPage->pMoveItemThreadHandle =
		ecore_thread_feedback_run
		(__mp_personal_ctrl_move_item_thread_main_cb,
		 __mp_personal_ctrl_thread_notify_cb,
		 __mp_personal_ctrl_move_thread_finish,
		 __mp_personal_ctrl_move_thread_cancel, (void *) pUserData,
		 EINA_TRUE);
	return FALSE;
}

static bool __mp_personal_ctrl_create_progress_popup(Evas_Object *
		pHandle,
		void *pUserData)
{
	if (!g_mpPersonalPage || !g_mpPersonalPage->pPopUpHandle) {
		VideoLogError("g_mpPersonalPage is null.");
		return FALSE;
	}
	char edj_path[1024] = {0};

	char *path = app_get_resource_path();
	snprintf(edj_path, 1024, "%s%s/%s", path, "edje", VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ);
	free(path);
	Evas_Object *pPopupLayout =
		elm_layout_add(g_mpPersonalPage->pPopUpHandle);
	elm_layout_file_set(pPopupLayout, edj_path,
			    "popup_center_progress_move_copy");
	evas_object_size_hint_weight_set(pPopupLayout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	if (!pPopupLayout) {
		VideoLogError("g_mpPersonalPage is null.");
		return FALSE;

	}
	g_mpPersonalPage->pPopupLayout = pPopupLayout;


	Evas_Object *pProgressbar =
		elm_progressbar_add(g_mpPersonalPage->pPopUpHandle);
	elm_object_style_set(pProgressbar, "list_progress");
	elm_progressbar_horizontal_set(pProgressbar, EINA_TRUE);
	evas_object_size_hint_align_set(pProgressbar, EVAS_HINT_FILL,
					EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(pProgressbar, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_progressbar_value_set(pProgressbar, 0.0);
	evas_object_show(pProgressbar);
	g_mpPersonalPage->pProgressbar = pProgressbar;


	if (g_mpPersonalPage->nPersonalViewStyle == 1) {
		elm_object_domain_translatable_part_text_set(pPopupLayout,
				"elm.title",
				VIDEOS_STRING,
				MP_TEXT_MOVING);
		elm_object_domain_part_text_translatable_set(pPopupLayout,
				"elm.title",
				VIDEOS_STRING,
				EINA_TRUE);
	} else {
		elm_object_domain_translatable_part_text_set(pPopupLayout,
				"elm.title",
				VIDEOS_STRING,
				MP_TEXT_MOVING);
		elm_object_domain_part_text_translatable_set(pPopupLayout,
				"elm.title",
				VIDEOS_STRING,
				EINA_TRUE);
	}

	elm_object_part_content_set(pPopupLayout, "elm.swallow.content",
				    pProgressbar);

	__mp_personal_ctrl_update_percent(TRUE, TRUE);

	elm_object_content_set(g_mpPersonalPage->pPopUpHandle, pPopupLayout);
	Evas_Object *pButton = elm_button_add(g_mpPersonalPage->pPopUpHandle);
	elm_object_style_set(pButton, "popup_button/default");
	elm_object_domain_translatable_text_set(pButton, VIDEOS_STRING,
						VIDEOS_COMMON_BUTTON_CANCEL_IDS);
	elm_object_part_content_set(g_mpPersonalPage->pPopUpHandle, "button1",
				    pButton);
	evas_object_smart_callback_add(pButton, "clicked",
				       mp_personal_ctrl_button_popup_cb,
				       pUserData);

	evas_object_show(g_mpPersonalPage->pPopUpHandle);
	return TRUE;

}

/*//////////////////////////////////////////////////////////*/
/* External function*/
/*//////////////////////////////////////////////////////////*/
bool mp_personal_ctrl_show(bool *pCheckedItemsEditList,
			   int pCheckedItemsEditListSize, int nViewStyle,
			   bool bFolderMove,
			   PersonalCtrlCbFunc pPersonalCtrlCb,
			   void *pUserData)
{
	if (!pCheckedItemsEditList) {
		VideoLogError("No have popup message is null.");
		return FALSE;
	}

	mp_personal_ctrl_move_popup_handle();
	g_mpPersonalPage =
		(stPersonalPage *) calloc(1, sizeof(stPersonalPage));
	if (!g_mpPersonalPage) {
		VideoLogError("g_mpPersonalPage is NULL");
		return FALSE;
	}

	VideoLogInfo("pCheckedItemsEditListSize : %d",
		     pCheckedItemsEditListSize);


	g_mpPersonalPage->PersonalCtrlUserCbFunc = pPersonalCtrlCb;
	g_mpPersonalPage->nCheckedEditListSize = pCheckedItemsEditListSize;
	g_mpPersonalPage->pCheckedItemsEditList = pCheckedItemsEditList;
	g_mpPersonalPage->nPersonalViewStyle = nViewStyle;
	g_mpPersonalPage->bCancelMove = FALSE;
	g_mpPersonalPage->bFolderMove = bFolderMove;
	g_mpPersonalPage->nTotalMoveItem = 0;
	g_mpPersonalPage->nTotalSize = 0;
	g_mpPersonalPage->nMovingSize = 0;

	int nCounter = 0;
	for (nCounter = 0; nCounter < g_mpPersonalPage->nCheckedEditListSize;
			nCounter++) {
		if (g_mpPersonalPage->pCheckedItemsEditList[nCounter]) {
			++g_mpPersonalPage->nTotalMoveItem;
		}
	}

	VideoLogInfo("nTotalMoveItem : %d", g_mpPersonalPage->nTotalMoveItem);

	g_mpPersonalPage->pPopUpHandle =
		elm_popup_add(mp_util_get_main_window_handle());

	eext_object_event_callback_add(g_mpPersonalPage->pPopUpHandle,
				       EEXT_CALLBACK_BACK,
				       mp_personal_ctrl_button_popup_cb,
				       NULL);

	bool bRet =
		__mp_personal_ctrl_create_progress_popup(g_mpPersonalPage->
				pPopUpHandle, pUserData);
	if (!bRet) {
		VideoLogError("popup is failed");
		mp_personal_ctrl_move_popup_handle();
		return FALSE;
	}
	MP_DEL_TIMER(g_mpPersonalPage->pStartMoveTimer);
	g_mpPersonalPage->pStartMoveTimer =
		ecore_timer_add(0.3, __mp_personal_ctrl_start_move_timer_cb,
				pUserData);

	return TRUE;
}
