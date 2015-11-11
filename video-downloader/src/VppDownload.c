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


#include <Ecore.h>
#include <Ecore_File.h>
#include <Eina.h>
#include <glib.h>
#include <app_control.h>
#include <download.h>

#include "mp-video-log.h"
#include "VppDownload.h"
#include "vp-file-util.h"

/* download cb type. for download pipe data */
typedef enum {
	VPP_DOWNLOAD_CB_TYPE_STATE_CHANGED,
	VPP_DOWNLOAD_CB_TYPE_PROGRESS,
} VppDownloadCbType;


// download item
typedef struct {
	int					   	id;			   	/* downloader id */

	char					*pDownloadUrl;  /* download url */
	char					*pDstFolder;	/* destination path */
	char					*pDstName;	  	/* destination path */

	char					*pTempPath;		/* tepmdownload path */
	char					*pEtag;			/* eTag */

	VppDownloadItemType	 	eType;		   	/* download item type */

	vpp_dl_progress_cb	   	progressCb;	   	/* download progress callback */
	vpp_dl_state_changed_cb	stateChangedCb; /* download state changed callback */

	bool					bDownloading;	/* downloading state */

	bool					bSetNoti;		/* quick pannel download view state. default FALSE */

	Ecore_Pipe				*pDownloadPipe;	/* internal pipe */

	void				   	*pUserdata;	   	/* user data */
} __VppDownloadItem;


// download list
typedef struct {
	Ecore_Timer	*downloadTimer;				/* internal timer */

	int			currentDownloadingCount;	/* downloading item count */

	Eina_List	*downloadList;				/* download item list */
} __VppDownloadListData;


/* download pipe data */
typedef struct {
	__VppDownloadItem	*pItem;			/* download item */
	unsigned long long 	received;		/* received file size from progress callback*/
	VppDownloadCbType 	eCbType;		/* pipe callback type */
	VppDownloadState	eState;			/* downloader state */
} VppDownloadPipeData;


__VppDownloadListData *g_download_list = NULL;


///////////////////////////////////////////////////////////////////////////////////////////
//	local functions proto types
///////////////////////////////////////////////////////////////////////////////////////////
void __VppDownloadPrintErr(download_error_e err);

void __VppDownloadListInitialize();
void __VppDestoryDownloadListDestory();

bool __VppDownloadCreate(__VppDownloadItem *pItem);
bool __VppDownloadStart(__VppDownloadItem *pItem, bool bResume);

static void __VppDownloadStateChangedCb(int download_id, download_state_e state, void *user_data);
static void __VppDownloadProgressCb(int download_id, unsigned long long received, void *user_data);

unsigned int __VppDownloadListCount();
bool __VppDownloadListAdd(__VppDownloadItem *pItem, bool bAppend);
void __VppDownloadListRemove(__VppDownloadItem *pItem);

void __VppDownloadListNextTrigger();
void __VppDownloadStartTimer();

static void __VppDownloadPipeCb(void *data, void *pdata, unsigned int nbyte);
static Eina_Bool __VppDownloadTimerCb(void *data);


///////////////////////////////////////////////////////////////////////////////////////////
//	download api
///////////////////////////////////////////////////////////////////////////////////////////


VppDownloadHandle
VppDownloadCreateItem(const char *pUrl, const char *pDstFolder, const char *pDstName, VppDownloadItemType eType, VppDownloadCallback cb, void *userData)
{
	__VppDownloadItem*	pItem	= NULL;

	if (pUrl == NULL) {
		VideoLogError("pUrl == NULL!!!");
		return NULL;
	}


	if (pDstFolder == NULL) {
		VideoLogError("pDstFolder == NULL!!!");
		return NULL;
	}

	if (cb.stateChangedCb == NULL) {
		VideoLogError("cb.stateChangedCb == NULL!!!");
		return NULL;
	}

	pItem						= calloc(1, sizeof(__VppDownloadItem));
	if (pItem == NULL) {
		VideoLogError("pItem calloc fail!!!");
		return NULL;
	}

	pItem->pDownloadUrl		= strdup(pUrl);
	pItem->pDstFolder		= strdup(pDstFolder);
	pItem->eType			= eType;
	pItem->progressCb		= cb.progressCb;
	pItem->stateChangedCb	= cb.stateChangedCb;
	pItem->pUserdata		= userData;
	pItem->bSetNoti			= TRUE;

	if (pDstName != NULL) {
		pItem->pDstName = strdup(pDstName);
	}

	if (eType == VPP_DOWNLOAD_ITEM_TYPE_VIDEO_FILE || eType == VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE) {
		pItem->bSetNoti			= TRUE;
	} else if (eType >= VPP_DOWNLOAD_ITEM_TYPE_VIDEO_INDEX_FILE && eType <= VPP_DOWNLOAD_ITEM_TYPE_RATING_IMAGE) {
		pItem->bSetNoti			= FALSE;
	} else {
		VideoLogError("eType error : [%d]", eType);
	}

	pItem->pDownloadPipe	= ecore_pipe_add(__VppDownloadPipeCb, (const void*)pItem);

	return (VppDownloadHandle)pItem;
}


