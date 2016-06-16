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

#include <regex.h>
//#include <Ecore_X.h>
#include <efl_extension.h>
#include <errno.h>
#include <notification.h>
#include <mime_type.h>
#include <net_connection.h>
//#include <telephony_network.h>
#include <app.h>
#include <app_manager.h>
#include <glib/gstdio.h>
#include <metadata_extractor.h>
#include <storage.h>
#include <system_settings.h>

#include <sys/statvfs.h>
#include <unicode/udat.h>
#include <unicode/ustring.h>
#include <unicode/uloc.h>
#include <unicode/ucal.h>
#include <unicode/udatpg.h>
#include <unicode/utmscale.h>
#include <runtime_info.h>

#include "vp-play-type-define.h"
#include "vp-play-string-define.h"
#include "vp-play-value-define.h"
#include "vp-play-macro-define.h"

#ifdef ENABLE_DRM_FEATURE
#include "vp-drm.h"
#endif
#include "vp-play-util.h"

/* check temp */
#include "vp-play-log.h"
#include "vp-media-contents.h"
#include "vp-file-util.h"
#include "utils_i18n.h"

#define FILE_SIZE_LEN_MAX			64
#define UG_DATE_FORMAT_12			"yMMMdhms"
#define UG_DATE_FORMAT_24			"yMMMdHms"
#define EXT_SIZE 				128
#define EXT_SPLITTER				'.'
#define VP_PLAY_NAME_PATTERN			"[\\<>:;*\"|?/]"

enum VP_FILE_SIZE_TYPE {
	SIZE_BYTE = 0,
	SIZE_KB,
	SIZE_MB,
	SIZE_GB
};

static char *_vp_play_util_convert_string(const char *szLocale,
        const char *szCustomSkeleton,
        const char *szTimezone,
        UDate st_Date)
{
#define UG_ICU_ARR_LENGTH 			256

	// Copy a byte string encoded in the default codepage to a ustring.
	// Copies at most n characters. The result will be null terminated if the length of src is less than n.
	// Performs a host byte to UChar conversion.

	i18n_uchar ucustomSkeleton[UG_ICU_ARR_LENGTH] = { 0, };

	if (i18n_ustring_copy_ua_n
	        (ucustomSkeleton, szCustomSkeleton, UG_ICU_ARR_LENGTH) == NULL) {
		VideoLogInfo("i18n_ustring_copy_au_n() error.");
		return NULL;
	}

	i18n_uchar utimezone[UG_ICU_ARR_LENGTH] = { 0, };

	if (i18n_ustring_copy_ua_n(utimezone, szTimezone, UG_ICU_ARR_LENGTH)
	        == NULL) {
		VideoLogInfo("i18n_ustring_copy_au_n() error.");
		return NULL;
	}

	i18n_error_code_e status = I18N_ERROR_NONE;
	i18n_udatepg_h generator;
	i18n_udate_format_h formatter;

	i18n_uchar bestPattern[UG_ICU_ARR_LENGTH] = { 0, };
	i18n_uchar formatted[UG_ICU_ARR_LENGTH] = { 0, };
	char formattedString[UG_ICU_ARR_LENGTH] = { 0, };
	int32_t bestPatternLength, formattedLength;

	status = i18n_ucalendar_set_default_timezone(utimezone);
	if (status != I18N_ERROR_NONE) {
		VideoLogInfo("i18n_timezone_set_default() is failed.");
		return NULL;
	}

	status = i18n_ulocale_set_default(getenv("LC_TIME"));
	if (status != I18N_ERROR_NONE) {
		VideoLogInfo("i18n_ulocale_set_default() is failed.");
		return NULL;
	}

	status = i18n_udatepg_create(szLocale, &generator);
	if (generator == NULL || status != I18N_ERROR_NONE) {
		return NULL;
	}

	status =
	    i18n_udatepg_get_best_pattern(generator, ucustomSkeleton,
	                                  i18n_ustring_get_length
	                                  (ucustomSkeleton), bestPattern,
	                                  UG_ICU_ARR_LENGTH,
	                                  &bestPatternLength);
	if (bestPatternLength <= 0 || status != I18N_ERROR_NONE) {
		i18n_udatepg_destroy(generator);
		return NULL;
	}

	status =
	    i18n_udate_create(I18N_UDATE_MEDIUM, I18N_UDATE_MEDIUM, szLocale,
	                      NULL, -1, bestPattern, -1, &formatter);
	if (formatter == NULL || status != I18N_ERROR_NONE) {
		i18n_udatepg_destroy(generator);
		return NULL;
	}

	status =
	    i18n_udate_format_date(formatter, st_Date, formatted,
	                           UG_ICU_ARR_LENGTH, NULL, &formattedLength);
	if (formattedLength <= 0 || status != I18N_ERROR_NONE) {
		VideoLogInfo("i18n_udate_format_date() is failed.");
		i18n_udate_destroy(formatter);
		i18n_udatepg_destroy(generator);
		return NULL;
	}

	i18n_ustring_copy_au(formattedString, formatted);
	status = i18n_udatepg_destroy(generator);
	status = i18n_udate_destroy(formatter);

	if (strlen(formattedString) == 0) {
		return NULL;
	}

	return strdup(formattedString);
}



