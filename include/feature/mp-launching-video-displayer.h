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

#ifndef __LAUNCHING_VIDEO_DISPLAYER__
#define	 __LAUNCHING_VIDEO_DISPLAYER__


typedef enum
{
	MP_PLAYER_TYPE_VIDEO				= 0,
	MP_PLAYER_TYPE_STREAMING,
	MP_PLAYER_TYPE_MAX,
}MpPlayerType;


void mp_launch_video_play(char* szMediaUrl, MpPlayerType nPlayerType, char *szDeviceID);
void mp_launch_video_stop(void);
bool mp_launch_video_allow_check(void);

#endif
