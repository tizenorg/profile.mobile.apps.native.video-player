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

#include <stdbool.h>
#include <Elementary.h>

typedef void (*HollICEventCbFunc)(bool bCoverState, void* pUserData);

typedef void *hollic_handle;


hollic_handle vp_hollic_create_handle(HollICEventCbFunc pEventCb, void* pUserData);
void vp_hollic_destroy_handle(hollic_handle pHollICHandle);

