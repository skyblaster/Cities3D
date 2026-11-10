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
#include "AnyEngine.h"
#include "IAnySerializer.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	WX_DECLARE_HASH_STRING_HASH_MAP(IAnySerializerPtr, AnyHash);
	AnyHash sAnyTypeHash;
	AnyHash sAnyTypeIDHash;
}

//---------------------------- PUBLIC           -----------------------------//
void AnyEngine::Stop()
{
	// Serializers are registered by plugin static constructors (CoreRules).
	// PluginEngine keeps those DLLs mapped for the process lifetime, so the
	// constructors do not run again after an engine restart. Clearing the
	// hashes here left every DataObject save without a serializer (LAN rules
	// hit this first via NET.SendRule). Leave registrations in place; plugin
	// static destructors still call RemoveAnySerializer at process exit.
}

void AnyEngine::AddAnySerializer(const IAnySerializerPtr &serializer)
{
	wxASSERT(serializer);

	sAnyTypeHash[serializer->Type()] = serializer;
	sAnyTypeIDHash[serializer->TypeID()] = serializer;
}

void AnyEngine::RemoveAnySerializer(const IAnySerializerPtr &serializer)
{
	wxASSERT(serializer);

	sAnyTypeHash.erase(serializer->Type());
	sAnyTypeIDHash.erase(serializer->TypeID());
}

const IAnySerializerPtr &AnyEngine::FindAnySerializerByType(
	const HashString &type) const
{
	static const IAnySerializerPtr sEmpty;

	AnyHash::const_iterator it = sAnyTypeHash.find(type);
	if(sAnyTypeHash.end() == it)
	{
		wxASSERT_MSG(false, wxT("Missing AnySerializer for type."));
		return sEmpty;
	}

	wxASSERT(it->second);
	return it->second;
}

const IAnySerializerPtr &AnyEngine::FindAnySerializerByTypeID(
	const HashString &typeID) const
{
	static const IAnySerializerPtr sEmpty;

	AnyHash::const_iterator it = sAnyTypeIDHash.find(typeID);
	if(sAnyTypeIDHash.end() == it)
	{
		return sEmpty;
	}

	return it->second;
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//



