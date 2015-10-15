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

typedef	void *vp_thumb_db_h;


typedef struct _ThumbDBItem {
	int	nIdx;
	char	*szThumbPath;
	int	nFileSize;
}ThumbDBItem;

#ifdef _cplusplus
extern "C"
{
#endif


vp_thumb_db_h vp_thumb_db_create();
void vp_thumb_db_destroy(vp_thumb_db_h hThumbDB);
bool vp_thumb_db_set_media_id(vp_thumb_db_h hThumbDB, const char *szMediaID);
bool vp_thumb_db_set_media_url(vp_thumb_db_h hThumbDB, const char *szMediaURL);
bool vp_thumb_db_get_items(vp_thumb_db_h hThumbDB, GList **pThumbList);
bool vp_thumb_db_get_all_items_media_id(vp_thumb_db_h hThumbDB, GList **pThumbList);
bool vp_thumb_db_insert(vp_thumb_db_h hThumbDB, const char *szThumbPath, int nIdx);
bool vp_thumb_db_delete(vp_thumb_db_h hThumbDB);
bool vp_thumb_db_item_count(vp_thumb_db_h hThumbDB, int *nCount);
bool vp_thumb_db_item_free(ThumbDBItem *pItem);


#ifdef _cplusplus
}
#endif