char *vp_play_util_get_title_from_path(char *szFilePath)
{
	if (szFilePath == NULL || strlen(szFilePath) <= 0) {
		VideoLogError("File Path is NULL");
		return NULL;
	}

	char *szTitle = NULL;
	const char *filename = vp_file_get(szFilePath);
	if (filename == NULL) {
		return NULL;
	}
	szTitle = vp_strip_ext(filename);

	return szTitle;
}

char *vp_play_util_get_filename_from_path(char *szFilePath)
{
	if (szFilePath == NULL || strlen(szFilePath) <= 0) {
		VideoLogError("File Path is NULL");
		return NULL;
	}
	const char *filename = vp_file_get(szFilePath);
	if (filename) {
		return g_strdup(filename);
	}
	return NULL;
}

char *vp_play_util_get_folder_from_path(char *szFilePath)
{
	if (szFilePath == NULL) {
		VideoLogError("File Path is NULL");
		return NULL;
	}
	char *szFile = NULL;
	char *szTitleName = NULL;
	int nLength = 0;

	nLength = strlen(szFilePath);

	if (nLength <= 0) {
		VideoLogError("Invalid buffer length");
		return NULL;
	}

	szTitleName = strrchr(szFilePath, '/');

	if (szTitleName) {
		szTitleName++;
	} else {
		szTitleName = szFilePath;
	}

	if (szTitleName == NULL) {
		VideoLogError("Invalid buffer");
		return NULL;

	}
	szFile = calloc(1, nLength - strlen(szTitleName));

	if (szFile != NULL) {
		strncpy(szFile, szFilePath, nLength - strlen(szTitleName) - 1);
	}

	return szFile;
}

bool vp_play_util_key_grab(Elm_Win *pWin, const char *szKeyName)
{
	if (!pWin) {
		VideoLogInfo("Invalid window handle.");
		return FALSE;
	}

	Eina_Bool ret = eext_win_keygrab_set(pWin, szKeyName);
	VideoLogInfo("%s key grab retruns: %d", szKeyName, ret);

	return TRUE;
}

bool vp_play_util_key_ungrab(Elm_Win *pWin, const char *szKeyName)
{
	if (!pWin) {
		VideoLogInfo("Invalid window handle.");
		return FALSE;
	}

	Eina_Bool ret = eext_win_keygrab_unset(pWin, szKeyName);
	VideoLogInfo("%s key ungrab retruns: %d", szKeyName, ret);

	return TRUE;
}

void vp_play_util_fb_visible_off(Evas_Object *pWin, bool bVisible)
{
	VideoLogInfo("");

	if (pWin == NULL) {
		VideoLogError("Window handle is NULL");
		return;
	}
	const char *pProfile = elm_win_profile_get(pWin);
	if (!pProfile) {
		VideoLogError("pProfile is NULL");
		return;
	}

	VideoLogWarning("Skip for open : %s", pProfile);

	return;
}


bool vp_play_util_get_landscape_check(int nRotateVal)
{
	video_play_rotate_t nRotate = VIDEO_PLAY_ROTATE_NONE;

	nRotate = (video_play_rotate_t) nRotateVal;

	if (nRotate == VIDEO_PLAY_ROTATE_90 ||
	        nRotate == VIDEO_PLAY_ROTATE_270) {
		return TRUE;
	}
	return FALSE;
}

int vp_play_util_get_root_window_angle(Evas_Object *pWin)
{
	int nAngle = 0;
	unsigned char *pProp_data = NULL;
	if (pWin == NULL) {
		VideoLogError("Window handle is NULL");
		return nAngle;
	}
#if 0//Tizen3.0 Build error
	int nRet = 0;
	int nCount = 0;
	Ecore_X_Window xwin = elm_win_xwindow_get(pWin);
	Ecore_X_Window root = ecore_x_window_root_get(xwin);
	nRet = ecore_x_window_prop_property_get(root,
	                                        ECORE_X_ATOM_E_ILLUME_ROTATE_ROOT_ANGLE,
	                                        ECORE_X_ATOM_CARDINAL, 32,
	                                        &pProp_data, &nCount);

	if (nRet && pProp_data) {
		memcpy(&nAngle, pProp_data, sizeof(int));
	} else {
		nAngle = 0;
	}
#endif
	VP_FREE(pProp_data);
	return nAngle;
}

