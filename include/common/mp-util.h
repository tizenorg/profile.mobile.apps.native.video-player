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



#ifndef _MP_UTIL_
#define _MP_UTIL_

#include <stdbool.h>
#include <Ecore_Evas.h>

#include "mp-video-list-view-as-ctrl.h"
#include <efl_extension.h>

typedef void (*MpUtilExitCbFunc)(void);
typedef void (*mpColorThemeChangedCb)(void *pUserData);

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MP_FREE_STRING(str)		do { if(str){free(str); str = NULL;} } while(0)
#define MP_DEL_OBJ(obj) 		do { if(obj){evas_object_del(obj); obj = NULL;} } while(0)
#define MP_DEL_TIMER(timer) 	do { if(timer){ecore_timer_del(timer); timer = NULL;} } while(0)
#define MP_DEL_IDLER(idler) 	do { if(idler){ecore_idler_del(idler); idler = NULL;} } while(0)
#define MP_DEL_PIPE(pipe) 		do { if(pipe){ecore_pipe_del(pipe); pipe = NULL;} } while(0)
#define MP_DEL_ITC(Itc) 		do { if(Itc){elm_genlist_item_class_free(Itc); Itc = NULL;} } while(0)
#define MP_DEL_GRID_ITC(Itc)	do { if(Itc){elm_gengrid_item_class_free(Itc); Itc = NULL;} } while(0)

#define MP_SCALE			elm_config_scale_get()


typedef enum
{
	NORMAL_POPUP_RETURN_TYPE_UNKNOW		= 0,
	NORMAL_POPUP_RETURN_TYPE_YES,
	NORMAL_POPUP_RETURN_TYPE_NO,
	NORMAL_POPUP_RETURN_TYPE_ERR,
}NormalPopUpRetType;


typedef enum
{
	VIDEO_ROTATE_UNKNOWN				= 0,
	VIDEO_ROTATE_PORTRAIT_NORMAL,
	VIDEO_ROTATE_PORTRAIT_REVERSE,
	VIDEO_ROTATE_LANDSCAPE_NORMAL,
	VIDEO_ROTATE_LANDSCAPE_REVERSE,
}MpVideoRotateType;


typedef enum
{
	POPUP_PROCESSING_NO_BUTTON	= 0,
	POPUP_PROCESSING_ONE_BUTTON,

}MpLoadingPopUpType;


/////////////////////////////////////////
// tap type
int mp_util_get_main_tab_type();
void mp_util_set_main_tab_type( int nType );

/////////////////////////////////////////
// Windows utility
void mp_util_reset_for_relauching(void);
void mp_util_set_main_window_handle(void *pWindowHandle);
void* mp_util_get_main_window_handle(void);

void mp_util_set_main_conformat(void *pConformant);
void* mp_util_get_main_conformat(void);

void mp_util_create_main_theme(void);
void mp_util_set_color_theme_changed_cb(mpColorThemeChangedCb fChanged);
void mp_util_free_main_theme(void);

void mp_util_show_indicator(void);
void mp_util_hide_indicator(void);

/////////////////////////////////////////
// Popup utility
void mp_util_delete_popup_handle(void);
bool mp_util_notify_popup(void *pUserData, char *szTitle, char *szContents, void *PopupButtonCallbackFunc);
bool mp_util_yes_no_popup(void *pUserData, char *szTitle, char *szContents, void *PopupYesButtonCallbackFunc, void *PopupNoButtonCallbackFunc);
bool mp_util_remove_all_videos_question_popup(void *pUserData, MpListViewAsType viewType, void *PopupDeleteButtonCallbackFunc, void *PopupCancelButtonCallbackFunc, void *PopupHWMouseBackUpCbFunc, void *PopupHWKeyBackCbFunc);
bool mp_util_remove_question_popup(int pUserData, MpListViewAsType viewType, void *PopupDeleteButtonCallbackFunc, void *PopupCancelButtonCallbackFunc, void *PopupHWMouseBackUpCbFunc, void *PopupHWKeyBackCbFunc);
bool mp_util_remove_folder_question_popup(void *pUserData, void *PopupDeleteButtonCallbackFunc, void *PopupCancelButtonCallbackFunc, void *PopupHWMouseBackUpCbFunc, void *PopupHWKeyBackCbFunc);
bool mp_util_download_question_popup(void *pUserData, char *szTitle, char *szContents, void *PopupCancelBtnCallbackFunc, void *PopupDownloadBtnCallbackFunc);
bool mp_util_progress_popup(char* szLabel);
void mp_util_status_bar_noti_popup_show(const char *szStr);
void* mp_util_create_button_icon(void *pParent, const char *szIconPath, bool bSizeUp, bool bSizeDown, int nAspectWidth, int nAspectHeight, void *pUserData, char* szButtonStyle, void *func);
void* mp_util_create_style_button(void *pParent, const char* szButtonText, const void *pUserData, char* szButtonStyle, void *func);
bool mp_util_ticker_toast_popup(const char *text, bool info_style, bool bottom);
bool mp_util_ticker_toast_with_timeout_cb_popup(const char *text, Evas_Smart_Cb timeout_func);

