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


#include <metadata_extractor.h>
#include <mime_type.h>

#include <unicode/udat.h>
#include <unicode/ustring.h>
#include <unicode/uloc.h>
#include <unicode/ucal.h>
#include <unicode/udatpg.h>
#include <unicode/utmscale.h>
#include <system_settings.h>

#include "mp-util.h"
#ifdef ENABLE_DRM_FEATURE
#include "mp-drm-ctrl.h"
#endif
#include "mp-video-log.h"
#include "video-player.h"
#include "mp-video-info-ctrl.h"
#include "mp-video-string-define.h"
#include "utils_i18n.h"

enum VIDEO_PLAYER_FILE_SIZE_TYPE
{
	SIZE_BYTE = 0,
	SIZE_KB,
	SIZE_MB,
	SIZE_GB
};

bool mp_info_ctrl_get_gps(char *szUriPath, double *dLongitude, double *dLatitude)
{
	if (!szUriPath) {
		VideoLogInfo("[ERR] No exist szUriPath.");
		return FALSE;
	}
#ifdef ENABLE_DRM_FEATURE
	if (mp_drm_is_drm_file(szUriPath)) {
		VideoLogInfo("It's drm file.");
		return FALSE;
	}
#endif
	metadata_extractor_h pMetadata;

	char *szTmp = NULL;

	*dLatitude = 0.0;
	*dLongitude = 0.0;

	if (metadata_extractor_create(&pMetadata) != METADATA_EXTRACTOR_ERROR_NONE)
	{
		VideoLogInfo("[ERR] - metadata_extractor_create()");
		goto Execption;
	}

	if (metadata_extractor_set_path(pMetadata, szUriPath) != METADATA_EXTRACTOR_ERROR_NONE)
	{
		VideoLogInfo("[ERR] - metadata_extractor_set_path()");
		goto Execption;
	}

	if (metadata_extractor_get_metadata(pMetadata, METADATA_LATITUDE, &szTmp) != METADATA_EXTRACTOR_ERROR_NONE)
	{
		VideoLogInfo("[ERR] - metadata_extractor_get_metadata() - METADATA_LATITUDE");
		goto Execption;
	}
	else
	{
		if (szTmp)
		{
			VideoLogInfo("METADATA_LATITUDE : %s", szTmp);
			*dLatitude = atof(szTmp);

			free(szTmp);
			szTmp = NULL;
		}
		else
		{
			*dLatitude = 0.0;
		}


	}

	if (metadata_extractor_get_metadata(pMetadata, METADATA_LONGITUDE, &szTmp) != METADATA_EXTRACTOR_ERROR_NONE)
	{
		VideoLogInfo("[ERR] - metadata_extractor_get_metadata() - METADATA_LONGITUDE");
		if (szTmp)
		{
			free(szTmp);
			szTmp = NULL;
		}
		*dLongitude = 0.0;
		goto Execption;
	}
	else
	{
		if (szTmp)
		{
			VideoLogInfo("METADATA_LONGITUDE : %s", szTmp);
			*dLongitude = atof(szTmp);
			free(szTmp);
			szTmp = NULL;
		}
		else
		{
			*dLongitude = 0.0;
		}
	}

	if (pMetadata)
	{
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}

	return TRUE;

Execption:
	if (pMetadata)
	{
		metadata_extractor_destroy(pMetadata);
		pMetadata = NULL;
	}
	return FALSE;
}

bool mp_info_ctrl_get_resolution(char *szWidthResolution, char *szHeightResolution, void *pUserData)
{
	if (!pUserData)
	{
		VideoLogError("[ERR] No exist pUserData.");
		return FALSE;
	}

	// TO-DO : It is possible to get resolution size from media service.

	return TRUE;
}