VppDownloadHandle
VppDownloadCreateItemById(int nId, const char *pUrl, const char *pDstFolder, const char *pDstName, VppDownloadItemType eType, VppDownloadCallback cb, void *userData)
{
	__VppDownloadItem*	pItem	= NULL;

	if (nId <= 0) {
		VideoLogError("nId error : [%d]!!!", nId);
		return NULL;
	}


	pItem						= VppDownloadCreateItem(pUrl, pDstFolder, pDstName, eType, cb, userData);
	if (pItem == NULL) {
		VideoLogError("VppDownloadCreateItem fail!!!");
		return NULL;
	}

	pItem->id					= nId;

	if (VppDownloadSetCb((VppDownloadHandle)pItem, cb, pItem->pUserdata) == FALSE) {
		VideoLogError("VppDownloadSetCb error");
		VppDownloadDestroyItem((VppDownloadHandle)pItem);

		return NULL;
	}

	return (VppDownloadHandle)pItem;
}


void
VppDownloadDestroyItem(VppDownloadHandle pHandle)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;
	download_error_e	error	= DOWNLOAD_ERROR_NONE;
	char				*pPath	= NULL;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return;
	}

	if (pItem->pDownloadPipe != NULL) {
		ecore_pipe_del(pItem->pDownloadPipe);
		pItem->pDownloadPipe	= NULL;
	}

	pPath	= VppDownloadGetDownloadedFilePath(pHandle);

	if (pItem->pDownloadUrl != NULL) {
		free(pItem->pDownloadUrl);
		pItem->pDownloadUrl	= NULL;
	}

	if (pItem->pDstFolder != NULL) {
		free(pItem->pDstFolder);
		pItem->pDstFolder	= NULL;
	}

	if (pItem->pDstName != NULL) {
		free(pItem->pDstName);
		pItem->pDstName	= NULL;
	}

	if (pItem->pTempPath != NULL) {
		free(pItem->pTempPath);
		pItem->pTempPath	= NULL;
	}

	if (pItem->pEtag != NULL) {
		free(pItem->pEtag);
		pItem->pEtag	= NULL;
	}

	if (pItem->id > 0) {
		VppDownloadUnsetCb(pItem);

		VppDownloadState	state	= VppDownloadGetState(pItem);

		/*
		if (state != VPP_DOWNLOAD_STATE_AGENT_ERROR && state != VPP_DOWNLOAD_STATE_APP_ERROR)
		{
			if (state >= VPP_DOWNLOAD_STATE_READY && state <= VPP_DOWNLOAD_STATE_PAUSED)
			{
				VppDownloadCancel(pItem);
			}
		}
		*/

		if (state != VPP_DOWNLOAD_STATE_COMPLETED) {
			if (pPath != NULL && vp_file_exists((const char*)pPath) == EINA_TRUE) {
				vp_file_unlink((const char*)pPath);
			}
		}

		error	= download_set_notification_type(pItem->id, DOWNLOAD_NOTIFICATION_TYPE_NONE);
		if (error != DOWNLOAD_ERROR_NONE) {
			__VppDownloadPrintErr(error);
		}

		error	= download_destroy(pItem->id);
		if (error != DOWNLOAD_ERROR_NONE) {
			__VppDownloadPrintErr(error);
		}
	}

	if (pItem->eType < VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE || pItem->eType > VPP_DOWNLOAD_ITEM_TYPE_VIDEO_CAPTION_FILE) {
		__VppDownloadListRemove(pHandle);
	}

	if (pPath != NULL) {
		free(pPath);
		pPath	= NULL;
	}

	free(pItem);
	pItem	= NULL;
}


void
VppDownloadDestroyItemById(int nId)
{
	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	error	= download_destroy(nId);
	if (error != DOWNLOAD_ERROR_NONE) {
		__VppDownloadPrintErr(error);
	}
}


void
VppDownloadClearItem(VppDownloadHandle pHandle)
{

	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;
//	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return;
	}

	if (pItem->pDownloadPipe != NULL) {
		ecore_pipe_del(pItem->pDownloadPipe);
		pItem->pDownloadPipe	= NULL;
	}

	if (pItem->pDownloadUrl != NULL) {
		free(pItem->pDownloadUrl);
		pItem->pDownloadUrl	= NULL;
	}

	if (pItem->pDstFolder != NULL) {
		free(pItem->pDstFolder);
		pItem->pDstFolder	= NULL;
	}

	if (pItem->pDstName != NULL) {
		free(pItem->pDstName);
		pItem->pDstName	= NULL;
	}

	if (pItem->id > 0) {
		VppDownloadUnsetCb(pItem);
	}

	free(pItem);
	pItem	= NULL;
}


