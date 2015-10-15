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

#include <app.h>
#include <glib.h>
#include <unistd.h>
#include <stdbool.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "mp-util-media-service.h"
#include "mp-video-log.h"
#include "mp-util.h"
#include "mp-video-list-view-thumbnail.h"

typedef struct _vp_gesture_t vp_gesture_s;
typedef struct _vp_pinch_event_t vp_pinch_event_s;
typedef enum _vp_pinch_plan_t vp_pinch_plan_e;
typedef Eina_Bool (*vp_gesture_cb) (Evas_Object *gesture, void *data);

struct _vp_gesture_t {
	st_VideoListViewMainViewWidget *ad;
	Evas_Object *parent;
	Evas_Object *gesture;
	vp_gesture_cb zoom_in_cb;
	vp_gesture_cb zoom_out_cb;
	void *zoom_in_data;
	void *zoom_out_data;

	int dis_old;
	int pinch_dis_old;
	Eina_List *s_event_elist;
	Ecore_Job *pinch_job;
	int next_plan;
};

struct _vp_pinch_event_t {
	int device;

	struct vp_prev {
		Evas_Coord x;
		Evas_Coord y;
	} prev;

	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;
	Evas_Object *object;
	Ecore_Timer *hold_timer;
	int ts;		/* Time stamp to calculate whether scrolling or moving */
	int v;		/* Velocity */
	int pinch_dis;
	Evas_Object *pinch_obj;	/* Object between thumb and index */
	Evas_Object *test;	/* To see dis center */
};

enum _vp_pinch_plan_t {
	VP_PINCH_PLAN_NONE,
	VP_PINCH_PLAN_OUT,
	VP_PINCH_PLAN_IN,
	VP_PINCH_PLAN_MAX
};

typedef enum _vp_zoom_mode {
	VP_ZOOM_NONE,
	VP_ZOOM_IN_DEFAULT,
	VP_ZOOM_IN_MAX,
} vp_zoom_mode;

#define VP_IF_DEL_TIMER(timer) \
	do { \
		if(timer != NULL) { \
			ecore_timer_del(timer); \
			timer = NULL; \
		} \
	} while (0)

#define VP_FREE(ptr) \
	do { \
		free(ptr); \
		ptr = NULL; \
	} while (0)

#define VP_IF_DEL_JOB(job) \
	do { \
		if(job != NULL) { \
			ecore_job_del(job); \
			job = NULL; \
		} \
	} while (0)

#define VP_PINCH_TOUCH_HOLD_RANGE 80
#define VP_PINCH_TOUCH_FACTOR 4
#define VP_PINCH_HOLD_TIME_DELAY 0.35f
#define VP_GESTURE_KEY_DATA "gesture_data"

int _vp_gesture_set_zoom_in_cb(Evas_Object *gesture, vp_gesture_cb cb, void *data);
int _vp_gesture_set_zoom_out_cb(Evas_Object *gesture, vp_gesture_cb cb, void *data);
int _vp_pinch_add_event(void *data, Evas_Object *layout);
Eina_Bool __vp_pinch_zoom_out_cb(Evas_Object *gesture, void *data);
Eina_Bool __vp_pinch_zoom_in_cb(Evas_Object *gesture, void *data);