/*
static int __vp_play_util_add_home_result_cb(int ret, int pid, void *data)
{
	VideoLogInfo("Client pid: %d, Return: %d", pid, ret);
	if (ret == SHORTCUT_SUCCESS) {
		vp_play_util_status_noti_show(VP_PLAY_STRING_COM_ADDED);
	} else if (SHORTCUT_STATUS_IS_CARED(ret)) {
	//	In this case, the homescreen handles error case already.
	//	It launch the popup or status message for user.
		VideoLogInfo("result code type: %d", SHORTCUT_ERROR_CODE(ret));
	} else if (SHORTCUT_STATUS_IS_ERROR(ret)) {
		VideoLogInfo("error code type: %d", SHORTCUT_ERROR_CODE(ret));
	}
	return 0;
}*/
/*
bool vp_play_util_add_to_home(const char *szMediaId, const char *szThumbnailURL)
{
	if (szMediaId == NULL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	int nRet = 0;
	char *szContentType = NULL;

	szContentType = g_strdup_printf("video-player:%s", szMediaId);

	nRet = add_to_home_livebox("org.tizen.videos", VP_VIDEO_PLAYER_TEXT, LIVEBOX_TYPE_2x2,
				   szContentType, szThumbnailURL, 30.0, 1,
				   __vp_play_util_add_home_result_cb, NULL);


	if (nRet != SHORTCUT_SUCCESS) {
		VideoLogError("add_to_home_livebox is Fail : %d", nRet);
		VP_FREE(szContentType);
		return FALSE;
	}

	VP_FREE(szContentType);

	return TRUE;
}
*/
bool vp_play_util_local_file_check(char *szMediaURL)
{
	if (szMediaURL == NULL) {
		VideoLogError("Invalid path!!");
		return FALSE;
	}

	char *path = szMediaURL;

	if (strstr(szMediaURL, VP_PLAY_FILE_PREFIX) != NULL) {
		path += strlen(VP_PLAY_FILE_PREFIX);
	}

	if (!g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
		g_access(path, R_OK);
		char *error_msg = calloc(1, 1024);
		if (error_msg) {
			strerror_r(errno, error_msg, 1024);
			VideoLogError("failed to access file: %d %s", errno, error_msg);
			free(error_msg);
		}
		return FALSE;
	}

	return TRUE;
}

bool vp_play_util_get_subtitle_path(const char *szMediaURL,
                                    char **szSubtitle)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL");
		return FALSE;
	}

	if (strlen(szMediaURL) < 1) {
		VideoLogError("szMediaURL is invalid Length");
		return FALSE;
	}

	struct stat buf;
	int nCount = 0;
	int nStrLength = 0;
	int nOutputLen = 0;

	char *szExt = NULL;
	char szExtWildkey[6][5] =
	{ ".srt", ".sub", ".smi", ".SRT", ".SUB", ".SMI" };
	char *szCheckFilePath = NULL;

	*szSubtitle = NULL;

	nOutputLen = strlen(szMediaURL) + 5;
	szCheckFilePath = calloc(1, sizeof(char) * nOutputLen);
	if (!szCheckFilePath) {
		VideoLogError("failed to allocate memory");
		return FALSE;
	}

	szExt = strrchr(szMediaURL, '.');
	if (szExt) {
		nStrLength = strlen(szMediaURL) - strlen(szExt);
	} else {
		nStrLength = strlen(szMediaURL);
	}

	for (nCount = 0; nCount < 6; nCount++) {
		memset(szCheckFilePath, 0, nOutputLen);
		strncpy(szCheckFilePath, szMediaURL, nStrLength);
		strncat(szCheckFilePath, szExtWildkey[nCount],
		        strlen(szExtWildkey[nCount]));

		if (!stat(szCheckFilePath, &buf)) {
			VideoLogWarning("Success finding subtitle file. - %d, %s",
			                nCount, szCheckFilePath);
			VP_STRDUP(*szSubtitle, szCheckFilePath);
			break;
		}
	}

	VP_FREE(szCheckFilePath);

	return TRUE;
}

double vp_play_util_get_sys_time(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double) tv.tv_usec / 1E6;
}

bool vp_play_util_status_noti_show(const char *szStr)
{
	if (!szStr) {
		VideoLogError("szStr is NULL.");
		return FALSE;
	}

	VideoLogInfo("");

	int nRet = notification_status_message_post(szStr);
	if (nRet != 0) {
		VideoLogError("notification_status_message_post() : [0x%x]",
		              nRet);
		return FALSE;
	}

	return TRUE;
}

char *vp_play_util_remove_prefix_to_url(char *szURL)
{
	if (szURL == NULL) {
		VideoLogError("szURL is NULL");
		return NULL;
	}

	if (strstr(szURL, VP_PLAY_FILE_PREFIX)) {
		char *szTemp =
		    calloc(1, strlen(szURL) - strlen(VP_PLAY_FILE_PREFIX) + 1);
		if (szTemp) {
			snprintf(szTemp, strlen(szURL) - strlen(VP_PLAY_FILE_PREFIX) + 1,
			         "%s", szURL + strlen(VP_PLAY_FILE_PREFIX));
		}
		return szTemp;
	}

	return NULL;
}

bool vp_play_util_check_sdp_url(const char *szUri)
{
	if (!szUri) {
		VideoLogInfo("szUri IS null");
		return FALSE;
	}
	char *szResult = strstr(szUri, ".sdp");
	if (szResult) {
		return TRUE;
	}
	return FALSE;
}

