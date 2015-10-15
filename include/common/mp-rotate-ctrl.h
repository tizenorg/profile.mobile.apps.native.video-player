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


#ifndef _VIDEO_ROTATE_CTRL_
#define  _VIDEO_ROTATE_CTRL_

#include <app.h>
#include <stdbool.h>
#include <Ecore_Evas.h>

typedef enum
{
	VIDEOS_ROTATE_NONE = 0x00,
	VIDEOS_ROTATE_90,
	VIDEOS_ROTATE_180,
	VIDEOS_ROTATE_270
}VideosRotateStatus_t;


void mp_rotate_ctrl_init(void);
void mp_rotate_ctrl_destroy(void);
VideosRotateStatus_t  mp_rotate_ctrl_get_current_state(void);
void mp_rotate_ctrl_set_state(int nAngle);
bool mp_rotate_ctrl_check_landspace();
bool mp_rotate_ctrl_check_landspace_by_win(Evas_Object *pWin);

#endif
