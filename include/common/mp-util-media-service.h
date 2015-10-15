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



#ifndef _MP_UTIL_DATABASE_
#define _MP_UTIL_DATABASE_

#define RECENTLY_PLAYED_VIDEO_ITEM_MAX		20

typedef enum
{
	MP_MEDIA_VIDEO_FILE_TYPE			= 0,
	MP_MEDIA_CAMERA_FILE_TYPE,
	MP_MEDIA_ALL_FILE_TYPE,
	MP_MEDIA_FILE_TYPE_MAX,
}MpMediaSvcVideoFileType;

typedef enum
{
	MP_MEDIA_SORT_BY_NONE				= 0,
	MP_MEDIA_SORT_BY_RECENTLY_VIEWED,
	MP_MEDIA_SORT_BY_RECENTLY_ADDED,
	MP_MEDIA_SORT_BY_NAME,
	MP_MEDIA_SORT_BY_SIZE,
	MP_MEDIA_SORT_BY_TYPE,
	MP_MEDIA_SORT_BY_MAX,
}MpMediaSvcSortType;


typedef enum
{
	MP_MEDIA_LIST_TYPE_FOLDER			= 0,
	MP_MEDIA_LIST_TYPE_ALL_FOLDER_GALLERY,
	MP_MEDIA_LIST_TYPE_TAG_GALLERY,
	MP_MEDIA_LIST_TYPE_FAVORITE,
	MP_MEDIA_LIST_TYPE_MAX,
}MpMediaSvcListType;

typedef enum
{
	MP_MEDIA_TYPE_STORAGE_UNKNOWN		= 0,
	MP_MEDIA_TYPE_STORAGE_INTERNAL,
	MP_MEDIA_TYPE_STORAGE_EXTERNAL,
	MP_MEDIA_TYPE_STORAGE_DROPBOX,
	MP_MEDIA_TYPE_STORAGE_MAX,
}MpMediaStorageType;


typedef void (*UpdateDatabaseCbFunc)(void *pUserData, float fTimer, int UpdateStyle);

void mp_util_svc_init_session(void);
void mp_util_svc_finish_session(void);

////////////////////////////////////////////////////////
// Manage video item list

// For gallery list
bool mp_util_svc_extract_video_list_from_folder(char *szFolderPath, int nSortType, int nItemType);    // For myfiles.
bool mp_util_svc_extract_video_list_by_item_type(int nSortType, int nItemType);		                // For video list by item type. (All/Phone/Cloud)
bool mp_util_svc_extract_video_list_from_favorite(int nSortType);                                     // For favorite naming of gallery.
bool mp_util_svc_extract_video_list_for_search_view(const char *keyword, int nItemType);               // For only video type files of video search view
bool mp_util_svc_index_list_for_search_view(const char *keyword, void **index_list);             // For only video type files of video search view
bool mp_util_svc_index_list_for_cloud_view(void **index_list, int *nListSize);                   // get cloud data from media list

void mp_util_svc_destory_video_list(void);
bool mp_util_svc_destory_video_item_list(void);
bool mp_util_svc_remove_item_by_url_with_list(char *szFilePath);
bool mp_util_svc_remove_item_by_list_index(int nVideoItemIndex);

bool mp_util_svc_get_video_id_by_video_url(const char *szPath, char **szVideoID);
char* mp_util_svc_get_video_Thumbnail_by_video_url(const char *szMediaUri);
int mp_util_svc_get_video_width_by_video_url(const char *szMediaUri);
int mp_util_svc_get_video_height_by_video_url(const char *szMediaUri);
unsigned int mp_util_svc_get_video_last_play_time_by_video_url(const char *szPath);
unsigned int mp_util_svc_get_video_duration_by_video_url(const char *szPath);
int mp_util_svc_get_video_file_size_by_video_url(const char *szPath);
void mp_util_svc_set_video_last_played_time_by_url(char* szMediaUri, unsigned int nPosition);
bool mp_util_svc_get_video_current_url_pre_next_item(char* szCurrMediaUri, char* szPreMediaUri, char* szNextMediaUri,bool bLoop);

// For video list
char* mp_util_svc_get_video_url(const int nVideoItemIndex);
char* mp_util_svc_get_video_thumbnail(const int nVideoItemIndex);
char* mp_util_svc_get_video_title(const int nVideoItemIndex);
int mp_util_svc_get_video_duration_time(const int nVideoItemIndex);
int mp_util_svc_get_video_last_played_pos(const int nVideoItemIndex);
unsigned long long mp_util_svc_get_video_item_size(const int nVideoItemIndex);