char *vp_play_util_get_sdp_url(const char *szUri)
{
	if (!szUri) {
		VideoLogInfo("szUri IS null");
		return NULL;
	}

	char *szTokenUrl = g_strdup(szUri);
	char **splited_str = eina_str_split(szTokenUrl, "?", 0);

	int uri_idx = 0;
	char *parsed_url = NULL;

	for (uri_idx = 0; splited_str[uri_idx]; uri_idx++) {
		VideoLogInfo("szUrl = %s", splited_str[uri_idx]);
		if (eina_str_has_prefix(splited_str[uri_idx], "http")) {	// http(or https)
			parsed_url = g_strdup(splited_str[uri_idx]);
			VideoLogInfo("parsed_url = %s", parsed_url);
			break;
		}
	}

	VP_FREE(splited_str[0]);
	VP_FREE(splited_str);
	VP_FREE(szTokenUrl);

	return parsed_url;
}

void vp_play_util_convert_file_size(long long lSize, char **szSize)
{
	unsigned long int size = (unsigned long int) lSize;

	int nCount = 0;
	while (size >= (BYTE_SIZE)) {
		size /= BYTE_SIZE;
		nCount++;
	}

	if (nCount == SIZE_BYTE) {
		*szSize = g_strdup_printf("%ld %s", size, VP_COM_BODY_B);
	} else if (nCount == SIZE_KB) {
		*szSize = g_strdup_printf("%ld %s", size, VP_COM_BODY_KB);
	} else if (nCount == SIZE_MB) {
		*szSize = g_strdup_printf("%ld %s", size, VP_COM_BODY_MB);
	} else if (nCount == SIZE_GB) {
		*szSize = g_strdup_printf("%ld %s", size, VP_COM_BODY_GB);
	}
}

void vp_play_util_get_convert_time(double dtime, char **szFileDate)
{
	char *szSkeleton = NULL;

	szSkeleton = UG_DATE_FORMAT_24;

	char *szLocale = NULL;	// eg en_US.UTF-8
	int retcode =
	    system_settings_get_value_string
	    (SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &szLocale);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE) || (szLocale == NULL)) {
		VideoLogInfo("Cannot get region format.");
		VP_STRDUP(szLocale, "en_US");	//// Default value.
	} else {
		char *find = strstr(szLocale, "UTF-8");
		if (find) {
			int diff = find - szLocale;
			if (diff > 0) {
				szLocale[diff - 1] = '\0';
			}
		}
	}

	char *szTimezone = NULL;
	retcode =
	    system_settings_get_value_string
	    (SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &szTimezone);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE) || (szTimezone == NULL)) {
		VideoLogWarning("Cannot get time zone.");
		VP_STRDUP(szTimezone, VP_PLAY_STRING_COM_UNKNOWN);
	}

	*szFileDate =
	    _vp_play_util_convert_string(szLocale, szSkeleton, szTimezone,
	                                 (UDate) dtime * 1000);
	if (*szFileDate == NULL) {
		VideoLogWarning("Cannot get time string.");
		VP_STRDUP(*szFileDate, VP_PLAY_STRING_COM_UNKNOWN);
	}
	VP_FREE(szTimezone);
	VP_FREE(szLocale);
}

bool vp_play_util_get_file_info(const char *szMediaURL, char **szFileSize,
                                char **szFileDate, char **szFileExtention)
{
	if (!szMediaURL) {
		VideoLogError("szMediaURL is NULL.");
		return FALSE;
	}

	struct stat statbuf;
	if (stat(szMediaURL, &statbuf) == -1) {
		VideoSecureLogError("%s file is NULL", szMediaURL);
		return FALSE;
	}

	int nCount = 0;
	size_t size = statbuf.st_size;
	unsigned long int lsize = (unsigned long int) size;
	//It is strange, when open below code, compile error, show could not find runtime_info_get_value_bool.
	//bool hours_24 = true;
	//int ret = -1;

	/*ret = runtime_info_get_value_bool(RUNTIME_INFO_KEY_24HOUR_CLOCK_FORMAT_ENABLED, &hours_24);
	   if (ret != RUNTIME_INFO_ERROR_NONE)
	   {
	   VideoLogError("Cannot get 24 hours format");
	   return NULL;
	   } */

	vp_play_util_get_convert_time((double) statbuf.st_mtime, szFileDate);

	while (size >= (BYTE_SIZE)) {
		lsize = size;
		size /= BYTE_SIZE;
		nCount++;
	}

	if (nCount == SIZE_BYTE) {
		*szFileSize = g_strdup_printf("%zu %s", size, VP_COM_BODY_B);
	} else if (nCount == SIZE_KB) {
		*szFileSize = g_strdup_printf("%zu %s", size, VP_COM_BODY_KB);
	} else if (nCount == SIZE_MB) {
		*szFileSize = g_strdup_printf("%zu %s", size, VP_COM_BODY_MB);
	} else if (nCount == SIZE_GB) {
		*szFileSize = g_strdup_printf("%zu %s", size, VP_COM_BODY_GB);
	}

	VideoLogInfo("Size = %lu, OrigSize=%zu", lsize, size);

	char *szExt = NULL;
#ifdef ENABLE_DRM_FEATURE
	char *szMime = NULL;
	char **dTempExt = NULL;

	int nArrayLen = 0;
	int nErr = 0;
	if (vp_drm_get_file_mime_type(szMediaURL, &szMime)) {
		nErr =
		    mime_type_get_file_extension(szMime, &dTempExt, &nArrayLen);

		VP_FREE(szMime);

		if (nErr == MIME_TYPE_ERROR_NONE && nArrayLen > 0) {
			if (dTempExt[0] != NULL) {
				szExt = strrchr(dTempExt[0], EXT_SPLITTER);
			}

			if (szExt == NULL) {
				*szFileExtention =
				    g_strdup_printf(VP_PLAY_STRING_COM_UNKNOWN);
			} else {
				VP_STRDUP(*szFileExtention, (szExt + 1));
			}

			int i = 0;
			for (; i < nArrayLen; i++) {
				if (dTempExt[i] != NULL) {
					free(dTempExt[i]);
				}
			}
		} else {
			*szFileExtention =
			    g_strdup_printf(VP_PLAY_STRING_COM_UNKNOWN);
		}
		VP_FREE(dTempExt);
	} else
#endif
	{

		szExt = strrchr(szMediaURL, EXT_SPLITTER);
		if (szExt == NULL) {
			*szFileExtention =
			    g_strdup_printf(VP_PLAY_STRING_COM_UNKNOWN);
		} else {
			VP_STRDUP(*szFileExtention, (szExt + 1));
		}
	}

	return TRUE;
}


