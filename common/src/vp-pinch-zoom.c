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

#include "vp-pinch-zoom.h"

static vp_pinch_event_s *__vp_gesture_create_event_obj(void *data,
		Evas_Object *
		object, int device)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	vp_pinch_event_s *ev = NULL;
	ev = calloc(1, sizeof(vp_pinch_event_s));
	if (ev == NULL) {
		VideoLogDebug("Cannot allocate event_t");
		return NULL;
	}

	ev->object = object;
	ev->device = device;
	evas_object_geometry_get(object, &ev->x, &ev->y, &ev->w, &ev->h);

	gesture_d->s_event_elist =
		eina_list_append(gesture_d->s_event_elist, ev);
	return ev;
}

static int __vp_gesture_destroy_event_obj(void *data,
		vp_pinch_event_s *ev)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	ev->pinch_obj = NULL;
	ev->pinch_dis = 0;
	gesture_d->s_event_elist =
		eina_list_remove(gesture_d->s_event_elist, ev);
	VP_IF_DEL_TIMER(ev->hold_timer);

	VP_FREE(ev);
	return 0;
}

static vp_pinch_event_s *__vp_gesture_get_event_obj(void *data,
		int device)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	Eina_List *l = NULL;
	vp_pinch_event_s *ev = NULL;

	EINA_LIST_FOREACH(gesture_d->s_event_elist, l, ev) {
		if (ev && ev->device == device)
			break;
		ev = NULL;
	}

	return ev;
}

static int __vp_gesture_get_distance(Evas_Coord x1, Evas_Coord y1,
				     Evas_Coord x2, Evas_Coord y2)
{
	int dis, dx, dy;

	dx = x1 - x2;
	dy = y1 - y2;

	dis = (int)sqrt(dx * dx + dy * dy);
	return dis;
}

static Eina_Bool __vp_gesture_hold_timer_cb(void *data)
{
	vp_pinch_event_s *ev0 = (vp_pinch_event_s *)data;
	VP_IF_DEL_TIMER(ev0->hold_timer);
	return ECORE_CALLBACK_CANCEL;
}

static int __vp_gesture_get_multi_device(void *data)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	Eina_List *l = NULL;
	vp_pinch_event_s *ev = NULL;

	EINA_LIST_FOREACH(gesture_d->s_event_elist, l, ev) {
		if (ev && ev->device != 0)
			return ev->device;
	}
	return 0;
}

static void __vp_gesture_mouse_down_event(void *data, Evas *e,
		Evas_Object *evas_obj,
		void *ei)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *)ei;
	vp_pinch_event_s *ev0;
	ev0 = __vp_gesture_get_event_obj(data, 0);
	if (ev0)
		return;

	ev0 = __vp_gesture_create_event_obj(data, evas_obj, 0);
	if (!ev0)
		return;

	ev0->hold_timer = NULL;
	ev0->prev.x = ev->output.x;
	ev0->prev.y = ev->output.y;
}

static void __vp_gesture_multi_down_event(void *data, Evas *evas,
		Evas_Object *obj, void *ei)
{
	vp_pinch_event_s *ev;
	Evas_Event_Multi_Down *down = (Evas_Event_Multi_Down *)ei;
	ev = __vp_gesture_get_event_obj(data, down->device);
	if (ev)
		return;

	ev = __vp_gesture_create_event_obj(data, obj, down->device);
	if (!ev)
		return;

	ev->hold_timer = NULL;
	ev->prev.x = down->output.x;
	ev->prev.y = down->output.y;
}

static void __vp_gesture_mouse_up_event(void *data, Evas *e,
					Evas_Object *obj, void *ei)
{
	int mdevice;
	vp_pinch_event_s *ev0;
	vp_pinch_event_s *ev = NULL;

	ev0 = __vp_gesture_get_event_obj(data, 0);
	if (ev0 == NULL) {
		VideoLogDebug("Cannot get event0");
		return;
	}

	mdevice = __vp_gesture_get_multi_device(data);
	if (mdevice == 0) {
	} else {
		ev = __vp_gesture_get_event_obj(data, mdevice);
		if (ev == NULL) {
			return;
		}

		VP_IF_DEL_TIMER(ev->hold_timer);
		ev->hold_timer =
			ecore_timer_add(VP_PINCH_HOLD_TIME_DELAY,
					__vp_gesture_hold_timer_cb, ev);
	}

	__vp_gesture_destroy_event_obj(data, ev0);
}

