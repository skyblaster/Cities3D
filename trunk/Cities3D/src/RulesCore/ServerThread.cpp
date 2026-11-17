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
#include <cstdio>

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

void wxServerThread::ShutdownFromMain()
{
	wxMutexLocker lock(mLock);

	for(size_t i = 0; i < mServers.size(); ++i)
	{
		if(NULL != mServers[i])
		{
			mServers[i]->Close();
			mServers[i]->Destroy();
			mServers[i] = NULL;
		}
	}
	mServers.clear();
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
void *wxServerThread::Entry()
{
	std::vector<wxSocketServer *> servers;
	if(false == NetworkEndpoint::CreateListeners(
		static_cast<wxUint16>(mPort), wxSOCKET_REUSEADDR_FLAG, servers))
	{
		wxLogError(wxT("Unable to start a server on port %d."), 
			static_cast<int>(mPort));
		return NULL;
	}

	{
		wxMutexLocker lock(mLock);
		mServers.swap(servers);
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
		std::vector<wxSocketServer *> listeners;

		{
			wxMutexLocker lock(mLock);
			listeners = mServers;
		}

		if(true == listeners.empty())
		{
			// ShutdownFromMain already closed listeners; exit promptly.
			break;
		}

		for(size_t i = 0; i < listeners.size(); ++i)
		{
			if(NULL == listeners[i])
			{
				continue;
			}

			wxCities3DSocket *socket = new wxCities3DSocket;

			if(false == listeners[i]->AcceptWith(*socket, false))
			{
				delete socket;
				continue;
			}

			// Reject half-open accepts (seen with some IPv6 stacks).
			if( (false == socket->IsOk()) ||
				(false == socket->IsConnected()))
			{
				socket->Destroy();
				continue;
			}

			accepted = true;

			// Since we can be killed at any conceivable time, test before every
			// new action.
			if(false == TestDestroy())
			{
				wxLogDebug(wxT("Server connection accepted."));
				std::fprintf(stderr, "cities3d: server accepted a client\n");
				std::fflush(stderr);

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
				socket->Destroy();
			}
		}

		if(false == accepted)
		{
			// Short sleep so Delete() becomes responsive quickly on Stop.
			Sleep(100);
		}
	}

	return NULL;
}

void wxServerThread::OnExit()
{
	wxLogDebug(wxT("Server thread exiting."));

	// Prefer ShutdownFromMain (main thread) for Close/Destroy. If we get here
	// without that, only Close — never Destroy from a worker while Delete()
	// blocks the UI thread.
	wxMutexLocker lock(mLock);
	for(size_t i = 0; i < mServers.size(); ++i)
	{
		if(NULL != mServers[i])
		{
			mServers[i]->Close();
			mServers[i] = NULL;
		}
	}
	mServers.clear();
}