bool vp_play_util_check_streaming(const char *szURL)
{
	if (szURL == NULL) {
		VideoLogInfo("szURL is NULL");
		return FALSE;
	}

	VideoSecureLogInfo("szURL : %s", szURL);

	if (strstr(szURL, "rtp") != NULL) {
		return TRUE;
	} else if (strstr(szURL, "rtsp") != NULL) {
		return TRUE;
	} else if (strstr(szURL, "http") != NULL) {
		return TRUE;
	} else if (strstr(szURL, "https") != NULL) {
		return TRUE;
	}

	return FALSE;
}


bool vp_play_util_rtsp_url_check(const char *szURL)
{
	if (szURL == NULL) {
		VideoLogInfo("szURL is NULL");
		return FALSE;
	}

	VideoSecureLogInfo("szURL : %s", szURL);

	static const char *rtsp_protocal_name[] = {
		"rtsp://",
		"rtp://",
		NULL,
	};

	int i = 0;
	while (rtsp_protocal_name[i] != NULL) {
		if (strlen(szURL) > strlen(rtsp_protocal_name[i])) {
			if (strncmp
			        (szURL, rtsp_protocal_name[i],
			         strlen(rtsp_protocal_name[i])) == 0) {
				return true;
			}
		}

		i++;
	}

	return FALSE;
}


bool vp_play_util_calculator_position(Evas_Coord_Rectangle rtSrc,
                                      Evas_Coord_Rectangle rtDst,
                                      Evas_Coord_Rectangle *rtResult,
                                      int nType)
{
	video_play_display_mode_t nMode = (video_play_display_mode_t) nType;

	double src_ratio = 0;
	double dst_ratio = 0;

	if (nMode == VIDEO_DISPLAY_LETTER_BOX) {
		src_ratio = (double) rtSrc.w / rtSrc.h;
		dst_ratio = (double) rtDst.w / rtDst.h;
		if (src_ratio > dst_ratio) {
			rtResult->w = rtDst.w;
			rtResult->h = rtDst.w / src_ratio;
			rtResult->x = 0;
			rtResult->y = (rtDst.h - rtResult->h) / 2;
		} else if (src_ratio < dst_ratio) {
			rtResult->w = rtDst.h * src_ratio;
			rtResult->h = rtDst.h;
			rtResult->x = (rtDst.w - rtResult->w) / 2;
			rtResult->y = 0;
		} else {
			rtResult->x = 0;
			rtResult->y = 0;
			rtResult->w = rtDst.w;
			rtResult->h = rtDst.h;
		}
	} else if (nMode == VIDEO_DISPLAY_ORIGIN) {
		rtResult->w = (((rtSrc.w) < (rtDst.w)) ? (rtSrc.w) : (rtDst.w));
		rtResult->h = (((rtSrc.h) < (rtDst.h)) ? (rtSrc.h) : (rtDst.h));
		rtResult->x = (rtDst.w - rtResult->w) / 2;
		rtResult->y = (rtDst.h - rtResult->h) / 2;

	} else if ((nMode == VIDEO_DISPLAY_FULL)
	           || (nMode == VIDEO_DISPLAY_CROPPED_FULL)) {
		rtResult->w = rtDst.w;
		rtResult->h = rtDst.h;
		rtResult->x = 0;
		rtResult->y = 0;

	}

	return TRUE;
}

bool vp_play_util_save_file(const char *szFileName, char *szBuffer,
                            int nSize)
{
	if (!szFileName) {
		VideoLogError("szFileName is NULL");
		return FALSE;
	}

	if (!szBuffer) {
		VideoLogError("szBuffer is NULL");
		return FALSE;
	}
	if (g_file_test(szFileName, G_FILE_TEST_EXISTS)) {
		int flag = g_remove(szFileName);
		if (flag != 0) {
			VideoLogError("g_remove is fail");
		}
	}

	if (!g_file_set_contents
	        (szFileName, (const char *) szBuffer, nSize, NULL)) {
		VideoLogError("g_file_set_contents is fail");
		return FALSE;
	}

	return TRUE;
}

