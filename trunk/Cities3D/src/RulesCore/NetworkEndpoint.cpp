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
#include <wx/socket.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#ifdef __WXMSW__
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

//---------------------------- USER INCLUDES    -----------------------------//
#include "NetworkEndpoint.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef wxUSE_IPV6
#define wxUSE_IPV6 0
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	bool IsAnyHost(const wxString& host)
	{
		return host.IsEmpty() ||
			host == wxT("0.0.0.0") ||
			host == wxT("::") ||
			host == wxT("[::]");
	}

	wxString HostForGetAddrInfo(const wxString& host)
	{
		return NetworkEndpoint::NormalizeHost(host);
	}

	bool IsIpv6LinkLocal(const addrinfo* ai)
	{
		if(NULL == ai || AF_INET6 != ai->ai_family || NULL == ai->ai_addr)
		{
			return false;
		}

		const sockaddr_in6* sin6 =
			reinterpret_cast<const sockaddr_in6*>(ai->ai_addr);
		// fe80::/10
		return (0xfe == sin6->sin6_addr.s6_addr[0]) &&
			(0x80 == (sin6->sin6_addr.s6_addr[1] & 0xc0));
	}

	// Lower rank is tried first: global/ULA IPv6, then IPv4, then link-local.
	int AddressRank(const addrinfo* ai)
	{
		if(NULL == ai)
		{
			return 100;
		}
		if(AF_INET6 == ai->ai_family)
		{
			return IsIpv6LinkLocal(ai) ? 2 : 0;
		}
		if(AF_INET == ai->ai_family)
		{
			return 1;
		}
		return 100;
	}

	bool FillWxAddress(const addrinfo* ai, unsigned short port,
		wxIPV4address& out4
#if wxUSE_IPV6
		, wxIPV6address& out6, bool& is6
#endif
		, wxString& numericOut)
	{
#if wxUSE_IPV6
		is6 = false;
#endif
		numericOut.clear();
		if(NULL == ai || NULL == ai->ai_addr)
		{
			return false;
		}

		char host[NI_MAXHOST] = {0};
		if(0 != getnameinfo(ai->ai_addr, ai->ai_addrlen, host, sizeof(host),
			NULL, 0, NI_NUMERICHOST))
		{
			return false;
		}

		wxString numeric(host, wxConvUTF8);
		// Strip "%iface" scope id for wx Hostname().
		const int pct = numeric.Find(wxT('%'));
		if(wxNOT_FOUND != pct)
		{
			numeric = numeric.Left(pct);
		}
		numericOut = numeric;

		if(AF_INET == ai->ai_family)
		{
			out4.Hostname(numeric);
			out4.Service(port);
			return true;
		}

#if wxUSE_IPV6
		if(AF_INET6 == ai->ai_family)
		{
			out6.Hostname(numeric);
			out6.Service(port);
			is6 = true;
			return true;
		}
#endif

		return false;
	}
}

//---------------------------- PUBLIC           -----------------------------//
NetworkEndpoint::Address::Address()
: port(7176)
{
}

NetworkEndpoint::Address::Address(const wxString& host, wxUint16 port)
: host(NormalizeHost(host))
, port(port)
{
}

bool NetworkEndpoint::Address::IsAny() const
{
	return IsAnyHost(host);
}

bool NetworkEndpoint::Address::SetHost(const wxString& value)
{
	wxString normalized = NormalizeHost(value);
	if(false == HostLooksValid(normalized))
	{
		return false;
	}
	host = normalized;
	return true;
}

void NetworkEndpoint::Address::SetPort(wxUint16 value)
{
	port = value;
}

void NetworkEndpoint::Address::SetAny(wxUint16 value)
{
	host = wxT("0.0.0.0");
	port = value;
}

wxString NetworkEndpoint::Address::DisplayHost() const
{
	if(true == IsAny())
	{
		return wxT("0.0.0.0");
	}

	if(wxNOT_FOUND != host.Find(wxT(':')))
	{
		return wxT("[") + host + wxT("]");
	}

	return host;
}

wxString NetworkEndpoint::Address::DisplayEndpoint() const
{
	return wxString::Format(wxT("%s:%u"), DisplayHost().c_str(),
		static_cast<unsigned int>(port));
}

wxString NetworkEndpoint::NormalizeHost(const wxString& host)
{
	wxString out = host;
	out.Trim(true).Trim(false);
	if( (2 <= out.Length()) &&
		(wxT('[') == out[0]) &&
		(wxT(']') == out[out.Length() - 1]))
	{
		out = out.Mid(1, out.Length() - 2);
	}
	return out;
}