int mp_util_svc_get_video_item_width(const int nVideoItemIndex);
int mp_util_svc_get_video_item_height(const int nVideoItemIndex);
time_t mp_util_svc_get_video_item_data_time(const int nVideoItemIndex);
char *mp_util_svc_get_video_item_extension(const int nVideoItemIndex);
double mp_util_svc_get_video_item_longitude(const int nVideoItemIndex);
double mp_util_svc_get_video_item_latitude(const int nVideoItemIndex);
char *mp_util_svc_get_video_item_modified_time(const int nVideoItemIndex);

//void MpUtilMediaSvcUpdateVideoItem(const int nVideoItemIndex);
int mp_util_svc_get_video_list_size_for_checking_index(void);
int mp_util_svc_get_number_of_video_item_by_type(void);
int mp_util_svc_get_total_video_type_files(void);
void mp_util_svc_print_list_info(void);
int mp_util_svc_find_video_item_index_by_url(const char *szPath);
char *mp_util_svc_get_video_id_by_index(const int nVideoItemIndex);
MpMediaStorageType mp_util_svc_get_video_storage(const int nVideoItemIndex);
bool mp_util_svc_is_cloud_storage(const int nVideoItemIndex);
int mp_util_svc_get_cloud_total_from_video_list(void);
bool mp_util_svc_remove_item_from_db_by_list_index(int nVideoItemIndex);

// For folder list
bool mp_util_svc_extract_video_folder_list_by_type(int nSortType, int nItemType);

bool mp_util_svc_destory_video_folder_list(void);
int mp_util_svc_get_video_folder_size(void);
char* mp_util_svc_get_video_folder_name(int nVideoFolderIndex);
char* mp_util_svc_get_video_folder_url(int nVideoFolderIndex);
MpMediaStorageType mp_util_svc_get_folder_storage(const int nVideoFolderIndex);
char* mp_util_svc_get_video_folder_id(int nVideoFolderIndex);
char* mp_util_get_folder_thumbnail(int nVideoFolderIndex, int nSortType);

void mp_util_svc_register_thumbnail_update_func(int nVideoItemIndex, void* pUserCallbackFunc, void* pUserData);
void mp_util_svc_cancel_thumbnail_update(int nVideoItemIndex);
void mp_util_svc_update_thumbnail_info(const int nVideoItemIndex);
bool mp_util_svc_remove_folder_list_item_from_list(int nVideoItemIndex);


///////////////////////////////////////////////////////
// For livebox list
bool mp_util_svc_extract_last_played_item(void);
bool mp_util_svc_destory_last_played_item_list(void);
char* mp_util_svc_get_last_played_video_item_thumbnail(const int nVideoItemIndex);


////////////////////////////////////////////////////////
// Util
bool mp_util_svc_insert_file_to_media_content_db(const char* szFilePath);
bool mp_util_svc_set_update_db_cb_func(UpdateDatabaseCbFunc pUpdateDbCbFunc);

void mp_util_svc_unset_update_db_cb_func(void);

char* mp_util_svc_get_duration_str_time(int nVideoItemIndex);
bool mp_util_svc_filter_drm_video_item_list(void);
bool mp_util_svc_check_no_drm_video(void);
bool mp_util_svc_scan_file(const char *szFilePath);

/*need to free szTitle, output nWidth, nHeight, szTitle, nDuration,  nPlayedTime*/
bool mp_util_svc_get_video_detail_by_video_url(const char *szPath,
	int *nWidth, int *nHeight, char **szTitle, unsigned int *nDuration, unsigned int *nPlayedTime);

//personal
bool mp_util_svc_extract_personal_video_folder_list(int nSortType, bool bPersonal);
bool mp_util_svc_extract_video_list_by_personal(int nSortType, int nItemType, int nViewType);
bool mp_util_svc_extract_video_list_from_folder_by_personal(char *szFolderPath, int nSortType, int nItemType, int nViewType);
bool mp_util_svc_check_valid_personal(int nViewType);
void* mp_util_svc_get_video_item(const int nVideoItemIndex);
int mp_util_svc_move_from_db(const char *path);
void mp_util_svc_move_to_db(const int nVideoItemIndex, const char *szVideoPath);
bool mp_util_svc_remove_list_item_from_list(int nVideoItemIndex);
bool mp_util_svc_is_device_storage(const int nVideoItemIndex);
bool mp_util_svc_is_personal_data(const int nVideoItemIndex);

//sort by
void mp_util_svc_ctrl_sort_list(int SortType);
bool mp_util_svc_get_frame_by_position(const char *szFilePath, const char *szDestImage, int nPosition, int nWidth, int nHeight);

int mp_util_svc_get_wall_video_index(void *pInvalidList);

//used for check videoid and folderid
bool mp_util_svc_check_valid_media_id(char *pMediaId, int style, int *nMediaIndex);

bool mp_util_folder_is_cloud_storage(const int nFolderIndex);

#endif // _MP_UTIL_DATABASE_