bool vp_play_util_get_network_status(void)
{
	connection_h connection;
	connection_type_e net_state;
	bool bSuccessful = true;

	int err = 0;

	err = connection_create(&connection);
	if (err != CONNECTION_ERROR_NONE) {
		VideoLogWarning("connection_create error. err is [%d]", err);
		bSuccessful = false;
	}

	if (connection) {
		err = connection_get_type(connection, &net_state);
		connection_destroy(connection);

		if (err != CONNECTION_ERROR_NONE) {
			VideoLogWarning("connection_get_type error. err is [%d]",
			                err);
			bSuccessful = false;
		}

		VideoLogInfo("net_state is [%d]", net_state);

		if (net_state == CONNECTION_TYPE_DISCONNECTED) {
			bSuccessful = false;
		}
	}


	return bSuccessful;
}

bool vp_play_util_check_personal_status(const char *szPreviewUrl)
{
	Eina_Bool bRet = EINA_FALSE;
	if (szPreviewUrl) {
		bRet =
		    eina_str_has_prefix(szPreviewUrl, VP_PLAY_PERSONAL_HEAD_STR);
	}
	VideoSecureLogInfo("eina_str_has_prefix=%s:%d", szPreviewUrl,
	                   (int) bRet);
	return (bool) bRet;
}


bool vp_play_util_is_exist_subtitle_from_path(const char *szPath)
{

	if (szPath == NULL) {
		VideoLogError("szPath is NULL");
		return FALSE;
	}

	Eina_List *pFileList = NULL;
	Eina_List *pItemList = NULL;
	char *szFileName = NULL;
	char szExtWildkey[6][5] =
	{ ".srt", ".sub", ".smi", ".SRT", ".SUB", ".SMI" };
	int nCount = 0;


	pFileList = vp_file_ls(szPath);

	if (pFileList == NULL) {
		VideoLogError("pFileList is NULL");
		return FALSE;
	}

	EINA_LIST_FOREACH(pFileList, pItemList, szFileName) {
		if (szFileName == NULL) {
			continue;
		} else {
			char *szName = vp_strip_ext(szFileName);
			char *szExt = NULL;

			int nIdx = 0;
			if (szName == NULL) {
				continue;
			}
			szExt = szFileName + strlen(szName);
			/*if (szExt == NULL) {
				VP_FREE(szName);
				continue;
			}*/
			for (nIdx = 0; nIdx < 6; nIdx++) {
				if (strcmp(szExt, szExtWildkey[nIdx]) == 0) {
					nCount++;
				}
			}
			VP_FREE(szName);
		}
	}

	eina_list_free(pFileList);

	if (nCount > 0) {
		return TRUE;
	}
	return FALSE;
}

bool vp_play_util_get_subtitles_from_path(const char *szPath,
        GList **pSubtitleList)
{
	if (szPath == NULL) {
		VideoLogError("szPath is NULL");
		return FALSE;
	}

	Eina_List *pFileList = NULL;
	Eina_List *pItemList = NULL;
	char *szFileName = NULL;
	char szExtWildkey[6][5] =
	{ ".srt", ".sub", ".smi", ".SRT", ".SUB", ".SMI" };


	pFileList = vp_file_ls(szPath);

	if (pFileList == NULL) {
		VideoLogError("pFileList is NULL");
		return FALSE;
	}

	EINA_LIST_FOREACH(pFileList, pItemList, szFileName) {
		if (szFileName == NULL) {
			continue;
		} else {
			char *szName = vp_strip_ext(szFileName);
			char *szExt = NULL;

			int nIdx = 0;
			if (szName == NULL) {
				continue;
			}
			szExt = szFileName + strlen(szName);
			/*if (szExt == NULL) {
				VP_FREE(szName);
				continue;
			}*/
			for (nIdx = 0; nIdx < 6; nIdx++) {
				if (strcmp(szExt, szExtWildkey[nIdx]) == 0) {
					char *szSubtitleName = NULL;
					szSubtitleName =
					    g_strdup_printf("%s/%s", szPath, szFileName);
					*pSubtitleList =
					    g_list_append(*pSubtitleList, szSubtitleName);
				}
			}
			VP_FREE(szName);
		}
	}

	eina_list_free(pFileList);

	return TRUE;
}

bool vp_play_util_get_subtitle_path_list(GList **subtitleList)
{
	if (NULL == subtitleList) {
		VideoLogError("subtitleList is NULL");
		return FALSE;
	}

	if (!vp_media_contents_subtitle_list_get(subtitleList)) {
		VideoLogError("vp_media_contents_subtitle_list_get failed.");
		return FALSE;
	}

	return TRUE;
}


bool vp_play_util_app_resume(void)
{
	char *app_id = NULL;
	int ret = app_get_id(&app_id);
	if (ret != APP_ERROR_NONE || !app_id) {
		VideoLogError("app_get_id().. [0x%x], app_id[%s]", ret, app_id);
		return FALSE;
	}

	app_context_h context = NULL;
	ret = app_manager_get_app_context(app_id, &context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		VideoLogError("app_manager_get_app_context().. [0x%x]", ret);
		VP_FREE(app_id);
		return FALSE;
	}

	ret = app_manager_resume_app(context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		VideoLogError("app_manager_resume_app().. [0x%x]", ret);
		VP_FREE(app_id);
		return FALSE;
	}

	VP_FREE(app_id);
	app_context_destroy(context);

	return TRUE;
}


