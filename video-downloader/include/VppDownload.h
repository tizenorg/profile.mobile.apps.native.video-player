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


#ifndef __VPP_DOWNLOAD_H__
#define __VPP_DOWNLOAD_H__


#include <stdbool.h>


/* downloader handle */
typedef	void *VppDownloadHandle;


/**
 * @brief Enumerations of download state
 */
typedef enum
{
	VPP_DOWNLOAD_STATE_AGENT_ERROR 	= -2,	/**< agent error */
	VPP_DOWNLOAD_STATE_APP_ERROR 	= -1,	/**< application error */
	VPP_DOWNLOAD_STATE_NONE 		= 0,	/**< download_state_e :  DOWNLOAD_STATE_NONE */
	VPP_DOWNLOAD_STATE_READY, 				/**< download_state_e :  DOWNLOAD_STATE_READY */
	VPP_DOWNLOAD_STATE_QUEUED, 				/**< download_state_e :  DOWNLOAD_STATE_QUEUED */
	VPP_DOWNLOAD_STATE_DOWNLOADING, 		/**< download_state_e :  DOWNLOAD_STATE_DOWNLOADING */
	VPP_DOWNLOAD_STATE_PAUSED, 				/**< download_state_e :  DOWNLOAD_STATE_PAUSED */
	VPP_DOWNLOAD_STATE_COMPLETED, 			/**< download_state_e :  DOWNLOAD_STATE_COMPLETED */
	VPP_DOWNLOAD_STATE_FAILED, 				/**< download_state_e :  DOWNLOAD_STATE_FAILED */
	VPP_DOWNLOAD_STATE_CANCELED, 			/**< download_state_e :  DOWNLOAD_STATE_CANCELED */
} VppDownloadState;


/**
 * @brief Enumerations of download item type
 */
typedef enum
{
	VPP_DOWNLOAD_ITEM_TYPE_NONE 				= 0,	/**< NONE */

	VPP_DOWNLOAD_ITEM_TYPE_VIDEO_FILE,					/**< video file */

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// these values only supprot store
	VPP_DOWNLOAD_ITEM_TYPE_STORE_VIDEO_FILE,			/**< store video file */
	VPP_DOWNLOAD_ITEM_TYPE_VIDEO_INDEX_FILE,			/**< store video infex file. It will be reomved when download video file is completed. */
	VPP_DOWNLOAD_ITEM_TYPE_VIDEO_CAPTION_FILE,			/**< store video caption file */
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VPP_DOWNLOAD_ITEM_TYPE_THUMBNAIL_IMAGE,				/* product thumbnail image */
	VPP_DOWNLOAD_ITEM_TYPE_LARGE_POSTER_IMAGE,			/* product large poster image */
	VPP_DOWNLOAD_ITEM_TYPE_CATEGORY_IMAGE,				/* category thumbnail image */
	VPP_DOWNLOAD_ITEM_TYPE_RATING_IMAGE,				/* user rating image */
	VPP_DOWNLOAD_ITEM_TYPE_CAPION_FILE,					/* caption file */
} VppDownloadItemType;


/* downloader callback */
typedef void (*vpp_dl_progress_cb) ( VppDownloadHandle pHandle, unsigned long long received, unsigned long long total, void *userData );
typedef void (*vpp_dl_state_changed_cb) ( VppDownloadHandle pHandle, VppDownloadState eState, const char *pPath, void *userData );


// download callback structure
typedef struct
{
	vpp_dl_progress_cb		progressCb;
	vpp_dl_state_changed_cb	stateChangedCb;
}VppDownloadCallback;


#ifdef _cplusplus
extern "C"
{
#endif


VppDownloadHandle VppDownloadCreateItem( const char *pUrl, const char *pDstFolder, const char *pDstName, VppDownloadItemType eType, VppDownloadCallback cb, void *userData );
VppDownloadHandle VppDownloadCreateItemById( int nId, const char *pUrl, const char *pDstFolder, const char *pDstName, VppDownloadItemType eType, VppDownloadCallback cb, void *userData );

void VppDownloadDestroyItem( VppDownloadHandle pHandle );
void VppDownloadDestroyItemById( int nId );
void VppDownloadClearItem( VppDownloadHandle pHandle );

bool VppDownloadStartVideoStoreVideoItem( VppDownloadHandle pHandle, void *pOnGoingServiceData, void*pCompleteServiceData );	// pServiceData is app control. It is service extra data at quickpanel service.


bool VppDownloadSetCb( VppDownloadHandle pHandle, VppDownloadCallback cb, void *userData );
bool VppDownloadUnsetCb( VppDownloadHandle pHandle );

bool VppDownloadRequestAppend( VppDownloadHandle pHandle );
bool VppDownloadRequestPrepend( VppDownloadHandle pHandle );

bool VppDownloadPause( VppDownloadHandle pHandle );
bool VppDownloadResume( VppDownloadHandle pHandle );

bool VppDownloadCancel( VppDownloadHandle pHandle );

bool VppDownloadStoreVideoRestore( VppDownloadHandle pHandle );

VppDownloadState VppDownloadGetState( VppDownloadHandle pHandle );
VppDownloadState VppDownloadGetStateById( int nDownloadId );

char *VppDownloadGetDownloadedFilePath( VppDownloadHandle pHandle );
int VppDownloadGetDownloadId( VppDownloadHandle pHandle );

bool VppDownloadSetNotificationAppControl( VppDownloadHandle pHandle, void *pData );
void* VppDownloadGetNotificationAppControl( VppDownloadHandle pHandle );

void VppDownloadResetUserData(void *pDataItem);

#ifdef _cplusplus
}
#endif


#endif	// __VPP_DOWNLOAD_H__