bool
VppDownloadStartVideoStoreVideoItem(VppDownloadHandle pHandle, void *pOnGoingServiceData, void *pCompleteServiceData)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return FALSE;
	}

	if (pItem->id <= 0) {
		if (__VppDownloadCreate(pItem) == FALSE) {
			VideoLogError("__VppDownloadCreate faile !!!");
			return FALSE;
		}
	}

	if (pOnGoingServiceData != NULL) {
		int		error	= 0;

		error	= download_set_notification_app_control(pItem->id, DOWNLOAD_NOTIFICATION_APP_CONTROL_TYPE_ONGOING, (app_control_h)pOnGoingServiceData);
		if (error != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_get_downloaded_file_path error");
			__VppDownloadPrintErr(error);

			return FALSE;
		}
	}

	if (pCompleteServiceData != NULL) {
		int		error	= 0;

		error	= download_set_notification_app_control(pItem->id, DOWNLOAD_NOTIFICATION_APP_CONTROL_TYPE_COMPLETE, (app_control_h)pCompleteServiceData);
		if (error != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_get_downloaded_file_path error");
			__VppDownloadPrintErr(error);

			return FALSE;
		}
	}

	if (__VppDownloadStart((__VppDownloadItem*)pHandle, FALSE) == FALSE) {
		VideoLogError("fail __VppDownloadStart!!!");
		return FALSE;
	}

	return TRUE;
}


bool
VppDownloadSetCb(VppDownloadHandle pHandle, VppDownloadCallback cb, void *userData)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return FALSE;
	}

	VppDownloadUnsetCb(pHandle);

	pItem->pUserdata	= userData;

	if (cb.progressCb != NULL) {
		pItem->progressCb		= cb.progressCb;
		if (pItem->id > 0) {
			error	= download_set_progress_cb(pItem->id, __VppDownloadProgressCb, pItem);
			if (error != DOWNLOAD_ERROR_NONE) {
				__VppDownloadPrintErr(error);
			}
		}
	}

	if (cb.stateChangedCb != NULL) {
		pItem->stateChangedCb	= cb.stateChangedCb;
		if (pItem->id > 0) {
			error	= download_set_state_changed_cb(pItem->id, __VppDownloadStateChangedCb, pItem);
			if (error != DOWNLOAD_ERROR_NONE) {
				__VppDownloadPrintErr(error);
			}
		}
	}

	if (error != DOWNLOAD_ERROR_NONE) {
		return FALSE;
	}

	return TRUE;
}


bool
VppDownloadUnsetCb(VppDownloadHandle pHandle)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return FALSE;
	}

	if (pItem->id > 0) {
		if (pItem->progressCb	!= NULL) {
			error	= download_unset_progress_cb(pItem->id);
			if (error != DOWNLOAD_ERROR_NONE) {
				__VppDownloadPrintErr(error);
			}
		}

		if (pItem->stateChangedCb	!= NULL) {
			error	= download_unset_state_changed_cb(pItem->id);
			if (error != DOWNLOAD_ERROR_NONE) {
				__VppDownloadPrintErr(error);
			}
		}
	}

	pItem->progressCb 		= NULL;
	pItem->stateChangedCb 	= NULL;
	pItem->pUserdata		= NULL;

	if (error != DOWNLOAD_ERROR_NONE) {
		return FALSE;
	}

	return TRUE;
}


bool
VppDownloadRequestAppend(VppDownloadHandle pHandle)
{
	return __VppDownloadListAdd((__VppDownloadItem*)pHandle, TRUE);
}


bool
VppDownloadRequestPrepend(VppDownloadHandle pHandle)
{
	return __VppDownloadListAdd((__VppDownloadItem*)pHandle, FALSE);
}


bool
VppDownloadPause(VppDownloadHandle pHandle)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return FALSE;
	}

	if (pItem->id <= 0) {
		VideoLogError("pItem->id : [%d]!!!", pItem->id);
		return FALSE;
	}

	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	error	= download_pause(pItem->id);
	if (error != DOWNLOAD_ERROR_NONE) {
		__VppDownloadPrintErr(error);

		return FALSE;
	}

	return TRUE;
}


bool
VppDownloadResume(VppDownloadHandle pHandle)
{
	return __VppDownloadStart((__VppDownloadItem*)pHandle, TRUE);
}


bool
VppDownloadCancel(VppDownloadHandle pHandle)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return FALSE;
	}

	if (pItem->id <= 0) {
		VideoLogError("pItem->id : [%d]!!!", pItem->id);
		return FALSE;
	}

	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	error	= download_cancel(pItem->id);
	if (error != DOWNLOAD_ERROR_NONE) {
		__VppDownloadPrintErr(error);

		return FALSE;
	}

	return TRUE;
}