void* mp_util_create_title_icon_btn(void *pParent, const char*icon_path, Evas_Smart_Cb pFunc, void *pUserData);
bool mp_util_loading_popup(MpLoadingPopUpType style ,char *szLabel, Evas_Smart_Cb response_cb, void *pUserData);
bool mp_util_info_popup(char *szLabel, Evas_Smart_Cb response_cb, Evas_Event_Cb focus_cb, void *pUserData, const char *szDomain);
bool mp_util_info_second_popup(Evas_Object *pParent, char *szLabel, Evas_Smart_Cb response_cb, Evas_Event_Cb focus_cb, void *pUserData, const char *szDomain);
void* mp_util_get_current_popup_object();
void mp_util_text_and_check_popup(Evas_Smart_Cb response_cb, void *cancel_cb, void *ok_cb, void *check_changed_cb, char *szLabel,void *pUserData);
bool mp_util_loading_left_text_popup(MpLoadingPopUpType style ,char *szLabel, Evas_Smart_Cb response_cb, void *pUserData);
bool mp_util_check_video_file(const char *szFullPath);
bool mp_util_check_empty_dir(const char *szDir);

bool mp_util_call_off(void);

bool mp_util_check_local_file_exist(char *uri);
char* mp_util_get_file_size(unsigned long long size);
bool mp_util_get_file_title_from_file_path(char* szFilepath, char* szTitle, int nLengthTitle);
char *mp_util_get_file_name_from_file_path(const char *szFilePath);
bool mp_util_get_icon_path_from_thumbnail(const char *szMediaURL, char **szIconURL);
char *mp_util_get_display_name_from_path_and_title(const char *szFilePath, const char *szTitle);
char *mp_util_get_title_from_path(char *szFilePath);

int mp_util_rename_the_same_file(const char *old_path, char **szNewestPath);

void mp_util_convert_time(int nVideoTime, char* szRetTime, bool bIsHideHour);
int mp_util_get_pid(void);

const char* mp_util_str_str_same_case(const char* str, const char* subStr);
bool mp_util_get_network_result();

char *mp_util_get_dir_by_path(const char *file_path);
char *mp_util_get_folder_icon_path(int nVideoFolderIndex);

bool mp_util_get_subtitle_path(const char *szMediaURL, char **szSubtitle);
bool mp_util_get_file_size_by_path(const char *szMediaURL, unsigned long long *nSize);

bool mp_util_get_image_size(Evas_Object *pParent, const char *szMediaURL, int *nWidth, int *nHeight);

//personal page
bool mp_util_get_personal_status(void);
bool mp_util_check_video_personal_status(const char* szPreviewUrl);
int mp_util_rename_file(const char *old_path, const char *new_path, char **szNewestPath);
int mp_util_set_dest_file(const char *pDestPath);


//drm
bool mp_util_check_drm_lock(const char *szMediaURL, bool *bLock);
void mp_util_convert_sec_time(int nVideoTime, char* szRetTime, bool bIsHideHour);

Evas_Object* mp_util_create_preload_image(Evas_Object *parent, const char *path, int nIconWidth);
bool mp_util_save_preview_image(const char *szFilePath);

char *mp_util_get_preview_image_path(const char *szFilePath);

// progresss loading object
Evas_Object* mp_util_create_embedded_progress();
void mp_util_destroy_embedded_progress( Evas_Object *pProgress );

Evas_Object* mp_util_create_wall_preload_image(Evas_Object *parent,  int nIconWidth);

bool mp_util_meta_get_width_and_height(const char *szFilePath, int *nWidth, int *nHeight);

bool mp_util_delete_wall_file(char *szVideoId);

Evas_Object* mp_util_create_gengrid(Evas_Object *parent, int nWidth, int nHeight, bool bHorizontal);

Evas_Object *mp_util_create_folder_icon(void *pParent, const char*icon_path);
bool mp_util_create_folder_sub_icon(void *pLayout, int nVideoFolderIndex);
bool mp_util_create_video_sub_icon(void *pLayout, int nVideoItemIndex);
unsigned long long mp_util_get_disk_available_space(const char *szFilePath);

void mp_util_virtual_keypad_disabled_set(Eina_Bool bDisabled);

int mp_util_set_translate_str(Evas_Object *obj, const char *str);

#endif // _MP_UTIL_