bool vp_play_util_check_white_space(char *szText)
{
	if (szText == NULL) {
		return TRUE;
	}
	bool bIsWhiteSpace = FALSE;

	if (strlen(szText) == 2) {
		if (szText[0] == 0xc2 && szText[1] == 0xa0) {
			VideoLogError("== No Break Space ==");
			bIsWhiteSpace = TRUE;
		}
	}

	g_strstrip(szText);
	if (strlen(szText) == 0) {
		VideoLogError("== NULL String ==");
		bIsWhiteSpace = TRUE;
	}

	return bIsWhiteSpace;
}


bool vp_play_util_check_valid_file_name(const char *szFileName)
{
	if (szFileName == NULL) {
		VideoLogError("szFileName is NULL");
		return FALSE;
	}

	char *pattern = NULL;
	int z, cflags = 0;
	bool bReturn = FALSE;
	char ebuf[128];
	regex_t reg;
	regmatch_t pm[1];
	const size_t nmatch = 1;

	/*ToDo: ignore the file star with . */
	if (strncmp(szFileName, ".", 1) == 0) {
		return FALSE;
	}

	pattern = VP_PLAY_NAME_PATTERN;
	z = regcomp(&reg, pattern, cflags);

	if (z != 0) {
		regerror(z, &reg, ebuf, sizeof(ebuf));
		VideoLogError("%s: pattern '%s'", ebuf, pattern);
		return FALSE;
	}

	z = regexec(&reg, szFileName, nmatch, pm, 0);
	if (z == REG_NOMATCH) {
		bReturn = TRUE;
	} else {
		bReturn = FALSE;
	}
	regfree(&reg);

	return bReturn;
}

unsigned long long vp_play_util_get_disk_available_space(const char
        *szFilePath)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return 0;
	}
	int nRet = 0;
	struct statvfs info;
	if (szFilePath
	        && 0 == strncmp(szFilePath, VP_ROOT_PATH_MMC,
	                        strlen(VP_ROOT_PATH_MMC))) {
		if (-1 == statvfs(VP_ROOT_PATH_MMC, &info)) {
			return 0;
		}
	} else {
		if (vp_is_dir(szFilePath)) {
			nRet = statvfs(szFilePath, &info);
			VideoLogInfo("this is folder");
		} else {
			nRet = statvfs(VP_PLAY_MEDIA_DIR, &info);
		}
		if (nRet < 0) {
			return 0;
		}
	}

	return (info.f_bsize) * info.f_bavail;
}

unsigned long long vp_play_util_get_file_size(const char *szFilePath)
{
	if (szFilePath == NULL) {
		VideoLogError("szFilePath is NULL");
		return 0;
	}

	unsigned long long lSize = 0;

	lSize = (unsigned long long) vp_file_size(szFilePath);

	return lSize;
}

bool vp_play_util_get_city(const char *szLocation, char **szCity)
{
#define LOCATION_MAX_LEN 			256

	if (!szLocation) {
		VideoLogError("szLocation is invalid");
		return FALSE;
	}

	char *pTemp = strstr(szLocation, "CITY[");
	if (!pTemp) {
		VideoLogError("pTemp is invalid");
		return FALSE;
	}
	int len = strlen("CITY[");
	char *pTemp1 = strstr(pTemp + len, "]");
	char city_temp[LOCATION_MAX_LEN] = { 0, };

	if (pTemp1 - (pTemp + len) > 0) {
		strncpy(city_temp, pTemp + len, pTemp1 - (pTemp + len));
		*szCity = strdup(city_temp);
	}

	return true;
}

bool vp_play_util_is_Xwindow_focused(Ecore_X_Window nXwinID)
{
#if 0//Tizen3.0 Build error
	Ecore_X_Window nXWindowFocus = 0;

	nXWindowFocus = ecore_x_window_focus_get();

	if (nXWindowFocus == nXwinID) {
		return TRUE;
	} else
#endif
		return TRUE;
}

char *vp_play_util_convert_rgba_to_hex(int r, int g, int b, int a)
{
	char *pColorHex;

	pColorHex =
	    g_strdup_printf("#%08lx",
	                    (long unsigned int)(r << 24 | g << 16 | b << 8 |
	                                        a));

	return pColorHex;
}

bool vp_play_util_convert_hex_to_rgba(char *pColorHex, int *r, int *g,
                                      int *b, int *a)
{
	if (pColorHex == NULL) {
		return FALSE;
	}

	char *szColor = g_strndup(pColorHex + 1, 8);

	unsigned long lColor = strtoul(szColor, NULL, 16);

	int tr, tg, tb, ta;
	tr = lColor >> 24;

	if (tr < 0) {
		tr += 0x100;
	}
	tg = (lColor >> 16) & 0xff;
	tb = (lColor >> 8) & 0xff;
	ta = lColor & 0xff;

	*r = tr;
	*g = tg;
	*b = tb;
	*a = ta;
	free(szColor);

	return TRUE;
}

