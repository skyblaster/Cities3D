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
#include "Rule.h"
#include "RuleSetDatabase.h"
#include "Controller.h"
#include "HTML.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

//---------------------------- RULES            -----------------------------//

//---------------------------------------------------------------------------//
// Class: RuleGenerateSystemMessageHotseat
// 
// Formats a gameplay message and sends it to the Events pane.
//
// Derived From:
//     <Rule>
//
// Project:
//     <CoreRules>
//
// RuleSet:
//     <Hotseat>
//
// Mixin To:
//     <RuleGenerateSystemMessage>
//
class RuleGenerateSystemMessageHotseat : public Rule
{
public:
	virtual void Execute(const DataObject &object)
	{
		wxString str = object.read<wxString>();
		wxDateTime time = wxDateTime::Now();
		if(2 <= object.numItems())
		{
			time = object.read<wxDateTime>(1);
		}

		wxString timestamp = swBracketLeft + time.FormatISOTime() +
			swBracketRightSpace;
		HTML::Font(timestamp, shSystemMessageTimestamp, shSystemMessageTimestamp);

		HTML::Font(
			HTML::Italics(
				HTML::Bold(str)));

		str = timestamp + str;
		HTML::Break(str);

		Controller::get().Transmit(shEventSystemMessage, DataObject(str));
	}
};

IMPLEMENT_RULE_KEYED_MIXIN(RuleGenerateSystemMessageHotseat, Hotseat,
						   RuleGenerateSystemMessage, HOTSEAT)
