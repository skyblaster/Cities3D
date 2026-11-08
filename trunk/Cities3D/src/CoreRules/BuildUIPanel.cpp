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
#include "BuildUIPanel.h"
#include "GameEngine.h"
#include "IPluginEngine.h"
#include "BuildData.h"
#include "BuildUI.h"
#include "SkinManager.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	const wxInt32 sBuildIconPad = 2;
	const wxInt32 sBuildIconMin = 50;
}

//---------------------------- PUBLIC           -----------------------------//
wxBuildUIPanel::wxBuildUIPanel(wxWindow* parent, wxSettlersGLCanvas* canvas)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0)
, mMaxPerRow(0)
, mRowCount(0)
, mInResize(false)
{
	SetBackgroundColour(SKIN.Element(shBackground));

	wxSizer* horz = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* vert = new wxBoxSizer(wxVERTICAL);

    typedef std::vector<BuildDataPtr> BuildDataArray;
	BuildDataArray array;
	PLUGIN.LoadBuildData(array);

	// Create a map of prioritized build data to use.
	typedef std::multimap<wxInt32, BuildDataPtr> BuildDataMap;
	typedef std::map<wxInt32, BuildDataMap> RowMap;
	RowMap rowMap;

	BuildDataArray::const_iterator it = array.begin(), itEnd = array.end();
	for(; it != itEnd; ++it)
	{
		BuildDataPtr data = (*it);

		// Sort by row, then priority within the row.
		rowMap[data->mRow].insert(std::make_pair(data->mPriority, data));
	}

	typedef std::vector<wxSizer*> SizerArray;
	SizerArray sizers(rowMap.size(), 0);

	for(size_t i = 0; i < rowMap.size(); ++i)
	{
		sizers[i] = new wxBoxSizer(wxHORIZONTAL);
	}

	wxSize minSize = wxSize(0, 0);
	size_t index = 0;
	bool calcX = true;
	bool calcY = true;

	// Now that everything is in rows by priority, create the UIs.
    RowMap::const_iterator it2 = rowMap.begin(), it2End = rowMap.end();
    for(; it2 != it2End; ++it2)
    {
		// Go in reverse order here because sizers add left to right, but the
		// priority goes right to left.
        BuildDataMap::const_reverse_iterator it3 = it2->second.rbegin(),
			it3End = it2->second.rend();

		// If it's a new row, use the next sizer.
		wxSizer* sizer = sizers[index];
		++index;

		wxInt32 iconsInRow = 0;
		// Equal stretch on both ends and between icons. A proportion on the
		// icon itself cannot do this: the icon's max size is its min size, so
		// wx leaves the spare width in one block on the right.
		sizer->AddStretchSpacer(1);
        for(; it3 != it3End; ++it3)
        {
			wxBuildUI* ui = new wxBuildUI(this, it3->second, canvas);
			mBuildUIs.push_back(ui);
			++iconsInRow;

			const wxSize iconSize(sBuildIconMin, sBuildIconMin);
			ui->SetMinSize(iconSize);
			ui->SetMaxSize(iconSize);
            
			wxSize min = ui->GetMinSize();
			if(true == calcX)
			{
				minSize.x += min.x + sBuildIconPad;
			}
			if(true == calcY)
			{
				minSize.y += min.y + sBuildIconPad;
				calcY = false;
			}

			sizer->Add(ui, 0, wxALIGN_CENTER_VERTICAL | wxALL, sBuildIconPad);
			sizer->AddStretchSpacer(1);
		}

		if(iconsInRow > mMaxPerRow)
		{
			mMaxPerRow = iconsInRow;
		}

		calcX = false;
		calcY = true;
		vert->Add(sizer, 0, wxEXPAND);
    }

	mRowCount = static_cast<wxInt32>(index);

	vert->AddStretchSpacer(1);
	minSize += wxSize(2, 2);

	horz->Add(vert, 1, wxEXPAND | wxTOP | wxLEFT | wxBOTTOM, 2);

	SetMinSize(minSize);
	SetSizer(horz);
	Bind(wxEVT_SIZE, &wxBuildUIPanel::OnPanelSize, this);
	CallAfter(&wxBuildUIPanel::ResizeIcons);
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
void wxBuildUIPanel::OnPanelSize(wxSizeEvent& event)
{
	ResizeIcons();
	event.Skip();
}

void wxBuildUIPanel::ResizeIcons()
{
	if( (true == mInResize) ||
		(0 >= mMaxPerRow) ||
		(0 >= mRowCount) ||
		(true == mBuildUIs.empty()))
	{
		return;
	}

	mInResize = true;

	const wxSize client = GetClientSize();
	const wxInt32 width = client.GetWidth();
	const wxInt32 height = client.GetHeight();
	const wxInt32 padX = 2 + (mMaxPerRow * (sBuildIconPad * 2));
	const wxInt32 padY = 4 + (mRowCount * (sBuildIconPad * 2));
	if( (padX >= width) ||
		(padY >= height))
	{
		mInResize = false;
		return;
	}

	// Fit a square in both directions. Sizing from width alone makes a very
	// wide pane squash the icons, and the model is then drawn stretched.
	const wxInt32 cellW = (width - padX) / mMaxPerRow;
	const wxInt32 cellH = (height - padY) / mRowCount;
	wxInt32 cell = std::min(cellW, cellH);
	if(1 > cell)
	{
		cell = 1;
	}

	const wxSize iconSize(cell, cell);
	for(size_t i = 0; i < mBuildUIs.size(); ++i)
	{
		wxBuildUI* ui = mBuildUIs[i];
		if(NULL == ui)
		{
			continue;
		}

		ui->SetMinSize(iconSize);
		ui->SetMaxSize(iconSize);
	}

	if(NULL != GetSizer())
	{
		GetSizer()->Layout();
	}

	mInResize = false;
}
