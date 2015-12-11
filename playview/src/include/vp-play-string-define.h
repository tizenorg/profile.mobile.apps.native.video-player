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

#include <app.h>

#define VP_PLAY_FILE_PREFIX		"file://"


/* window name */
#define VP_PLAY_MAIN_WINDOW_NAME		"video-player"
#define VP_PLAY_MAIN_MULTI_WINDOW_NAME		"multi_view"

#define VP_LOCAL_STR_PREFIX			"video-player"
#define VP_SYS_STR_PREFIX			"sys_string"

#define VP_PLAY_IMAGE_BUFFER_PATH		"/opt/usr/media/.video_thumb/.image_buff.jpg"

#define VP_PLAY_SCREEN_SHOT_DIR			"/opt/usr/media/Screenshots"

#define VP_PLAY_MEDIA_DIR					"/opt/usr/media/Videos"
#define VP_PLAY_PERSONAL_HEAD_STR			"/opt/storage/PersonalStorage"
#define VP_ROOT_PATH_MMC			"/opt/storage/sdcard"


/* EDJ FILE PATH */
#define VP_PLAY_CUSTOM_THEME			"pv-custom-winset.edj"
#define VP_PLAY_RESROUCE_EDJ_PATH		"pv-resource.edj"
#define VP_PLAY_POPUP_EDJ_PATH			"pv-popup.edj"

#define VP_PLAY_NORMAL_MAIN_EDJ			"pv-normal-main.edj"
#define VP_PLAY_NORMAL_TOP_CONTROL_EDJ		"pv-normal-top-control.edj"
#define VP_PLAY_NORMAL_CONTROL_EDJ		"pv-normal-control.edj"
#define VP_PLAY_NORMAL_FUNCTION_EDJ_PATH	"pv-normal-function.edj"

#define VP_PLAY_BOOKMARK_EDJ_PATH		"pv-bookmark.edj"
#define VP_PLAY_BOOKMARK_ITEM_EDJ_PATH		"pv-bookmark-item.edj"
#define VP_PLAY_PROGRESSBAR_EDJ_PATH		"pv-progressbar.edj"
#define VP_PLAY_VOLUME_EDJ_PATH			"pv-volume.edj"
#define VP_PLAY_VOLUME_POPUP_EDJ_PATH		"pv-volume-popup.edj"
#define VP_PLAY_BRIGHTNESS_POPUP_EDJ_PATH	"pv-brightness-popup.edj"
#define VP_PLAY_ZOOM_GUIDE_EDJ_PATH		"pv-zoom-guide.edj"


#define VP_PLAY_SUBTITLE_EDJ_PATH		"pv-subtitle.edj"

#define VP_PLAY_MULTI_MAIN_EDJ			"pv-multi-main.edj"

#define VP_PLAY_SPEED_POPUP_EDJ			"pv-speed-popup.edj"
#define VP_PLAY_GESTURE_EDJ				"pv-gesture.edj"
#define VP_PLAY_SUBTITLE_COLOR_POPUP_EDJ		"pv-subtitle-color-popup.edj"

/* EDJ GROUP & SWALLOW*/
#define VP_PLAY_CUSTOM_LAYOUT_KLASS		"layout"
#define VP_PLAY_CUSTOM_LAYOUT_GROUP		"application"
#define VP_PLAY_CUSTOM_LAYOUT_STYLE		"play_view"
#define VP_PLAY_CUSTOM_MULTI_LAYOUT_STYLE	"multi_view"

#define VP_PLAY_EDJ_GROUP_GESTURE		"pv.gesture"

#define VP_PLAY_EDJ_GROUP_NORMAL_MAIN		"pv.normal.main"
#define VP_PLAY_EDJ_GROUP_NORMAL_TOP_CONTROL	"pv.normal.top"
#define VP_PLAY_EDJ_GROUP_NORMAL_CONTROL	"pv.normal.control"
#define VP_PLAY_EDJ_GROUP_NORMAL_FUNCTION	"pv.normal.function"


#define VP_PLAY_EDJ_GROUP_MULTI			"pv.multi.main"
#define VP_PLAY_EDJ_GROUP_PROGRESSBAR		"pv.progress"
#define VP_PLAY_EDJ_GROUP_BOOKMARK		"pv.bookmark"
#define VP_PLAY_EDJ_GROUP_BOOKMARK_ITEM		"pv.bookmark.item"
#define VP_PLAY_EDJ_GROUP_BOOKMARK_ITEM_END	"pv.bookmark.item.end"
#define VP_PLAY_EDJ_GROUP_ZOOM_GUIDE		"pv.zoom.guide"
#define VP_PLAY_EDJ_GROUP_ZOOM_GUIDE_ITEM	"pv.zoom.guide.item"

#define VP_PLAY_EDJ_GROUP_TRIM_MAIN		"pv.trim.main"
#define VP_PLAY_EDJ_GROUP_TRIM_TOP_CONTROL	"pv.trim.top"
#define VP_PLAY_EDJ_GROUP_TRIM_CONTROL		"pv.trim.control"
#define VP_PLAY_EDJ_GROUP_VOLUME		"pv.volume"
#define VP_PLAY_EDJ_GROUP_VOLUME_POPUP		"pv.volume.popup"
#define VP_PLAY_EDJ_GROUP_BRIGHTNESS_POPUP	"pv.brightness.popup"

#define VP_PLAY_EDJ_GROUP_PLAY_SPEED_POPUP	"pv.play.speed.popup"

#define VP_PLAY_EDJ_GROUP_PLAY_SUBTITLE_COLOR_POPUP	"pv.subtitle-color-popup"

#define VP_PLAY_EDJ_GROUP_CONNECT_POPUP		"pv.connect.popup"


#define VP_PLAY_EDJ_GROUP_SUBTITLE		"pv.subtitle"
#define VP_PLAY_EDJ_GROUP_SUBTITLE_MULTI		"pv.multi-subtitle"
#define VP_PLAY_EDJ_GROUP_LOADING_ANI		"pv.loading.ani"

#define VP_PLAY_EDJ_GROUP_TRIM_ITEM		"pv.trim.item"

#define VP_PLAY_EDJ_GROUP_CHAPTER_MAIN		"pv.chapter.main"

#define VP_PLAY_EDJ_GROUP_TAG_BUDDY		"pv.tag.buddy"
#define VP_PLAY_EDJ_GROUP_TAG_PREVIEW		"pv.tag.preview"
#define VP_PLAY_EDJ_GROUP_TAG_NARMAL		"pv.tag.buddy.normal"



#define VP_PLAY_EDJ_GROUP_SESNOR_MAIN		"pv.sensor.popup"
#define VP_PLAY_EDJ_GROUP_SESNOR_IMAGE		"pv.sensor.popup.image"

#define VP_PLAY_EDJ_GROUP_TITLE_LIST_POPUP	"pv.title.list"
#define VP_PLAY_EDJ_GROUP_PROGRESS_POPUP	"pv.popup.progress"
#define VP_PLAY_EDJ_GROUP_ENTRY_POPUP		"pv.popup.entry"
#define VP_PLAY_EDJ_GROUP_CAPTION_POPUP		"pv.popup.caption"
#define VP_PLAY_EDJ_GROUP_TITLE_POPUP		"pv.popup.title"

#define VP_PLAY_EDJ_GROUP_MICROSEEK_TEXT		"pv.normal.main.microseek.text"
#define VP_PLAY_EDJ_GROUP_MICROSEEK_IMAGE		"pv.normal.main.microseek.image"
#define VP_PLAY_EDJ_GROUP_FLICKJUMP_SEEK		"pv.normal.main.flickjump.seek"

//#define VP_PLAY_SWALLOW_BUTTON_ICON		"icon"
#define VP_PLAY_SWALLOW_BUTTON_ICON		"elm.swallow.content"
#define VP_PLAY_SWALLOW_BUTTON_PRESS_ICON	"elm.swallow.content.press"
#define VP_PLAY_SWALLOW_BUTTON_DIM_ICON		"elm.swallow.content.dim"


/* normal view swallow */
#define VP_PLAY_SWALLOW_NORMAL_LOCKSCREEN	"pv.normal.main.lockscreen"

#define VP_PLAY_SWALLOW_NORMAL_IMAGE_BUFFER	"pv.normal.main.image.buffer"
#define VP_PLAY_SWALLOW_NORMAL_ERROR_IMAGE	"pv.normal.main.error.image"

#define VP_PLAY_SWALLOW_NORMAL_TOP_CONTROL	"pv.normal.top.control"
#define VP_PLAY_SWALLOW_NORMAL_CONTROL		"pv.normal.main.control"
#define VP_PLAY_SWALLOW_NORMAL_ROTATE	"pv.normal.main.rotate.icon"

