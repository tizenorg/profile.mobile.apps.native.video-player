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


#ifndef _VIDEO_VALUE_DEFINE_
#define  _VIDEO_VALUE_DEFINE_

//PLAY_TIME_FORMAT
#define PLAY_TIME_FORMAT "02u:%02u:%02u"


//PLAY_TIME_ARGS
#define PLAY_TIME_ARGS(t) \
        (t) / (60 * 60), \
        ((t) / 60) % 60, \
        (t) % 60

#define PLAY_TIME_LENGTH 9

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define _EDJ(x) elm_layout_edje_get(x)


#ifdef PREFIX
#undef PREFIX
#endif

#ifdef LOCALEDIR
#undef LOCALEDIR
#endif

//#define VIDEO_UTIL_PERSONAL_HEAD_STR		"/opt/usr/media/PersonalPage"
#define VIDEO_UTIL_PERSONAL_HEAD_STR			"/opt/storage/PersonalStorage"

#define MP_UTIL_VCONF_KEY_PERSONAL_TAG			"memory/setting/personal"


#define PREFIX								"/usr/apps/org.tizen.videos"
#define EDJ_PATH							"/usr/apps/org.tizen.videos/res/edje"
#define LOCALEDIR							"/usr/apps/org.tizen.videos/res/locale"

#define VIDEO_BOOKMARK_DIR					"/opt"PREFIX"/data"
#define VIDEO_THUMB_DATA_DIR				"/opt/usr/media/.video_thumb"
#define VIDEO_TRIM_DATA_DIR					"/opt/usr/media/.video_thumb"
#define VIDEO_DOWNLOAD_FILE_DIR				"/opt/usr/media/Videos"
#define VIDEO_CHAPTER_VIEW_DATA_DIR			"/opt/usr/media/.video_thumb"

#define VIDEO_LIST_VIEW_ICON_VIDEO_DEFAULT	"player_library_list_thumbnail_default.png"
#define VIDEO_LIST_VIEW_ICON_SEARCH			"icon/T01_2_icon_search.png"

#define VIDEO_LIST_VIEW_ICON_VIDEO			"icon/T01_2_icon_video.png"
#define VIDEO_LIST_VIEW_NO_ITEM				"icon/T01-2_video_noitem.png"
#define VIDEO_SOFT_BACK_BUTTON 				"T01_icon_soft_back.png"
#define VIDEO_LIST_VIEW_PREVIEW_NO_ITEM				"icon/T01-2_video_thumbnail_default.png"
#define VIDEO_LIST_VIEW_ICON_SELECT_ALL				"icon/T02_icon_edit.png"
#define VIDEO_LIST_VIEW_ICON_PERSONAL_LOCK		"icon/T01_2_video_lock_45x45.png"
#define VIDEO_LIST_VIEW_ICON_DROPBOX				"icon/T01_2_thumbnail_icon_drop_box.png"
#define VIDEO_LIST_VIEW_ICON_REFRESH				"icon/T01_Device_icon_index_refresh_bg.png"
#define VIDEO_LIST_VIEW_ICON_REFRESH_FOCUS		"icon/T01_Device_icon_index_refresh_focus_ef.png"

//folder view
#define VIDEO_LIST_VIEW_ICON_FOLDER_PHONE			"icon/player_folder_list_icon_folder.png"
#define VIDEO_LIST_VIEW_ICON_FOLDER_PHONE_EF		"icon/player_folder_list_icon_folder_ef.png"
#define VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX			"icon/player_folder_list_icon_dropbox.png"
#define VIDEO_LIST_VIEW_ICON_FOLDER_DROPBOX_EF		"icon/player_folder_list_icon_dropbox_ef.png"
#define VIDEO_LIST_VIEW_ICON_FOLDER_EXTERNAL			"icon/T01_2_list_icon_inner_usb.png"
#define VIDEO_LIST_VIEW_ICON_FOLDER_SDCARD			"icon/T01_2_list_icon_sd_card.png"

#define	VIDEO_STORE_PATH					".video-store"

#define VIDEO_PLAYER_TOOLBAR_ICON_VIDEOS				"toolbar/T01-2_controlbar_icon_video.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_RECORDED			"toolbar/T01-2_controlbar_icon_recorded.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_BACK				"toolbar/T01-2_btn_toolbar_back.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_CANCEL			"toolbar/T01-2_icon_toolbar_cancel.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_DELETE				"toolbar/icon_delete.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_MOVE_TO_PERSONAL	"toolbar/more_button_icon_add_to_your_personal_storage.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_MOVE_TO_NORMAL	"toolbar/more_button_icon_remove_from_your_personal_storage.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_DONE				"toolbar/T01-2_icon_toolbar_done.png"


//Use for more button popup
#define VIDEO_PLAYER_TOOLBAR_ICON_SIGN_IN		"toolbar/icon_signin.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_DOWNLOAD		"toolbar/icon_download.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_REFRESH		"toolbar/icon_refresh.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_SORT_BY		"toolbar/icon_sort_by.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_VIEW_AS		"toolbar/icon_view_as.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_SHARE			"toolbar/icon_share_via.png"
#define VIDEO_PLAYER_TOOLBAR_ICON_NEAR_BY_DEVICES	"toolbar/icon_nearbydevices.png"