static char *mp_info_ctrl_converting_string(const char *szLocale, const char *szCustomSkeleton, const char *szTimezone, UDate st_Date)
{
#define UG_ICU_ARR_LENGTH 			256

	// Copy a byte string encoded in the default codepage to a ustring.
	// Copies at most n characters. The result will be null terminated if the length of src is less than n.
	// Performs a host byte to UChar conversion.

	i18n_uchar ucustomSkeleton[UG_ICU_ARR_LENGTH] = {0,};

	if (i18n_ustring_copy_ua_n(ucustomSkeleton, szCustomSkeleton, UG_ICU_ARR_LENGTH) == NULL) {
		VideoLogError("i18n_ustring_copy_au_n() error.");
		return NULL;
	}

	i18n_uchar utimezone[UG_ICU_ARR_LENGTH] = {0,};

	if (i18n_ustring_copy_ua_n(utimezone, szTimezone, UG_ICU_ARR_LENGTH) == NULL) {
		VideoLogError("i18n_ustring_copy_au_n() error.");
		return NULL;
	}

	i18n_error_code_e status = I18N_ERROR_NONE;
	i18n_udatepg_h generator;
	i18n_udate_format_h formatter;

	i18n_uchar bestPattern[UG_ICU_ARR_LENGTH] = {0,};
	i18n_uchar formatted[UG_ICU_ARR_LENGTH] = {0,};
	char formattedString[UG_ICU_ARR_LENGTH] = {0,};
	int32_t bestPatternLength, formattedLength;

	status = i18n_ucalendar_set_default_timezone(utimezone);
	if (status != I18N_ERROR_NONE) {
		VideoLogError("i18n_timezone_set_default() is failed.");
		return NULL;
	}

	status = i18n_ulocale_set_default(getenv("LC_TIME"));
	if (status != I18N_ERROR_NONE) {
		VideoLogError("i18n_timezone_set_default() is failed.");
		return NULL;
	}

	status = i18n_udatepg_create(szLocale, &generator);
	if (generator == NULL || status != I18N_ERROR_NONE) {
		return NULL;
	}

	status = i18n_udatepg_get_best_pattern(generator, ucustomSkeleton, i18n_ustring_get_length(ucustomSkeleton), bestPattern, UG_ICU_ARR_LENGTH, &bestPatternLength);
	if (bestPatternLength <= 0 || status != I18N_ERROR_NONE) {
		i18n_udatepg_destroy(generator);
		return NULL;
	}


	char buffer[UG_ICU_ARR_LENGTH] = {0,};

	i18n_ustring_copy_au(buffer, bestPattern);
	VideoLogInfo("bestPattern = %s", buffer);

	int i = 0;
	int len = strlen(buffer);
	for (i = 0; i < len; i++) {
		if (buffer[i] == 'K')// K is 0~11, it is used at japan icu
		{
			buffer[i] = 'h';// h is 1~12
		}
	}

	VideoLogInfo("buffer = %s", buffer);

	i18n_uchar ubestpattern[UG_ICU_ARR_LENGTH] = {0,};
	if (i18n_ustring_copy_ua_n(ubestpattern, buffer, UG_ICU_ARR_LENGTH) == NULL) {
		i18n_udatepg_destroy(generator);
		VideoLogError("i18n_ustring_copy_au_n() error.");
		return NULL;
	}

	status = i18n_udate_create(I18N_UDATE_MEDIUM, I18N_UDATE_MEDIUM, szLocale, NULL, -1, ubestpattern, -1, &formatter);
	if (formatter == NULL || status != I18N_ERROR_NONE) {
		i18n_udatepg_destroy(generator);
		return NULL;
	}

	status = i18n_udate_format_date(formatter, st_Date, formatted, UG_ICU_ARR_LENGTH, NULL, &formattedLength);
	if (formattedLength <= 0 || status != I18N_ERROR_NONE) {
		i18n_udatepg_destroy(generator);
		i18n_udate_destroy(formatter);
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

char *mp_info_ctrl_get_data_of_file(time_t mtime)
{
#define UG_DATE_FORMAT_12			"yMMMdhms"
#define UG_DATE_FORMAT_24			"yMMMdHms"

	VideoLogInfo("");

	char *szSkeleton = NULL;

	bool hours_24 = false;
	int ret = -1;

	ret = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &hours_24);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE)
	{
		VideoLogError("Cannot get 24 hours format");
		return NULL;
	}

	if (hours_24 == true)
	{
		szSkeleton = UG_DATE_FORMAT_24;
	}
	else
	{
		szSkeleton = UG_DATE_FORMAT_12;
	}

	char *szLocale = NULL;		// eg en_US.UTF-8
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &szLocale);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE) || (szLocale == NULL))
	{
		VideoLogInfo("Cannot get region format.");
		szLocale = strdup("en_US");			// Default value.
	}
	else
	{
		char *find = strstr(szLocale, "UTF-8");
		if (find)
		{
			int diff = find - szLocale;
			if (diff > 0)
			{
				szLocale[diff-1] = '\0';
			}
		}
	}

	char *szTimezone = NULL;
	retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &szTimezone);
	if ((retcode != SYSTEM_SETTINGS_ERROR_NONE) || (szTimezone == NULL))
	{
		VideoLogError("Cannot get time zone.");
		MP_FREE_STRING(szLocale);
		return strdup("N/A");
	}

	VideoLogInfo("Locale : %s TimeZone : %s TimeFormat : %s", szLocale, szSkeleton, szTimezone);

	char *szDatestr = NULL;
	szDatestr = mp_info_ctrl_converting_string(szLocale, szSkeleton, szTimezone, (UDate)mtime * 1000);
	if (!szDatestr)
	{
		VideoLogError("Cannot get time string.");
		MP_FREE_STRING(szLocale);
		MP_FREE_STRING(szTimezone);
		return strdup("N/A");
	}

	VideoLogInfo("ICU Date : %s", szDatestr);

	MP_FREE_STRING(szLocale);
	MP_FREE_STRING(szTimezone);

	return szDatestr;
}