bool
VppDownloadStoreVideoRestore(VppDownloadHandle pHandle)
{
	__VppDownloadItem*	pItem		= (__VppDownloadItem*)pHandle;

	unsigned long long	lFileSize	= 0;

	int					nRet		= 0;
	char 				szTemp[32] 	= {0,};

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return FALSE;
	}

	if (pItem->pTempPath == NULL) {
		VideoLogError("pItem->pTempPath!!!");
		return FALSE;
	}

	if (pItem->pEtag == NULL) {
		VideoLogError("pItem->pEtag!!!");
		return FALSE;
	}

	nRet	= download_set_temp_file_path(pItem->id, pItem->pTempPath);
	if (nRet != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_set_temp_file_path failed!!!");
		__VppDownloadPrintErr(nRet);
		return FALSE;
	}

	nRet	= download_add_http_header_field(pItem->id, "If-Range", pItem->pEtag);
	if (nRet != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_add_http_header_field failed!!!");
		__VppDownloadPrintErr(nRet);
		return FALSE;
	}

	lFileSize	= (unsigned long long)vp_file_size((const char*)pItem->pTempPath);

	snprintf(szTemp, sizeof(szTemp), "bytes=%llu-", lFileSize);

	nRet	= download_add_http_header_field(pItem->id, "Range", szTemp);
	if (nRet != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_add_http_header_field failed!!!");
		__VppDownloadPrintErr(nRet);
		return FALSE;
	}

	nRet	= download_start(pItem->id);
	if (nRet != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_start failed!!!");
		__VppDownloadPrintErr(nRet);
		return FALSE;
	}

	return TRUE;
}


VppDownloadState
VppDownloadGetState(VppDownloadHandle pHandle)
{
	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return VPP_DOWNLOAD_STATE_APP_ERROR;
	}

	if (pItem->id <= 0) {
		VideoLogError("id error: [%d]", pItem->id);
		return VPP_DOWNLOAD_STATE_APP_ERROR;
	}

	download_error_e	error	= DOWNLOAD_ERROR_NONE;
	download_state_e  	state	= DOWNLOAD_STATE_NONE;

	error	= download_get_state(pItem->id, &state);
	if (error != DOWNLOAD_ERROR_NONE) {
		__VppDownloadPrintErr(error);
		return VPP_DOWNLOAD_STATE_AGENT_ERROR;
	}

	return (VppDownloadState)state;
}


VppDownloadState
VppDownloadGetStateById(int nDownloadId)
{
	if (nDownloadId <= 0) {
		VideoLogError("id error: [%d]", nDownloadId);
		return VPP_DOWNLOAD_STATE_APP_ERROR;
	}

	download_error_e	error	= DOWNLOAD_ERROR_NONE;
	download_state_e  	state	= DOWNLOAD_STATE_NONE;

	error	= download_get_state(nDownloadId, &state);
	if (error != DOWNLOAD_ERROR_NONE) {
		__VppDownloadPrintErr(error);
		return VPP_DOWNLOAD_STATE_AGENT_ERROR;
	}

	return (VppDownloadState)state;
}


char*
VppDownloadGetDownloadedFilePath(VppDownloadHandle pHandle)
{
	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL");
		return NULL;
	}

	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	char				*pPath	= NULL;
	char	sDestPath[PATH_MAX] = {0,};

	if (pItem->id <= 0) {
		VideoLogWarning("id error: [%d]", pItem->id);
		return NULL;
	}

	if (pItem->pDstFolder != NULL && pItem->pDstName != NULL) {
		if (pItem->pDstFolder[strlen(pItem->pDstFolder) - 1] == '/') {
			snprintf(sDestPath, PATH_MAX, "%s%s", pItem->pDstFolder, pItem->pDstName);
		} else {
			snprintf(sDestPath, PATH_MAX, "%s/%s", pItem->pDstFolder, pItem->pDstName);
		}

		VideoSecureLogDebug("Path : [%s]:%s,%s,%d", sDestPath,  pItem->pDstFolder[strlen(pItem->pDstFolder)], pItem->pDstFolder, strlen(pItem->pDstFolder));

		return strdup(sDestPath);
	} else {
		error	= download_get_downloaded_file_path(pItem->id, &pPath);
		if (error != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_get_downloaded_file_path error");
			__VppDownloadPrintErr(error);

			if (pPath != NULL) {
				free(pPath);
				pPath	= NULL;
			}
		}

		if (pPath != NULL) {
			VideoSecureLogDebug("Path : [%s]", pPath);
			return pPath;
		} else {
			VideoLogDebug("pPath == NULL");
			return NULL;
		}
	}
}


int
VppDownloadGetDownloadId(VppDownloadHandle pHandle)
{
	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL ");
		return 0;
	}

	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	return pItem->id;
}


bool
VppDownloadSetNotificationAppControl(VppDownloadHandle pHandle, void *pData)
{
	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL ");
		return FALSE;
	}

	if (pData == NULL) {
		VideoLogError("pData == NULL ");
		return FALSE;
	}

	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	error	= download_set_notification_app_control(pItem->id, DOWNLOAD_NOTIFICATION_APP_CONTROL_TYPE_ONGOING, (app_control_h)pData);
	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_get_downloaded_file_path error");
		__VppDownloadPrintErr(error);

		return FALSE;
	}

	return TRUE;
}