#define VIDEO_PLAYER_MAIN_EDJ				EDJ_PATH"/main.edj"
#define VIDEO_PLAYER_CONTROL_EDJ			EDJ_PATH"/control.edj"
#define VIDEO_PLAYER_PANEL_EDJ				EDJ_PATH"/panel.edj"
#define VIDEO_PLAYER_PROGRESS_EDJ			EDJ_PATH"/progress.edj"
#define VIDEO_PLAYER_TRIM_EDJ				EDJ_PATH"/trim.edj"
#define VIDEO_PLAYER_TRIM_ITEM_EDJ			EDJ_PATH"/trim_item.edj"
#define VIDEO_PLAYER_PROGRESS_TRIM_EDJ		EDJ_PATH"/progress_trim.edj"
#define VIDEO_PLAYER_ADD_TAG				EDJ_PATH"/add_tag.edj"
#define VIDEO_PLAYER_ANIMATION_EDJ			EDJ_PATH"/animation.edj"
#define VIDEO_PLAYER_DETAIL_EDJ				EDJ_PATH"/detail.edj"
#define VIDEO_PLAYER_SUBTITLE_EDJ			EDJ_PATH"/subtitle.edj"
#define VIDEO_PLAYER_SUBTITLE_SYNC_EDJ		EDJ_PATH"/subtitle_sync.edj"
#define VIDEO_PLAYER_MULTI_WIN_EDJ			EDJ_PATH"/multi_win.edj"
#define VIDEO_PLAYER_SETTING_EDJ			EDJ_PATH"/setting.edj"
#define VIDEO_PLAYER_LANDSCAPE_THUMB_EDJ	EDJ_PATH"/landscape_thumbnail.edj"
#define VIDEO_PLAYER_CHAPTER_VIEW_EDJ		EDJ_PATH"/vp-chapter-view.edj"
#define VIDEO_PLAYER_SPEEDBOX_VIEW_EDJ		EDJ_PATH"/vp-speedbox-popup.edj"
#define VIDEO_PLAYER_VOLUME_POPUP_EDJ		EDJ_PATH"/vp-volume-popup.edj"
#define VIDEO_PLAYER_REMOVE_LIST_EDJ		EDJ_PATH"/vp-remove-list-internal.edj"
#define VIDEO_PLAYER_LISTVIEW_EDJ			EDJ_PATH"/vp-main-listview-internal.edj"
#define VIDEO_PLAYER_ASF_DEVICE_LIST_EDJ	EDJ_PATH"/vp-device-list-internal.edj"
#define VIDEO_PLAYER_PREVIEW_MOVIE_EDJ		EDJ_PATH"/vp-main-listview-preview.edj"
#define VIDEO_PLAYER_SEARCH_VIEW_DIM_EDJ	EDJ_PATH"/vp-search-view-internal.edj"
#define VIDEO_PLAYER_IMAGE_NAME_EDJ			EDJ_PATH"/vp-images.edj"
#define VIDEO_PLAYER_STORE_IMAGE_EDJ			EDJ_PATH"/VppStoreImage.edj"
#define VIDEO_PLAYER_MOTION_POPUP_EDJ		EDJ_PATH"/vp-motion-popup.edj"
#define VIDEO_PLAYER_NO_ITEM_EDJ			EDJ_PATH"/vp-genlist-no-item.edj"
#define VIDEO_PLAYER_NUMBER_INFO_EDJ		EDJ_PATH"/vp-numberinfo.edj"

#define VIDEO_CUSTOM_THEME					EDJ_PATH"/vp-custom-winset-style.edj"
#define VIDEO_PLAYER_OPTION_BUTTON_EDJ		EDJ_PATH"/option_button.edj"
#define VIDEO_PLAYER_POPUP_PROGRESSBAR_EDJ	EDJ_PATH"/popup_progressbar.edj"
#define VIDEO_PLAYER_SEARCH_VIEW_LAYOUT_EDJ	EDJ_PATH"/search-view-layout.edj"
#define VIDEO_PLAYER_DOWNLOAD_VIEW_LOADING_EDJ	EDJ_PATH"/VppStorePopupProgressbar.edj"
#define VIDEO_PLAYER_OPTION_CTXPOPUP		EDJ_PATH"/vp-ctxpopup-option.edj"

#define VIDEO_PLAYER_THUMBNAIL_VIEW_EDJ		EDJ_PATH"/vp-thumbnail-view.edj"
#define VIDEO_BASIC_LAYOUT_EDJ				EDJ_PATH"/vp-main-layout.edj"
#define VIDEO_BASIC_NAVIFRAME_EDJ			EDJ_PATH"/vp-naviframe.edj"

#define MAIN_EDJ_GROUP							"main"
#define CONTROL_EDJ_GROUP						"control"
#define PROGRESS_EDJ_GROUP						"progress"
#define BOOKMARK_EDJ_GROUP						"bookmark"
#define BOOKMARK_ITEM_EDJ_GROUP					"bookmark.item"
#define BOOKMARK_REMOVE_EDJ_GROUP				"bookmark.end"
#define TRIM_EDJ_GROUP							"trim.control"
#define TRIM_ITEM_EDJ_GROUP						"trim.item"
#define CHAPTER_VIEW_EDJ_GROUP					"chapter"
#define SPEEDBOX_EDJ_GROUP						"speedbox"
#define VOLUME_EDJ_GROUP						"volume"

