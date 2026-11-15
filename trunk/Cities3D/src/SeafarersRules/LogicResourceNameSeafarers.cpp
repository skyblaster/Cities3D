/*
 *  Seafarers3D - Copyright (C) 2001-2009 Jason Fugate (saladyears@gmail.com)
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
#include "Logic.h"
#include "RuleSetDatabase.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

//---------------------------- LOGIC            -----------------------------//

//---------------------------------------------------------------------------//
// Class: LogicResourceNameSeafarers
// 
// Provides translation of resource names for Seafarers.
//
// Derived From:
//     <Logic>
//
// Project:
//     <SeafarersRules>
//
// RuleSet:
//     <Seafarers>
//
// Mixin To:
//     <LogicResourceName>
//
class LogicResourceNameRandom : public Logic
{
public:
	virtual void Decide(const DataObject &, DataObject &output)
	{
		output = DataObject(stRandom);
	}
};

IMPLEMENT_LOGIC_KEYED_MIXIN(LogicResourceNameRandom, Random, 
							LogicResourceName, SEAFARERS);

// Hidden tiles (Oceans III-VI, etc.) - popup hover must resolve a name.
class LogicResourceNameHidden : public Logic
{
public:
	virtual void Decide(const DataObject &, DataObject &output)
	{
		output = DataObject(stHidden);
	}
};

IMPLEMENT_LOGIC_KEYED_MIXIN(LogicResourceNameHidden, Hidden, 
							LogicResourceName, HIDDEN_TILES);

// Chipless tiles before reveal use the Chipless type on some maps.
class LogicResourceNameChipless : public Logic
{
public:
	virtual void Decide(const DataObject &, DataObject &output)
	{
		output = DataObject(stChipless);
	}
};

IMPLEMENT_LOGIC_KEYED_MIXIN(LogicResourceNameChipless, Chipless, 
							LogicResourceName, CHIPLESS_TILES);

// Unassigned ports during port placement.
class LogicResourceNamePortRandom : public Logic
{
public:
	virtual void Decide(const DataObject &, DataObject &output)
	{
		output = DataObject(stPortRandom);
	}
};

IMPLEMENT_LOGIC_KEYED_MIXIN(LogicResourceNamePortRandom, PortRandom, 
							LogicResourceName, SEAFARERS);

