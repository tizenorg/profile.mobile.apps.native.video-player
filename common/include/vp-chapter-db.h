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

typedef	void *vp_chapter_db_h;


typedef struct _ChapterDBItem {
	int	nIdx;
	char	*szChapterPath;
	int	nFileSize;
}ChapterDBItem;

#ifdef _cplusplus
extern "C"
{
#endif


vp_chapter_db_h vp_chapter_db_create();
void vp_chapter_db_destroy(vp_chapter_db_h hChapterDB);
bool vp_chapter_db_set_media_id(vp_chapter_db_h hChapterDB, const char *szMediaID);
bool vp_chapter_db_set_media_url(vp_chapter_db_h hChapterDB, const char *szMediaURL);
bool vp_chapter_db_get_items(vp_chapter_db_h hChapterDB, GList **pChapterList);
bool vp_chapter_db_get_all_items_media_id(vp_chapter_db_h hChapterDB, GList **pChapterList);
bool vp_chapter_db_insert(vp_chapter_db_h hChapterDB, const char *szChapterPath, int nIdx);
bool vp_chapter_db_delete(vp_chapter_db_h hChapterDB);
bool vp_chapter_db_item_count(vp_chapter_db_h hChapterDB, int *nCount);
bool vp_chapter_db_item_free(ChapterDBItem *pItem);


#ifdef _cplusplus
}
#endif

