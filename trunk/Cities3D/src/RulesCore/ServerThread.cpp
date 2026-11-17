/*
 *  Cities3D - Copyright (C) 2001-2009 Jason Fugate (saladyears@gmail.com)
 * 
 *  This program is free software; you can redistribute it and/or modify it 
 *  under the terms of the GNU General Public License as published by the Free 
 *  Software Foundation; either version 2 of the License, or (at your option) 
 *  any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 *  for more details.
 */
#include "stdwx.h"
#include "style.h"	//READ THIS BEFORE MAKING ANY CHANGES TO THIS FILE!!!

//---------------------------- SYSTEM INCLUDES  -----------------------------//

//---------------------------- USER INCLUDES    -----------------------------//
#include "ServerThread.h"
#include "Cities3DSocket.h"
#include "ServerGameThread.h"
#include "RuleEvent.h"
#include "NetworkEndpoint.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

//---------------------------- PUBLIC           -----------------------------//
wxServerThread::wxServerThread(const size_t port, wxEvtHandler * const handler,
	wxServerGameThread *thread)
: mPort(port)
, mHandler(handler)
, mGameThread(thread)
{
	wxASSERT(NULL != handler);
	wxASSERT(NULL != thread);
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
void *wxServerThread::Entry()
{
	if(false == NetworkEndpoint::CreateListeners(
		static_cast<wxUint16>(mPort), wxSOCKET_REUSEADDR_FLAG, mServers))
	{
		wxLogError(wxT("Unable to start a server on port %d."), 
			static_cast<int>(mPort));
		return NULL;
	}

	wxLogDebug(wxT("Server listening on port %d (%d socket(s))"), 
		static_cast<int>(mPort), static_cast<int>(mServers.size()));

	// We're listening, so pass along a rule getting things going to the 
	// event handler.
	wxRuleEvent event(shNetworkRuleServerStart, DataObject(), true);
	mHandler->AddPendingEvent(event);

	// Now, loop, waiting for connections to happen.
	while(false == TestDestroy())
	{
		bool accepted = false;

		for(size_t i = 0; i < mServers.size(); ++i)
		{
			wxCities3DSocket *socket = new wxCities3DSocket;

			if(false == mServers[i]->AcceptWith(*socket, false))
			{
				delete socket;
				socket = NULL;
				continue;
			}

			accepted = true;

			// Since we can be killed at any conceivable time, test before every
			// new action.
			if(false == TestDestroy())
			{
				wxLogDebug(wxT("Server connection accepted."));

				wxRuleEvent event(shNetworkRuleServerConnection, 
					DataObject(mHandler, socket), true);
				mHandler->AddPendingEvent(event);

				if(false == TestDestroy())
				{
					mGameThread->AddSocket(socket);
				}
			}
			else
			{
				delete socket;
			}
		}

		if(false == accepted)
		{
			Sleep(1000);
		}
	}

	return NULL;
}

void wxServerThread::OnExit()
{
	wxLogDebug(wxT("Server thread exiting."));

	for(size_t i = 0; i < mServers.size(); ++i)
	{
		if(NULL != mServers[i])
		{
			mServers[i]->Destroy();
			mServers[i] = NULL;
		}
	}
	mServers.clear();
}