#define PANEL_EDJ_GROUP							"panel"
#define	FAVORITE_EDJ_GROUP						"favorite"
#define ANIMATION_EDJ_GROUP						"animation"
#define DETAIL_EDJ_GROUP						"detail"
#define SUBTITLE_EDJ_GROUP						"subtitle"
#define SUBTITLE_SYNC_EDJ_GROUP					"subtitle.sync"
#define PANEL_BUTTON_EDJ_GROUP					"panel.button"
#define OPTION_EDJ_GROUP						"option"
//#define LISTVIEW_EDJ_GROUP					"listview"
#define MULTIWIN_EDJ_GROUP						"mw"
#define SETTING_EDJ_GROUP						"setting"
#define LANDSCAPE_THUMB_EDJ_GROUP				"landscape.thumb"
#define SEARCH_VIEW_LAYOUT_GROUP				"video-player.search-view.layout"
#define REMOVE_LIST_EDJ_GROUP					"main.listview.internal"
#define LISTVIEW_EDJ_GROUP						"main.listview.internal"
#define ASF_DEVICE_LIST_EDJ_GROUP				"asf.device.list.internal"
#define PREVIEW_MOVIE_EDJ_GROUP					"listview.preview.movie"
#define PREVIEW_MOVIE_LANDSCAPE_EDJ_GROUP		"listview.preview.movie.landscape"
#define SEARCH_VIEW_DIM_EDJ_GROUP				"search.view.internal"
#define GENLIST_NOITEM_EDJ_GROUP                "listview.genlist.noitem"
#define GENLIST_NOITEM_SIGNIN_EDJ_GROUP			"listview.genlist.noitem.signin"
#define PREVIEW_MOVIE_NOCOTENT__EDJ_GROUP_L		"listview.preview.movie.landspace.nocontent"
#define PREVIEW_MOVIE_NOCOTENT_EDJ_GROUP		"listview.preview.movie.nocontent"



#define SWALLOW_MAIN_CONTROL					"main.panel"
#define SWALLOW_MAIN_BOOKMARK					"main.bookmark"
#define SWALLOW_MAIN_ANIMATION					"main.animation"
#define SWALLOW_MAIN_PANEL_BUTTON				"main.panel.button"
#define SWALLOW_MAIN_OPTION_BUTTON				"main.option.button"
#define SWALLOW_SETTING_LISTVIEW				"setting.list"

#define SWALLOW_BOOKMARK_GRID					"bookmark.swallow"
#define SWALLOW_PANEL_CONTROL					"panel.control"
#define SWALLOW_PANEL_PROGRESS_BAR				"panel.progress"
#define SWALLOW_DETAIL_LIST						"detail.list"
#define SWALLOW_TRIM_THUMBNAIL					"trim.control.thumb"
#define SWALLOW_LISTVIEW_CONTENT				"main.listview.content"
#define SWALLOW_ASF_DEVICE_LIST_CONTENT			"asf.device.list.content"
#define SWALLOW_LISTVIEW_SELECTION_INFO			"main.listview.selection.info"
#define SWALLOW_LISTVIEW_PREVIEW_MOVIE_PIXMAP	"listview.preview.movie.pixmap"
#define SWALLOW_LISTVIEW_PREVIEW_PROGRESS_BAR	"listview.preview.movie.progressbar"
#define SWALLOW_LISTVIEW_NO_ITEM_BUTTON			"listview.genlist.noitem.button"
#define SWALLOW_LISTVIEW_NO_ITEM_SIGNIN_BUTTON	"listview.genlist.noitem.signin.button"
#define SWALLOW_LISTVIEW_NO_ITEM_TEXT			"listview.genlist.noitem.text"
#define SWALLOW_LISTVIEW_NO_ITEM_SIGNIN_TEXT	"listview.genlist.noitem.signin.text"
#define SWALLOW_LISTVIEW_PREVIEW_LANDSCAPE_PROGRESS_BAR			"listview.preview.movie.landscape.progressbar"
#define SWALLOW_LISTVIEW_PREVIEW_LANDSCAPE_MOVIE_PIXMAP			"listview.preview.movie.landscape.pixmap"
#define SWALLOW_LISTVIEW_PREVIEW_NOCOTENT_PIXMAP				"listview.preview.movie.nocontent.pixmap"

#define SWALLOW_LISTVIEW_PROGRESS_CONTENT		"main.listview.progress"






#define PART_BUTTON_ICON 						"icon"
#define MAIN_SPEED_TEXT							"main.speedbox.txt"
#define SUBTITLE_TEXT							"subtitle.txt"

#define PART_MAIN_ROTATE_BUTTON					"main.rotate.button"

#define SIGNAL_MAIN_ROTATE_BUTTON_SHOW			"signal.main.rotate.button.show"
#define SIGNAL_MAIN_ROTATE_BUTTON_HIDE			"signal.main.rotate.button.hide"

#define SIGNAL_MAIN_ROTATE_BUTTON_CLICK			"signal.main.rotate.button.clicked"

#define SIGNAL_CONTROL_PART_PLAY_BUTTON			"control.play.button"
#define SIGNAL_CONTROL_PART_BACK_BUTTON			"control.back.button"
#define SIGNAL_CONTROL_PART_VOL_BUTTON			"control.volume.button"
#define SIGNAL_CONTROL_PART_MUTE_BUTTON			"control.volume.mute.button"

#define SIGNAL_CONTROL_PART_SOUNDPATH_BUTTON	"control.soundpath.button"
#define SIGNAL_CONTROL_PART_SCREENSIZE_BUTTON	"control.screensize.button"
#define SIGNAL_CONTROL_PART_FF_BUTTON			"control.normal.ff.button"
#define SIGNAL_CONTROL_PART_REW_BUTTON			"control.normal.rew.button"
#define SIGNAL_CONTROL_PART_TRIM_FF_BUTTON		"control.trim.ff.button"
#define SIGNAL_CONTROL_PART_TRIM_REW_BUTTON		"control.trim.rew.button"

#define PART_CONTROL_TRIM_DONE_BUTTON			"control.trim.done.button"
#define PART_CONTROL_TRIM_CANCEL_BUTTON			"control.trim.cancel.button"
#define PART_CONTROL_SPEED_BOX_REW				"control.normal.rew.speed.swallow"
#define PART_CONTROL_SPEED_BOX_FF				"control.normal.ff.speed.swallow"

#define PART_BOOKMARK_ADD						"bookmark.add"
#define PART_BOOKMARK_DEL						"bookmark.end.btn"

