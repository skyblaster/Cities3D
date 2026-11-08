/*
 *  Cities3D - Copyright (C) 2001-2009 Jason Fugate (saladyears@gmail.com)
 * 
 *  This program is free software; you can redistribute it and/or modify it 
 *  under the terms of the GNU General Public License as published by the Free 
 *  Software Foundation; either version 2 of the License, or (at your option) 
 *  any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but 
    10| *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 *  for more details.
 */
#include "stdwx.h"
#include "style.h"	//READ THIS BEFORE MAKING ANY CHANGES TO THIS FILE!!!

//---------------------------- SYSTEM INCLUDES  -----------------------------//
#include <wx/ffile.h>

//---------------------------- USER INCLUDES    -----------------------------//
#include "Rule.h"
#include "RuleSetDatabase.h"
#include "Game.h"
#include "FilePath.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	void WriteResetObjectFlagsStage(const wxString &stage)
	{
		wxFFile breadcrumb(FilePath::LastRuleBreadcrumb(), wxT("w"));
		if(true == breadcrumb.IsOpened())
		{
			breadcrumb.Write(wxT("RuleResetObjectFlags:") + stage);
			breadcrumb.Flush();
		}
	}
}

//---------------------------- RULES            -----------------------------//

//---------------------------------------------------------------------------//
// Class: RuleResetObjectFlags
// 
// Resets the <IGameObject> flags for any <IGameObjects> in the game belonging
// to the players.
//
// Derived From:
//     <Rule>
//
// Project:
//     <CoreRules>
//
// RuleSet:
//     <Core>
//
class RuleResetObjectFlags : public Rule
{
public:
	virtual void Execute(const DataObject &object)
	{
		WriteResetObjectFlagsStage(wxT("start"));

		typedef std::vector<IGameObject::ObjectFlag> FlagArray;
		FlagArray flags;

		//read desired flags out of the DataObject
		for(size_t i = 0; i < object.numItems(); ++i)
		{
			try
			{
				flags.push_back(object.read<IGameObject::ObjectFlag>(i));
			}
			catch(const boost::bad_any_cast &)
			{
				try
				{
					const wxInt32 value = object.read<wxInt32>(i);
					if( (0 <= value) &&
						(16 > value))
					{
						flags.push_back(
							static_cast<IGameObject::ObjectFlag>(value));
					}
				}
				catch(const boost::bad_any_cast &)
				{
				}
			}
		}

		WriteResetObjectFlagsStage(wxT("players"));

		const wxInt32 players = numPlayers();
		for(wxInt32 i = 0; i < players; ++i)
		{
			playerGame(i).reapplyObjectFlags(
				flags.empty() ? NULL : &flags[0], flags.size());
		}

		WriteResetObjectFlagsStage(wxT("hexes"));

		GetGame()->reapplyHexObjectFlags(
			flags.empty() ? NULL : &flags[0], flags.size());

		WriteResetObjectFlagsStage(wxT("done"));
	}
};

IMPLEMENT_RULE(RuleResetObjectFlags, CORE)
