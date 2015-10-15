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
#include <glib.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-value-define.h"
#include "mp-video-string-define.h"
#include "mp-video-type-define.h"

#include "mp-video-detail-view.h"
#include "mp-video-info-ctrl.h"
#include "mp-util-media-service.h"
#include "mp-util-widget-ctrl.h"
#include "mp-video-list-sort-ctrl.h"
#include "mp-video-list-view-as-ctrl.h"
#include "mp-video-view-popup-ctrl.h"
#include "vp-util.h"

typedef struct _VideoDetailView {
	/*obj*/
	void *pNaviFrameHandle;
	void *pNaviFrameItem;
	Evas_Object *pGenList;
	Evas_Object *pBox;

	/*data*/
	Elm_Genlist_Item_Class *stDetailItc;
	mpDetailViewCbFunc DetailViewUserCbFunc;
	stDetailInfo *pDetailInfo;
	char *pMediaId;
	bool isFolder;
	int nListCount;

} stDetailView;


static stDetailView *g_pDetailViewHandle = NULL;


/*//////////////////////////////////////////////////////////*/
/* Internal function*/
/*//////////////////////////////////////////////////////////*/
static void __mp_detail_view_rotate_cb(void *data, Evas_Object *obj,
				       void *event_info);
static Eina_Bool __mp_detail_view_cancel_btn_cb(void *pUserData,
		Elm_Object_Item *pItem);
/*static void __mp_detail_view_mouse_up_cb(void *pUserData, Evas *pEvas, Evas_Object *pObject, void *pEventInfo);*/

void mp_detail_view_delete_handle(void)
{
	VideoLogInfo("");
	if (!g_pDetailViewHandle) {
		VideoLogError("g_pDetailViewHandle is NULL");
		return;
	}

	evas_object_smart_callback_del((Evas_Object *)
				       mp_util_get_main_window_handle(),
				       "rotation,changed",
				       __mp_detail_view_rotate_cb);

	if (g_pDetailViewHandle->pDetailInfo) {
		mp_detail_view_free_detail_info(g_pDetailViewHandle->pDetailInfo);
		g_pDetailViewHandle->pDetailInfo = NULL;

	}
	MP_FREE_STRING(g_pDetailViewHandle->pMediaId);

	MP_DEL_ITC(g_pDetailViewHandle->stDetailItc);
	MP_DEL_OBJ(g_pDetailViewHandle->pGenList);
	MP_DEL_OBJ(g_pDetailViewHandle->pBox);
	MP_FREE_STRING(g_pDetailViewHandle);
}

static void __mp_detail_view_reset(void)
{
	VideoLogInfo("");
	if (!g_pDetailViewHandle) {
		VideoLogError("g_pDetailViewHandle is NULL");
		return;
	}

	evas_object_smart_callback_del((Evas_Object *)
				       mp_util_get_main_window_handle(),
				       "rotation,changed",
				       __mp_detail_view_rotate_cb);

	if (g_pDetailViewHandle->pDetailInfo) {
		mp_detail_view_free_detail_info(g_pDetailViewHandle->pDetailInfo);
		g_pDetailViewHandle->pDetailInfo = NULL;

	}
	MP_FREE_STRING(g_pDetailViewHandle->pMediaId);

	MP_DEL_ITC(g_pDetailViewHandle->stDetailItc);
	g_pDetailViewHandle->pGenList = NULL;
	g_pDetailViewHandle->pBox = NULL;

	MP_FREE_STRING(g_pDetailViewHandle);
}