#define SIGNAL_VOLUME_PORTRAIT_MODE				"signal.volume.potrait.mode"
#define SIGNAL_VOLUME_LANDSCAPE_MODE			"signal.volume.landscape.mode"

#define SIGNAL_SPEEDBOX_SET_MODE_1				"signal.speedbox.set.mode.1"
#define SIGNAL_SPEEDBOX_SET_MODE_2				"signal.speedbox.set.mode.2"
#define SIGNAL_SPEEDBOX_SET_MODE_3				"signal.speedbox.set.mode.3"

#define SIGNAL_SPEEDBOX_SHOW					"signal.speedbox.visible.on"
#define SIGNAL_SPEEDBOX_HIDE					"signal.speedbox.visible.off"

#define SIGNAL_CONTROL_MODE_NORMAL				"signal.control.mode.normal"
#define SIGNAL_CONTROL_MODE_BOOKMARK			"signal.control.mode.bookmark"
#define SIGNAL_CONTROL_MODE_TRIM				"signal.control.mode.trim"
#define SIGNAL_CONTROL_MODE_STREAMING			"signal.control.mode.streaming"
#define SIGNAL_CONTROL_MODE_DLNA				"signal.control.mode.dlna"
#define SIGNAL_CONTROL_MODE_VISIBLE_OFF			"signal.control.mode.visible.off"
#define SIGNAL_CONTROL_BACK_BUTTON_UP			"signal.control.back.button.up"

#define SIGNAL_CONTROL_VOL_BUTTON_DOWN			"signal.control.volume.button.down"
#define SIGNAL_CONTROL_VOL_BUTTON_UP			"signal.control.volume.button.up"

#define SIGNAL_CONTROL_MUTE_BUTTON_DOWN			"signal.control.volume.mute.button.down"
#define SIGNAL_CONTROL_MUTE_BUTTON_UP			"signal.control.volume.mute.button.up"

#define SIGNAL_CONTROL_SOUNDPATH_BUTTON_DOWN		"signal.control.soundpath.button.down"
#define SIGNAL_CONTROL_SOUNDPATH_BUTTON_UP			"signal.control.soundpath.button.up"
#define SIGNAL_CONTROL_SCREENSIZE_NORMAL			"signal.control.screensize.normal"
#define SIGNAL_CONTROL_SCREENSIZE_EXPAND			"signal.control.screensize.expand"
#define SIGNAL_CONTROL_NORMAL_FF_BUTTON_DOWN		"signal.control.normal.ff.button.down"
#define SIGNAL_CONTROL_NORMAL_FF_BUTTON_UP		"signal.control.normal.ff.button.up"
#define SIGNAL_CONTROL_NORMAL_REW_BUTTON_DOWN		"signal.control.normal.rew.button.down"
#define SIGNAL_CONTROL_NORMAL_REW_BUTTON_UP		"signal.control.normal.rew.button.up"
//#define SIGNAL_CONTROL_NORMAL_FF_STEP			"signal.control.normal.ff.button.step"
//#define SIGNAL_CONTROL_NORMAL_REW_STEP			"signal.control.normal.rew.button.step"
//#define SIGNAL_CONTROL_NORMAL_FF_LANDSCAPE_STEP		"signal.control.normal.ff.button.landscape.step"
//#define SIGNAL_CONTROL_NORMAL_REW_LANDSCAPE_STEP	"signal.control.normal.rew.button.landscape.step"
//#define SIGNAL_CONTROL_TRIM_FF_STEP					"signal.control.trim.ff.button.step"
//#define SIGNAL_CONTROL_TRIM_REW_STEP				"signal.control.trim.rew.button.step"

#define SIGNAL_PANEL_BUTTON_SA_DOWN					"signal.panel.button.sa.down"
#define SIGNAL_PANEL_BUTTON_SA_UP					"signal.panel.button.sa.up"
#define SIGNAL_PANEL_BUTTON_REPEAT_DOWN				"signal.panel.button.repeat.down"
#define SIGNAL_PANEL_BUTTON_REPEAT_UP				"signal.panel.button.repeat.up"

#define SIGNAL_BOOKMARK_ADD_DOWN					"signal.bookmark.add.down"
#define SIGNAL_BOOKMARK_ADD_UP						"signal.bookmark.add.up"

#define SIGNAL_BOOKMARK_EXPAND						"signal.bookmark.expand"
#define SIGNAL_BOOKMARK_NORMAL						"signal.bookmark.normal"
#define SIGNAL_BOOKMARK_DEL_DOWN					"signal.bookmark.end.btn.down"

#define SIGNAL_CONTROL_TRIM_DONE_BUTTON_DOWN		"signal.control.trim.done.button.down"
#define SIGNAL_CONTROL_TRIM_CANCEL_BUTTON_DOWN		"signal.control.trim.cancel.button.down"

#define SIGNAL_CONTROL_TRIM_FF_BUTTON_DOWN			"signal.control.trim.ff.button.down"
#define SIGNAL_CONTROL_TRIM_FF_BUTTON_UP			"signal.control.trim.ff.button.up"
#define SIGNAL_CONTROL_TRIM_REW_BUTTON_DOWN			"signal.control.trim.rew.button.down"
#define SIGNAL_CONTROL_TRIM_REW_BUTTON_UP			"signal.control.trim.rew.button.up"

#define SIGNAL_CONTROL_SOUNDPATH_BUTTON_ACTIVE		"signal.control.soundpath.button.active"
#define SIGNAL_CONTROL_SOUNDPATH_BUTTON_UNACTIVE	"signal.control.soundpath.button.unactive"