void*
VppDownloadGetNotificationAppControl(VppDownloadHandle pHandle)
{
	download_error_e	error	= DOWNLOAD_ERROR_NONE;
	app_control_h				*b		= NULL;

	if (pHandle == NULL) {
		VideoLogError("pHandle == NULL ");
		return NULL;
	}

	__VppDownloadItem*	pItem	= (__VppDownloadItem*)pHandle;

	error	= download_get_notification_app_control(pItem->id, DOWNLOAD_NOTIFICATION_APP_CONTROL_TYPE_ONGOING, b);
	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_get_downloaded_file_path error");
		__VppDownloadPrintErr(error);

		return NULL;
	}

	return (void*)b;
}


///////////////////////////////////////////////////////////////////////////////////////////
//	local function implementations
///////////////////////////////////////////////////////////////////////////////////////////
void
__VppDownloadPrintErr(download_error_e err)
{
	switch (err) {
	case DOWNLOAD_ERROR_NONE: {
		VideoLogError("err == DOWNLOAD_ERROR_NONE");
	}
	break;
	case DOWNLOAD_ERROR_INVALID_PARAMETER: {
		VideoLogError("err == DOWNLOAD_ERROR_INVALID_PARAMETER");
	}
	break;
	case DOWNLOAD_ERROR_OUT_OF_MEMORY: {
		VideoLogError("err == DOWNLOAD_ERROR_OUT_OF_MEMORY");
	}
	break;
	case DOWNLOAD_ERROR_NETWORK_UNREACHABLE: {
		VideoLogError("err == DOWNLOAD_ERROR_NETWORK_UNREACHABLE");
	}
	break;
	case DOWNLOAD_ERROR_CONNECTION_TIMED_OUT: {
		VideoLogError("err == DOWNLOAD_ERROR_CONNECTION_TIMED_OUT");
	}
	break;
	case DOWNLOAD_ERROR_NO_SPACE: {
		VideoLogError("err == DOWNLOAD_ERROR_NO_SPACE");
	}
	break;
	case DOWNLOAD_ERROR_FIELD_NOT_FOUND: {
		VideoLogError("err == DOWNLOAD_ERROR_FIELD_NOT_FOUND");
	}
	break;
	case DOWNLOAD_ERROR_INVALID_STATE: {
		VideoLogError("err == DOWNLOAD_ERROR_INVALID_STATE");
	}
	break;
	case DOWNLOAD_ERROR_CONNECTION_FAILED: {
		VideoLogError("err == DOWNLOAD_ERROR_CONNECTION_FAILED");
	}
	break;
	case DOWNLOAD_ERROR_INVALID_URL: {
		VideoLogError("err == DOWNLOAD_ERROR_INVALID_URL");
	}
	break;
	case DOWNLOAD_ERROR_INVALID_DESTINATION: {
		VideoLogError("err == DOWNLOAD_ERROR_INVALID_DESTINATION");
	}
	break;
	case DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS: {
		VideoLogError("err == DOWNLOAD_ERROR_TOO_MANY_DOWNLOADS");
	}
	break;
	case DOWNLOAD_ERROR_QUEUE_FULL: {
		VideoLogError("err == DOWNLOAD_ERROR_QUEUE_FULL");
	}
	break;
	case DOWNLOAD_ERROR_ALREADY_COMPLETED: {
		VideoLogError("err == DOWNLOAD_ERROR_ALREADY_COMPLETED");
	}
	break;
	case DOWNLOAD_ERROR_FILE_ALREADY_EXISTS: {
		VideoLogError("err == DOWNLOAD_ERROR_FILE_ALREADY_EXISTS");
	}
	break;
	case DOWNLOAD_ERROR_TOO_MANY_REDIRECTS: {
		VideoLogError("err == DOWNLOAD_ERROR_TOO_MANY_REDIRECTS");
	}
	break;
	case DOWNLOAD_ERROR_UNHANDLED_HTTP_CODE: {
		VideoLogError("err == DOWNLOAD_ERROR_UNHANDLED_HTTP_CODE");
	}
	break;
	case DOWNLOAD_ERROR_NO_DATA: {
		VideoLogError("err == DOWNLOAD_ERROR_NO_DATA");
	}
	break;
	case DOWNLOAD_ERROR_IO_ERROR: {
		VideoLogError("err == DOWNLOAD_ERROR_IO_ERROR");
	}
	break;
	default: {
		VideoLogError("UNKNOW err == [%d]", err);
	}
	break;
	}
}


void
__VppDownloadListInitialize()
{
	if (g_download_list != NULL) {
		__VppDestoryDownloadListDestory();
	}

	g_download_list					= calloc(1, sizeof(__VppDownloadListData));
}