static void __vp_gesture_multi_up_event(void *data, Evas *evas,
					Evas_Object *obj, void *ei)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	Evas_Event_Multi_Up *up = (Evas_Event_Multi_Up *)ei;
	vp_pinch_event_s *ev0 = NULL;
	vp_pinch_event_s *ev = NULL;
	ev = __vp_gesture_get_event_obj(data, up->device);
	if (ev == NULL) {
		return;
	}

	gesture_d->dis_old = 0;
	gesture_d->pinch_dis_old = 0;
	ev0 = __vp_gesture_get_event_obj(data, 0);
	if (ev0) {
		VP_IF_DEL_TIMER(ev0->hold_timer);
		ev0->hold_timer =
			ecore_timer_add(VP_PINCH_HOLD_TIME_DELAY,
					__vp_gesture_hold_timer_cb, ev0);
	} else {
		/* up when device 0 is off */
	}
	__vp_gesture_destroy_event_obj(data, ev);
}

static void __vp_gesture_mouse_move_event(void *data, Evas *e,
		Evas_Object *obj, void *ei)
{
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)ei;
	vp_pinch_event_s *ev0;
	ev0 = __vp_gesture_get_event_obj(data, 0);
	if (ev0 == NULL) {
		return;
	}
	ev0->prev.x = ev->cur.output.x;
	ev0->prev.y = ev->cur.output.y;

	__vp_gesture_get_multi_device(data);
}

static void __vp_gesture_zoom_out_job_cb(void *data)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	if (gesture_d->next_plan != VP_PINCH_PLAN_OUT) {
		VideoLogWarning("State is wrong[plan:%d]!", gesture_d->next_plan);
		VP_IF_DEL_JOB(gesture_d->pinch_job);
		return;
	}

	if (gesture_d->zoom_out_cb)
		gesture_d->zoom_out_cb(gesture_d->gesture,
				       gesture_d->zoom_out_data);

	VP_IF_DEL_JOB(gesture_d->pinch_job);
}

static void __vp_gesture_zoom_in_job_cb(void *data)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *)data;
	if (gesture_d->next_plan != VP_PINCH_PLAN_IN) {
		VideoLogWarning("State is wrong[plan:%d]!", gesture_d->next_plan);
		VP_IF_DEL_JOB(gesture_d->pinch_job);
		return;
	}

	if (gesture_d->zoom_in_cb)
		gesture_d->zoom_in_cb(gesture_d->gesture,
				      gesture_d->zoom_in_data);


	VP_IF_DEL_JOB(gesture_d->pinch_job);
}

