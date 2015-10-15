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


#ifndef _STREAMING_PLAYER_H_
#define _STREAMING_PLAYER_H_


typedef enum
{
	MP_NETWORK_CLOSE					= 0,
	MP_NETWORK_ON,
	MP_NETWORK_MAX,
}MpNetworkConfig;


typedef enum
{
	MP_NETWORK_TYPE_OFF					= 0,
	MP_NETWORK_TYPE_CELLULAR,
	MP_NETWORK_TYPE_WIFI,
	MP_NETWORK_TYPE_MAX,
}MpNetworkType;


typedef enum
{
	MP_NETWORK_WIFI_OFF					= 0,
	MP_NETWORK_WIFI_NOT_CONNECTED,
	MP_NETWORK_WIFI_CONNECTED,
	MP_NETWORK_WIFI_MAX,
}MpNetworkWifiState;


typedef enum
{
	MP_WIFI_DIRECT_DEACTIVATED			= 0,
	MP_WIFI_DIRECT_ACTIVATED,
	MP_WIFI_DIRECT_DISCOVERING,
	MP_WIFI_DIRECT_CONNECTED,
	MP_WIFI_DIRECT_GROUP_OWNER,
	MP_WIFI_DIRECT_MAX,
}MpNetworkWifiDirectState;

typedef enum
{
	MP_NETWORK_CELLULAR_ON				= 0,
	MP_NETWORK_CELLULAR_3G_OPTION_OFF,
	MP_NETWORK_CELLULAR_ROAMING_OFF,
	MP_NETWORK_CELLULAR_FLIGHT_MODE,
	MP_NETWORK_CELLULAR_NO_SERVICE,
	MP_NETWORK_CELLULAR_MAX,
}MpNetworkCellularState;

#endif /* _STREAMING_PLAYER_H_ */