void vp_play_util_set_object_size(Evas_Object *obj, int w, int h, int id)
{
	Edje_Message_Int_Set *msg =
	    (Edje_Message_Int_Set *) malloc(sizeof(Edje_Message_Int_Set) +
	                                    3 * sizeof(int));
	if (!msg) {
		return;
	}
	msg->count = 2;
	msg->val[0] = w;
	msg->val[1] = h;
	edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, id, msg);
	free(msg);
}

void vp_play_util_set_object_color(Evas_Object *obj, int r, int g, int b,
                                   int a, int id)
{
	Edje_Message_Int_Set *msg =
	    (Edje_Message_Int_Set *) malloc(sizeof(Edje_Message_Int_Set) +
	                                    4 * sizeof(int));
	if (!msg) {
		return;
	}
	msg->count = 4;
	msg->val[0] = r;
	msg->val[1] = g;
	msg->val[2] = b;
	msg->val[3] = a;

	edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, id, msg);
	free(msg);
}

void vp_play_util_set_object_offset(Evas_Object *obj, int left, int top,
                                    int id)
{
	Edje_Message_Int_Set *msg =
	    (Edje_Message_Int_Set *) malloc(sizeof(Edje_Message_Int_Set) +
	                                    2 * sizeof(float));
	if (!msg) {
		return;
	}
	msg->count = 2;
	msg->val[0] = left;
	msg->val[1] = top;
	edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, id, msg);
	free(msg);
}

#if 0//Tizen3.0 Build error
static int __vp_play_util_append_variant(DBusMessageIter *iter, const char *sig, const char *param[])
{
	VideoLogInfo("");

	char *ch;
	int i;
	int int_type;
	uint64_t int64_type;

	if (!sig || !param) {
		VideoLogInfo("!sig || !param");
		return 0;
	}

	for (ch = (char*)sig, i = 0; *ch != '\0'; ++i, ++ch) {
		VideoLogInfo("i=%d, ch = %c param = %s", i, *ch, param[i]);

		switch (*ch) {
		case 'i':
			int_type = atoi(param[i]);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &int_type);
			break;
		case 'u':
			int_type = atoi(param[i]);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &int_type);
			break;
		case 't':
			int64_type = atoi(param[i]);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &int64_type);
			break;
		case 's':
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &(param[i]));
			break;
		default:
			return -EINVAL;
		}
	}

	return 0;
}

#define DBUS_REPLY_TIMEOUT (120 * 1000)
DBusMessage * vp_play_util_invoke_dbus_method(const char *dest, const char *path,
        const char *interface, const char *method,
        const char *sig, const char *param[])
{
	VideoLogInfo("");

	DBusConnection *conn;
	DBusMessage *msg;
	DBusMessageIter iter;
	DBusMessage *reply;
	DBusError err;
	int r;

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (!conn) {
		VideoLogError("dbus_bus_get error");
		return NULL;
	}

	msg = dbus_message_new_method_call(dest, path, interface, method);
	if (!msg) {
		VideoLogError("dbus_message_new_method_call error!");
		return NULL;
	}

	dbus_message_iter_init_append(msg, &iter);

	r = __vp_play_util_append_variant(&iter, sig, param);
	if (r < 0) {
		VideoLogError("append_variant error(%d)", r);
		dbus_message_unref(msg);
		return NULL;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, DBUS_REPLY_TIMEOUT, &err);
	if (!reply) {
		VideoLogError("dbus_connection_send error(No reply)");
	}

	if (dbus_error_is_set(&err)) {
		VideoLogError("dbus_connection_send error(%s:%s)", err.name, err.message);
		reply = NULL;
	}

	dbus_message_unref(msg);
	dbus_error_free(&err);

	return reply;
}

void
vp_play_util_set_lock_power_key()
{
	const char *arr[] = {"lcdon", "gotostatenow", "holdkeyblock", "0"};


	DBusMessage *msg;

	msg = vp_play_util_invoke_dbus_method("org.tizen.system.deviced", "/Org/Tizen/System/DeviceD/Display",
	                                      "org.tizen.system.deviced.display", "lockstate", "sssi", arr);

	if (!msg) {
		VideoLogError("vp_play_util_invoke_dbus_method failed");
		return;
	}
}

void
vp_play_util_set_unlock_power_key()
{
	const char *arr[] = {"lcdon", "resettimer"};

	DBusMessage *msg;

	msg = vp_play_util_invoke_dbus_method("org.tizen.system.deviced", "/Org/Tizen/System/DeviceD/Display",
	                                      "org.tizen.system.deviced.display", "unlockstate", "ss", arr);

	if (!msg) {
		VideoLogError("vp_play_util_invoke_dbus_method failed");
		return;
	}
}
#else
void
vp_play_util_set_lock_power_key()
{
}

void
vp_play_util_set_unlock_power_key()
{
}
#endif
void vp_play_util_focus_next_object_set(Evas_Object *obj,
                                        Evas_Object *next,
                                        Elm_Focus_Direction dir)
{
	if (obj) {
		elm_object_focus_next_object_set(obj, next, dir);
	}

}
