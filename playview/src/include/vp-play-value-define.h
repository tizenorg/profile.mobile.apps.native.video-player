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

/* SYSTEM KEY VALUE */
#define VP_PAUSE_KEY					"XF86AudioPause"
#define VP_PLAY_KEY					"XF86AudioPlay"
#define VP_STOP_KEY					"XF86AudioStop"
#define VP_NEXT_KEY					"XF86AudioNext"
#define VP_PREV_KEY					"XF86AudioPrev"
#define VP_FORWARD_KEY					"XF86AudioForward"
#define VP_REWIND_KEY					"XF86AudioRewind"
#define VP_END_KEY					"XF86Stop"
#define VP_HOME_KEY					"XF86Home"
#define VP_KEY_CANCLE					"Cancel"
#define VP_POWER_KEY					"XF86PowerOff"
#define VP_MENU_KEY					"XF86Menu"
#define VP_BACK_KEY					"XF86Back"
#define VP_MEDIA_KEY					"XF86AudioMedia"
#define VP_SCREENSAVER_KEY				"XF86ScreenSaver"

#define VP_PLAYPAUSE_KEY				"XF86AudioPlayPause"
#define VP_PLAY_NEXT_SONG_KEY				"XF86AudioNext"
#define VP_PLAY_PREV_SONG_KEY				"XF86AudioPrev"

#define VP_VOLUME_DOWN_KEY				"XF86AudioLowerVolume"
#define VP_VOLUME_UP_KEY				"XF86AudioRaiseVolume"
#define VP_VOLUME_MUTE					"XF86AudioMute"

#define VP_KEY_GRAB_SHARED				SHARED_GRAB
#define VP_KEY_GRAB_EXCLUSIVE				EXCLUSIVE_GRAB
#define VP_KEY_GRAB_TOP_POSITION			TOP_POSITION_GRAB
#define VP_KEY_GRAB_OR_EXCLUSIVE			OR_EXCLUSIVE_GRAB

#define VP_URL_LEN_MAX					4096
#define VP_STRING_LEN_MAX				4096

#define VP_BOOKMARK_ITEM_MAX_COUNT			15

#define VP_THUMB_DEFAULT_WIDTH				176
#define VP_THUMB_DEFAULT_HEIGHT				144

#define VP_POPUP_MIN_ITEMS					4
#define VP_POPUP_LIST_LOADING_HEIGHT		76
#define VP_POPUP_LIST_CLOSED_CAPTION_HEIGHT	72
#define VP_POPUP_TITLE_HEIGHT			72
#define VP_POPUP_BUTTON_HEIGHT			72

#define VP_POPUP_MAX_HEIGHT				290
#define VP_POPUP_NO_TITLE_MAX_HEIGHT			(VP_POPUP_MAX_HEIGHT-VP_POPUP_TITLE_HEIGHT)
#define VP_POPUP_HEIGHT(height) 			(((height) > VP_POPUP_NO_TITLE_MAX_HEIGHT) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT) : (height))
#define VP_POPUP_LIST_HEIGHT(count) 			45*count
#define VP_POPUP_LIST_2_TEXT_HEIGHT(count) 		(((140*count) > VP_POPUP_NO_TITLE_MAX_HEIGHT) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT) : (140*count))
#define VP_POPUP_LIST_2_TEXT_1_ICON_HEIGHT(count) 	(((135*count-1) > VP_POPUP_NO_TITLE_MAX_HEIGHT) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT) : (135*count-1))
#define VP_POPUP_LOADING_LIST_HEIGHT(count) 		(((140*count+VP_POPUP_LIST_LOADING_HEIGHT) > 840) ? 840/*common box height video,gallery,image viewer (1080+VP_POPUP_LIST_LOADING_HEIGHT)*/ : (140*count+VP_POPUP_LIST_LOADING_HEIGHT))
#define VP_POPUP_SUBTITLE_2_TEXT_HEIGHT(count) 		(((135*count-1) > VP_POPUP_NO_TITLE_MAX_HEIGHT) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT) : (135*count-1))
#define VP_POPUP_LIST_2_TEXT_HEIGHT_DETAIL(count) 		(((134*count-1) > VP_POPUP_NO_TITLE_MAX_HEIGHT) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT) : (135*count-1))

