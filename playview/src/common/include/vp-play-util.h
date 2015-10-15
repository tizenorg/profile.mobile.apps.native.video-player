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

#include <glib.h>

#include <stdbool.h>
#include <Elementary.h>

char *vp_play_util_get_title_from_path(char *szFilePath);
char *vp_play_util_get_filename_from_path(char *szFilePath);
char *vp_play_util_get_folder_from_path(char *szFilePath);
bool vp_play_util_key_grab(Elm_Win *pWin, const char *szKeyName);
bool vp_play_util_key_ungrab(Elm_Win *pWin, const char *szKeyName);
void vp_play_util_fb_visible_off(Evas_Object *pWin, bool bVisible);
bool vp_play_util_get_landscape_check(int nRotateVal);
//bool vp_play_util_add_to_home(const char *szMediaId, const char *szThumbnailURL);
bool vp_play_util_local_file_check(char *szMediaURL);
bool vp_play_util_get_subtitle_path(const char *szMediaURL, char **szSubtitle);
double vp_play_util_get_sys_time(void);
bool vp_play_util_status_noti_show(const char *szStr);
char *vp_play_util_remove_prefix_to_url(char *szURL);
void vp_play_util_convert_file_size(long long lSize, char **szSize);
void vp_play_util_get_convert_time(double dtime, char **szFileDate);
bool vp_play_util_get_file_info(const char *szMediaURL, char **szFileSize, char **szFileDate, char **szFileExtention);
bool vp_play_util_check_streaming(const char *szURL);
bool vp_play_util_rtsp_url_check(const char *szURL);
bool vp_play_util_calculator_position(Evas_Coord_Rectangle rtSrc, Evas_Coord_Rectangle rtDst, Evas_Coord_Rectangle *rtResult, int nType);
bool vp_play_util_check_sdp_url(const char *szUri);
char *vp_play_util_get_sdp_url(const char *szUri);
bool vp_play_util_save_file(const char *szFileName, char *szBuffer, int nSize);
bool vp_play_util_get_network_status(void);
bool vp_play_util_is_exist_subtitle_from_path(const char *szPath);
bool vp_play_util_get_subtitles_from_path(const char *szPath, GList **pSubtitleList);
bool vp_play_util_get_subtitle_path_list(GList **subtitleList);
bool vp_play_util_check_personal_status(const char* szPreviewUrl);

int vp_play_util_get_root_window_angle(Evas_Object *pWin);

bool vp_play_util_app_resume(void);
bool vp_play_util_check_white_space(char *szText);
bool vp_play_util_check_valid_file_name(const char *szFileName);
unsigned long long vp_play_util_get_disk_available_space(const char *szFilePath);
unsigned long long vp_play_util_get_file_size(const char *szFilePath);

bool vp_play_util_get_city(const char *szLocation, char **szCity);
bool vp_play_util_is_Xwindow_focused(Ecore_X_Window nXwinID);

char *vp_play_util_convert_rgba_to_hex(int r,int g,int b,int a);
bool vp_play_util_convert_hex_to_rgba(char *pColorHex, int *r,int *g,int *b,int *a);
void vp_play_util_set_object_size(Evas_Object *obj, int w, int h, int id);
void vp_play_util_set_object_color(Evas_Object *obj, int r, int g, int b, int a, int id);
void vp_play_util_set_object_offset(Evas_Object *obj, int left, int top, int id);

void vp_play_util_set_lock_power_key();
void vp_play_util_set_unlock_power_key();

void vp_play_util_focus_next_object_set(Evas_Object *obj, Evas_Object *next, Elm_Focus_Direction dir);