void
__VppDestoryDownloadListDestory()
{
	if (g_download_list == NULL) {
		VideoLogError("g_download_list == NULL!!!");
		return;
	}

	if (g_download_list->downloadTimer != NULL) {
		ecore_timer_del(g_download_list->downloadTimer);
		g_download_list->downloadTimer	= NULL;
	}

	if (g_download_list->downloadList != NULL) {
		if (__VppDownloadListCount() > 0) {
			VideoLogError("list count error[%d]!!!. some data are not destoryed!!!", __VppDownloadListCount());
		}

		Eina_List			*l;
		__VppDownloadItem	*pItem;

		EINA_LIST_FOREACH(g_download_list->downloadList, l, pItem) {
			if (pItem != NULL) {
				VideoLogError("list item is not freed!!!");
				VideoSecureLogError("list item type[%d], url[%s], pDstFolder[%s], pDstName[%s]!!!", pItem->eType, pItem->pDownloadUrl, pItem->pDstFolder, pItem->pDstName);

				VppDownloadDestroyItem(pItem);
			}
		}

		eina_list_free(g_download_list->downloadList);
		g_download_list->downloadList	= NULL;
	}

	free(g_download_list);
	g_download_list	= NULL;
}


bool
__VppDownloadCreate(__VppDownloadItem* pItem)
{
	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	error	= download_create(&(pItem->id));
	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_create error");
		goto ERROR_HANDLE;
	}

	error	= download_set_url(pItem->id, (const char*)pItem->pDownloadUrl);
	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_set_url error");
		goto ERROR_HANDLE;
	}

	error	= download_set_destination(pItem->id, (const char*)(pItem->pDstFolder));
	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_set_destination error");
		goto ERROR_HANDLE;
	}

	if (pItem->pDstName != NULL) {
		char	szDestName[PATH_MAX]	= {0,};

		if (pItem->eType != VPP_DOWNLOAD_ITEM_TYPE_VIDEO_FILE && pItem->eType != VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE) {
			snprintf(szDestName, PATH_MAX, "%s%s", pItem->pDstName, ".temp");
		} else {
			int dstlength = strlen(pItem->pDstName);
			if (dstlength + 1 <= PATH_MAX) {
				strncpy(szDestName, pItem->pDstName, dstlength);
			}
		}

		error	= download_set_file_name(pItem->id, (const char*)szDestName);
		if (error != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_set_file_name error");
			goto ERROR_HANDLE;
		}
	}

	if (pItem->bSetNoti == TRUE) {
		error	= download_set_notification_type(pItem->id, DOWNLOAD_NOTIFICATION_TYPE_ALL);
	} else {
		error	= download_set_notification_type(pItem->id, DOWNLOAD_NOTIFICATION_TYPE_NONE);
	}

	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_set_notification error");
		goto ERROR_HANDLE;
	}

	VppDownloadCallback cb	= {0,};
	cb.progressCb			= pItem->progressCb;
	cb.stateChangedCb		= pItem->stateChangedCb;

	if (VppDownloadSetCb((VppDownloadHandle)pItem, cb, pItem->pUserdata) == FALSE) {
		VideoLogError("VppDownloadSetCb error");
		VppDownloadDestroyItem((VppDownloadHandle)pItem);

		return FALSE;
	}

	return TRUE;

ERROR_HANDLE:
	__VppDownloadPrintErr(error);

	if (pItem->id > 0) {
		error	= download_destroy(pItem->id);
		if (error != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_destroy error");
			__VppDownloadPrintErr(error);
		}

		pItem->id	= 0;
	}
	//VppDownloadDestroyItem((VppDownloadHandle)pItem);

	return FALSE;
}


bool
__VppDownloadStart(__VppDownloadItem* pItem, bool bResume)
{
	if (pItem == NULL) {
		VideoLogError("pItem == NULL!!!");
		return FALSE;
	}

	if (pItem->id <= 0) {
		VideoLogError("pItem->id : [%d]!!!", pItem->id);
		return FALSE;
	}

	if (bResume == FALSE) {
		VideoLogDebug("download start!!!");
	} else {
		VideoLogDebug("download resume!!!");
	}

	download_error_e	error	= DOWNLOAD_ERROR_NONE;

	if (pItem->eType == VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE) {
		error	= download_set_auto_download(pItem->id, TRUE);
		if (error != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_set_auto_download error!!!");
			__VppDownloadPrintErr(error);
		}
	}

	error = download_start(pItem->id);
	if (error != DOWNLOAD_ERROR_NONE) {
		VideoLogError("download_start error!!!");
		__VppDownloadPrintErr(error);
		return FALSE;
	}

	return TRUE;
}


