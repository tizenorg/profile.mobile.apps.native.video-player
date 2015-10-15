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

#ifndef __MP_VIDEO_TYPE_DEFINE__
#define __MP_VIDEO_TYPE_DEFINE__

 typedef enum
 {
	 LIST_TAB_TYPE_PERSONAL 	 = 0,
	 LIST_TAB_TYPE_DEVICES,
	 LIST_TAB_TYPE_MAX,
 }MpVideoListTabType;


typedef enum
{
	LIST_UPDATE_TYPE_NONE		= 0,
	LIST_UPDATE_TYPE_ALL,		//DATA AND VIEW
	LIST_UPDATE_TYPE_VIEW,		//ONLY UPDATE VIEW, NOT UPDATE DATA
	LIST_UPDATE_TYPE_DATA,		//ONLY UPDATE DATA, NOT UPDATE VIEW
	LIST_UPDATE_TYPE_MAX		//RESERVED FIELD
}MpListUpdateType;

typedef enum
{
	MEDIA_TYPE_FILE			= 0,				/**< File type*/
	MEDIA_TYPE_DIRECTORY 	= 1,				/**< Directory type*/
} MpMediaType;


#endif

