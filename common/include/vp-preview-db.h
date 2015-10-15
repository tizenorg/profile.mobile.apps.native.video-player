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

typedef	void *vp_preview_db_h;


#ifdef _cplusplus
extern "C"
{
#endif

vp_preview_db_h vp_preview_db_create();
void vp_preview_db_destroy(vp_preview_db_h hPreviewDB);
bool vp_preview_db_set_media_id(vp_preview_db_h hPreviewDB, const char *szMediaURL);
bool vp_preview_db_set_media_url(vp_preview_db_h hPreviewDB, const char *szMediaURL);
bool vp_preview_db_get_file_path(vp_preview_db_h hPreviewDB, char **szFilePath);
bool vp_preview_db_get_file_size(vp_preview_db_h hPreviewDB, int *nFileSize);
bool vp_preview_db_item_count(vp_preview_db_h hPreviewDB, int *nCount);
bool vp_preview_db_get_all_items_media_id(vp_preview_db_h hPreviewDB, GList **pPreviewList);
bool vp_preview_db_insert(vp_preview_db_h hPreviewDB, const char *szPreviewPath);
bool vp_preview_db_delete(vp_preview_db_h hPreviewDB);


#ifdef _cplusplus
}
#endif