#define VP_PLAY_SWALLOW_NORMAL_VOLUME		"pv.normal.main.volume.icon"
#define VP_PLAY_SWALLOW_NORMAL_SCREENSIZE	"pv.normal.control.toolbar.screensize"
#define VP_PLAY_SWALLOW_NORMAL_POPUP_PLAYER     "pv.normal.control.toolbar.popupPlayer"

#define VP_PLAY_SWALLOW_NORMAL_PLAY		"pv.normal.control.play"
#define VP_PLAY_SWALLOW_NORMAL_REW		"pv.normal.control.rew"
#define VP_PLAY_SWALLOW_NORMAL_FF		"pv.normal.control.ff"
#define VP_PLAY_SWALLOW_NORMAL_PLAY_FOCUS		"pv.normal.control.play.focus"


#define VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_PORTRAIT 	"pv.normal.main.screenshot.portrait"
#define VP_PLAY_SWALLOW_NORMAL_SCREENSHOT_LANDSCAPE 	"pv.normal.main.screenshot.landscape"

#define VP_PLAY_SWALLOW_NORMAL_BOOKMARK		"pv.normal.control.bookmark"
#define VP_PLAY_SWALLOW_NORMAL_PROGRESS		"pv.normal.control.progress"
#define VP_PLAY_SWALLOW_NORMAL_FUNCTION		"pv.normal.control.function"

#define VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE	"pv.normal.top.capture"
#define VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE_REW	"pv.normal.top.capture.rew"
#define VP_PLAY_SWALLOW_NORMAL_TOP_CAPTURE_FF	"pv.normal.top.capture.ff"
#define VP_PLAY_SWALLOW_NORMAL_TOP_ROTATE	"pv.normal.top.rotate"

#define VP_PLAY_SWALLOW_NORMAL_ZOOM_GUIDE	"pv.normal.main.zoom.guide.rect"

#define VP_PLAY_SWALLOW_PROGRESS_BG		"pv.progress.bg"
#define VP_PLAY_SWALLOW_PROGRESS_PLAYING_LABEL	"pv.progress.playing.txt"
#define VP_PLAY_SWALLOW_PROGRESS_DURATION_LABEL	"pv.progress.duration.txt"
#define VP_PLAY_SWALLOW_PROGRESS_DRAG_RECT	"pv.progress.playing"
#define VP_PLAY_SWALLOW_PROGRESS_BAR	 	"pv.progress.bar.bg"
#define VP_PLAY_SWALLOW_PROGRESS_POINT		"pv.progress.point"

#define VP_PLAY_SWALLOW_BOOKMARK_ADD_BUTTON	"pv.bookmark.add.button"
#define VP_PLAY_SWALLOW_BOOKMARK_LIST		"pv.bookmark.item.list"

#define VP_PLAY_SWALLOW_BOOKMARK_ITEM_ICON	"pv.bookmark.item.icon"
#define VP_PLAY_SWALLOW_BOOKMARK_ITEM_TXT	"pv.bookmark.item.txt"
#define VP_PLAY_SWALLOW_BOOKMARK_ITEM_DEL	"pv.bookmark.item.del"

#define VP_PLAY_SWALLOW_VOLUME_DRAG_RECT	"pv.volume.playing"
#define VP_PLAY_SWALLOW_VOLUME_VALUE_LABEL	"pv.volume.txt"

#define VP_PLAY_SWALLOW_VOLUME_POPUP_DRAG_RECT		"pv.volume.popup.playing"
#define VP_PLAY_SWALLOW_VOLUME_POPUP_VALUE_LABEL	"pv.volume.popup.txt"

#define VP_PLAY_SWALLOW_BRIGHTNESS_POPUP_DRAG_RECT	"pv.brightness.popup.playing"
#define VP_PLAY_SWALLOW_BRIGHTNESS_POPUP_VALUE_LABEL	"pv.brightness.popup.txt"
#define VP_PLAY_SWALLOW_BRIGHTNESS_POPUP_ICON		"pv.brightness.popup.icon"

#define VP_PLAY_SWALLOW_NORMAL_FUNCTION_1	"pv.normal.function.button.1"
#define VP_PLAY_SWALLOW_NORMAL_FUNCTION_2	"pv.normal.function.button.2"
#define VP_PLAY_SWALLOW_NORMAL_FUNCTION_3	"pv.normal.function.button.3"

#define VP_PLAY_SWALLOW_NORMAL_SUBTITLE		"pv.normal.main.subtitle"
#define VP_PLAY_SWALLOW_NORMAL_LOADING_ANI	"pv.normal.main.loading"


#define VP_PLAY_SWALLOW_NORMAL_SINK		"pv.normal.videosink"

/* zoom guide swallow */

/* multi view swallow */
#define VP_PLAY_SWALLOW_MULTI_IMAGE_BUFFER	"pv.multi.image.buffer"

#define VP_PLAY_SWALLOW_MULTI_SINK		"pv.multi.videosink"

#define VP_PLAY_SWALLOW_MULTI_CLOSE		"pv.multi.close"
#define VP_PLAY_SWALLOW_MULTI_FULL_SIZE		"pv.multi.full.size"
#define VP_PLAY_SWALLOW_MULTI_MINIMIZE		"pv.multi.minimize"
#define VP_PLAY_SWALLOW_MULTI_RESIZE		"pv.multi.resize"
#define VP_PLAY_SWALLOW_MULTI_PLAY_PAUSE	"pv.multi.control.play"
#define VP_PLAY_SWALLOW_MULTI_PLAY_FOCUS	"pv.multi.control.play.focus"
#define VP_PLAY_SWALLOW_MULTI_REW		"pv.multi.control.rew"
#define VP_PLAY_SWALLOW_MULTI_FF		"pv.multi.control.ff"
#define VP_PLAY_SWALLOW_MULTI_LOADING_ANI	"pv.multi.main.loading"
#define VP_PLAY_SWALLOW_MULTI_SUBTITLE		"pv.multi.main.subtitle"


/* trim view swallow */
#define VP_PLAY_SWALLOW_TRIM_TOP_CONTROL	"pv.trim.top.control"
#define VP_PLAY_SWALLOW_TRIM_CONTROL		"pv.trim.main.control"

#define VP_PLAY_SWALLOW_TRIM_ROTATE		"pv.trim.top.rotate"

#define VP_PLAY_SWALLOW_TRIM_SAVE		"pv.trim.control.toolbar.save"
#define VP_PLAY_SWALLOW_TRIM_BACK		"pv.trim.control.toolbar.back"

#define VP_PLAY_SWALLOW_TRIM_PLAY		"pv.trim.control.play"
#define VP_PLAY_SWALLOW_TRIM_REW		"pv.trim.control.rew"
#define VP_PLAY_SWALLOW_TRIM_FF			"pv.trim.control.ff"

#define VP_PLAY_SWALLOW_TRIM_GENGRID		"pv.trim.control.gengrid.rect"
#define VP_PLAY_SWALLOW_TRIM_ITEM		"pv.trim.item.icon"

#define VP_PLAY_SWALLOW_TRIM_DRAG_RECT		"pv.trim.control.playing"
#define VP_PLAY_SWALLOW_TRIM_START_LABEL	"pv.trim.start.txt"
#define VP_PLAY_SWALLOW_TRIM_TOTAL_LABEL	"pv.trim.total.txt"

#define VP_PLAY_SWALLOW_TRIM_LOADING_ANI	"pv.trim.main.loading"
#define VP_PLAY_SWALLOW_TRIM_BUTTON		"pv.trim.main.button"

/* tag view swallow */
#define VP_PLAY_SWALLOW_TAG_PREVIEW		"pv.tag.buddy.preview"
#define VP_PLAY_SWALLOW_TAG_LIST		"pv.tag.buddy.list"

#define VP_PLAY_SWALLOW_TAG_PREVIEW_IMAGE	"pv.tag.preview.image"
#define VP_PLAY_SWALLOW_TAG_PREVIEW_ACCUWEATHER	"pv.tag.preview.accuweather"
#define VP_PLAY_SWALLOW_TAG_PREVIEW_TABLE	"pv.tag.preview.table"
#define VP_PLAY_SWALLOW_TAG_PREVIEW_TIME	"pv.tag.preview.time"

#define VP_PLAY_SWALLOW_TAG_PREVIEW_TIME_SHADOW		"pv.tag.preview.time.shadow"
#define VP_PLAY_SWALLOW_TAG_PREVIEW_TABLE_SHADOW	"pv.tag.preview.table.shadow"

#define VP_PLAY_SWALLOW_TAG_BG				"pv.tag.bg"