static char *__mp_detail_view_genlist_text_get_cb(const void *pUserData,
		Evas_Object *pObj,
		const char *pPart)
{
	if (!pUserData || !g_pDetailViewHandle) {
		VideoLogError("pUserData is NULL");
		return NULL;
	}

	char *szTxt = (char *) pUserData;

	VideoLogWarning("%s : %s", pPart, szTxt);

	if (!g_strcmp0(pPart, "elm.text.main.left.top")) {

		return strdup(szTxt);
	} else if (!g_strcmp0(pPart, "elm.text.sub.left.bottom")) {

		stDetailInfo *pDetailInfo = g_pDetailViewHandle->pDetailInfo;

		if (pDetailInfo == NULL) {
			return NULL;
		}

		if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_TITLE)) {
			if (pDetailInfo->szTitle)
				return elm_entry_utf8_to_markup(pDetailInfo->szTitle);
		} else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_FORMAT)) {
			if (pDetailInfo->szFormat)
				return elm_entry_utf8_to_markup(pDetailInfo->szFormat);
		}
		/*else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_DATE)) {
		   if (pDetailInfo->szDate)
		   return elm_entry_utf8_to_markup(pDetailInfo->szDate);
		   } */
		else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_SIZE)) {
			if (pDetailInfo->szSize)
				return elm_entry_utf8_to_markup(pDetailInfo->szSize);
		} else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_RESOLUTION)) {
			if (pDetailInfo->szResolution)
				return elm_entry_utf8_to_markup(pDetailInfo->
								szResolution);
		} else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_LAST_MODIFIED)) {
			if (pDetailInfo->szLastModifiedTime)
				return elm_entry_utf8_to_markup(pDetailInfo->
								szLastModifiedTime);
		} else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_LOCATION)) {
			if (pDetailInfo->szLocation)
				return elm_entry_utf8_to_markup(pDetailInfo->szLocation);
		}
		/*else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_LATITUDE)) {
		   if (pDetailInfo->szLatitude)  {
		   if (atof(pDetailInfo->szLatitude) != VIDEO_DETAIL_VALUE_GPS_DEFAULT)
		   return elm_entry_utf8_to_markup(pDetailInfo->szLatitude);
		   else
		   return g_strdup(VIDEOS_DETAILVIEW_BODY_UNKNOWN);
		   }
		   }
		   else if (!g_strcmp0(szTxt, VIDEOS_DETAILVIEW_BODY_LONGITUDE)) {
		   if (pDetailInfo->szLongitude) {
		   if (atof(pDetailInfo->szLongitude) != VIDEO_DETAIL_VALUE_GPS_DEFAULT)
		   return elm_entry_utf8_to_markup(pDetailInfo->szLongitude);
		   else
		   return g_strdup(VIDEOS_DETAILVIEW_BODY_UNKNOWN);
		   }
		   } */
	}

	return NULL;
}

static Eina_Bool __mp_detail_view_cancel_btn_cb(void *pUserData,
		Elm_Object_Item *pItem)
{
	if (g_pDetailViewHandle && g_pDetailViewHandle->DetailViewUserCbFunc) {
		g_pDetailViewHandle->DetailViewUserCbFunc();
	}

	return EINA_TRUE;
}

static void __mp_detail_view_genlist_realized(void *data,
		Evas_Object *obj,
		void *event_info)
{
	if (!event_info || !obj) {
		VideoLogError("event info is invalid");
		return;
	}
	Elm_Object_Item *pItem = elm_genlist_last_item_get(obj);

	if (pItem) {
		elm_object_item_signal_emit(pItem, "elm,state,bottomline,hide",
					    "");
	}
}