static void __vp_gesture_multi_move_event(void *data, Evas *evas,
		Evas_Object *obj, void *ei)
{
	vp_gesture_s *gesture_d = (vp_gesture_s *) data;
	Evas_Event_Multi_Move *move = (Evas_Event_Multi_Move *)ei;
	int dis_new;
	vp_pinch_event_s *ev0;
	vp_pinch_event_s *ev;
	ev = __vp_gesture_get_event_obj(data, move->device);
	if (ev == NULL) {
		return;
	}
	ev->prev.x = move->cur.output.x;
	ev->prev.y = move->cur.output.y;

	ev0 = __vp_gesture_get_event_obj(data, 0);
	if (ev0 == NULL) {
		return;
	}

	dis_new = __vp_gesture_get_distance(ev0->prev.x, ev0->prev.y,
					    ev->prev.x, ev->prev.y);

	int dis_old = gesture_d->dis_old;
	if (dis_old != 0) {
		if (dis_old - dis_new > 0
				&& ev->pinch_dis > VP_PINCH_TOUCH_HOLD_RANGE) {
			if (gesture_d->pinch_dis_old
					&& ev->pinch_dis <
					(gesture_d->pinch_dis_old * VP_PINCH_TOUCH_FACTOR)) {
				ev->pinch_dis += (dis_old - dis_new);
				gesture_d->dis_old = dis_new;
				return;
			}

			gesture_d->next_plan = VP_PINCH_PLAN_OUT;	/* plan to zoom-out */
			if (!gesture_d->pinch_job) {
				gesture_d->pinch_job =
					ecore_job_add(__vp_gesture_zoom_out_job_cb, data);
			} else {
				VideoLogInfo("Added job pinch zoom out");
			}

			gesture_d->pinch_dis_old = ev->pinch_dis;
			ev->pinch_dis = 0;
		} else if (dis_old - dis_new < 0
				&& ev->pinch_dis < -VP_PINCH_TOUCH_HOLD_RANGE) {
			if (gesture_d->pinch_dis_old
					&& ev->pinch_dis >
					(gesture_d->pinch_dis_old * VP_PINCH_TOUCH_FACTOR)) {
				ev->pinch_dis += (dis_old - dis_new);
				gesture_d->dis_old = dis_new;
				return;
			}

			gesture_d->next_plan = VP_PINCH_PLAN_IN;	/* plan to zoom-in */
			if (!gesture_d->pinch_job) {
				VideoLogInfo("Add job pinch zoom in");
				gesture_d->pinch_job =
					ecore_job_add(__vp_gesture_zoom_in_job_cb, data);
			} else {
				VideoLogWarning("Added job pinch zoom in");
			}

			gesture_d->pinch_dis_old = ev->pinch_dis;
			ev->pinch_dis = 0;
		}
		ev->pinch_dis += (dis_old - dis_new);
	}
	VideoLogInfo("dis_new: %d, dis_old: %d, pinch_dis %d", dis_new,
		     dis_old, ev->pinch_dis);

	/* Reset dis_old value */
	gesture_d->dis_old = dis_new;
}

static void __vp_gesture_del_cb(void *data, Evas *e, Evas_Object *obj,
				void *ei)
{
	VideoLogWarning("Delete gesture ---");
	if (data) {
		vp_gesture_s *gesture_d = (vp_gesture_s *)data;
		VP_IF_DEL_JOB(gesture_d->pinch_job);
		evas_object_data_del(gesture_d->gesture, VP_GESTURE_KEY_DATA);
		VP_FREE(gesture_d);
	}
	VideoLogWarning("Delete gesture +++");
}

Evas_Object *_vp_gesture_add(void *data, Evas_Object *parent)
{
	if (data == NULL || parent == NULL) {
		VideoLogInfo("Error input parameters");
		return NULL;
	}

	Evas_Object *gesture = NULL;

	gesture = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_color_set(gesture, 0, 0, 0, 0);

	vp_gesture_s *gesture_d =
		(vp_gesture_s *)calloc(1, sizeof(vp_gesture_s));
	if (gesture_d == NULL) {
		evas_object_del(gesture);
		return NULL;
	}

	gesture_d->ad = data;
	gesture_d->parent = parent;
	gesture_d->gesture = gesture;

	evas_object_event_callback_add(gesture, EVAS_CALLBACK_MOUSE_DOWN,
				       __vp_gesture_mouse_down_event,
				       gesture_d);
	evas_object_event_callback_add(gesture, EVAS_CALLBACK_MOUSE_UP,
				       __vp_gesture_mouse_up_event,
				       gesture_d);
	evas_object_event_callback_add(gesture, EVAS_CALLBACK_MOUSE_MOVE,
				       __vp_gesture_mouse_move_event,
				       gesture_d);
	evas_object_event_callback_add(gesture, EVAS_CALLBACK_MULTI_DOWN,
				       __vp_gesture_multi_down_event,
				       gesture_d);
	evas_object_event_callback_add(gesture, EVAS_CALLBACK_MULTI_UP,
				       __vp_gesture_multi_up_event,
				       gesture_d);
	evas_object_event_callback_add(gesture, EVAS_CALLBACK_MULTI_MOVE,
				       __vp_gesture_multi_move_event,
				       gesture_d);

	evas_object_data_set(gesture, VP_GESTURE_KEY_DATA,
			     (void *)gesture_d);
	evas_object_event_callback_add(gesture, EVAS_CALLBACK_DEL,
				       __vp_gesture_del_cb, gesture_d);
	return gesture;
}

