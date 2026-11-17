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

#ifdef __linux__ 

//---------------------------- SYSTEM INCLUDES  -----------------------------//
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <cstring>

//---------------------------- USER INCLUDES    -----------------------------//
#include "network/Network.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

//---------------------------- PUBLIC           -----------------------------//
void Network::GetMacAddress(wxString& strMacAddress)
{
	if(false == strMacAddress.IsEmpty())
	{
		// we have already got the mac address
		return;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(-1 != sock)
	{
		struct ifreq ifr;
		struct ifconf ifc;
		struct ifreq *IFR;
		char buf[1024];

		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = buf;
		ioctl(sock, SIOCGIFCONF, &ifc);

		IFR = ifc.ifc_req;
		for(int i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++)
		{
			strcpy(ifr.ifr_name, IFR->ifr_name);
			if(0 == ioctl(sock, SIOCGIFFLAGS, &ifr))
			{
				if(ifr.ifr_flags & IFF_LOOPBACK)
				{
					// we don't need the loopback device
					continue;
				}

				if(0 == ioctl(sock, SIOCGIFHWADDR, &ifr))
				{
					unsigned char addr[7];

					addr[6] = '\0';
					bcopy(ifr.ifr_hwaddr.sa_data, &addr, 6);

					for(unsigned int c = 0; c < 6; ++c)
					{
						if(0 != c)
						{
							strMacAddress += wxT("-");
						}

						unsigned char c1 = addr[c] & 0xF;
						unsigned char c2 = addr[c] >> 4;

						strMacAddress += (c1 < 10) ? '0' + c1 : 'A' - 10 + c1;
						strMacAddress += (c2 < 10) ? '0' + c2 : 'A' - 10 + c2;
					}

					break;
				}
			}
		}

		close(sock);
	}

	if(true == strMacAddress.IsEmpty())
	{
		// Things are hosed, just make up something.
		strMacAddress = wxT("no_adapter");
	}
}

void Network::GetIPAddresses(StringArray& i_addresses)
{
	// Enumerate IPv4 and IPv6 addresses on local interfaces.
	ifaddrs* ifaddr = NULL;
	if(0 != getifaddrs(&ifaddr) || NULL == ifaddr)
	{
		return;
	}

	for(ifaddrs* ifa = ifaddr; NULL != ifa; ifa = ifa->ifa_next)
	{
		if(NULL == ifa->ifa_addr)
		{
			continue;
		}

		const int family = ifa->ifa_addr->sa_family;
		if( (AF_INET != family) && (AF_INET6 != family) )
		{
			continue;
		}

		// Skip interfaces that are down or loopback-only for advertising.
		if(0 == (ifa->ifa_flags & IFF_UP))
		{
			continue;
		}
		if(0 != (ifa->ifa_flags & IFF_LOOPBACK))
		{
			continue;
		}

		char host[NI_MAXHOST] = {0};
		const socklen_t addrlen = (AF_INET == family)
			? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
		if(0 != getnameinfo(ifa->ifa_addr, addrlen, host, sizeof(host),
			NULL, 0, NI_NUMERICHOST))
		{
			continue;
		}

		wxString ip(host, wxConvUTF8);
		// Drop IPv6 scope id suffix (e.g. fe80::1%eth0).
		const int pct = ip.Find(wxT('%'));
		if(wxNOT_FOUND != pct)
		{
			ip = ip.Left(pct);
		}

		bool exists = false;
		for(size_t i = 0; i < i_addresses.size(); ++i)
		{
			if(i_addresses[i] == ip)
			{
				exists = true;
				break;
			}
		}
		if(false == exists)
		{
			i_addresses.push_back(ip);
		}
	}

	freeifaddrs(ifaddr);
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//

#endif // __linux__