/* normal part */
#define VP_PLAY_PART_NORMAL_PLAYING_SPEED_TXT		"pv.normal.main.playingspeed.txt"
#define VP_PLAY_PART_NORMAL_SPEED_TXT		"pv.normal.main.speed.txt"

/* multi part */
#define VP_PLAY_PART_MULTI_TITLE		"pv.multi.title.txt"
#define VP_PLAY_PART_MULTI_NOCONTENTS_TXT	"pv.multi.nocontents.txt"

#define VP_PLAY_PART_MULTI_SPEED_TXT		"pv.multi.main.speed.txt"


/* trim drag part */
#define VP_PLAY_PART_TRIM_DRAG_PLAYING		"pv.trim.control.playing"
#define VP_PLAY_PART_TRIM_DRAG_START		"pv.trim.control.confine.start"
#define VP_PLAY_PART_TRIM_DRAG_END		"pv.trim.control.confine.end"



/* subtitle part */
#define VP_PLAY_PART_SUBTITLE_TEXT			"pv.multi.subtitle.label.txt"
#define VP_PLAY_SWALLOW_SUBTITLE_TEXT		"pv.subtitle.swallow.txt"


/* flick jump seek part */
#define VP_PLAY_PART_FLICKJUMP_SEEK_REW_TEXT		"pv.normal.main.flickjump.seek.rewtext"
#define VP_PLAY_PART_FLICKJUMP_SEEK_FF_TEXT		"pv.normal.main.flickjump.seek.fftext"


/* signal emit strings */
#define VP_PLAY_COMMON_SIGNAL_DRAG			"drag"
#define VP_PLAY_COMMON_SIGNAL_DRAG_START		"drag,start"
#define VP_PLAY_COMMON_SIGNAL_DRAG_STOP			"drag,stop"

#define VP_MULTI_VIEW_SIGNAL_SHOW_CONTROL		"signal.pv.multi.show.control"
#define VP_MULTI_VIEW_SIGNAL_HIDE_CONTROL		"signal.pv.multi.hide.control"

#define VP_MULTI_SIGNAL_MAIN_SPEED_SHOW		"signal.pv.multi.main.speed.show"
#define VP_MULTI_SIGNAL_MAIN_SPEED_HIDE		"signal.pv.multi.main.speed.hide"

#define VP_MULTI_VIEW_SIGNAL_SHOW_NOCONTENTS		"signal.pv.multi.show.nocontents"
#define VP_MULTI_VIEW_SIGNAL_HIDE_NOCONTENTS		"signal.pv.multi.hide.nocontents"

#define VP_MULTI_VIEW_TITLE_FOCUS_IN_CONTROL		"signal.pv.multi.focus.in.control"
#define VP_MULTI_VIEW_TITLE_FOCUS_OUT_CONTROL		"signal.pv.multi.focus.out.control"

#define VP_MULTI_VIEW_TITLE_CLOSE_BTN_PRESSED		"signal.pv.multi.close.pressed"
#define VP_MULTI_VIEW_TITLE_CLOSE_BTN_RELEASED		"signal.pv.multi.close.released"

#define VP_MULTI_VIEW_TITLE_FULLSIZE_BTN_PRESSED	"signal.pv.multi.fullsize.pressed"
#define VP_MULTI_VIEW_TITLE_FULLSIZE_BTN_RELEASED	"signal.pv.multi.fullsize.released"

#define VP_MULTI_VIEW_TITLE_MINIMIZE_BTN_PRESSED	"signal.pv.multi.minimize.pressed"
#define VP_MULTI_VIEW_TITLE_MINIMIZE_BTN_RELEASED	"signal.pv.multi.minimize.released"

#define VP_NORMAL_SIGNAL_VIDEO_OFF			"signal.pv.normal.main.video.off"
#define VP_NORMAL_SIGNAL_VIDEO_ON			"signal.pv.normal.main.video.on"

#define VP_NORMAL_SIGNAL_ZOOM_GUIDE_SHOW		"signal.pv.zoom.guide.show"
#define VP_NORMAL_SIGNAL_ZOOM_GUIDE_HIDE		"signal.pv.zoom.guide.hide"

#define VP_NORMAL_SIGNAL_LOCKSCREEN_SHOW		"signal.pv.normal.lockscreen.show"
#define VP_NORMAL_SIGNAL_LOCKSCREEN_HIDE		"signal.pv.normal.lockscreen.hide"

#define VP_NORMAL_SIGNAL_NO_CONTNES_SHOW		"signal.pv.normal.no.contents.show"
#define VP_NORMAL_SIGNAL_NO_CONTNES_HIDE		"signal.pv.normal.no.contents.hide"

#define VP_NORMAL_SIGNAL_NAVIFRAME_TITLE_SHOW		"elm,state,title,show"
#define VP_NORMAL_SIGNAL_NAVIFRAME_TITLE_HIDE		"elm,state,title,hide"
#define VP_NORMAL_SIGNAL_NAVIFRAME_TITLE_NO_EFFECT_HIDE	"elm,state,title,no,effect,hide"

#define VP_NORMAL_SIGNAL_MAIN_SHOW_FINISH		"signal.pv.normal.main.control.show.finish"
#define VP_NORMAL_SIGNAL_MAIN_HIDE_FINISH		"signal.pv.normal.main.control.hide.finish"

#define VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_SHOW		"signal.pv.normal.main.playingspeed.show"
#define VP_NORMAL_SIGNAL_MAIN_PLAYING_SPEED_HIDE		"signal.pv.normal.main.playingspeed.hide"

#define VP_NORMAL_SIGNAL_MAIN_SPEED_SHOW		"signal.pv.normal.main.speed.show"
#define VP_NORMAL_SIGNAL_MAIN_SPEED_HIDE		"signal.pv.normal.main.speed.hide"

#define VP_NORMAL_SIGNAL_MAIN_SCREEN_PORTRAIT_SHOW	"signal.pv.normal.main.screenshot.portrait.show"
#define VP_NORMAL_SIGNAL_MAIN_SCREEN_LANDSCAPE_SHOW	"signal.pv.normal.main.screenshot.landscape.show"
#define VP_NORMAL_SIGNAL_MAIN_SCREEN_HIDE		"signal.pv.normal.main.screenshot.hide"

#define VP_NORMAL_SIGNAL_MAIN_BOOKMARK_ON		"signal.pv.normal.main.bookmark.on"
#define VP_NORMAL_SIGNAL_MAIN_BOOKMARK_OFF		"signal.pv.normal.main.bookmark.off"

#define VP_NORMAL_SIGNAL_MAIN_PORTRAIT_SHOW		"signal.pv.normal.main.portrait.show"
#define VP_NORMAL_SIGNAL_MAIN_LANDSCAPE_SHOW		"signal.pv.normal.main.landscape.show"

#define VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_PORTRAIT_SHOW	"signal.pv.normal.main.no.effect.portrait.show"
#define VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_LANDSCAPE_SHOW	"signal.pv.normal.main.no.effect.landscape.show"

#define VP_NORMAL_SIGNAL_MAIN_NO_EFFECT_HIDE		"signal.pv.normal.main.no.effect.hide"
#define VP_NORMAL_SIGNAL_MAIN_HIDE			"signal.pv.normal.main.control.hide"
#define VP_NORMAL_SIGNAL_MAIN_ROTATE_SHOW			"signal.pv.normal.main.rotate.icon.show"
#define VP_NORMAL_SIGNAL_MAIN_ROTATE_HIDE			"signal.pv.normal.main.rotate.icon.hide"
#define VP_NORMAL_SIGNAL_MAIN_VOLUME_PORTRAIT_SHOW			"signal.pv.normal.main.volume.icon.portrait.show"
#define VP_NORMAL_SIGNAL_MAIN_VOLUME_LANDSCAPE_SHOW			"signal.pv.normal.main.volume.icon.landscape.show"
#define VP_NORMAL_SIGNAL_MAIN_VOLUME_HIDE			"signal.pv.normal.main.volume.icon.hide"

#define VP_NORMAL_SIGNAL_MAIN_SUBTITLE_PORTRAIT		"signal.pv.normal.main.portrait.subtitle"
#define VP_NORMAL_SIGNAL_MAIN_SUBTITLE_HIDE		"signal.pv.normal.main.subtitle.hide"

#define VP_NORMAL_SIGNAL_CONTROL_PORTRAIT_MODE		"signal.pv.normal.control.portrait.mode"
#define VP_NORMAL_SIGNAL_CONTROL_LANDSCAPE_MODE		"signal.pv.normal.control.landscape.mode"