int _vp_gesture_set_zoom_in_cb(Evas_Object *gesture, vp_gesture_cb cb,
			       void *data)
{
	if (gesture == NULL || data == NULL) {
		VideoLogInfo("Event Object is NULL");
		return 0;
	}

	vp_gesture_s *gesture_d = NULL;

	gesture_d =
		(vp_gesture_s *)evas_object_data_get(gesture,
				VP_GESTURE_KEY_DATA);
	if (gesture_d) {
		gesture_d->zoom_in_cb = cb;
		gesture_d->zoom_in_data = data;
	}
	return 0;
}

int _vp_gesture_set_zoom_out_cb(Evas_Object *gesture, vp_gesture_cb cb,
				void *data)
{
	if (gesture == NULL || data == NULL) {
		VideoLogInfo("Event Object is NULL");
		return 0;
	}

	vp_gesture_s *gesture_d = NULL;

	gesture_d =
		(vp_gesture_s *)evas_object_data_get(gesture,
				VP_GESTURE_KEY_DATA);

	if (gesture_d != NULL) {
		gesture_d->zoom_out_cb = cb;
		gesture_d->zoom_out_data = data;
	}

	return 0;
}

Eina_Bool __vp_pinch_zoom_out_cb(Evas_Object *gesture, void *data)
{
	if (gesture == NULL || data == NULL) {
		VideoLogInfo("Event Object is NULL");
		return EINA_FALSE;
	}

	st_VideoListViewMainViewWidget *ad =
		(st_VideoListViewMainViewWidget *)data;
	VideoLogInfo("%d", ad->zoom_level);
	if (ad->zoom_level > VP_ZOOM_IN_DEFAULT) {
		ad->zoom_level--;
		if (_grid_view_zoom_out(data, NULL) == 0)
			ad->zoom_level = VP_ZOOM_IN_DEFAULT;
	}

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool __vp_pinch_zoom_in_cb(Evas_Object *gesture, void *data)
{
	if (gesture == NULL || data == NULL) {
		VideoLogInfo("Event Object is NULL");
		return EINA_FALSE;
	}

	st_VideoListViewMainViewWidget *ad =
		(st_VideoListViewMainViewWidget *) data;

	if ((ad->zoom_level >= VP_ZOOM_IN_DEFAULT) &&
			(ad->zoom_level < VP_ZOOM_IN_MAX)) {
		ad->zoom_level++;
		if (_grid_view_zoom_in(data, NULL) == 0)
			ad->zoom_level = VP_ZOOM_IN_MAX;
	}

	return ECORE_CALLBACK_CANCEL;
}

int _vp_pinch_add_event(void *data, Evas_Object *layout)
{
	if (data == NULL || layout == NULL) {
		VideoLogError("Error input parameters");
		return 0;
	}

	st_VideoListViewMainViewWidget *ad =
		(st_VideoListViewMainViewWidget *)data;

	/* Set initialize level */
	ad->zoom_level = VP_ZOOM_IN_DEFAULT;

	Evas_Object *gesture = _vp_gesture_add(data, layout);
	_vp_gesture_set_zoom_in_cb(gesture, __vp_pinch_zoom_in_cb, data);
	_vp_gesture_set_zoom_out_cb(gesture, __vp_pinch_zoom_out_cb, data);
	elm_object_part_content_set(layout, "gesture", gesture);
	ad->gesture = gesture;
	return 0;
}
