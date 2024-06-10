/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "quakedef.h"
#include "net_api_int.h"

net_api_t netapi = 
{
	&Net_InitNetworking,
	&Net_Status,
	&Net_SendRequest,
	&Net_CancelRequest,
	&Net_CancelAllRequests,
	&Net_AdrToString,
	&Net_CompareAdr,
	&Net_StringToAdr,
	&Info_ValueForKey,
	&Info_RemoveKey,
	&Info_SetValueForStarKey
};

void Net_InitNetworking()
{
	//TODO: implement - Solokiller
}

void Net_APIShutDown()
{
	//TODO: implement - Solokiller
}

void Net_Status( net_status_t* status )
{
	//TODO: implement - Solokiller
}

void Net_SendRequest( int context, int request, int flags, double timeout, netadr_t* remote_address, net_api_response_func_t response )
{
	//TODO: implement - Solokiller
}

void Net_CancelRequest( int context )
{
	//TODO: implement - Solokiller
}

void Net_CancelAllRequests()
{
	//TODO: implement - Solokiller
}

char* Net_AdrToString( netadr_t* a )
{
	//TODO: implement - Solokiller
	return "";
}

int Net_CompareAdr( netadr_t* a, netadr_t* b )
{
	//TODO: implement - Solokiller
	return false;
}

int Net_StringToAdr( char* s, netadr_t* a )
{
	//TODO: implement - Solokiller
	return false;
}
