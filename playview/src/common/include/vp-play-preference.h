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
#include <Elementary.h>

void vp_play_preference_init();

bool vp_play_preference_set_multi_play_status(bool bMultiPlay);
bool vp_play_preference_get_multi_play_status(bool *bMultiPlay);
bool vp_play_preference_set_mirroring_warning_status(bool bMultiPlay);
bool vp_play_preference_get_mirroring_warning_status(bool *bWarning);
bool vp_play_preference_get_subtitle_show_key(bool *bShow);
bool vp_play_preference_set_subtitle_show_key(bool bShow);
bool vp_play_preference_set_subtitle_font_name_key(const char *szFontName);
bool vp_play_preference_get_subtitle_font_name_key(char **szFontName);
bool vp_play_preference_get_subtitle_size_key(int *nSize);
bool vp_play_preference_set_subtitle_size_key(int nSize);


bool vp_play_preference_get_subtitle_edge_key(int *nEdge);
bool vp_play_preference_set_subtitle_edge_key(int nEdge);
bool vp_play_preference_set_subtitle_alignment_key(int nAlignment);
bool vp_play_preference_get_subtitle_alignment_key(int *nAlignment);
bool vp_play_preference_set_subtitle_font_color_key(int nColor);
bool vp_play_preference_get_subtitle_font_color_key(int *nColor);
bool vp_play_preference_set_subtitle_font_color_hex_key(const char *szColorHex);
bool vp_play_preference_get_subtitle_font_color_hex_key(char **szColorHex);
bool vp_play_preference_set_subtitle_bg_color_hex_key(const char *szColorHex);
bool vp_play_preference_get_subtitle_bg_color_hex_key(char **szColorHex);
bool vp_play_preference_set_subtitle_caption_win_color_hex_key(const char *szColorHex);
bool vp_play_preference_get_subtitle_caption_win_color_hex_key(char **szColorHex);
bool vp_play_preference_get_subtitle_bg_color_key(int *nColor);
bool vp_play_preference_set_subtitle_bg_color_key(int nColor);
bool vp_play_preference_get_repeat_mode_key(int *nMode);
bool vp_play_preference_set_repeat_mode_key(int nMode);
bool vp_play_preference_get_screen_mode_key(int *nMode);
bool vp_play_preference_set_screen_mode_key(int nMode);
bool vp_play_preference_get_capture_on_key(bool *bEnable);
bool vp_play_preference_set_capture_on_key(bool bEnable);

bool vp_play_preference_get_tag_active_key(bool *bEnable);
bool vp_play_preference_set_tag_active_key(bool bEnable);

bool vp_play_preference_get_tag_weather_key(bool *bEnable);
bool vp_play_preference_set_tag_weather_key(bool bEnable);
bool vp_play_preference_get_tag_location_key(bool *bEnable);
bool vp_play_preference_set_tag_location_key(bool bEnable);
bool vp_play_preference_get_tag_edit_weather_key(int *nWeather);
bool vp_play_preference_set_tag_edit_weather_key(int nWeather);


bool vp_play_preference_get_sort_type_key(int *nType);
bool vp_play_preference_set_preview_url_videos(const char *szMediaURL);
bool vp_play_preference_get_preview_url_videos(char **szMediaURL);
bool vp_play_preference_set_preview_audio_track(int nAudioTrack);
bool vp_play_preference_get_preview_audio_track(int *nAudioTrack);
bool vp_play_preference_get_sensor_asked_state(bool *bAsked);
bool vp_play_preference_set_sensor_asked_state(bool bAsked);
bool vp_play_preference_get_easy_mode(bool *bEnable);
bool vp_play_preference_get_allow_dock_connect_key(bool *bEnable);
bool vp_play_preference_set_allow_dock_connect_key(bool bEnable);
bool vp_play_preference_get_extern_mode_key(bool *bEnable);
bool vp_play_preference_set_extern_mode_key(bool bEnable);
bool vp_play_preference_get_sound_alive_status(int *nStatus);
bool vp_play_preference_set_sound_alive_status(int nStatus);

