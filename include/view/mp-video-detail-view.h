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

#ifndef  _VIDEO_PLAYER_DETAIL_VIEW_
#define  _VIDEO_PLAYER_DETAIL_VIEW_

typedef struct _VideoDetailInfo{
	char *szTitle;
	char *szSize;
	char *szFormat;
	//char *szDate;
	char *szResolution;
	char *szLastModifiedTime;
	char *szLocation;
	//char *szLatitude;
	//char *szLongitude;
}stDetailInfo;

typedef void(*mpDetailViewCbFunc)(void);

void mp_detail_view_free_detail_info(stDetailInfo *pDetailInfo);
//bool mp_detail_view_popup_show(void *pUserData, mpDetailCtrlCbFunc pDetailCtrlCb, stDetailInfo *pDetailInfo);
void mp_detail_view_push(void* pNaviFrame, bool IsFolder, const char *pMediaId, stDetailInfo *pDetailInfo);

void mp_detail_view_delete_handle(void);
bool mp_detail_view_get_video_detail_info(int nVideoItemIndex, stDetailInfo **pDetailInfo);
bool mp_detail_view_get_folder_detail_info(int nFolderItemIndex, stDetailInfo **pDetailInfo);
void mp_detail_view_update(void);

#endif // __MP_VIDEO_DETAIL_VIEW__

