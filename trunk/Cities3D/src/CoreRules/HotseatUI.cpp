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
#include <wx/aui/aui.h>

//---------------------------- USER INCLUDES    -----------------------------//
#include "UIPlugin.h"
#include "RuleSetDatabase.h"
#include "Controller.h"
#include "GameEngine.h"

#include "GameSetupPane.h"
#include "DataObject.h"
#include "Map.h"
#include "IUIEngine.h"
#include "IRuleEngine.h"
#include "EventWindow.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

//---------------------------- PUBLIC           -----------------------------//
class HotseatUI : public UIPlugin
{
public:
	HotseatUI()
	{
		GUI.RegisterMainMenuItem(MODULE, HOTSEAT, swPlay, 100, 
			stHotseat, shHotseat, swHotseat);
	}

	~HotseatUI()
	{
	}

	virtual void HandleMainMenuClick(const wxString& key)
	{
		wxASSERT(swHotseat == key);
		
		wxGameSetupPane* pane = new wxGameSetupPane(GetMainFrame());

		Controller::get().Transmit(shEventChangeMainPane, 
			DataObject(static_cast<PaneMixin*>(pane), stHotseatSetup));
	}

	virtual void CreateUI(const GamePtr&, wxAuiManager& manager)
	{
		// Keep ProvidesUI() false so the saved-layout key stays
		// CoreDevCards (etc.). A true ProvidesUI() renamed the layout
		// file and wxAui loaded no default, which rearranged every pane.
		wxWindow* frame = GetMainFrame();
		wxSize minSize = wxSize(165, 150);
		wxEventWindow* events = new wxEventWindow(frame, wxID_ANY);

		manager.AddPane(events, wxAuiPaneInfo().Name(swEvents)
			.Caption(stEvents).MinSize(minSize)
			.CloseButton(false).MaximizeButton(false)
			.Dockable(true).Right().Layer(1).Position(2));
	}
};

IMPLEMENT_UI_PLUGIN(HOTSEAT, HotseatUI)

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