static void
__VppDownloadStateChangedCb(int download_id, download_state_e state, void *user_data)
{
	__VppDownloadItem	*pItem		= (__VppDownloadItem*)user_data;

	VideoLogDebug("state : [%d]", state);

	if (user_data == NULL) {
		VideoLogError("user_data == NULL!!!");
		return;
	}

	if (pItem->eType == VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE && state == DOWNLOAD_STATE_DOWNLOADING) {
		int	nRet	= 0;
		if (pItem->pTempPath != NULL) {
			free(pItem->pTempPath);
			pItem->pTempPath	= NULL;
		}
		nRet	= download_get_temp_path(pItem->id, &pItem->pTempPath);
		if (nRet != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_get_temp_path failed!!!");
			__VppDownloadPrintErr((download_error_e)nRet);
		}


		if (pItem->pEtag != NULL) {
			free(pItem->pEtag);
			pItem->pEtag	= NULL;
		}
		nRet	= download_get_etag(pItem->id, &pItem->pEtag);
		if (nRet != DOWNLOAD_ERROR_NONE) {
			VideoLogError("download_get_temp_path failed!!!");
			__VppDownloadPrintErr((download_error_e)nRet);
		}
	}

	if (state >= DOWNLOAD_STATE_COMPLETED && state <= DOWNLOAD_STATE_CANCELED) {
		if (pItem->bDownloading == TRUE) {
			pItem->bDownloading	= FALSE;

			if (pItem->eType < VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE || pItem->eType > VPP_DOWNLOAD_ITEM_TYPE_VIDEO_CAPTION_FILE) {
				--(g_download_list->currentDownloadingCount);
			}
		}
	}

	VppDownloadPipeData	pipeData	= {0,};

	pipeData.eCbType		= VPP_DOWNLOAD_CB_TYPE_STATE_CHANGED;
	pipeData.eState			= (VppDownloadState)state;
	pipeData.pItem			= pItem;

	ecore_pipe_write(pItem->pDownloadPipe, &pipeData, sizeof(VppDownloadPipeData));
}


static void
__VppDownloadProgressCb(int download_id, unsigned long long received, void *user_data)
{
	__VppDownloadItem	*pItem		= (__VppDownloadItem*)user_data;

	VppDownloadPipeData	pipeData	= {0,};

	if (user_data == NULL) {
		VideoLogError("user_data == NULL!!!");
		return;
	}

	pipeData.eCbType	= VPP_DOWNLOAD_CB_TYPE_PROGRESS;
	pipeData.eState		= VPP_DOWNLOAD_STATE_DOWNLOADING;
	pipeData.pItem		= (__VppDownloadItem*)user_data;
	pipeData.received	= received;

	ecore_pipe_write(pItem->pDownloadPipe, &pipeData, sizeof(VppDownloadPipeData));
}


unsigned int
__VppDownloadListCount()
{
	if (g_download_list != NULL && g_download_list->downloadList != NULL) {
		if (g_download_list->downloadList->accounting != NULL) {
			return eina_list_count(g_download_list->downloadList);
		}
	}

	return 0;
}


bool
__VppDownloadListAdd(__VppDownloadItem *pItem, bool bAppend)
{
	if (pItem == NULL) {
		VideoLogError("pItem == NULL!!!");
		return FALSE;
	}

	if (g_download_list == NULL) {
		__VppDownloadListInitialize();
	}

	if (bAppend == TRUE) {
		g_download_list->downloadList	= eina_list_append(g_download_list->downloadList, (void*)pItem);

		if (g_download_list->downloadList == NULL) {
			VideoLogError("eina_list_append faile !!!");
			return FALSE;
		}
	} else {
		g_download_list->downloadList	= eina_list_prepend(g_download_list->downloadList, (void*)pItem);

		if (g_download_list->downloadList == NULL) {
			VideoLogError("eina_list_prepend faile !!!");
			return FALSE;
		}
	}

	__VppDownloadListNextTrigger();

	return TRUE;
}


void
__VppDownloadListRemove(__VppDownloadItem *pItem)
{
	if (pItem == NULL) {
		VideoLogError("pItem == NULL!!!");
		return;
	}

	if (g_download_list == NULL) {
		VideoLogError("g_download_list == NULL!!!");
		return;
	}

	if (g_download_list->downloadList == NULL) {
		VideoLogError("g_download_list->downloadList == NULL!!!");
		return;
	}

	g_download_list->downloadList	= eina_list_remove(g_download_list->downloadList, (void*)pItem);
	if (__VppDownloadListCount() <= 0) {
		__VppDestoryDownloadListDestory();
	}
}