#define VP_NORMAL_SIGNAL_FUNCTION_PORTRAIT_MODE		"signal.pv.normal.function.portrait.mode"
#define VP_NORMAL_SIGNAL_FUNCTION_LANDSCAPE_MODE	"signal.pv.normal.function.landscape.mode"

#define VP_NORMAL_SIGNAL_PROGRESS_PORTRAIT_MODE		"signal.pv.normal.progress.portrait.mode"
#define VP_NORMAL_SIGNAL_PROGRESS_LANDSCAPE_MODE	"signal.pv.normal.progress.landscape.mode"
#define VP_NORMAL_SIGNAL_PROGRESS_OPACITY_DEFAULT	"signal.pv.normal.progress.opacity_default"
#define VP_NORMAL_SIGNAL_PROGRESS_OPACITY_60		"signal.pv.normal.progress.opacity_60"

#define VP_NORMAL_SIGNAL_PROGRESS_PRESS			"signal.pv.normal.progress.click"
#define VP_NORMAL_SIGNAL_PROGRESS_UNPRESS		"signal.pv.normal.progress.unclick"

#define VP_NORMAL_SIGNAL_VOLUME_PORTRAIT_MODE		"signal.pv.volume.portrait.mode"
#define VP_NORMAL_SIGNAL_VOLUME_LANDSCAPE_MODE		"signal.pv.volume.landscape.mode"

#define VP_NORMAL_SIGNAL_MICROSEEK_TEXT_SHOW				"signal.pv.normal.main.microseek.text.show"
#define VP_NORMAL_SIGNAL_MICROSEEK_TEXT_HIDE				"signal.pv.normal.main.microseek.text.hide"
#define VP_NORMAL_SIGNAL_MICROSEEK_SPEED_TEXT		"pv.normal.main.microseek.speed.text"

#define VP_NORMAL_SIGNAL_MICROSEEK_IMAGE_SHOW				"signal.pv.normal.main.microseek.image.show"
#define VP_NORMAL_SIGNAL_MICROSEEK_IMAGE_HIDE				"signal.pv.normal.main.microseek.image.hide"

#define VP_NORMAL_SIGNAL_FLICKJUMP_FF			"signal.pv.normal.main.flickjump.ff"
#define VP_NORMAL_SIGNAL_FLICKJUMP_REW			"signal.pv.normal.main.flickjump.rew"


#define VP_SUBTITLE_SIGNAL_MAIN_PORTRAIT_SHOW	"signal.pv.subtitle.main.portrait.show"
#define VP_SUBTITLE_SIGNAL_MAIN_LANDSCAPE_SHOW	"signal.pv.subtitle.main.landscape.show"
#define VP_SUBTITLE_SIGNAL_GREP_BOX_SHOW		"signal.pv.subtitle.grep.box.show"
#define VP_SUBTITLE_SIGNAL_GREP_BOX_HIDE		"signal.pv.subtitle.grep.box.hide"
#define VP_SUBTITLE_SIGNAL_CAPTION_WINDOW_HIDE		"signal.pv.subtitle.caption.window.hide"

#define VP_CHAPTER_SIGNAL_FUNCTION_PORTRAIT_MODE	"signal.pv.chapter.portrait.mode"
#define VP_CHAPTER_SIGNAL_FUNCTION_LANDSCAPE_MODE	"signal.pv.chapter.landscape.mode"

#define VP_POPUP_SIGNAL_CAPTION_PORTRAIT_MODE          "signal.pv.popup.caption.portrait.mode"
#define VP_POPUP_SIGNAL_CAPTION_LANDSCAPE_MODE         "signal.pv.popup.caption.landscape.mode"

#define VP_TAG_BUDDY_SIGNAL_HIDE		"signal.pv.tag.bg.hide"
#define VP_TAG_BUDDY_SIGNAL_SHOW		"signal.pv.tag.bg.show"

#define VP_TAG_BUDDY_SIGNAL_WEATHER_JP		"signal.pv.tag.weathernews"
#define VP_TAG_BUDDY_SIGNAL_ACCUWEATHER		"signal.pv.tag.accuweather"

//multi-window
#define VP_NORMAL_SIGNAL_CONTROL_P_SHOW			"signal.pv.normal.control.show.p"
#define VP_NORMAL_SIGNAL_CONTROL_RFP_SHOW		"signal.pv.normal.control.show.r.f.p"
#define VP_NORMAL_SIGNAL_CONTROL_VSFRP_SHOW		"signal.pv.normal.control.show.v.s.f.r.p"
#define VP_NORMAL_SIGNAL_FUNCTION_1_3_SHOW		"signal.pv.normal.function.show.1.3"
#define VP_NORMAL_SIGNAL_CONTROL_VS_HIDE		"signal.pv.normal.control.hide.v.s"
#define VP_NORMAL_SIGNAL_CONTROL_VSFR_HIDE		"signal.pv.normal.control.hide.v.s.f.r"
#define VP_NORMAL_SIGNAL_CONTROL_VSFRP_HIDE		"signal.pv.normal.control.hide.v.s.f.r.p"
#define VP_NORMAL_SIGNAL_FUNCTION_1_3_HIDE		"signal.pv.normal.function.hide.1.3"

//trim view
#define VP_TRIM_SIGNAL_CONTROL_PLAYING_SHOW		"signal.pv.trim.control.show.playing"
#define VP_TRIM_SIGNAL_CONTROL_PLAYING_HIDE		"signal.pv.trim.control.hide.playing"

/* resource groups(image) */
#define VP_PLAY_MULTI_VIEW_CLOSE		"multi/my_title_icon_delete.png"
#define VP_PLAY_MULTI_VIEW_FULLSIZE		"multi/my_title_icon_maximize.png"
#define VP_PLAY_MULTI_VIEW_RESIZE		"multi/mw_frame_handler.png"
#define VP_PLAY_MULTI_VIEW_RESIZE_PRESS		"multi/mw_frame_handler_press.png"
#define VP_PLAY_MULTI_VIEW_MINSIZE		"multi/my_title_icon_mimimize.png"


#define VP_PLAY_MULTI_VIEW_FF			"multi/T01_2_mw_control_icon_ff.png"
#define VP_PLAY_MULTI_VIEW_FF_PRESS		"multi/T01_2_mw_control_icon_ff_press.png"
#define VP_PLAY_MULTI_VIEW_REW			"multi/T01_2_mw_control_icon_rew.png"
#define VP_PLAY_MULTI_VIEW_REW_PRESS		"multi/T01_2_mw_control_icon_rew_press.png"
#define VP_PLAY_MULTI_VIEW_PLAY			"multi/T01_2_mw_control_icon_play.png"
#define VP_PLAY_MULTI_VIEW_PLAY_PRESS		"multi/T01_2_mw_control_icon_play_press.png"
#define VP_PLAY_MULTI_VIEW_PAUSE		"multi/T01_2_mw_control_icon_pause.png"
#define VP_PLAY_MULTI_VIEW_PAUSE_PRESS		"multi/T01_2_mw_control_icon_pause_press.png"

#define VP_PLAY_NAVIFRAME_RES_SAVE		"button/T01_2_icon_check.png"
#define VP_PLAY_NAVIFRAME_RES_SAVE_PRESS	"button/T01_2_icon_check_press.png"
#define VP_PLAY_NAVIFRAME_RES_SAVE_DIM		"button/T01_2_icon_check_dim.png"

#define VP_PLAY_NAVIFRAME_RES_BACK		"button/T01_2_icon_close.png"
#define VP_PLAY_NAVIFRAME_RES_BACK_PRESS	"button/T01_2_icon_close_press.png"
#define VP_PLAY_NAVIFRAME_RES_BACK_DIM		"button/T01_2_icon_close_dim.png"


#define VP_PLAY_NORMAL_RES_RESUME		"control/T01-2_control_circle_icon_play.png"
#define VP_PLAY_NORMAL_RES_PAUSE		"control/T01-2_control_circle_icon_pause.png"
#define VP_PLAY_NORMAL_RES_PREV			"control/T01-2_control_circle_icon_rewind.png"
#define VP_PLAY_NORMAL_RES_NEXT			"control/T01-2_control_circle_icon_next.png"

#define VP_PLAY_NORMAL_RES_CAPTURE		"capture/T01_2_icon_capture.png"
#define VP_PLAY_NORMAL_RES_CAPTURE_PRESS	"capture/T01_2_icon_capture_press.png"
#define VP_PLAY_NORMAL_RES_CAPTURE_DIM		"capture/T01_2_icon_capture_dim.png"

#define VP_PLAY_NORMAL_RES_CAPTURE_REW		"capture/T01-2_icon_left.png"
#define VP_PLAY_NORMAL_RES_CAPTURE_REW_PRESS	"capture/T01-2_icon_left_press.png"
#define VP_PLAY_NORMAL_RES_CAPTURE_REW_DIM	"capture/T01-2_icon_left_dim.png"