#ifdef ENABLE_DRM_FEATURE
static char *mp_info_ctrl_get_drm_file_extension(char *szPath)
{
#define EXT_SIZE 		128
#define EXT_SPLITTER	'.'

	if (!szPath) {
		VideoLogInfo("[ERR] No exist szPath.");
		return NULL;
	}

	char szDrmContentType[STR_LEN_MAX] = {0,};

	mp_drm_get_file_extension(szPath, szDrmContentType, STR_LEN_MAX);
	VideoLogInfo("szDrmContentType : %s", szDrmContentType);

	char	**dTempExt 		= NULL;
	char 	*szExt 			= NULL;
	char 	*pFileExtention	= NULL;

	int 	nArrayLen		= 0;
	int 	nErr 			= -1;

	nErr = mime_type_get_file_extension(szDrmContentType, &dTempExt, &nArrayLen);

	if (nErr == MIME_TYPE_ERROR_NONE && nArrayLen > 0) {
		if (dTempExt[0] != NULL)
		{
			szExt = strrchr(dTempExt[0], EXT_SPLITTER);
		}

		if (szExt != NULL && szExt + 1 != NULL)
		{
			pFileExtention	= strdup(szExt + 1);
		}
	}

	int i = 0;
	for (; i < nArrayLen; i++)
	{
		MP_FREE_STRING(dTempExt[i]);
	}

	MP_FREE_STRING(dTempExt);

	return pFileExtention;
}
#endif

char *mp_info_ctrl_get_file_extension(char *szPath)
{
	if (!szPath) {
		VideoLogError("Cannot get file extension. path is NULL");
		return strdup("Unknown");
	}
#ifdef ENABLE_DRM_FEATURE
	if (mp_drm_is_drm_file(szPath)) {

		char *szFileExtension = mp_info_ctrl_get_drm_file_extension(szPath);
		if (szFileExtension) {
			VideoLogInfo("Drm file extension. - %s", szFileExtension);
			return szFileExtension;
		} else {
			VideoLogInfo("No have file extension.");
			return strdup("Unknown");
		}
	} else
#endif
	{
		char *szExt = NULL;
		szExt = strrchr(szPath, '.');

		if ((szExt != NULL) && ((szExt+1) != NULL)) {
			return strdup(szExt + 1);
		}
	}

	return strdup("Unknown");
}

bool mp_info_ctrl_get_file_info(char *szUriPath, char *szFileDate, int nFileDateSize, char *szFileExtension, int nFileExtensionSize, char *szFileSize, int nFilesizeSize)
{
	if (!szUriPath) {
		VideoLogInfo("[ERR] No exist szUriPath.");
		return FALSE;
	}

	struct stat statbuf;

	if (stat(szUriPath, &statbuf) == -1) {
		VideoLogInfo("%s file is NULL", szUriPath);
		return FALSE;
	}

	memset(szFileDate, 0, nFileDateSize);
	memset(szFileExtension, 0, nFileExtensionSize);
	memset(szFileSize, 0, nFilesizeSize);

	char *szTmpDateOfFile = mp_info_ctrl_get_data_of_file(statbuf.st_mtime);
	char *szTmpFileExtension = mp_info_ctrl_get_file_extension(szUriPath);
	char *szTmpFileSize = mp_util_get_file_size((unsigned long long)statbuf.st_size);

	if (szTmpDateOfFile) {
		VideoLogInfo("szTmpDateOfFile : %s", szTmpDateOfFile);
		strncpy(szFileDate, szTmpDateOfFile, nFileDateSize);
		free(szTmpDateOfFile);
	}

	if (szTmpFileExtension) {
		VideoLogInfo("szTmpFileExtension : %s", szTmpFileExtension);
		strncpy(szFileExtension, szTmpFileExtension, nFileExtensionSize);
		free(szTmpFileExtension);
	}

	if (szTmpFileSize) {
		VideoLogInfo("szTmpFileSize : %s", szTmpFileSize);
		strncpy(szFileSize, szTmpFileSize, nFilesizeSize);
		free(szTmpFileSize);
	}

	return TRUE;
}