#define PART_OPTION_SCREEN_SIZE				"option.screensize"
#define PART_OPTION_SHARE					"option.share"
#define PART_OPTION_BOOKMARK				"option.bookmark"
#define	PART_OPTION_MULTIWIN				"option.multiwin"
#define PART_OPTION_MORE					"option.more"
#define PART_OPTION_SOUND_ALIVE				"option.sa"
#define PART_OPTION_TRIM					"option.trim"
#define PART_OPTION_REPEAT					"option.repeat"
#define PART_OPTION_CONNECT_INFO			"option.connectinfo"

#define SIGNAL_OPTION_SHARE_CLICKED			"signal.option.share.clicked"
#define SIGNAL_OPTION_BOOKMARK_CLICKED			"signal.option.bookmark.clicked"
#define SIGNAL_OPTION_MULTIWIN_CLICKED			"signal.option.multiwin.clicked"
#define SIGNAL_OPTION_MORE_CLICKED			"signal.option.more.clicked"
#define SIGNAL_OPTION_SOUND_ALIVE_CLICKED		"signal.option.sa.clicked"
#define SIGNAL_OPTION_TRIM_CLICKED			"signal.option.trim.clicked"
#define SIGNAL_OPTION_REPEAT_UP				"signal.option.repeat.clicked"
#define SIGNAL_OPTION_CONNECT_INFO_UP			"signal.option.connectinfo.clicked"

#define SIGNAL_LAYOUT_PIXMAP_BG_SHOW		"signal.layout.pixmap.bg.show"
#define SIGNAL_LAYOUT_PIXMAP_BG_HIDE		"signal.layout.pixmap.bg.hide"

#define SIGNAL_LAYOUT_INDICATOR_SHOW		"signal.layout.indicator.show"
#define SIGNAL_LAYOUT_INDICATOR_HIDE		"signal.layout.indicator.hide"

#define PART_MW_CLOSE					"mw.close"
#define PART_MW_PANEL_HIDE				"mw.panel.hide"
#define PART_MW_PANEL_SHOW				"mw.panel.show"
#define PART_MW_FULL_SIZE				"mw.full.size"
#define PART_MW_REW						"mw.control.rew"
#define PART_MW_PLAY					"mw.control.play"
#define PART_MW_PAUSE					"mw.control.pause"
#define PART_MW_FF						"mw.control.ff"
#define PART_MW_RESIZE					"mw.resize"

#define SIGNAL_CONTORL_SHOW_FINISH			"signal.control.show.finish"
#define SIGNAL_CONTORL_HIDE_FINISH			"signal.control.hide.finish"

#define SIGNAL_NAVIFRAME_TITLE_SHOW			"elm,state,title,show,custom"
#define SIGNAL_NAVIFRAME_TITLE_HIDE			"elm,state,title,hide,custom"

#define SIGNAL_MAIN_CONTROL_PORTRAIT_SHOW		"signal.main.control.portrait.show"
#define SIGNAL_MAIN_CONTROL_LANDSCAPE_SHOW		"signal.main.control.landscape.show"

#define SIGNAL_MAIN_CONTROL_NORMAL_HIDE			"signal.main.control.hide"
#define SIGNAL_MAIN_CONTROL_NO_EFFECT_HIDE		"signal.main.control.no.effect.hide"
#define SIGNAL_MAIN_CONTROL_LANDSCAPE_BOOKMAKR_HIDE	"signal.main.control.hide.with.bookmark"

#define SIGNAL_MW_CLOSE_CLICKED				"signal.mw.close.clicked"
#define SIGNAL_MW_PANEL_HIDE_CLICKED			"signal.mw.panel.hide.clicked"
#define SIGNAL_MW_PANEL_SHOW_CLICKED			"signal.mw.panel.show.clicked"
#define SIGNAL_MW_FULL_SIZE_CLICKED			"signal.mw.full.size.clicked"
#define SIGNAL_MW_REW_CLICKED				"signal.mw.control.rew.clicked"
#define SIGNAL_MW_PLAY_CLICKED				"signal.mw.control.play.clicked"
#define SIGNAL_MW_PAUSE_CLICKED				"signal.mw.control.pause.clicked"
#define SIGNAL_MW_FF_CLICKED				"signal.mw.control.ff.clicked"
#define SIGNAL_MW_RESIZE_DOWN				"signal.mw.resize.down"

#define SIGNAL_MW_HIDE_CONTROL				"signal.mw.hide.control"
#define SIGNAL_MW_SHOW_CONTROL				"signal.mw.show.control"

#define SIGNAL_MW_SET_PAUSE_STATUS			"signal.mw.set.pause.status"
#define SIGNAL_MW_SET_PLAY_STATUS			"signal.mw.set.play.status"

#define SIGNAL_MW_PIXMAP_SHOW				"signal.mw.image.show"
#define SIGNAL_MW_PIXMAP_HIDE				"signal.mw.image.hide"

#define SIGNAL_MW_PIXMAP_ALPHA_SET			"signal.mw.image.alpha.set"
#define SIGNAL_MW_PIXMAP_ALPHA_UNSET			"signal.mw.image.alpha.unset"

#define SIGNAL_OPTION_SELECT_BG_CLEAR			"signal.option.select.bg.clear"

#define SIGNAL_OPTION_SCREEN_SIZE_NORMAL		"signal.option.screensize.normal"
#define SIGNAL_OPTION_SCREEN_SIZE_FULL			"signal.option.screensize.full"
#define SIGNAL_OPTION_SCREEN_SIZE_ORIGIN		"signal.option.screensize.origin"

#define SIGNAL_OPTION_REPEAT_ON				"signal.option.repeat.on"
#define SIGNAL_OPTION_REPEAT_OFF			"signal.option.repeat.off"

#define SIGNAL_OPTION_MODE_LOCAL			"signal.option.mode.local"
#define SIGNAL_OPTION_MODE_MYFILE			"signal.option.mode.myfile"
#define SIGNAL_OPTION_MODE_EMAIL			"signal.option.mode.email"
#define SIGNAL_OPTION_MODE_WEB				"signal.option.mode.web"
#define SIGNAL_OPTION_MODE_MESSAGE			"signal.option.mode.message"