#define VP_POPUP_MAX_HEIGHT_L				255
#define VP_POPUP_NO_TITLE_MAX_HEIGHT_L			(VP_POPUP_MAX_HEIGHT_L-VP_POPUP_TITLE_HEIGHT)
#define VP_POPUP_HEIGHT_L(height) 			(((height) > VP_POPUP_NO_TITLE_MAX_HEIGHT_L) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT_L) : (height))
#define VP_POPUP_LIST_HEIGHT_L(count) 			45*count
#define VP_POPUP_LIST_2_TEXT_HEIGHT_L(count) 		(((140*count) > VP_POPUP_NO_TITLE_MAX_HEIGHT_L) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT_L) : (140*count))
#define VP_POPUP_LIST_2_TEXT_1_ICON_HEIGHT_L(count) 	(((135*count-1) > VP_POPUP_NO_TITLE_MAX_HEIGHT_L) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT_L) : (135*count-1))
#define VP_POPUP_LOADING_LIST_HEIGHT_L(count) 		(((140*count+VP_POPUP_LIST_LOADING_HEIGHT) > 468) ? 468/*common box height video,gallery,image viewer (520+VP_POPUP_LIST_LOADING_HEIGHT)*/ : (140*count+VP_POPUP_LIST_LOADING_HEIGHT))
#define VP_POPUP_SUBTITLE_2_TEXT_HEIGHT_L(count) 		(((135*count-1) > VP_POPUP_NO_TITLE_MAX_HEIGHT_L) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT_L) : (135*count-1))
#define VP_POPUP_LIST_2_TEXT_HEIGHT_DETAIL_L(count) 		(((134*count-1) > VP_POPUP_NO_TITLE_MAX_HEIGHT_L) ? (VP_POPUP_NO_TITLE_MAX_HEIGHT_L) : (134*count-1))


#define VP_POPUP_LIST_WIDTH				200


#define PROGRESSBAR_TIME_LENGTH 	9
#define PROGRESSBAR_TIME_FORMAT 	"02u:%02u:%02u"

#define PROGRESSBAR_TIME_ARGS(t) \
        (t) / (60 * 60), \
        ((t) / 60) % 60, \
        (t) % 60


#define VP_ACCUMULATED_DATE		86400
#define VP_RO_MESSAGE_LEN		1024
#define VP_MOUNTH_COUNT			12
#define VP_DAY_COUNT			30
#define VP_HOUR_COUNT			24

#define VP_SUBTITLE_SIZE_SMALL_VALUE	24
#define VP_SUBTITLE_SIZE_MEDIUM_VALUE	32
#define VP_SUBTITLE_SIZE_LARGE_VALUE	40

#define BYTE_SIZE 			1024
#define VP_MB(x)   			((x) << 20)

#define VP_FILE_ENTRY_LEN_MAX		255
#define VP_FILE_ENTRY_CHAR_LEN_MAX	50
#define VP_FILE_TRIM_DURATION_MIN	1000

#define VP_PORTRAIT_H			1280
#define VP_MULTI_WIDGET_W		116
#define VP_MULTI_WIDGET_H		572
#define VP_MULTI_WIDGET_PADDING_W	10
#define VP_MULTI_WIDGET_START_H		(-275)
#define VP_MULTI_WIDGET_END_H		440
#define VP_MULTI_WIDGET_SCALE		0.5
#define VP_MULTI_VOLUM_W		116
#define VP_MULTI_VOLUM_H		635

#define VP_DETAIL_VALUE_GPS_DEFAULT	-200.0

/* Kiran Share operation */
#define VP_SHARE_OPERATION_SINGLE "http://tizen.org/appcontrol/operation/share"
#define VP_SHARE_OPERATION_MULTIPLE "http://tizen.org/appcontrol/operation/multi_share"
#define VP_SHARE_SVC_FILE_PATH "http://tizen.org/appcontrol/data/path"