void
__VppDownloadListNextTrigger()
{
	if (g_download_list == NULL) {
		VideoLogError("g_download_list == NULL!!!");
		return;
	}

	if (g_download_list->downloadList == NULL) {
		VideoLogError("g_download_list->downloadList == NULL!!!");
		return;
	}

	if (g_download_list->currentDownloadingCount > 5) {
		__VppDownloadStartTimer();
		return;
	}

	Eina_List			*l;
	__VppDownloadItem	*pItem;

	EINA_LIST_FOREACH(g_download_list->downloadList, l, pItem) {
		if (pItem == NULL) {
			continue;
		}

		if (pItem->bDownloading == TRUE) {
			continue;
		}

		if (pItem->id <= 0) {
			if (__VppDownloadCreate(pItem) == FALSE) {
				VideoLogError("__VppDownloadCreate faile !!!");
				__VppDownloadStartTimer();
				return;
			}
		}

		VppDownloadState	eState	= VppDownloadGetState((VppDownloadHandle)pItem);
		VideoLogDebug("eState : [%d]", eState);

		if (eState == VPP_DOWNLOAD_STATE_PAUSED) {
			pItem->bDownloading = TRUE;
			++(g_download_list->currentDownloadingCount);
		} else {
			if (__VppDownloadStart(pItem, FALSE) == TRUE) {
				pItem->bDownloading = TRUE;
				++(g_download_list->currentDownloadingCount);
			} else {
				VideoSecureLogError("__VppDownloadStart faile : [%s : %s]!!!", pItem->pDstFolder, pItem->pDstName);
				__VppDownloadStartTimer();
				return;
			}
		}
	}
}


void
__VppDownloadStartTimer()
{
	if (g_download_list == NULL) {
		VideoLogError("g_download_list == NULL!!!");
		return;
	}

	if (g_download_list->downloadTimer != NULL) {
		ecore_timer_del(g_download_list->downloadTimer);
		g_download_list->downloadTimer	= NULL;
	}

	g_download_list->downloadTimer = ecore_timer_add(1.0, __VppDownloadTimerCb, g_download_list);
}


static void
__VppDownloadPipeCb(void *data, void *pdata, unsigned int nbyte)
{
	VppDownloadPipeData	*pPipeData	= NULL;
	__VppDownloadItem	*pItem		= NULL;

	if (pdata == NULL) {
		VideoLogError("pdata == NULL!!!");
		return;
	}

	pPipeData	= (VppDownloadPipeData*)pdata;;
	pItem		= pPipeData->pItem;

	if (pItem == NULL) {
		VideoLogError("pItem == NULL!!!");
		return;
	}

	if (pPipeData->eCbType == VPP_DOWNLOAD_CB_TYPE_STATE_CHANGED) {
		char*	pDstPath				= VppDownloadGetDownloadedFilePath((VppDownloadHandle)pItem);

		if (pPipeData->eState == VPP_DOWNLOAD_STATE_COMPLETED) {
			char*	pTempDstPath	= NULL;
			int		error			= 0;

			error	= download_get_downloaded_file_path(pItem->id, &pTempDstPath);
			if (error != DOWNLOAD_ERROR_NONE) {
				VideoLogError("download_get_downloaded_file_path error");
				__VppDownloadPrintErr((download_error_e)error);
			} else {
				if (pTempDstPath != NULL && pDstPath != NULL) {
					if (strcmp(pTempDstPath, pDstPath) != 0) {
						if (vp_file_exists((const char*)pDstPath) == EINA_TRUE) {
							vp_file_unlink((const char*)pDstPath);
						}

						if (vp_file_mv(pTempDstPath, pDstPath) != EINA_TRUE) {
							VideoSecureLogError("vp_file_mv failed!!! src : [%s], dst : [%s]", pTempDstPath, pDstPath);
						}
					}
				}
			}

			if (pTempDstPath != NULL) {
				free(pTempDstPath);
				pTempDstPath	= NULL;
			}
		}

		if (pItem->stateChangedCb != NULL) {
			pItem->stateChangedCb((VppDownloadHandle)pItem, pPipeData->eState, (const char*)pDstPath, pItem->pUserdata);
		}

		if (pDstPath != NULL) {
			free(pDstPath);
			pDstPath	= NULL;
		}
	} else {
		if (pItem->progressCb != NULL) {
			unsigned long long	contentTotalSize	= 0;
			download_error_e	error				= download_get_content_size(pItem->id, &contentTotalSize);
			if (error != DOWNLOAD_ERROR_NONE) {
				VideoLogError("download_get_content_size error");
				__VppDownloadPrintErr(error);
				return;
			}

			pItem->progressCb((VppDownloadHandle)pItem, pPipeData->received, contentTotalSize, pItem->pUserdata);
		}
	}
}


static Eina_Bool
__VppDownloadTimerCb(void *data)
{
	if (g_download_list == NULL) {
		goto TIMER_CB_RETURN;
	}

	if (g_download_list->downloadList == NULL) {
		goto TIMER_CB_RETURN;
	}

	int cnt = __VppDownloadListCount();

	if (cnt <= 0) {
		goto TIMER_CB_RETURN;
	}

	__VppDownloadListNextTrigger();

TIMER_CB_RETURN:
	if (g_download_list != NULL) {
		g_download_list->downloadTimer	= NULL;
	}

	return ECORE_CALLBACK_CANCEL;
}


void VppDownloadResetUserData(void *pDataItem)
{
	__VppDownloadItem	*pItem	= (__VppDownloadItem*)pDataItem;

	pItem->pUserdata = NULL;
}