#define VP_PLAY_NORMAL_RES_CAPTURE_FF		"capture/T01-2_icon_right.png"
#define VP_PLAY_NORMAL_RES_CAPTURE_FF_PRESS	"capture/T01-2_icon_right_press.png"
#define VP_PLAY_NORMAL_RES_CAPTURE_FF_DIM	"capture/T01-2_icon_right_dim.png"


#define VP_PLAY_TRIM_RES_REW			"control/T01_2_control_circle_icon_reverse.png"
#define VP_PLAY_TRIM_RES_FF			"control/T01_2_control_circle_icon_forward.png"

#define VP_PLAY_NORMAL_RES_VOLUME		"function/T01_2_control_icon_volume.png"
#define VP_PLAY_NORMAL_RES_VOLUME_PRESS		"function/T01_2_control_icon_volume_press.png"
#define VP_PLAY_NORMAL_RES_VOLUME_DIM		"function/T01_2_control_icon_volume_dim.png"

#define VP_PLAY_NORMAL_RES_MUTE			"function/T01-2_control_icon_mute.png"
#define VP_PLAY_NORMAL_RES_MUTE_PRESS		"function/T01-2_control_icon_mute_press.png"
#define VP_PLAY_NORMAL_RES_MUTE_DIM		"function/T01-2_control_icon_mute_dim.png"

#define VP_PLAY_NORMAL_RES_BOOKMARK		"function/T01_2_control_icon_bookmark.png"
#define VP_PLAY_NORMAL_RES_BOOKMARK_PRESS	"function/T01_2_control_icon_bookmark_press.png"
#define VP_PLAY_NORMAL_RES_BOOKMARK_DIM		"function/T01_2_control_icon_bookmark_dim.png"

#define VP_PLAY_NORMAL_RES_POPUP_PLAYER 	"function/player_popup_view_icon_nomal.png"

#define VP_PLAY_NORMAL_RES_SCREEN_1_MODE	"function/T01_2_control_icon_screenmode03.png"
#define VP_PLAY_NORMAL_RES_SCREEN_1_MODE_PRESS	"function/T01_2_control_icon_screenmode03_press.png"
#define VP_PLAY_NORMAL_RES_SCREEN_1_MODE_DIM	"function/T01_2_control_icon_screenmode03_dim.png"

#define VP_PLAY_NORMAL_RES_SCREEN_2_MODE	"function/T01_2_control_icon_screenmode02.png"
#define VP_PLAY_NORMAL_RES_SCREEN_2_MODE_PRESS	"function/T01_2_control_icon_screenmode02_press.png"
#define VP_PLAY_NORMAL_RES_SCREEN_2_MODE_DIM	"function/T01_2_control_icon_screenmode02_dim.png"

#define VP_PLAY_NORMAL_RES_SCREEN_3_MODE	"function/T01_2_control_icon_screenmode.png"
#define VP_PLAY_NORMAL_RES_SCREEN_3_MODE_PRESS	"function/T01_2_control_icon_screenmode_press.png"
#define VP_PLAY_NORMAL_RES_SCREEN_3_MODE_DIM	"function/T01_2_control_icon_screenmode_dim.png"

#define VP_PLAY_NORMAL_RES_SCREEN_4_MODE			"function/T01_2_control_icon_screenmode04.png"
#define VP_PLAY_NORMAL_RES_SCREEN_4_MODE_PRESS	"function/T01_2_control_icon_screenmode04_press.png"
#define VP_PLAY_NORMAL_RES_SCREEN_4_MODE_DIM		"function/T01_2_control_icon_screenmode04_dim.png"

#define VP_PLAY_NORMAL_RES_MULTI_MODE		"function/T01_2_control_icon_multi_window.png"
#define VP_PLAY_NORMAL_RES_MULTI_MODE_PRESS	"function/T01_2_control_icon_multi_window_press.png"
#define VP_PLAY_NORMAL_RES_MULTI_MODE_DIM	"function/T01_2_control_icon_multi_window_dim.png"

#define VP_PLAY_NORMAL_RES_SHARE		"function/T01_2_control_icon_share.png"
#define VP_PLAY_NORMAL_RES_SHARE_PRESS		"function/T01_2_control_icon_share_press.png"
#define VP_PLAY_NORMAL_RES_SHARE_DIM		"function/T01_2_control_icon_share_dim.png"

#define VP_PLAY_NORMAL_RES_SOUND_ALIVE		"function/T01_2_control_icon_soundalive.png"
#define VP_PLAY_NORMAL_RES_SOUND_ALIVE_PRESS	"function/T01_2_control_icon_soundalive_press.png"
#define VP_PLAY_NORMAL_RES_SOUND_ALIVE_DIM	"function/T01_2_control_icon_soundalive_dim.png"

#define VP_PLAY_NORMAL_RES_REPEAT_OFF		"function/T01_2_control_icon_repeat_mode.png"
#define VP_PLAY_NORMAL_RES_REPEAT_OFF_PRESS	"function/T01_2_control_icon_repeat_mode_press.png"
#define VP_PLAY_NORMAL_RES_REPEAT_OFF_DIM	"function/T01_2_control_icon_repeat_mode_dim.png"

#define VP_PLAY_NORMAL_RES_REPEAT_ONE		"function/T01_2_control_icon_repeat_once.png"
#define VP_PLAY_NORMAL_RES_REPEAT_ONE_PRESS	"function/T01_2_control_icon_repeat_once_press.png"
#define VP_PLAY_NORMAL_RES_REPEAT_ONE_DIM	"function/T01_2_control_icon_repeat_once_dim.png"

#define VP_PLAY_NORMAL_RES_REPEAT_ALL		"function/T01_2_control_icon_repeat_all.png"
#define VP_PLAY_NORMAL_RES_REPEAT_ALL_PRESS	"function/T01_2_control_icon_repeat_all_press.png"
#define VP_PLAY_NORMAL_RES_REPEAT_ALL_DIM	"function/T01_2_control_icon_repeat_all_dim.png"

#define VP_PLAY_NORMAL_RES_TRIM			"function/T01_2_control_icon_trim.png"
#define VP_PLAY_NORMAL_RES_TRIM_PRESS		"function/T01_2_control_icon_trim_press.png"
#define VP_PLAY_NORMAL_RES_TRIM_DIM		"function/T01_2_control_icon_trim_dim.png"

#define VP_PLAY_NORMAL_RES_ROTATE		"button/T01_2_icon_rotate.png"
#define VP_PLAY_NORMAL_RES_ROTATE_PRESS		"button/T01_2_icon_rotate_press.png"
#define VP_PLAY_NORMAL_RES_ROTATE_DIM		"button/T01_2_icon_rotate_dim.png"

#define VP_PLAY_BOOKMARK_RES_ADD			"bookmark/T01_2_icon_bookmark_add.png"
#define VP_PLAY_BOOKMARK_RES_ADD_PRESS			"bookmark/T01_2_icon_bookmark_add_press.png"
#define VP_PLAY_BOOKMARK_RES_DEL			"bookmark/T01_2_button_remove.png"
#define VP_PLAY_BOOKMARK_RES_DEL_PRESS			"bookmark/T01_2_button_remove_press.png"

#define VP_PLAY_SENSOR_RES_TRUN_OVER_01			"sensor/B15_turn_over_01.png"
#define VP_PLAY_SENSOR_RES_TRUN_OVER_02			"sensor/B15_turn_over_02.png"
#define VP_PLAY_SENSOR_RES_TRUN_OVER_03			"sensor/B15_turn_over_03.png"

#define VP_PLAY_CTX_ADD_TO_HOME				"more/icon_add_to_home.png"
#define VP_PLAY_CTX_CHAPTER_PREVIEW			"more/icon_chapter_preview.png"
#define VP_PLAY_CTX_DETAIL				"more/icon_detail.png"
#define VP_PLAY_CTX_DOWNLOAD				"more/icon_download.png"
#define VP_PLAY_CTX_SETTING				"more/icon_setting.png"
#define VP_PLAY_CTX_SHARE_VIA				"more/icon_share_via.png"
#define VP_PLAY_CTX_TRIM				"more/icon_trim.png"
#define VP_PLAY_CTX_CONNECTION_INFO			"more/icon_nearbydevices.png"

#define VP_PLAY_RES_BUTTON_ADD				"button/T01-2_play_popup_icon_plus.png"
#define VP_PLAY_RES_BUTTON_ADD_PRESS			"button/T01-2_play_popup_icon_plus_press.png"

