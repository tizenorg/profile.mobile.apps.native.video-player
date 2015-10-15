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

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <errno.h>
#include <assert.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define _EDJ(o)				elm_layout_edje_get(o)
#define VP_SCALE			elm_config_scale_get()

#define VP_MAX_HEAP_SIZE		5*1024*1024  /*Malloc more than this will not be allowd*/

#define VP_ASSERT(scalar_exp)	 \
	assert(scalar_exp);	\


#define VP_CALLOC(ptr, no_elements, type)	 \
	do { \
		if((int)(no_elements) <= 0) \
		{ \
			ptr = NULL; \
		} \
		else if(VP_MAX_HEAP_SIZE < no_elements* sizeof (type)) \
		{ \
			assert(0); \
		} \
		else \
		{ \
			ptr = (type *) calloc (no_elements , sizeof (type)); \
			VP_ASSERT(ptr);		 \
		}\
	}while(0);

#define VP_SNPRINTF(dest,size,format,arg...)	\
		do { \
			if(dest != NULL && format != NULL)	\
			{	\
				snprintf(dest,size,format,##arg); \
			}	\
		}while(0);

		/* If the same pointer is passed to free twice, known as a double free.
		To avoid this, set pointers to NULL after passing	them to free:
		free(NULL) is safe (it does nothing). */

#define VP_FREE(ptr)	\
		do { \
			if(ptr != NULL) \
			{	\
				free(ptr);	\
				ptr = NULL;	\
			}	\
		}while(0);

#define VP_STRNCPY(dest,src,size)	\
		do { \
			if(src != NULL && dest != NULL && size > 0) \
			{	\
				strncpy(dest,src,size); \
			}	\
		}while(0);
	/*
	 * string wrappers
	 */
#define VP_STRDUP(destptr,sourceptr)\
		do{\
			if(sourceptr == NULL)\
				destptr = NULL; \
			else \
				destptr = strdup(sourceptr); \
		}while(0);



#define VP_EVAS_DEL(evas_obj)	\
		do { \
			if(evas_obj != NULL) \
			{	\
				evas_object_del(evas_obj); \
				evas_obj = NULL; \
			} \
		}while(0);

#define VP_EVAS_TIMER_DEL(evas_obj)	\
		do { \
			if(evas_obj != NULL) \
			{	\
				ecore_timer_del(evas_obj); \
				evas_obj = NULL; \
			} \
		}while(0);

#define VP_EVAS_IDLER_DEL(evas_obj)	\
		do { \
			if(evas_obj != NULL) \
			{	\
				ecore_idler_del(evas_obj); \
				evas_obj = NULL; \
			} \
		}while(0);


#define VP_EVAS_EVENT_HANDLER_DEL(evas_obj)	\
		do { \
			if(evas_obj != NULL) \
			{	\
				ecore_event_handler_del(evas_obj); \
				evas_obj = NULL; \
			} \
		}while(0);

#ifdef __cplusplus
	}
#endif /*  __cplusplus */


