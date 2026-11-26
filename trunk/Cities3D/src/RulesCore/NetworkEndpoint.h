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
#pragma once

#include "style.h"	//READ THIS BEFORE MAKING ANY CHANGES TO THIS FILE!!!

//---------------------------- SYSTEM INCLUDES  -----------------------------//
#include <vector>

//---------------------------- USER INCLUDES    -----------------------------//
#include "RulesExport.h"

//---------------------------- DEFINES          -----------------------------//

//---------------------------- TYPEDEFS         -----------------------------//
class wxSocketClient;
class wxSocketServer;

//---------------------------- CLASSES          -----------------------------//

//---------------------------------------------------------------------------//
// Namespace: NetworkEndpoint
//
// Dual-stack (IPv4 / IPv6) helpers for LAN sockets. Host names are resolved
// with getaddrinfo(AF_UNSPEC); servers listen on IPv4 and, when available,
// IPv6.
//
namespace NetworkEndpoint
{
	struct RULES_EXPORT_SPEC Address
	{
		wxString host;
		wxUint16 port;

		Address();
		Address(const wxString& host, wxUint16 port);

		// Empty, "0.0.0.0", or "::" means bind/listen on any interface.
		bool IsAny() const;

		// Sets host after AF_UNSPEC getaddrinfo validation (Any hosts skip DNS).
		bool SetHost(const wxString& host);

		void SetPort(wxUint16 port);
		void SetAny(wxUint16 port);

		// Host for UI (IPv6 literals wrapped in []).
		wxString DisplayHost() const;

		// "host:port" or "[v6]:port".
		wxString DisplayEndpoint() const;
	};

	// Strip a single pair of [] used around IPv6 literals.
	RULES_EXPORT_SPEC wxString NormalizeHost(const wxString& host);

	// True if getaddrinfo succeeds for host (AF_UNSPEC), or host is Any.
	RULES_EXPORT_SPEC bool HostLooksValid(const wxString& host);

	// Connect using every AF_INET / AF_INET6 result from getaddrinfo.
	RULES_EXPORT_SPEC bool Connect(wxSocketClient& client, const Address& addr);

	// Create one or more listening servers for the port. Caller owns pointers.
	RULES_EXPORT_SPEC bool CreateListeners(wxUint16 port, int flags,
		std::vector<wxSocketServer*>& servers);
}

//---------------------------- PROTOTYPES       -----------------------------//
