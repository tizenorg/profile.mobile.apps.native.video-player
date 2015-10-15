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

typedef	void *vp_thumb_gen_h;

typedef void(* vp_thumb_gen_progress_cb)(bool bSucess, int nIdx, char *szFileName, int nPosition, void *pUserData);
typedef void(* vp_thumb_gen_complete_cb)(int nItemCount, void *pUserData);
typedef void(* vp_thumb_gen_cancle_cb)(void *pUserData);


#ifdef _cplusplus
extern "C"
{
#endif

vp_thumb_gen_h vp_thumb_gen_create(const char *szMediaURL);
void vp_thumb_gen_destroy(vp_thumb_gen_h hThumbGen);

bool vp_thumb_gen_realize(vp_thumb_gen_h hThumbGen);
bool vp_thumb_gen_unrealize(vp_thumb_gen_h hThumbGen);

bool vp_thumb_gen_is_realize(vp_thumb_gen_h hThumbGen, bool *bIsRealize);

bool vp_thumb_gen_set_dest_size(vp_thumb_gen_h hThumbGen, int nWidth, int nHeight);
bool vp_thumb_gen_set_save_directory(vp_thumb_gen_h hThumbGen, char *szSaveDir);
bool vp_thumb_gen_set_start_position(vp_thumb_gen_h hThumbGen, int nPosition);
bool vp_thumb_gen_set_end_position(vp_thumb_gen_h hThumbGen, int nPosition);
bool vp_thumb_gen_set_start_index(vp_thumb_gen_h hThumbGen, int nIdx);
bool vp_thumb_gen_set_count(vp_thumb_gen_h hThumbGen, int nCount);
bool vp_thumb_gen_set_acuurate(vp_thumb_gen_h hThumbGen, bool bAcuurate);
bool vp_thumb_gen_set_scale(vp_thumb_gen_h hThumbGen, bool bSacle);


bool vp_thumb_gen_start(vp_thumb_gen_h hThumbGen);
bool vp_thumb_gen_cancle(vp_thumb_gen_h hThumbGen);


bool vp_thumb_gen_set_user_data(vp_thumb_gen_h hThumbGen, void *pUserData);
bool vp_thumb_gen_set_progress_cb(vp_thumb_gen_h hThumbGen, vp_thumb_gen_progress_cb progress_cb);
bool vp_thumb_gen_set_complete_cb(vp_thumb_gen_h hThumbGen, vp_thumb_gen_complete_cb complete_cb);
bool vp_thumb_gen_set_cancle_cb(vp_thumb_gen_h hThumbGen, vp_thumb_gen_cancle_cb cancle_cb);


#ifdef _cplusplus
}
#endif


