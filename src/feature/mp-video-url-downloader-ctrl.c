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


#include <Ecore_Evas.h>
#include <Elementary.h>
#include <download.h>

#include "mp-util.h"
#include "mp-video-log.h"
#include "mp-video-string-define.h"
#include "mp-video-value-define.h"
#include "mp-video-url-downloader-ctrl.h"
#include "vp-file-util.h"


static url_download_h pUrlDownloaderHandle = NULL;
static bool bIsCompletedDownload = FALSE;
static char szDownloadedPath[STR_LEN_MAX] = { 0, };

static char *szDownloadedUrl = NULL;
static char *szDownloadedDestDir = NULL;
static CompleteDownloadCbFunc UserCompleteCbFunc = NULL;


#define DMR_BASE_DIRECTORY	VIDEO_BOOKMARK_DIR"/dmr_icon"

void mp_download_url_ctrl_destroy(void)
{
	VideoLogInfo("");

	if (szDownloadedUrl) {
		free(szDownloadedUrl);
		szDownloadedUrl = NULL;
	}

	if (szDownloadedDestDir) {
		free(szDownloadedDestDir);
		szDownloadedDestDir = NULL;
	}

	if (pUrlDownloaderHandle) {
		url_download_destroy(pUrlDownloaderHandle);
		pUrlDownloaderHandle = NULL;
	}

	bIsCompletedDownload = FALSE;
	UserCompleteCbFunc = NULL;

	memset(szDownloadedPath, 0, STR_LEN_MAX);
}

static void mp_download_url_ctrl_complete_cb(url_download_h
		_UrlDownloaderHandle,
		const char *szPath,
		void *pUserData)
{
	if (!_UrlDownloaderHandle) {
		VideoLogInfo("[ERR]No have _UrlDownloaderHandle");
		return;
	}

	VideoLogInfo("");

	if (szDownloadedUrl) {
		free(szDownloadedUrl);
		szDownloadedUrl = NULL;
	}

	if (szDownloadedDestDir) {
		free(szDownloadedDestDir);
		szDownloadedDestDir = NULL;
	}

	memset(szDownloadedPath, 0, STR_LEN_MAX);

	strncpy(szDownloadedPath, szPath, STR_LEN_MAX - 1);
	url_download_get_url(_UrlDownloaderHandle, &szDownloadedUrl);
	url_download_get_destination(_UrlDownloaderHandle,
				     &szDownloadedDestDir);

	UserCompleteCbFunc(szDownloadedPath);

	mp_download_url_ctrl_destroy();
}

static void mp_download_url_ctrl_stop_cb(url_download_h
		_UrlDownloaderHandle,
		url_download_error_e nError,
		void *pUserData)
{
	if (!_UrlDownloaderHandle) {
		VideoLogInfo("[ERR]No have _UrlDownloaderHandle");
		return;
	}

	VideoLogInfo("");

	url_download_stop(_UrlDownloaderHandle);

	mp_download_url_ctrl_destroy();
}

void mp_download_url_ctrl_start(void *pUserData, char *szUrl,
				char *szIconPath,
				CompleteDownloadCbFunc
				pCompletedCallbackFunc)
{
	if (!pUserData) {
		VideoLogInfo("[ERR]No have pUserData");
		return;
	}

	VideoLogInfo("");

	mp_download_url_ctrl_destroy();

	UserCompleteCbFunc = pCompletedCallbackFunc;

	int nRet = 0;
	int nHandleId = 0;

	if (!vp_is_dir(DMR_BASE_DIRECTORY)) {
		if (!vp_mkpath(DMR_BASE_DIRECTORY)) {
			VideoLogInfo("Fail to create directory.");
			return;
		}
	}

	nRet = url_download_create(&pUrlDownloaderHandle);
	if (nRet != URL_DOWNLOAD_ERROR_NONE || !pUrlDownloaderHandle) {
		VideoLogInfo("Fail to create url downloader handle.");
		return;
	}

	nRet = url_download_set_url(pUrlDownloaderHandle, szUrl);
	if (nRet != URL_DOWNLOAD_ERROR_NONE) {
		VideoLogInfo("Fail to set url.");
		return;
	}

	nRet =
		url_download_set_destination(pUrlDownloaderHandle,
					     DMR_BASE_DIRECTORY);
	if (nRet != URL_DOWNLOAD_ERROR_NONE) {
		VideoLogInfo("Fail to set destination.");
		return;
	}

	nRet =
		url_download_set_completed_cb(pUrlDownloaderHandle,
					      mp_download_url_ctrl_complete_cb,
					      pUserData);
	if (nRet != URL_DOWNLOAD_ERROR_NONE) {
		VideoLogInfo("Fail to set completed callback.");
		return;
	}

	nRet =
		url_download_set_stopped_cb(pUrlDownloaderHandle,
					    mp_download_url_ctrl_stop_cb,
					    pUserData);
	if (nRet != URL_DOWNLOAD_ERROR_NONE) {
		VideoLogInfo("Fail to set stopped callback.");
		return;
	}

	nRet = url_download_start(pUrlDownloaderHandle, &nHandleId);
	if (nRet != URL_DOWNLOAD_ERROR_NONE) {
		VideoLogInfo("Fail to start.");
		return;
	}
}

void mp_download_url_ctrl_stop(void)
{
	VideoLogInfo("");

	if (!pUrlDownloaderHandle) {
		VideoLogInfo("Don't exist pUrlDownloaderHandle.");
		return;
	}

	int nRet = url_download_stop(pUrlDownloaderHandle);
	if (nRet != URL_DOWNLOAD_ERROR_NONE) {
		VideoLogInfo("Fail to stop.");
	}
}