#define SIGNAL_PANEL_BUTTON_REPEAT_OFF			"signal.panel.button.repeat.off"
#define SIGNAL_PANEL_BUTTON_REPEAT_ONE			"signal.panel.button.repeat.one"
#define SIGNAL_PANEL_BUTTON_REPEAT_ALL			"signal.panel.button.repeat.all"

#define SIGNAL_NAVIFRAME_PORTRATE_MODE			"signal.naviframe.portrate.mode"
#define SIGNAL_NAVIFRAME_LANDSCAPE_MODE			"signal.naviframe.landscape.mode"

#define SIGNAL_MAIN_PORTRATE_MODE			"signal.main.portrate.mode"
#define SIGNAL_MAIN_LANDSCAPE_MODE			"signal.main.landscape.mode"
#define SIGNAL_MAIN_TRIM_MODE				"signal.main.trim.mode"

#define SIGNAL_MAIN_LANDSCAPE_BOOKMAKR_SHOW		"signal.main.landscape.bookmark.show"
#define SIGNAL_MAIN_LANDSCAPE_BOOKMAKR_HIDE		"signal.main.landscape.bookmark.hide"

#define SIGNAL_PANEL_PORTRATE_MODE			"signal.panel.portrate.mode"
#define SIGNAL_PANEL_LANDSCAPE_MODE			"signal.panel.landscape.mode"
#define SIGNAL_PANEL_TRIM_MODE				"signal.panel.trim.mode"

#define SIGNAL_PROGRESS_PORTRATE_MODE			"signal.progress.portrate.mode"
#define SIGNAL_PROGRESS_LANDSCAPE_MODE			"signal.progress.landscape.mode"

#define SIGNAL_PROGRESS_POINTER_SHOW			"signal.progress.pointer.show"
#define SIGNAL_PROGRESS_POINTER_HIDE			"signal.progress.pointer.hide"

#define SIGNAL_PROGRESS_ADJUSTMENT_SHOW			"signal.progress.adjustment.show"
#define SIGNAL_PROGRESS_ADJUSTMENT_HIDE			"signal.progress.adjustment.hide"

#define SIGNAL_CONTROL_PORTRATE_MODE			"signal.control.portrate.mode"
#define SIGNAL_CONTROL_LANDSCAPE_MODE			"signal.control.landscape.mode"

#define SIGNAL_CONTROL_STREAMING_PORTRATE_MODE		"signal.control.streaming.portrate.mode"
#define SIGNAL_CONTROL_STREAMING_LANDSCAPE_MODE		"signal.control.streaming.landscape.mode"

#define SIGNAL_CONTROL_SET_MUTE				"signal.control.volume.set.mute"
#define SIGNAL_CONTROL_UNSET_MUTE			"signal.control.volume.unset.mute"

#define SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_SHOW	"signal.search.view.internal.dimming.show"
#define SIGNAL_SEARCH_VIEW_INTERNAL_DIMMING_HIDE	"signal.search.view.internal.dimming.hide"


// Callback signals.
#define SIGNAL_CONTROL_PLAY_BUTTON_DOWN			"signal.control.play.button.down"
#define SIGNAL_CONTROL_PLAY_BUTTON_UP			"signal.control.play.button.up"
#define SIGNAL_CONTROL_PLAY_PAUSE_PRESS			"signal.control.play.pause.press"
#define SIGNAL_CONTROL_PLAY_RESUME_PRESS		"signal.control.play.resume.press"

// Emit signals.
#define SIGNAL_CONTROL_PLAY_RESUME			"signal.control.play.resume"
#define SIGNAL_CONTROL_PLAY_PAUSE			"signal.control.play.pause"

#define SIGNAL_MAIN_MUTE_SHOW				"signal.main.mute.show"
#define SIGNAL_MAIN_MUTE_HIDE				"signal.main.mute.hide"

#define SIGNAL_MAIN_NOCONTENTS_SHOW			"signal.main.nocontents.show"
#define SIGNAL_MAIN_NOCONTENTS_HIDE			"signal.main.nocontents.hide"

#define SIGNAL_MAIN_SPEEDBOX_SHOW			"signal.main.speedbox.show"
#define SIGNAL_MAIN_SPEEDBOX_HIDE			"signal.main.speedbox.hide"

#define SIGNAL_MAIN_SCREEN_TOUCH_TOGGLE		"main.screen.touch.toggle"
#define SIGNAL_MAIN_ANIMATION_SHOW			"signal.main.animation.show"
#define SIGNAL_MAIN_ANIMATION_HIDE			"signal.main.animation.hide"

#define SIGNAL_ANIMATION_IMAGE_LOADING		"signal.loading.image.step"
#define SIGNAL_SUBTITLE_PORTRATE_MODE		"signal.subtitle.portrate.mode"
#define SIGNAL_SUBTITLE_LANDSCAPE_MODE		"signal.subtitle.landscape.mode"

#define SIGNAL_SUBTITLE_TXT_SHOW			"signal.subtitle.txt.show"
#define SIGNAL_SUBTITLE_TXT_HIDE			"signal.subtitle.txt.hide"

#define SIGNAL_SUBTITLE_LAYOUT_SHOW 		"signal.subtitle.layout.show"
#define SIGNAL_SUBTITLE_LAYOUT_HIDE 		"signal.subtitle.layout.hide"

#define SIGNAL_SUBTITLE_SIZE_SMALL			"signal.subtitle.size.small"
#define SIGNAL_SUBTITLE_SIZE_MEDIUM			"signal.subtitle.size.medium"
#define SIGNAL_SUBTITLE_SIZE_LARGE			"signal.subtitle.size.large"