#define VP_PLAY_RES_BUTTON_DEL				"button/T01-2_play_popup_icon_minus.png"
#define VP_PLAY_RES_BUTTON_DEL_PRESS			"button/T01-2_play_popup_icon_minus_press.png"

#define VP_PLAY_TAG_BUDDY_DEAFULT			"tag/T01-1_Tag_buddy_settings_bg.png"

#define VP_PLAY_NORMAL_RES_LOCK				"button/T01_2_icon_lock.png"
#define VP_PLAY_NORMAL_RES_LOCK_PRESS			"button/T01_2_icon_lock_press.png"
#define VP_PLAY_TAG_BUDDY_DEAFULT		"tag/T01-1_Tag_buddy_settings_bg.png"
#define VP_PLAY_TAG_BUDDY_LOGO			"tag/T01_2_icon_tag_buddy_cplogo.PNG"
#define VP_PLAY_TAG_BUDDY_LOGO_JP		"tag/U07_weather_logo.png"

#define VP_PLAY_NORMAL_RES_BLUETOOTH_HEADSET	"button/T01_2_icon_bluetooth_headset.png"
#define VP_PLAY_NORMAL_RES_EARPHONE				"button/T02_icon_soundPath.png"
#define VP_PLAY_NORMAL_RES_SPEAKER				"button/T02_icon_speaker.png"
#define VP_PLAY_NORMAL_RES_ICON_HDMI			"button/T02_icon_speaker.png"
#define VP_PLAY_NORMAL_RES_MIRRORING			"button/T02_icon_speaker.png"
#define VP_PLAY_NORMAL_RES_USB_AUDIOE			"button/T02_icon_speaker.png"
#define VP_PLAY_NORMAL_RES_DMC_ICON				"mini_controller/A01_15_popup_No_device.png"

#define VP_PLAY_NORMAL_AUDIO_ONLY_IMG		"sound_only.png"
#define VP_PLAY_NORMAL_AUDIO_ERROR_IMG		"player_error_screen.png"

#define VP_PLAY_BRIGHTNESS_POPUP_ICON_PATH 	"brightness/quick_icon_brightness_%02d.png"

/* string resources */
#define VP_PLAY_STRING_NO_TITLE					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_NO_TITLE"))
#define VP_PLAY_STRING_STREAMING_PLAYER				""

#define VP_PLAY_STRING_UNABLE_TO_PLAY_VIDEO_DURING_CALL		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_UNABLE_TO_PLAY_VIDEO_DURING_CALL"))

#define VP_PLAY_STRING_ERROR_UNABLE_CAPTURE_DRM_CONTENT		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_UNABLE_TO_CAPTURE"))//this id has been reomved from po file

#define VP_PLAY_STRING_ERROR_UNSUPPORTED_AUDIO			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_UNSUPPORTED_AUDIO_FORMAT"))
#define VP_PLAY_STRING_ERROR_UNSUPPORTED_VIDEO			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_UNSUPPORTED_VIDEO_FORMAT"))

#define VP_PLAY_STRING_ERROR_FILE_NOT_EXIST			(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_FILE_NOT_EXIST"))
#define VP_PLAY_STRING_ERROR_UNABLE_PLAY			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_UNABLE_TO_PLAY_VIDEO"))
#define VP_PLAY_STRING_ERROR_UNSUPPORTED_FORMAT			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_UNSUPPORTED_FILE_TYPE"))
#define VP_PLAY_STRING_ERROR_INVALID_URL			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_INVALID_URL"))
#define VP_PLAY_STRING_ERROR_CONNECTION_FAILED			(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_CONNECTION_FAILED"))
#define VP_PLAY_STRING_ERROR_BOOKMARK_LIST_FULL			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_BOOKMARK_LIST_FULL_DELETE_SOME_BOOKMARKS"))//this id has been removed from po file
#define VP_PLAY_STRING_ERROR_NO_SUBTITLE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_NO_SUBTITLES_AVAILABLE_M_STATUS"))
#define VP_PLAY_STRING_ERROR_SUBTITLE_FAIL			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_ERROR_OCCURRED_WITH_SUBTITLE_FILE"))
#define VP_PLAY_STRING_SUBTITLE_OFF			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_OFF"))
#define VP_PLAY_STRING_SUBTITLE_DONE		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_TPLATFORM_ACBUTTON_DONE_ABB"))
#define VP_PLAY_STRING_SUBTITLE_CANCEL			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_TPLATFORM_ACBUTTON_CANCEL_ABB"))

#define VP_PLAY_STRING_POPUP_SHARE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_SHARE"))
#define VP_PLAY_STRING_POPUP_DELETE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_IOPT3_DELETE"))
#define VP_PLAY_STRING_POPUP_DELETE_CONTENT		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_THIS_VIDEO_WILL_BE_DELETED"))
#define VP_PLAY_STRING_CANCEL					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SK_CANCEL_ABB"))

#define VP_PLAY_STRING_POPUP_AVAILABLE_DEVICES			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_HEADER_AVAILABLE_DEVICES_ABB"))
#define VP_PLAY_STRING_POPUP_SOUND_ALIVE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_SOUNDALIVE_ABB2"))
#define VP_PLAY_STRING_POPUP_SUBTITLE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_HEADER_SUBTITLES_HCC_ABB2"))
#define VP_PLAY_STRING_POPUP_REPEAT_SETTING			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_REPEAT_SETTING_ABB"))
#define VP_PLAY_STRING_POPUP_AUDIO_TRACK			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_AUDIO_TRACK_ABB"))
#define VP_PLAY_STRING_POPUP_SETTINGS				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_HEADER_SETTINGS"))
#define VP_PLAY_STRING_POPUP_PLAY_SPEED				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_PLAY_SPEED"))//this id has been reomved from po file
#define VP_PLAY_STRING_POPUP_CAPTURE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_CAPTURE"))//this id has been reomved from po file
#define VP_PLAY_STRING_POPUP_TAG_BUDDY				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_TAG_BUDDY"))//this id has been reomved from po file
#define VP_PLAY_STRING_POPUP_EDIT_WEATHER_TAG			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_HEADER_EDIT_WEATHER_TAG_ABB"))//this id has been reomved from po file

#define VP_PLAY_STRING_COM_PLAY					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SK3_PLAY"))
#define VP_PLAY_STRING_COM_PAUSE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_MUSIC_OPT_PAUSE"))
#define VP_PLAY_STRING_COM_PREVIOUS				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_BR_SK_PREVIOUS"))
#define VP_PLAY_STRING_COM_NEXT					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_BR_SK_NEXT"))
#define VP_PLAY_STRING_COM_VOLUME				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_VOLUME_M_SOUND"))
#define VP_PLAY_STRING_COM_ROTATE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_ROTATE"))//this id has been reomved from po file
#define VP_PLAY_STRING_COM_ON					(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_ON"))
#define VP_PLAY_STRING_COM_OFF					(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_OFF"))
#define VP_PLAY_STRING_COM_ENOUGH_MEMORY			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_NOT_ENOUGH_MEMORY_DELETE_SOME_ITEMS"))

#define VP_PLAY_STRING_COM_CLOSE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SK_CLOSE"))
#define VP_PLAY_STRING_COM_CLOSE_IDS			("IDS_VIDEO_SK_CLOSE")
#define VP_PLAY_STRING_COM_STOP_IDS			("IDS_COM_SK_STOP")
#define VP_PLAY_STRING_COM_REFRESH				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BUTTON_REFRESH"))
#define VP_PLAY_STRING_COM_REFRESH_IDS			("IDS_VIDEO_BUTTON_REFRESH")
#define VP_PLAY_STRING_COM_SCAN_IDS			("IDS_COM_BUTTON_SCAN")
#define VP_PLAY_STRING_COM_ERROR				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_CHATON_HEADER_ERROR_ABB"))
#define VP_PLAY_STRING_COM_CANCEL				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SK_CANCEL_ABB"))
#define VP_PLAY_STRING_COM_CANCEL_IDS			("IDS_VIDEO_SK_CANCEL_ABB")
#define VP_PLAY_STRING_COM_DONE					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BUTTON_DONE"))
#define VP_PLAY_STRING_COM_OK					(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_SK_OK"))
#define VP_PLAY_STRING_COM_OK_IDS				("IDS_COM_SK_OK")
#define VP_PLAY_STRING_COM_YES					(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_SK_YES"))
#define VP_PLAY_STRING_COM_YES_IDS				("IDS_COM_SK_YES")
#define VP_PLAY_STRING_COM_NO					(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_SK_NO"))
#define VP_PLAY_STRING_COM_NO_IDS				("IDS_COM_SK_NO")
#define VP_PLAY_STRING_COM_LOW_BATTERY				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_LOW_BATTERY"))
#define VP_PLAY_STRING_COM_DOWNLOAD				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BUTTON_DOWNLOAD"))
#define VP_PLAY_STRING_COM_DETAILS				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_DETAILS"))
#define VP_PLAY_STRING_COM_DEFAULT				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_DEFAULT"))
#define VP_PLAY_STRING_COM_UNKNOWN				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_UNKNOWN"))
#define VP_PLAY_STRING_COM_WAIT_BACK				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_PRESS_THE_BACK_KEY_AGAIN_TO_RETURN_TO_PREVIOUS_SCREEN"))

