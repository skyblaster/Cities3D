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
#include "LayoutManager.h"
#include "GameEngine.h"
#include "IPluginEngine.h"
#include "IUIEngine.h"
#include "FilePath.h"
#include "BaseListCtrl.h"
#include "ConfigDatabase.h"

//---------------------------- USER INCLUDES    -----------------------------//

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	void CreateLayoutPath(wxString& path)
	{
		IPluginEngine::StringPairArray plugins;
		PLUGIN.GetLoadedRuleSets(plugins);

		IPluginEngine::StringPairArray::const_iterator it, itEnd = plugins.end();
		for(it = plugins.begin(); it != itEnd; ++it)
		{
			const wxString& plugin = it->first;
			const wxString& ruleset = it->second;

			// Hotseat now creates an Events pane, but it must not change
			// the layout key (CoreDevCards, etc.) or restore has no default.
			if(wxT("Hotseat") == ruleset)
			{
				continue;
			}

			if(true == GUI.ProvidesUI(plugin, ruleset))
			{
				path += ruleset;
			}
		}
	}

	WX_DECLARE_STRING_HASH_MAP(wxString, LayoutMap);
	LayoutMap sDefaultLayouts;

	void EnsureDefaultLayouts()
	{
		if(false == sDefaultLayouts.empty())
		{
			return;
		}

		sDefaultLayouts[swCoreCities] = swCoreCitiesLayout;
		sDefaultLayouts[swCoreCitiesVolcano] = swCoreCitiesVolcanoLayout;
		sDefaultLayouts[swCoreDevCards] = swCoreDevCardsLayout;
		sDefaultLayouts[swCoreDevCardsVolcano] = swCoreDevCardsVolcanoLayout;
		sDefaultLayouts[swCoreNetworkCities] = swCoreNetworkCitiesLayout;
		sDefaultLayouts[swCoreNetworkCitiesVolcano] = swCoreNetworkCitiesVolcanoLayout;
		sDefaultLayouts[swCoreNetworkDevCards] = swCoreNetworkDevCardsLayout;
		sDefaultLayouts[swCoreNetworkDevCardsVolcano] = swCoreNetworkDevCardsVolcanoLayout;

		sDefaultLayouts[swCoreCitiesOnlineCities] = swCoreCitiesOnlineCitiesLayout;
		sDefaultLayouts[swCoreCitiesOnlineCitiesVolcano] = swCoreCitiesOnlineCitiesVolcanoLayout;
		sDefaultLayouts[swCoreCitiesOnlineDevCards] = swCoreCitiesOnlineDevCardsLayout;
		sDefaultLayouts[swCoreCitiesOnlineDevCardsVolcano] = swCoreCitiesOnlineDevCardsVolcanoLayout;
		sDefaultLayouts[swCoreCitiesOnlineNetworkCities] = swCoreCitiesOnlineNetworkCitiesLayout;
		sDefaultLayouts[swCoreCitiesOnlineNetworkCitiesVolcano] = swCoreCitiesOnlineNetworkCitiesVolcanoLayout;
		sDefaultLayouts[swCoreCitiesOnlineNetworkDevCards] = swCoreCitiesOnlineNetworkDevCardsLayout;
		sDefaultLayouts[swCoreCitiesOnlineNetworkDevCardsVolcano] = swCoreCitiesOnlineNetworkDevCardsVolcanoLayout;
	}

	wxString FindDefaultLayout(const wxString& layoutPath)
	{
		EnsureDefaultLayouts();

		LayoutMap::const_iterator it = sDefaultLayouts.find(layoutPath);
		if(sDefaultLayouts.end() != it)
		{
			return it->second;
		}

		wxString stripped = layoutPath;
		stripped.Replace(wxT("Hotseat"), wxEmptyString);
		it = sDefaultLayouts.find(stripped);
		if(sDefaultLayouts.end() != it)
		{
			return it->second;
		}

		return wxEmptyString;
	}

	void InjectEventsPane(wxString& layout)
	{
		if( (true == layout.IsEmpty()) ||
			(wxNOT_FOUND != layout.Find(wxT("name=Events"))))
		{
			return;
		}

		static const wxString eventsPane =
			wxT("|name=Events;caption=Events;state=2044;dir=2;layer=1;row=0;")
			wxT("pos=2;prop=118077;bestw=165;besth=150;minw=165;minh=150;")
			wxT("maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1");

		const int dock = layout.Find(wxT("|dock_size"));
		if(wxNOT_FOUND == dock)
		{
			layout += eventsPane;
		}
		else
		{
			layout.insert(static_cast<size_t>(dock), eventsPane);
		}
	}
}

//---------------------------- PUBLIC           -----------------------------//
void LayoutManager::LoadLayout(wxString& layout)
{
	wxString layoutPath;
	CreateLayoutPath(layoutPath);

	const wxString defaultLayout = FindDefaultLayout(layoutPath);
	wxString path = swLayouts + swForwardSlash + layoutPath;

	layout = CONFIG.read<wxString>(swConfigSectionUI, path, defaultLayout);
	if(true == layout.IsEmpty())
	{
		layout = defaultLayout;
	}

	InjectEventsPane(layout);
}

void LayoutManager::SaveLayout(const wxString& layout)
{
	wxString layoutPath;
	CreateLayoutPath(layoutPath);

	wxString path = swLayouts + swForwardSlash + layoutPath;

	CONFIG.write<wxString>(swConfigSectionUI, path, layout);
}

void LayoutManager::RestoreLayout(wxString& layout)
{
	wxString layoutPath;
	CreateLayoutPath(layoutPath);

	layout = FindDefaultLayout(layoutPath);
	InjectEventsPane(layout);

	wxString path = swLayouts + swForwardSlash + layoutPath;
	CONFIG.write<wxString>(swConfigSectionUI, path, layout);
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
