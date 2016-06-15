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


#ifndef __VP_SERVICE_PARSER__
#define	 __VP_SERVICE_PARSER__

#include <glib.h>
#include <Eina.h>
#include <app.h>
//#include <Ecore_X.h>

typedef enum
{
	VP_LAUNCH_TYPE_NONE			= 0,
	VP_LAUNCH_TYPE_STORE,
	VP_LAUNCH_TYPE_LIST,
	VP_LAUNCH_TYPE_SIMPLE,
	VP_LAUNCH_TYPE_EMAIL,
	VP_LAUNCH_TYPE_MMS,
	VP_LAUNCH_TYPE_MYFILE,
	VP_LAUNCH_TYPE_GALLERY,
	VP_LAUNCH_TYPE_IMAGE_VIEWER,
	VP_LAUNCH_TYPE_IMAGE_VIEWER_TRIM,
	VP_LAUNCH_TYPE_MULTI_PATH,
	VP_LAUNCH_TYPE_MAX,
}VpLaunchType;


typedef enum
{
	VP_VIDEO_SORT_TYPE_BY_NONE					= 0,

	VP_VIDEO_SORT_TYPE_BY_NAME_DESC,
	VP_VIDEO_SORT_TYPE_BY_NAME_ASC,
	VP_VIDEO_SORT_TYPE_BY_DATE_DESC,
	VP_VIDEO_SORT_TYPE_BY_DATE_ASC,

	VP_VIDEO_SORT_TYPE_BY_MAX,
}VpVideoSortType;


typedef enum
{
	VP_VIDEO_PLAY_LIST_TYPE_NONE				= 0,

	VP_VIDEO_PLAY_LIST_TYPE_FOLDER,
	VP_VIDEO_PLAY_LIST_TYPE_ALL_FOLDER_GALLERY,
	VP_VIDEO_PLAY_LIST_TYPE_TAG_GALLERY,
	VP_VIDEO_PLAY_LIST_TYPE_FAVORITE,
	VP_VIDEO_PLAY_LIST_TYPE_MAX,
}VpVideoPlayListType;


typedef struct
{
	VpLaunchType		eLaunchType;		// launch type

	VpVideoSortType		eSortType;			// only use gallery, image viewer and my files(?)
	VpVideoPlayListType	eListType;			// only use gallery, image viewer and my files(?)
#if 0//Tizen3.0 Build error
	Ecore_X_Window		nParentXID;
#else
	unsigned int			nParentXID;
#endif

	Eina_Bool		bLaunchMultiWindow;	// launch multi winodw
	Eina_Bool		bStreaming;			// launch streaming
	Eina_Bool		bStoreDownload;
	double			dStoreDL_FileSize;
	int				nStorePlayedTime;
	Eina_Bool		bStopAfterRepeat;

	char			*pMediaUri;			// play file path or url
	char			*szMediaTitle;			// media file title
	char			*szDeviceID;
	char			*szAutoPlay;
	char			*szDMRID;
	char			*szCookie;
	char			*szProxy;

	char			**szMultiPath;
	int			nMultiPath;

	char			**szMultiTitle;
	int			nMultiTitle;

	char			**szIsSameAP;		// Samsung link : remote / local content,
	int			nSameAP;

	char			**szMultiSubTitle;
	int			nMultiSubTitle;

	char			**szMultiPosition;
	int			nMultiPosition;

	char			**szMultiDuration;
	int			nMultiDuration;

	char		*pStoreOrderId;
	char		*pStoreServerId;
	char		*pStoreAppId;
	char		*pStoreMvId;
}VpServiceData;


#ifdef _cplusplus
extern "C"
{
#endif


Eina_Bool	VpServiceParserGetServiceData( app_control_h pAppSvcHandle, VpServiceData *pReceiveData );
void		VpServiceParserDestroyServiceData( VpServiceData *pServiceData );


#ifdef _cplusplus
}
#endif


#endif //__VP_SERVICE_PARSER__