#define VP_PLAY_STRING_DETAIL_TITLE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_TITLE"))
#define VP_PLAY_STRING_DETAIL_FORMAT				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_FILE_FORMAT"))
#define VP_PLAY_STRING_DETAIL_DATE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_HEADER_DATE"))
#define VP_PLAY_STRING_DETAIL_SIZE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_SIZE"))
#define VP_PLAY_STRING_DETAIL_RESOLUTION			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_RESOLUTION"))
#define VP_PLAY_STRING_DETAIL_LATITUDE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_LATITUDE"))
#define VP_PLAY_STRING_DETAIL_LONGITUDE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_LONGITUDE"))
#define VP_PLAY_STRING_DETAIL_LOCATION				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_LOCATION"))


#define VP_PLAY_STRING_SHARE_MESSAGE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_MESSAGE_ABB"))
#define VP_PLAY_STRING_SHARE_EMAIL				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_EMAIL_ABB"))
#define VP_PLAY_STRING_SHARE_BLUETOOTH				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_BLUETOOTH"))
#define VP_PLAY_STRING_SHARE_FTM				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_OPT1_WI_FI_DIRECT"))
#define VP_PLAY_STRING_SHARE_FACEBOOK				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_FACEBOOK_ABB"))
#define VP_PLAY_STRING_SHARE_YOUTUBE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_YOUTUBE_ABB"))
#define VP_PLAY_STRING_SHARE_NFC				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_NFC"))
#define VP_PLAY_STRING_SHARE_FILE_TRANSFER			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_FILE_TRANSFER_ABB"))
#define VP_PLAY_STRING_SHARE_DROPBOX				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_CLOUD"))

#define VP_PLAY_STRING_SOUND_ALIVE_NORMAL			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_NORMAL_M_SOUND_ABB"))
#define VP_PLAY_STRING_SOUND_ALIVE_VOICE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_VOICE"))
#define VP_PLAY_STRING_SOUND_ALIVE_MOVIE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_MOVIES"))
#define VP_PLAY_STRING_SOUND_ALIVE_71CH				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_VIRTUAL_71_CH"))//this id has been reomved from po file
#define VP_PLAY_STRING_71CH_ONLY_EARPHONE			_(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_PS_SOUND_EFFECTS_NOT_SUPPORTED_BY_SPEAKER"))

#define VP_PLAY_STRING_SUBTITLE_SYNC				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_SYNCHRONIZE"))
#define VP_PLAY_STRING_SUBTITLE_SELECT_SUBTITLES		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_MBODY_SELECT_SUBTITLES_HCC"))
#define VP_PLAY_STRING_SUBTITLE_LANGUAGE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_LANGUAGE"))
#define VP_PLAY_STRING_SUBTITLE_ALIGNMENT			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_MBODY_ALIGNMENT"))
#define VP_PLAY_STRING_SUBTITLE_FONT				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_FONT"))
#define VP_PLAY_STRING_SUBTITLE_SIZE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_MBODY_FONT_SIZE"))

#define VP_PLAY_STRING_SUBTITLE_EDGE				("Edge")
#define VP_PLAY_STRING_SUBTITLE_COLOR_AND_OPACITY	("Color and opacity")
#define VP_PLAY_STRING_SUBTITLE_OVERVIEW			("Overview")
#define VP_PLAY_STRING_SUBTITLE_TEXT				("Text")
#define VP_PLAY_STRING_SUBTITLE_OPACITY				("Opacity")
#define VP_PLAY_STRING_SUBTITLE_BG_COLOR			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_BACKGROUND_COLOUR"))
#define VP_PLAY_STRING_SUBTITLE_CAPTION_WINDOW		("Caption window")

#define VP_PLAY_STRING_ALIGNMENT_LEFT					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_LEFT_M_STATUS"))
#define VP_PLAY_STRING_ALIGNMENT_CENTER				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_CENTRE_M_STATUS"))
#define VP_PLAY_STRING_ALIGNMENT_RIGHT				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_RIGHT_M_STATUS"))

#define VP_PLAY_STRING_EDGE_NO_EDGE					("No edge")
#define VP_PLAY_STRING_EDGE_RAISED					("Raised")
#define VP_PLAY_STRING_EDGE_DEPRESSED				("Depressed")
#define VP_PLAY_STRING_EDGE_UNIFORM					("Uniform")
#define VP_PLAY_STRING_EDGE_DROP_SHADOW				("Drop shadow")


#define VP_PLAY_STRING_SUBTITLE_SECS				_(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_PS_SECONDS"))//this is has been removed from po file

#define VP_PLAY_STRING_SUBTITLE_SIZE_LARGE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_LARGE_M_STATUS"))
#define VP_PLAY_STRING_SUBTITLE_SIZE_MEDIUM			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_MEDIUM_M_STATUS"))
#define VP_PLAY_STRING_SUBTITLE_SIZE_SMALL			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_SBODY_SMALL_M_STATUS"))

#define VP_PLAY_STRING_SUBTITLE_COLOR_BLACK			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_BLACK"))//this is has been removed from po file
#define VP_PLAY_STRING_SUBTITLE_COLOR_BLUE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_BLUE"))//this is has been removed from po file
#define VP_PLAY_STRING_SUBTITLE_COLOR_GREEN			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_GREEN"))//this is has been removed from po file
#define VP_PLAY_STRING_SUBTITLE_COLOR_WHITE			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_WHITE"))//this is has been removed from po file
#define VP_PLAY_STRING_SUBTITLE_COLOR_RED			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_RED_ABB"))
#define VP_PLAY_STRING_SUBTITLE_COLOR_YELLOW		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_YELLOW_M_COLOUR"))
#define VP_PLAY_STRING_SUBTITLE_COLOR_MAGENTA		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_MAGENTA_M_COLOUR"))
#define VP_PLAY_STRING_SUBTITLE_COLOR_CYAN			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_CYAN_M_COLOUR"))
#define VP_PLAY_STRING_SUBTITLE_COLOR_CUSTOM		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_CUSTOM"))
#define VP_PLAY_STRING_SUBTITLE_COLOR_NONE			(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_NONE"))

#define VP_PLAY_STRING_REPEAT_OFF				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_NO_REPEAT"))
#define VP_PLAY_STRING_REPEAT_ALL_OFF				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_STOP_AFTER_ALL_ABB"))
#define VP_PLAY_STRING_REPEAT_ONE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_REPEAT_CURRENT_ABB"))
#define VP_PLAY_STRING_REPEAT_ALL				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_REPEAT_ALL_ABB"))

#define VP_PLAY_STRING_SOUND_PATH				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_HEADER_AUDIO_DEVICE_ABB"))
#define VP_PLAY_STRING_SOUND_SPEAKER				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BUTTON_SPEAKER_ABB"))
#define VP_PLAY_STRING_SOUND_EARJACK				(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_OPT_HEADPHONES_ABB"))

#define VP_PLAY_STRING_SETTINGS					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_HEADER_SETTINGS"))

#define VP_PLAY_STRING_AUDIO_TRACK				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_OPT_AUDIO_TRACK_ABB"))
#define VP_PLAY_STRING_CONNECT_INFO				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_CONNECTION_INFO_ABB"))//this is has been removed from po file
#define VP_PLAY_STRING_CHNAGE_PLAYER				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_OPT_CHANGE_PLAYER"))//this is has been removed from po file
#define VP_PLAY_STRING_MY_DEVICE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_MY_DEVICE"))//this is has been removed from po file
#define VP_PLAY_STRING_SELECT_DEVICE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_HEADER_SELECT_DEVICE_ABB"))//this is has been removed from po file
#define VP_PLAY_STRING_UNABLE_TO_FF_REW				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_UNABLE_TO_USE_FAST_FORWARD_AND_REWIND_WHEN_NEARBY_DEVICES_ARE_ENABLED"))