#define SIGNAL_SUBTITLE_SYNC_SHOW			"signal.subtitle.sync.show"
#define SIGNAL_SUBTITLE_SYNC_HIDE			"signal.subtitle.sync.hide"


#define PART_SUBTITLE_SYNC_SLOW_BUTTON		"subtitle.sync.slow.button"
#define PART_SUBTITLE_SYNC_FAST_BUTTON		"subtitle.sync.fast.button"
#define PART_SUBTITLE_SYNC_TXT				"subtitle.sync.txt"

#define SIGNAL_SUBTITLE_SYNC_SLOW_DOWN		"signal.subtitle.sync.slow.button.down"
#define SIGNAL_SUBTITLE_SYNC_FAST_DOWN		"signal.subtitle.sync.fast.button.down"


#define SIGNAL_LOCKSCREEN_SHOW				"signal.lockscreen.show"
#define SIGNAL_LOCKSCREEN_HIDE				"signal.lockscreen.hide"


//#define SIGNAL_LISTVIEW_PORTRAIT_PREVIEW_SHOW		"signal.listview.preview.show"
//#define SIGNAL_LISTVIEW_PORTRAIT_PREVIEW_HIDE		"signal.listview.preview.hide"

#define SIGNAL_LISTVIEW_SELECTION_INFO_SHOW			"signal.main.listview.internal.selection.info.show"
#define SIGNAL_LISTVIEW_SELECTION_INFO_HIDE			"signal.main.listview.internal.selection.info.hide"

#define SIGNAL_ASF_VDIEO_ITEM_SELECTION_INFO_SHOW		"signal.asf.device.internal.selection.info.show"
#define SIGNAL_ASF_VIDEO_ITEM_SELECTION_INFO_HIDE		"signal.asf.device.internal.selection.info.hide"

#define SIGNAL_LISTVIEW_PREVIEW_MOVIE_EVENT_CLICKED		"signal.listview.preview.movie.event.clicked"

#define SIGNAL_LANDSCAPE_THUMB_PREVIEW_SHOW			"signal.landscape.thumb.preview.show"
#define SIGNAL_LANDSCAPE_THUMB_PREVIEW_HIDE			"signal.landscape.thumb.preview.hide"

#define SIGNAL_MOUSE_DOWN				"mouse,down,1"
#define SIGNAL_MOUSE_UP					"mouse,up,1"
#define SIGNAL_MOUSE_CLICK				"mouse,clicked,1"

//download
#define MP_DOWNLOAD_BUTTON_STYLE            "video/circle"

// Multi Window value
#define MP_MULTI_WINDOW_STARTUP_TYPE		"window_startup_type"
#define MP_MULTI_WINDOW_LAYOUT_ID			"window_layout_id"
#define MP_MULTI_WINDOW_OPERATION			"http://tizen.org/appcontrol/operation/main"

#define TAG_FAVOURITES					"Favourites"
#ifndef PACKAGE
#define PACKAGE "video-player"
#endif
#define PAUSE_KEY						"XF86AudioPause"
#define PLAY_KEY						"XF86AudioPlay"
#define STOP_KEY						"XF86AudioStop"
#define NEXT_KEY						"XF86AudioNext"
#define PREV_KEY						"XF86AudioPrev"
#define FORWARD_KEY						"XF86AudioForward"
#define REWIND_KEY						"XF86AudioRewind"
#define END_KEY							"XF86Stop"
#define HOME_KEY						"XF86Home"
#define KEY_CANCLE						"Cancel"
#define POWER_KEY						"XF86PowerOff"

#define VOLUME_DOWN_KEY					"XF86AudioLowerVolume"
#define VOLUME_UP_KEY					"XF86AudioRaiseVolume"

#define VOLUME_MUTE						0
#define KEY_GRAB_SHARED					SHARED_GRAB
#define KEY_GRAB_EXCLUSIVE				EXCLUSIVE_GRAB
#define KEY_GRAB_TOP_POSITION			TOP_POSITION_GRAB
#define KEY_GRAB_OR_EXCLUSIVE			OR_EXCLUSIVE_GRAB
#define STR_LEN_MAX						4096
#define SVC_LEN_MAX						2048
#define FORMAT_LEN_MAX					100


#define	XML_ITEM_NUM					6
#define _WIFI_MODE_						1
#define _3G_MODE_						2
#define USE_STREAMING_ANI				1
#define LONG_PRESS_INTERVAL				1.0
#define DOUBLE_CLICK_TIME				0.2

#define	SCREEN_TOUCH_TOGGLE_AREA_Y_START 0.065
#define	SCREEN_TOUCH_TOGGLE_AREA_Y_END	0.78

#define	TAG_LIST_SIZE					256

#define ANIMATION_TIMER_INTERVAL		0.05
#define ANIMATION_MAX_COUNT				30

#define BOOSTER_STEP_DISTANCE			99
#define TRIM_DETAIL_STEP_DISTANCE		30

#define SIGNAL_TEXT_MAX_LEN				64
#define CAPTION_TEXT_MAX_LEN			16
#define STR_UUID_LEN_MAX				37

#define DEFAULT_SCALE_WIDTH			720
#define DEFAULT_SCALE_HEIGHT			1280

#define DEFAULT_MW_WIDTH				496
#define DEFAULT_MW_HEIGHT				286

#define VIDEO_ICON_WIDTH				70	//90	//168
#define VIDEO_ICON_HEIGHT				45	//54	//124

#define VIDEO_LIST_PROGRESSBAR_LENGTH	70

#define NO_ITEM_GENLIST_WIDTH			720
#define NO_ITEM_GENLIST_HEIGHT			690

#define VIDEO_THUMB_WIDTH				176
#define VIDEO_THUMB_HEIGHT			144

