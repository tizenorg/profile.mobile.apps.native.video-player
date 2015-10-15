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



#pragma once

#include <stdbool.h>
#include <glib.h>
#include <Elementary.h>


typedef void (*MediaUpdateCb)(void* pUserData);



bool vp_media_contents_connect();
bool vp_media_contents_disconnect();

/* common functions */
bool vp_media_contents_get_video_id(const char *szFilePath, char **szVideoID);
bool vp_media_contents_get_video_thumbnail_path(const char *szFilePath, char **szThumbnailPath);
bool vp_media_contents_scan_file(const char *szFilePath);
bool vp_media_contents_update_db(const char *szMediaURL);
bool vp_media_contents_refresh_db(const char *szMediaID);

bool vp_media_contents_set_played_position(const char *szFilePath, int nPosition);
bool vp_media_contents_get_played_position(const char *szFilePath, int *nPosition);
bool vp_media_contents_set_played_time(const char *szFilePath);

bool vp_media_contents_get_content_info(const char *szFilePath, char **szTitle, int *nWidth, int *nHeight, int *nDuration, double *dLatitude, double *dLongitude, char **szTmpLastModified);
bool vp_media_metadata_get_gps_info(const char *szFilePath, double *dLatitude, double *dLongitude);
bool vp_media_metadata_get_duration(const char *szFilePath, int *nDuration);
bool vp_media_metadata_get_width(const char *szFilePath, int *nWidth);
bool vp_media_metadata_get_height(const char *szFilePath, int *nHeight);
bool vp_media_contents_get_cloud_attribute(const char *szFilePath, bool *bCloud);

/* bookmark functions */
bool vp_media_contents_bookmark_list_get(const char *szMediaID, GList **list);
bool vp_media_contents_bookmark_list_clear(GList *pList);
bool vp_media_contents_bookmark_insert(const char *szMediaID, int nPosition, const char *szFilePath);
bool vp_media_contents_bookmark_delete(const char *szMediaID, int nPosition);
bool vp_media_contents_bookmark_get_item_info(void *pItem, char **szFilePath, int *nPosition);
bool vp_media_contents_bookmark_delete_all(const char *szMediaID);

bool vp_media_contents_get_video_items_to_folder(int nSortType, char *szFolder, GList **pList);
bool vp_media_contents_get_video_items(int nSortType, GList **pList);
bool vp_media_contents_free_video_items(GList *pList);

bool vp_media_contents_get_next_file_path(const char *szMediaURL, char **szNextURL, bool bLoop, GList *pList);
bool vp_media_contents_get_prev_file_path(const char *szMediaURL, char **szPrevURL, bool bLoop, GList *pList);

bool vp_media_contents_set_update_cb(MediaUpdateCb pFunc, void *pUserData);
void vp_media_contents_unset_update_cb();

/*tag buddy */
bool vp_media_contents_set_location_tag(const char *szFilePath, const char *city);
bool vp_media_contents_get_location_tag(const char *szFilePath, char **szLocationTag);
bool vp_media_contents_set_weahter_tag(const char *szFilePath, const char *weather);
bool vp_media_contents_get_weahter_tag(const char *szFilePath, char **szWeatherTag);

/*subtitle file*/
bool vp_media_contents_subtitle_list_get(GList **list);

bool vp_media_contents_file_is_exist(const char *szFilePath);
bool vp_media_contents_del_video(const char *szFilePath);