#define VP_PLAY_STRING_DRM_UNABLE_TO_SHARE_DRM_FILE				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_POP_DRM_FILES_CANNOT_BE_SHARED"))
#define VP_PLAY_STRING_DRM_PS_1_MORE_TIME_START_NOW_Q				_(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_YOU_CAN_USE_PS_1_MORE_TIME_START_NOW_Q"))
#define VP_PLAY_STRING_DRM_PS_2_MORE_TIME_START_NOW_Q				_(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_YOU_CAN_USE_PS_2_MORE_TIMES_START_NOW_Q"))
#define VP_PLAY_STRING_DRM_PS_FOR_PD_DAYS_START_NOW_Q				_(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_YOU_CAN_USE_PS_FOR_PD_DAYS_START_NOW_Q"))
#define VP_PLAY_STRING_DRM_CURRENTLY_LOCKED_UNLOCK_Q				_(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_PS_CURRENTLY_LOCKED_UNLOCK_Q"))
#define VP_PLAY_STRING_DRM_CAN_USE_1_MORE_TIME_GET_ANOTHER_LICENCE_Q		(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_YOU_CAN_USE_PS_1_MORE_TIME_GET_ANOTHER_LICENCE_Q"))
#define VP_PLAY_STRING_DRM_CANNOT_USE_ANY_MORE_TIMES_GET_ANOTHER_LICENCE_Q	(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_YOU_CANNOT_USE_PS_ANY_MORE_TIMES_GET_ANOTHER_LICENCE_Q"))
#define VP_PLAY_STRING_DRM_PS_IS_ABOUT_TO_EXPIRE_GET_ANOTHER_LICENCE_Q		(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_POP_PS_IS_ABOUT_TO_EXPIRE_GET_ANOTHER_LICENCE_Q"))

#define VP_PLAY_STRING_DIVX_DRM_AUTHORIZATION_ERROR		"This phone is not authorized to play this DivX protected video."
#define VP_PLAY_STRING_DIVX_DRM_RENTAL_EXPIRED_ERROR		"This DivX rental has used %d of %d views. <br> This DivX rental has expired."
#define VP_PLAY_STRING_DIVX_DRM_RENTAL_CONFIRMATION_CHOICE	"This DivX rental has used %d of %d views. <br> Continue ?"

#define VP_PLAY_STRING_NO_VIDEOS				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_BODY_NO_VIDEOS"))

#define VP_PLAY_STRING_ACTIVATION				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_ACTIVATION"))

#define VP_PLAY_STRING_SENSOR_TURN_PHONE_OVER_TO_MUTE		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_MUSIC_BODY_TURN_PHONE_OVER_TO_MUTE_INCOMING_CALLS_MESSAGE_ALERTS_AND_ALARMS")) //maybe, it is not implemented yet.
#define VP_PLAY_STRING_SENSOR_TURN_PHONE_OVER_TO_MUTE_IDS	("IDS_MUSIC_BODY_TURN_PHONE_OVER_TO_MUTE_INCOMING_CALLS_MESSAGE_ALERTS_AND_ALARMS")
#define VP_PLAY_STRING_SENSOR_DONT_ASK_AGAIN			(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BUTTON_DONT_ASK_AGAIN"))
#define VP_PLAY_STRING_SENSOR_ENABLE_MOTION			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_MUSIC_BUTTON_ENABLE_MOTION")) // maybe, it is not implemented yet.
#define VP_PLAY_STRING_SENSOR_ENABLE_MOTION_IDS		("IDS_MUSIC_BUTTON_ENABLE_MOTION")

#define VP_VIDEO_STRING_EXIT_POPUP				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_SMEMO_POP_CLOSE_APPLICATION_Q"))//this id has been removed from po file

#define VP_VIDEO_PLAYER_TEXT					(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_VIDEO_PLAYER"))


#define VP_COM_BODY_KB								(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_KB"))
#define VP_COM_BODY_B								(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_B"))
#define VP_COM_BODY_MB								(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_MB"))
#define VP_COM_BODY_GB								(dgettext(VP_SYS_STR_PREFIX, "IDS_COM_BODY_GB"))

//this id has been reomved from po file
#define VP_PLAY_STRING_PRIVACY_POPUP				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_ST_POP_P1SS_IS_DISABLED_IN_THIS_APPLICATION_TO_ENABLE_THIS_FUNCTION_GO_TO_P2SS_SELECT_THIS_FUNCTION_AND_THEN_SELECT_THIS_APPLICATION"))

//need to support multi language
#define VP_PLAY_STRING_SOUND_HDMI				(dgettext(VP_LOCAL_STR_PREFIX,"IDS_VIDEO_OPT_HDMI"))
#define VP_PLAY_STRING_SOUND_MIRRORING			("Mirroring")
#define VP_PLAY_STRING_SOUND_USB_AUDIO			("USB Audio")

#define VP_PLAY_STRING_PREVIEW_SUBTITLE_CC			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_ACCS_BODY_CAPTIONS_WILL_LOOK_LIKE_THIS"))

#define VP_PLAY_STRING_FONT_DEFAULT				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_DEFAULT_FONT"))
#define VP_PLAY_STRING_FONT_UDRCOTHICM				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_UDRGOTHICM_M_FONT"))
#define VP_PLAY_STRING_FONT_UDMINCHO				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_UDMINCHO_M_FONT"))
#define VP_PLAY_STRING_FONT_POP					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_POP_M_FONT"))
#define VP_PLAY_STRING_FONT_CHOCO_COOKY				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_CHOCO_COOKY_M_FONT"))
#define VP_PLAY_STRING_FONT_COOL_JAZZ				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_COOL_JAZZ_M_FONT"))
#define VP_PLAY_STRING_FONT_ROSEMARY				(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_ROSEMARY_M_FONT"))

#ifdef K_FEATURE_MICROSEEK
#define VP_PLAY_STRING_SCRUBBING_AT_PSX_SPEED		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_TPOP_SCRUBBING_AT_PSX_SPEED"))
#endif

#define VP_PLAY_STRING_PD_SECS		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_BODY_PD_SECS"))//not show in po

#ifdef _SUBTITLE_MULTI_LANGUAGE
#define VP_PLAY_STRING_SELECTED_LANGUAGE         	("Select at least one") //to support multi-language
#endif

#define VP_PLAY_STRING_TRIM_VIEW_MIN_TIME        	("Duration must be longer that one second") //trim view

#define VP_PLAY_STRING_UNABLE_TO_SCAN	("Unable to scan")
#define VP_PLAY_STRING_UNABLE_TO_SCAN_FOR_THE_DEVICE	("Unable to scan for the device currently connected via Screen Mirroring while Wi-Fi tethering is enabled")
#define VP_PLAY_STRING_DO_NOT_SHOW_AGAIN	("Do not show again")

#define VP_PLAY_STRING_VIA_SCREEN_MIRRORING		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_DLNA_OPT_VIA_SCREEN_MIRRORING_ABB"))//now show in po
#define VP_PLAY_STRING_VIA_HOME_NETWORK		(dgettext(VP_LOCAL_STR_PREFIX, "IDS_DLNA_OPT_VIA_HOME_NETWORK_ABB"))//now show in po

#define VP_PLAY_STRING_UNSUPPORTED_CODEC					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_THE_VIDEO_CODEC_IS_NOT_SUPPORTED"))
#define VP_PLAY_STRING_VIDEO_CANNOT_PLAY						(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_HEADER_CANNOT_PLAY_VIDEO_ABB"))
#define VP_PLAY_STRING_VIDEO_CANNOT_PLAY_SUBTITLE_ERROR	(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_AN_ERROR_OCCURRED_WITH_THE_SUBTITLES"))
#define VP_PLAY_STRING_VIDEO_CORRUPTED						(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_THIS_FILE_IS_CORRUPTED"))
#define VP_PLAY_STRING_VIDEO_CORRUPTED_OR_SDCARD_REMOVED	(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_POP_THE_FILE_IS_CORRUPTED_OR_THE_SD_CARD_HAS_BEEN_REMOVED"))


#define VP_PLAY_STRING_ERROR_ONLY_AUDIO_DATA_SUPPORTED			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_TPOP_ONLY_AUDIO_DATA_SUPPORTED_BY_THIS_FILE_NOW_PLAYING_AUDIO"))
#define VP_PLAY_STRING_ERROR_ONLY_VIDEO_DATA_SUPPORTED			(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VPL_TPOP_ONLY_VIDEO_DATA_SUPPORTED_BY_THIS_FILE_NOW_PLAYING_VIDEO"))

#define VP_PLAY_STRING_DETAIL_VALUE_UNKNOWN					("Unknown")
#define VP_PLAY_STRING_DETAIL_LAST_MODIFIED					(dgettext(VP_LOCAL_STR_PREFIX, "IDS_VIDEO_HEADER_LAST_MODIFIED_M_DATE"))