#define VIDEO_PREVIEW_WIDTH			320
#define VIDEO_PREVIEW_HEIGHT			240

#define VIDEO_NUMBER_INFO_BAR_Y			1115

#define NUMBER_INFO_BAR_HEIGHT			57

#define VIDEO_POPUP_TITLE_H				72
#define VIDEO_POPUP_1_ITEM_H			46	//112
#define VIDEO_POPUP_2_ITEM_H			84	//135

#define VIDEO_POPUP_MAX_HEIGHT		752-72	//728
#define VIDEO_POPUP_MAX_HEIGHT_L		432-72	//392

#define VIDEO_POPUP_MIN_ITEMS			4

#define VIDEO_POPUP_LIST_HEIGHT(count) 			(((VIDEO_POPUP_1_ITEM_H*count) > VIDEO_POPUP_MAX_HEIGHT) ? (VIDEO_POPUP_MAX_HEIGHT) : (VIDEO_POPUP_1_ITEM_H*count))	//(((112*count) > 728) ? (728) : (112*count))
#define VIDEO_POPUP_LIST_HEIGHT_L(count) 		(((VIDEO_POPUP_1_ITEM_H*count-1) > VIDEO_POPUP_MAX_HEIGHT_L) ? (VIDEO_POPUP_MAX_HEIGHT_L) : (VIDEO_POPUP_1_ITEM_H*count-1))

#define VIDEO_POPUP_2_TEXT_HEIGHT(count) 		(((VIDEO_POPUP_2_ITEM_H*count-1) > VIDEO_POPUP_MAX_HEIGHT) ? (VIDEO_POPUP_MAX_HEIGHT) : (VIDEO_POPUP_2_ITEM_H*count-1))
#define VIDEO_POPUP_2_TEXT_HEIGHT_L(count) 		(((VIDEO_POPUP_2_ITEM_H*count-1) > VIDEO_POPUP_MAX_HEIGHT_L) ? (VIDEO_POPUP_MAX_HEIGHT_L) : (VIDEO_POPUP_2_ITEM_H*count-1))


#define VIDEO_POPUP_ITEM_W				120	//409	//614
#define VIDEO_POPUP_ITEM_LADSCAPE_W		480	//555	//832

#define VIDEO_POPUP_DETAIL_ITEM_W		367	//550

#define VIDEO_FILE_ENTRY_LEN_MAX		255
#define VIDEO_FILE_SEARCH_CHAR_LEN_MAX	50
#define VIDEO_CTXPOPUP_ITEM_W			500
#define VIDEO_CTXPOPUP_TITLE_H			54


#define PREVIEW_MOVIE_WIDTH					720
#define PREVIEW_MOVIE_HEIGHT				406
#define PREVIEW_MOVIE_WIDTH_LANDSCAPE		1280
#define PREVIEW_MOVIE_HEIGHT_LANDSCAPE		320
#define PREVIEW_MOVIE_NOCONTENT_LANDSCAPE	640
#define NO_CONTENTS_WIDTH				720
#define NO_CONTENTS_HEIGHT				897
#define VIDEO_MAIN_LIST_BLOCK_COUNT		12
#define VIDEO_GENLIST_BLOCK_COUNT		12
#define VIDEO_PERSONAL_UPDATE_TIMER		1.0

#define VIDEO_PREVIEW_WIDTH_ICON_LANDSCAPE	640
#define VIDEO_PREVIEW_HEIGHT_ICON_LANDSCAPE	320


//thumbnail view
#define	VP_GENLIST_THUMB_PORTRAIT_COUNT			2
#define VP_GENLIST_THUMB_LANDSPACE_COUNT		4
#define	VP_GENLIST_THUMB_PORTRAIT_COUNT_ZOOM_IN			1
#define VP_GENLIST_THUMB_LANDSPACE_COUNT_ZOOM_IN		2
#define VP_ALBUM_THUMB_ICON_HEIGHT				150
#define VP_ALBUM_THUMB_ICON_HEIGHT_ZOOM_IN			225
#define GRID_VIDEO_ICON_WIDTH					150
#define GRID_VIDEO_ICON_HEIGHT					210
#define GRID_VIDEO_TAB_PAD_HEIGHT				12

#define VIDEO_GENGRID_ICON_WIDTH				150

//landspace
#define VP_ALBUM_THUMB_ICON_HEIGHT_LAND			150
#define GRID_VIDEO_ICON_WIDTH_LAND				314
#define GRID_VIDEO_ICON_HEIGHT_LAND				186

#define VIDEO_GENGRID_ICON_WIDTH_LAND				132

//video wall
#define MP_VIDEO_WALL_START_POSITION		0
#define MP_VIDEO_WALL_DIR					"/opt/usr/media/.video_wall"
#define	MP_VIDEO_WALL_FRAME_FILE_PREFIX		"%s/.frame_buff.jpg"
#define	MP_VIDEO_WALL_GEN_FILE_NAME_PREFIX		"%s/preview.3gp"

#define VIDEO_SHARE_OPERATION_SINGLE "http://tizen.org/appcontrol/operation/share"
#define VIDEO_SHARE_OPERATION_MULTIPLE "http://tizen.org/appcontrol/operation/multi_share"
#define VIDEO_SHARE_SVC_FILE_PATH "http://tizen.org/appcontrol/data/path"

#define MP_VIDEO_WALL_DURATION				10000
#define MP_VIDEO_WALL_DEFAULT_FPS			10


#define GRID_VIDEO_ICON_WIDTH_T					352
#define GRID_VIDEO_ICON_HEIGHT_T				192

#define VIDEO_DETAIL_VALUE_GPS_DEFAULT	-200.0


#endif //_VIDEO_VALUE_DEFINE_