static bool __mp_detail_view_add_genlist_item(Evas_Object *pObj,
		MpListViewAsType nViewType)
{
	if (!pObj || !g_pDetailViewHandle
			|| !g_pDetailViewHandle->pDetailInfo) {
		VideoLogError("g_pDetailViewHandle is NULL");
		return FALSE;
	}
	MP_DEL_ITC(g_pDetailViewHandle->stDetailItc);
	g_pDetailViewHandle->stDetailItc = elm_genlist_item_class_new();

	g_pDetailViewHandle->stDetailItc->version =
		ELM_GENLIST_ITEM_CLASS_VERSION;
	g_pDetailViewHandle->stDetailItc->item_style = "2line.top";
	g_pDetailViewHandle->stDetailItc->func.text_get =
		(void *) __mp_detail_view_genlist_text_get_cb;
	g_pDetailViewHandle->stDetailItc->func.content_get = NULL;
	g_pDetailViewHandle->stDetailItc->func.state_get = NULL;
	g_pDetailViewHandle->stDetailItc->func.del = NULL;

	int nListCount = 0;

	Elm_Object_Item *pItem = NULL;
	stDetailInfo *pDetailInfo = g_pDetailViewHandle->pDetailInfo;

	if (!g_pDetailViewHandle->isFolder) {
		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *) VIDEOS_DETAILVIEW_BODY_TITLE,
						NULL, ELM_GENLIST_ITEM_NONE, NULL,
						NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *) VIDEOS_DETAILVIEW_BODY_SIZE,
						NULL, ELM_GENLIST_ITEM_NONE, NULL,
						NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *)
						VIDEOS_DETAILVIEW_BODY_FORMAT, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *)
						VIDEOS_DETAILVIEW_BODY_RESOLUTION,
						NULL, ELM_GENLIST_ITEM_NONE, NULL,
						NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		/*pItem = elm_genlist_item_append(pObj, g_pDetailViewHandle->stDetailItc, (void *)VIDEOS_DETAILVIEW_BODY_DATE, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		   elm_genlist_item_select_mode_set(pItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		   nListCount++; */

		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *)
						VIDEOS_DETAILVIEW_BODY_LAST_MODIFIED,
						NULL, ELM_GENLIST_ITEM_NONE, NULL,
						NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *)
						VIDEOS_DETAILVIEW_BODY_LOCATION, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		/*pItem = elm_genlist_item_append(pObj, g_pDetailViewHandle->stDetailItc, (void *)_(VIDEOS_DETAILVIEW_BODY_LATITUDE), NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		   elm_genlist_item_select_mode_set(pItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		   nListCount++;

		   pItem = elm_genlist_item_append(pObj, g_pDetailViewHandle->stDetailItc, (void *)_(VIDEOS_DETAILVIEW_BODY_LONGITUDE), NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		   elm_genlist_item_select_mode_set(pItem, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		   nListCount++; */
	} else {
		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *) VIDEOS_DETAILVIEW_BODY_TITLE,
						NULL, ELM_GENLIST_ITEM_NONE, NULL,
						NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;

		if (pDetailInfo->szSize) {
			pItem =
				elm_genlist_item_append(pObj,
							g_pDetailViewHandle->stDetailItc,
							(void *)
							VIDEOS_DETAILVIEW_BODY_SIZE, NULL,
							ELM_GENLIST_ITEM_NONE, NULL,
							NULL);
			elm_genlist_item_select_mode_set(pItem,
							 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			nListCount++;
		}

		pItem =
			elm_genlist_item_append(pObj,
						g_pDetailViewHandle->stDetailItc,
						(void *)
						VIDEOS_DETAILVIEW_BODY_LOCATION, NULL,
						ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(pItem,
						 ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		nListCount++;
	}

	g_pDetailViewHandle->nListCount = nListCount;

	return TRUE;
}

static void __mp_detail_view_rotate_cb(void *data, Evas_Object *obj,
				       void *event_info)
{
	if (!g_pDetailViewHandle) {
		VideoLogError("__mp_detail_view_rotate_cb IS null");
		return;
	}
	if (g_pDetailViewHandle->nListCount < VIDEO_POPUP_MIN_ITEMS) {
		return;
	}
	mp_widget_ctrl_set_popup_min_size(g_pDetailViewHandle->pBox,
					  g_pDetailViewHandle->nListCount,
					  VIDEOS_POPUP_114);

}

/*//////////////////////////////////////////////////////////*/
/* External function*/
/*//////////////////////////////////////////////////////////*/

void mp_detail_view_free_detail_info(stDetailInfo *pDetailInfo)
{
	if (pDetailInfo) {
		MP_FREE_STRING(pDetailInfo->szTitle);
		/*MP_FREE_STRING(pDetailInfo->szDate);*/
		MP_FREE_STRING(pDetailInfo->szFormat);
		MP_FREE_STRING(pDetailInfo->szSize);
		MP_FREE_STRING(pDetailInfo->szResolution);
		/*MP_FREE_STRING(pDetailInfo->szLatitude);*/
		/*MP_FREE_STRING(pDetailInfo->szLongitude);*/
		MP_FREE_STRING(pDetailInfo->szLocation);
		MP_FREE_STRING(pDetailInfo);
	}
}

static void __mp_detail_view_base_layout_del_cb(void *pUserData, Evas *e,
		Evas_Object *pObject,
		void *pEventInfo)
{
	__mp_detail_view_reset();
}

void __mp_details_soft_back_button_cb(void *data, Evas_Object *obj,
				      const char *emission,
				      const char *source)
{
	Evas_Object *pTopNaviFrame = NULL;
	pTopNaviFrame =
		elm_naviframe_item_pop(g_pDetailViewHandle->pNaviFrameHandle);
	evas_object_del(pTopNaviFrame);
}

void mp_detail_view_push(void *pNaviFrame, bool IsFolder,
			 const char *pMediaId, stDetailInfo *pDetailInfo)
{
	VideoLogInfo("");

	if (!pDetailInfo) {
		VideoLogError("invalid detail");
		return;
	}
	mp_detail_view_delete_handle();
	g_pDetailViewHandle =
		(stDetailView *) calloc(1, sizeof(stDetailView));
	if (!g_pDetailViewHandle) {
		VideoLogError("g_pDetailViewHandle is NULL");
		return;
	}

	g_pDetailViewHandle->pDetailInfo = calloc(1, sizeof(stDetailInfo));
	if (g_pDetailViewHandle->pDetailInfo == NULL) {
		VideoLogError("pDetailView alloc fail");
		mp_detail_view_delete_handle();
		return;
	}

	g_pDetailViewHandle->pMediaId = g_strdup(pMediaId);
	int nViewType = mp_view_as_ctrl_get_type();

	if (IsFolder) {
		VideoSecureLogInfo("%s,%s %s", pDetailInfo->szTitle,
				   pDetailInfo->szSize, pDetailInfo->szLocation);
		g_pDetailViewHandle->pDetailInfo->szTitle =
			g_strdup(pDetailInfo->szTitle);
		g_pDetailViewHandle->pDetailInfo->szSize =
			g_strdup(pDetailInfo->szSize);
		g_pDetailViewHandle->pDetailInfo->szLocation =
			g_strdup(pDetailInfo->szLocation);
	} else {
		VideoSecureLogInfo("%s,%s,%s,%s,%s", pDetailInfo->szTitle,
				   pDetailInfo->szFormat,
				   pDetailInfo->szResolution,
				   pDetailInfo->szLocation);
		g_pDetailViewHandle->pDetailInfo->szTitle =
			g_strdup(pDetailInfo->szTitle);
		/*g_pDetailViewHandle->pDetailInfo->szDate =  g_strdup(pDetailInfo->szDate);*/
		g_pDetailViewHandle->pDetailInfo->szFormat =
			g_strdup(pDetailInfo->szFormat);
		g_pDetailViewHandle->pDetailInfo->szSize =
			g_strdup(pDetailInfo->szSize);
		g_pDetailViewHandle->pDetailInfo->szLastModifiedTime =
			g_strdup(pDetailInfo->szLastModifiedTime);
		g_pDetailViewHandle->pDetailInfo->szResolution =
			g_strdup(pDetailInfo->szResolution);
		/*g_pDetailViewHandle->pDetailInfo->szLatitude = g_strdup(pDetailInfo->szLatitude);*/
		/*g_pDetailViewHandle->pDetailInfo->szLongitude = g_strdup(pDetailInfo->szLongitude);*/
		g_pDetailViewHandle->pDetailInfo->szLocation =
			g_strdup(pDetailInfo->szLocation);
	}

	g_pDetailViewHandle->DetailViewUserCbFunc = NULL;

	g_pDetailViewHandle->pNaviFrameHandle = pNaviFrame;
	g_pDetailViewHandle->isFolder = IsFolder;

	evas_object_smart_callback_add((Evas_Object *)
				       mp_util_get_main_window_handle(),
				       "rotation,changed",
				       __mp_detail_view_rotate_cb, NULL);


	/* Create genlist handle*/
	g_pDetailViewHandle->pGenList =
		elm_genlist_add(g_pDetailViewHandle->pNaviFrameHandle);
	evas_object_size_hint_weight_set(g_pDetailViewHandle->pGenList,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(g_pDetailViewHandle->pGenList,
					EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(g_pDetailViewHandle->pGenList,
				       "realized",
				       __mp_detail_view_genlist_realized,
				       NULL);

	elm_genlist_mode_set(g_pDetailViewHandle->pGenList,
			     ELM_LIST_COMPRESS);

	__mp_detail_view_add_genlist_item(g_pDetailViewHandle->pGenList,
					  nViewType);
	evas_object_event_callback_add(g_pDetailViewHandle->pGenList,
				       EVAS_CALLBACK_DEL,
				       __mp_detail_view_base_layout_del_cb,
				       NULL);

	g_pDetailViewHandle->pNaviFrameItem =
		elm_naviframe_item_push(g_pDetailViewHandle->pNaviFrameHandle,
					VIDEOS_DETAILVIEW_HEADER_DETAILS, NULL,
					NULL, g_pDetailViewHandle->pGenList,
					NULL);
	elm_naviframe_item_pop_cb_set(g_pDetailViewHandle->pNaviFrameItem,
				      __mp_detail_view_cancel_btn_cb, NULL);

	Evas_Object *back_btn =
		elm_button_add(g_pDetailViewHandle->pNaviFrameHandle);
	elm_object_style_set(back_btn, "back");
	elm_object_signal_callback_add(back_btn, "elm,action,click", "",
				       __mp_details_soft_back_button_cb,
				       (void *) g_pDetailViewHandle);
	elm_object_item_part_content_set(g_pDetailViewHandle->pNaviFrameItem,
					 "title_left_btn", back_btn);
	evas_object_show(back_btn);
}

bool mp_detail_view_get_video_detail_info(int nVideoItemIndex,
		stDetailInfo **pDetailInfo)
{
	char *szFilePath = NULL;
	char *szVideoTitle = NULL;

	char szTmpStr[FORMAT_LEN_MAX] = { 0, };
	char szTmpFileDate[FORMAT_LEN_MAX] = { 0, };
	char szTmpFileExtension[FORMAT_LEN_MAX] = { 0, };
	char szTmpFileSize[FORMAT_LEN_MAX] = { 0, };
	char *szTmpModifiedTime = NULL;

	/*double dLongitude = 0.0;*/
	/*double dLatitude = 0.0;*/

	int nWidth = 0;
	int nHeight = 0;
	stDetailInfo *pDetail = NULL;

	szFilePath = mp_util_svc_get_video_url(nVideoItemIndex);
	szVideoTitle = mp_util_svc_get_video_title(nVideoItemIndex);

	mp_info_ctrl_get_file_info(szFilePath, szTmpFileDate,
				   FORMAT_LEN_MAX - 1, szTmpFileExtension,
				   FORMAT_LEN_MAX - 1, szTmpFileSize,
				   FORMAT_LEN_MAX - 1);

	nWidth = mp_util_svc_get_video_item_width(nVideoItemIndex);
	nHeight = mp_util_svc_get_video_item_height(nVideoItemIndex);
	/*dLongitude = mp_util_svc_get_video_item_longitude(nVideoItemIndex);*/
	/*dLatitude = mp_util_svc_get_video_item_latitude(nVideoItemIndex);*/
	szTmpModifiedTime =
		mp_util_svc_get_video_item_modified_time(nVideoItemIndex);

	pDetail = calloc(1, sizeof(stDetailInfo));
	if (pDetail == NULL) {
		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szVideoTitle);
		VideoLogError("pDetailPopup alloc fail");
		return FALSE;
	}

	pDetail->szTitle = g_strdup(szVideoTitle);
	pDetail->szLocation = vp_util_convert_file_location(szFilePath);
	MP_FREE_STRING(szFilePath);
	MP_FREE_STRING(szVideoTitle);

	snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%s", szTmpFileExtension);
	pDetail->szFormat = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	/*snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%s", szTmpFileDate);
	   pDetail->szDate = g_strdup(szTmpStr);
	   memset(szTmpStr, 0, FORMAT_LEN_MAX); */

	snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%s", szTmpFileSize);
	pDetail->szSize = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	pDetail->szLastModifiedTime = g_strdup(szTmpModifiedTime);
	MP_FREE_STRING(szTmpModifiedTime);

	snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%d X %d", nWidth, nHeight);
	pDetail->szResolution = g_strdup(szTmpStr);
	memset(szTmpStr, 0, FORMAT_LEN_MAX);

	/*snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%f", dLatitude);
	   pDetail->szLatitude = g_strdup(szTmpStr);
	   memset(szTmpStr, 0, FORMAT_LEN_MAX);

	   snprintf(szTmpStr, FORMAT_LEN_MAX-1, "%f", dLongitude);
	   pDetail->szLongitude = g_strdup(szTmpStr);
	   memset(szTmpStr, 0, FORMAT_LEN_MAX); */
	*pDetailInfo = pDetail;

	return TRUE;
}

bool mp_detail_view_get_folder_detail_info(int nFolderItemIndex,
		stDetailInfo **pDetailInfo)
{
	int nIndex = 0;

	char *szFolderUrl =
		mp_util_svc_get_video_folder_url(nFolderItemIndex);

	mp_util_svc_destory_video_item_list();
	if (szFolderUrl) {
		mp_util_svc_extract_video_list_from_folder(szFolderUrl,
				mp_sort_ctrl_get_sort_state
				(), MP_LIST_VIEW_ALL);
	}

	int nVideoListSize = mp_util_svc_get_number_of_video_item_by_type();
	VideoLogInfo("nVideoListSize = %d", nVideoListSize);

	char szTmpStr[FORMAT_LEN_MAX] = { 0, };
	char *szFilePath = NULL;
	char *szFolderName = NULL;

	unsigned long long nSizeTmp = 0;
	unsigned long long nSizeTotal = 0;
	char *szTmpFileSize = NULL;

	stDetailInfo *pDetail = NULL;

	for (nIndex = 0; nIndex < nVideoListSize; nIndex++) {
		szFilePath = mp_util_svc_get_video_url(nIndex);
		nSizeTmp = 0;
		mp_util_get_file_size_by_path(szFilePath, &nSizeTmp);
		nSizeTotal += nSizeTmp;
		MP_FREE_STRING(szFilePath);
	}
	szTmpFileSize = mp_util_get_file_size(nSizeTotal);
	VideoLogInfo("nSizeTotal = %s", szTmpFileSize);

	pDetail = calloc(1, sizeof(stDetailInfo));
	if (pDetail == NULL) {
		MP_FREE_STRING(szFilePath);
		MP_FREE_STRING(szFolderName);
		MP_FREE_STRING(szFolderUrl);
		MP_FREE_STRING(szTmpFileSize);
		VideoLogError("pDetailPopup alloc fail");
		return FALSE;
	}

	szFolderName = mp_util_svc_get_video_folder_name(nFolderItemIndex);

	pDetail->szTitle = g_strdup(szFolderName);
	pDetail->szLocation = vp_util_convert_file_location(szFolderUrl);
	MP_FREE_STRING(szFolderName);
	MP_FREE_STRING(szFilePath);
	MP_FREE_STRING(szFolderUrl);

	if (szTmpFileSize) {
		snprintf(szTmpStr, FORMAT_LEN_MAX - 1, "%s", szTmpFileSize);
		pDetail->szSize = g_strdup(szTmpStr);
		memset(szTmpStr, 0, FORMAT_LEN_MAX);
		MP_FREE_STRING(szTmpFileSize);
	}

	*pDetailInfo = pDetail;

	return TRUE;
}

void mp_detail_view_update(void)
{
	if (!g_pDetailViewHandle) {
		return;
	}
	MpMediaType eMediaType = MEDIA_TYPE_FILE;
	if (g_pDetailViewHandle->isFolder)
		eMediaType = MEDIA_TYPE_DIRECTORY;

	bool bValidMedia = TRUE;
	int nIndex = 0;
	bValidMedia =
		mp_util_svc_check_valid_media_id(g_pDetailViewHandle->pMediaId,
				eMediaType, &nIndex);
	if (!bValidMedia) {
		elm_naviframe_item_pop(g_pDetailViewHandle->pNaviFrameHandle);
	}
}