bool NetworkEndpoint::HostLooksValid(const wxString& host)
{
	if(true == IsAnyHost(host))
	{
		return true;
	}

	const wxString query = HostForGetAddrInfo(host);
	addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;

	addrinfo* res = NULL;
	const int err = getaddrinfo(query.utf8_str(), NULL, &hints, &res);
	if(0 != err)
	{
		return false;
	}

	freeaddrinfo(res);
	return true;
}

bool NetworkEndpoint::Connect(wxSocketClient& client, const Address& addr)
{
	if(true == addr.IsAny())
	{
		wxLogError(wxT("Cannot connect to an any-address endpoint."));
		return false;
	}

	const wxString query = HostForGetAddrInfo(addr.host);
	char portBuf[16];
	std::snprintf(portBuf, sizeof(portBuf), "%u",
		static_cast<unsigned int>(addr.port));

	addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
#ifdef AI_ADDRCONFIG
	hints.ai_flags = AI_ADDRCONFIG;
#endif

	addrinfo* res = NULL;
	const int err = getaddrinfo(query.utf8_str(), portBuf, &hints, &res);
	if(0 != err || NULL == res)
	{
		wxLogDebug(wxT("getaddrinfo(%s) failed: %d"), query.c_str(), err);
		return false;
	}

	std::vector<addrinfo*> ordered;
	for(addrinfo* ai = res; NULL != ai; ai = ai->ai_next)
	{
		ordered.push_back(ai);
	}
	std::stable_sort(ordered.begin(), ordered.end(),
		[](const addrinfo* a, const addrinfo* b)
		{
			return AddressRank(a) < AddressRank(b);
		});

	bool connected = false;
	for(size_t i = 0; i < ordered.size(); ++i)
	{
		addrinfo* ai = ordered[i];
		wxIPV4address v4;
		wxString numeric;
#if wxUSE_IPV6
		wxIPV6address v6;
		bool is6 = false;
		if(false == FillWxAddress(ai, addr.port, v4, v6, is6, numeric))
#else
		if(false == FillWxAddress(ai, addr.port, v4, numeric))
#endif
		{
			continue;
		}

		if(client.IsConnected())
		{
			client.Close();
		}

#if wxUSE_IPV6
		if(true == is6)
		{
			std::fprintf(stderr,
				"cities3d: connecting via IPv6 to [%s]:%u (from %s)\n",
				numeric.mb_str().data(),
				static_cast<unsigned int>(addr.port),
				addr.host.mb_str().data());
			std::fflush(stderr);
			client.Connect(v6, false);
		}
		else
#endif
		{
			std::fprintf(stderr,
				"cities3d: connecting via IPv4 to %s:%u (from %s)\n",
				numeric.mb_str().data(),
				static_cast<unsigned int>(addr.port),
				addr.host.mb_str().data());
			std::fflush(stderr);
			client.Connect(v4, false);
		}

		client.WaitOnConnect(10);
		if(true == client.IsConnected())
		{
			connected = true;
			break;
		}
	}

	freeaddrinfo(res);
	return connected;
}

bool NetworkEndpoint::CreateListeners(wxUint16 port, int flags,
	std::vector<wxSocketServer*>& servers)
{
	servers.clear();

#if wxUSE_IPV6
	{
		wxIPV6address any6;
		any6.AnyAddress();
		any6.Service(port);
		wxSocketServer* server6 = new wxSocketServer(any6, flags);
		if(true == server6->Ok())
		{
			std::fprintf(stderr,
				"cities3d: listening on IPv6 [::]:%u\n",
				static_cast<unsigned int>(port));
			std::fflush(stderr);
			servers.push_back(server6);
		}
		else
		{
			server6->Close();
			server6->Destroy();
		}
	}
#endif

	// Always attempt IPv4. On Linux an IPv6 dual-stack listener may already
	// cover IPv4-mapped traffic; a failed second bind is fine.
	{
		wxIPV4address any4;
		any4.AnyAddress();
		any4.Service(port);
		wxSocketServer* server4 = new wxSocketServer(any4, flags);
		if(true == server4->Ok())
		{
			std::fprintf(stderr,
				"cities3d: listening on IPv4 0.0.0.0:%u\n",
				static_cast<unsigned int>(port));
			std::fflush(stderr);
			servers.push_back(server4);
		}
		else
		{
			server4->Close();
			server4->Destroy();
		}
	}

	return false == servers.empty();
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
