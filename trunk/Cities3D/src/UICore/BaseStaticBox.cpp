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
#include "BaseStaticBox.h"
#include "Controller.h"
#include "SkinManager.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	wxInt32 sCurve = 0;
	wxColour sOutline_BaseStaticBox;
}

//---------------------------- PUBLIC           -----------------------------//
#ifndef __WIN32__
wxStaticBoxSecondLabel::wxStaticBoxSecondLabel(wxWindow* box)
: mBox(box)
, mLabel(NULL)
{
	wxASSERT(NULL != box);
	wxASSERT(NULL != box->GetParent());

	mLabel = new wxStaticText(box->GetParent(), wxID_ANY, wxEmptyString);
	mLabel->Hide();

	box->Bind(wxEVT_SIZE, &wxStaticBoxSecondLabel::OnBoxSize, this);
}

wxStaticBoxSecondLabel::~wxStaticBoxSecondLabel()
{
	if(mBox)
	{
		mBox->Unbind(wxEVT_SIZE, &wxStaticBoxSecondLabel::OnBoxSize, this);
		mBox = NULL;
	}

	if( (mLabel) &&
		(false == mLabel->IsBeingDeleted()))
	{
		mLabel->Destroy();
		mLabel = NULL;
	}
}

void wxStaticBoxSecondLabel::SetLabel(const wxString& label)
{
	wxASSERT(NULL != mLabel);

	mLabel->SetLabel(label);
	if(true == label.IsEmpty())
	{
		mLabel->Hide();
	}
	else
	{
		mLabel->Show();
		Reposition();
		mLabel->Raise();
	}
}

void wxStaticBoxSecondLabel::SetForegroundColour(const wxColour& colour)
{
	wxASSERT(NULL != mLabel);
	mLabel->SetForegroundColour(colour);
}

void wxStaticBoxSecondLabel::SetBackgroundColour(const wxColour& colour)
{
	wxASSERT(NULL != mLabel);
	mLabel->SetBackgroundColour(colour);
}

void wxStaticBoxSecondLabel::SetFont(const wxFont& font)
{
	wxASSERT(NULL != mLabel);
	mLabel->SetFont(font);
	Reposition();
}

void wxStaticBoxSecondLabel::OnBoxSize(wxSizeEvent& event)
{
	Reposition();
	event.Skip();
}

void wxStaticBoxSecondLabel::Reposition()
{
	if( (NULL == mLabel) ||
		(NULL == mBox) ||
		(true == mLabel->GetLabel().IsEmpty()))
	{
		return;
	}

	const wxSize textSize = mLabel->GetBestSize();
	const wxSize boxSize = mBox->GetSize();
	const wxPoint boxPos = mBox->GetPosition();

	// Match MSW PaintForeground: rc.right - textWidth - 8.
	const int x = boxPos.x + boxSize.GetWidth() - textSize.GetWidth() - 8;
	const int y = boxPos.y;
	mLabel->SetSize(x, y, textSize.GetWidth(), textSize.GetHeight());
}

wxStaticBoxBorderOverlay::wxStaticBoxBorderOverlay(wxWindow* box)
: wxWindow(box->GetParent(), wxID_ANY, wxDefaultPosition, wxDefaultSize,
	wxBORDER_NONE)
, mBox(box)
, mDoubleBorder(false)
, mCurve(4)
{
	wxASSERT(NULL != box);
	wxASSERT(NULL != box->GetParent());

	SetBackgroundStyle(wxBG_STYLE_PAINT);
	mLeftColor = *wxWHITE;
	mRightColor = *wxWHITE;

	box->Bind(wxEVT_SIZE, &wxStaticBoxBorderOverlay::OnBoxSize, this);
	Bind(wxEVT_PAINT, &wxStaticBoxBorderOverlay::OnPaint, this);

	SyncToBox();
}

wxStaticBoxBorderOverlay::~wxStaticBoxBorderOverlay()
{
	if(mBox)
	{
		mBox->Unbind(wxEVT_SIZE, &wxStaticBoxBorderOverlay::OnBoxSize, this);
		mBox = NULL;
	}
}

void wxStaticBoxBorderOverlay::SetColors(const wxColour& left,
	const wxColour& right)
{
	mLeftColor = left;
	mRightColor = right;
	Refresh();
}

void wxStaticBoxBorderOverlay::SetDoubleBorder(bool enabled)
{
	mDoubleBorder = enabled;
	UpdateShape();
	Refresh();
}

void wxStaticBoxBorderOverlay::SetCurve(wxInt32 curve)
{
	mCurve = curve;
	Refresh();
}

void wxStaticBoxBorderOverlay::SyncToBox()
{
	if(NULL == mBox)
	{
		return;
	}

	const wxPoint pos = mBox->GetPosition();
	const wxSize size = mBox->GetSize();
	SetSize(pos.x, pos.y, size.GetWidth(), size.GetHeight());
	UpdateShape();
	// Do not Raise above title label overlays — those are created after this
	// window so they stay readable on the border ring.
	Refresh();
}

void wxStaticBoxBorderOverlay::OnBoxSize(wxSizeEvent& event)
{
	SyncToBox();
	event.Skip();
}

void wxStaticBoxBorderOverlay::UpdateShape()
{
	const wxSize size = GetSize();
	if( (4 > size.GetWidth()) ||
		(4 > size.GetHeight()))
	{
		return;
	}

	// Border ring only so trade content / labels show through the hole.
	const int inset = mDoubleBorder ? 4 : 2;
	wxRegion region(0, 0, size.GetWidth(), size.GetHeight());
	region.Subtract(wxRect(inset, inset,
		size.GetWidth() - (inset * 2), size.GetHeight() - (inset * 2)));
	SetShape(region);
}

void wxStaticBoxBorderOverlay::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	const wxSize size = GetClientSize();
	if( (0 >= size.GetWidth()) ||
		(0 >= size.GetHeight()))
	{
		return;
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	const int halfX = size.GetWidth() / 2;
	const int yStart = 0;
	const int height = size.GetHeight() - yStart;

	// Left half of the rounded frame.
	dc.SetPen(wxPen(mLeftColor, mDoubleBorder ? 2 : 1));
	dc.SetClippingRegion(0, 0, halfX, size.GetHeight());
	dc.DrawRoundedRectangle(0, yStart, size.GetWidth(), height, mCurve);
	if(true == mDoubleBorder)
	{
		dc.DrawRoundedRectangle(2, yStart + 2, size.GetWidth() - 4,
			height - 4, mCurve);
	}
	dc.DestroyClippingRegion();

	// Right half (may match left for a uniform Stockyard border).
	dc.SetPen(wxPen(mRightColor, mDoubleBorder ? 2 : 1));
	dc.SetClippingRegion(halfX, 0, size.GetWidth() - halfX, size.GetHeight());
	dc.DrawRoundedRectangle(0, yStart, size.GetWidth(), height, mCurve);
	if(true == mDoubleBorder)
	{
		dc.DrawRoundedRectangle(2, yStart + 2, size.GetWidth() - 4,
			height - 4, mCurve);
	}
	dc.DestroyClippingRegion();
}
#endif

wxBaseStaticBox::wxBaseStaticBox(wxWindow* parent, wxWindowID id, 
								 const wxString& label, const wxPoint& pos, 
								 const wxSize& size, const HashString &color,
								 const HashString &font)
: wxStaticBox(parent, id, label, pos, size)
, mColor(color)
, mFont(font)
{
	Controller::get().AddReceiver(shEventSkin, &wxBaseStaticBox::OnSkin, this);
#ifndef __WIN32__
	mSecondLabelOverlay.reset(new wxStaticBoxSecondLabel(this));
#endif
	OnSkin();
}

wxBaseStaticBox::~wxBaseStaticBox()
{
	Controller::get().RemoveReceiver(shEventSkin, &wxBaseStaticBox::OnSkin, 
			this);
}

void wxBaseStaticBox::SetSecondLabel(const wxString& label)
{
	mSecondLabel = label;
#ifndef __WIN32__
	if(mSecondLabelOverlay)
	{
		mSecondLabelOverlay->SetLabel(label);
	}
#endif
	Refresh();
}

//---------------------------- PROTECTED        -----------------------------//
#ifdef __WIN32__
void wxBaseStaticBox::PaintBackground(wxDC& dc, const struct tagRECT& rc)
{
	wxColour background = GetBackgroundColour();

	dc.SetPen(wxPen(background));
	dc.SetBrush(wxBrush(background));
	dc.DrawRectangle(0, 0, rc.right, rc.bottom);
}

void wxBaseStaticBox::PaintForeground(wxDC& dc, const struct tagRECT& rc)
{
	wxColour background = GetBackgroundColour();

	dc.SetFont(GetFont());
	dc.SetTextForeground(GetForegroundColour());
	dc.SetTextBackground(GetBackgroundColour());

	wxInt32 x, y;
	const wxString& label = GetLabel();
	dc.GetTextExtent(label, &x, &y);

	// Draw border.
	dc.SetPen(wxPen(sOutline_BaseStaticBox));
	dc.SetBrush(wxBrush(background));
	dc.DrawRoundedRectangle(0, y / 2, rc.right, rc.bottom - (y / 2), sCurve);

	// Black out area for text.
	dc.SetPen(wxPen(background));
	dc.SetBrush(wxBrush(background));
	dc.DrawRectangle(7, 0, x + 4, y);

	// Draw text.	
	dc.DrawText(label, 9, 0);

	// Draw second label if it exists.
	if(false == mSecondLabel.IsEmpty())
	{
		dc.GetTextExtent(mSecondLabel, &x, &y);

		// Black out area for text.
		dc.SetPen(wxPen(background));
		dc.SetBrush(wxBrush(background));
		dc.DrawRectangle(rc.right - x - 11, 0, x + 4, y);

		dc.DrawText(mSecondLabel, rc.right - x - 8, 0);
	}
}
#endif

//---------------------------- PRIVATE          -----------------------------//
void wxBaseStaticBox::OnSkin(wxInt32)
{
	SetBackgroundColour(SKIN.Element(shBackground));
	SetForegroundColour(SKIN.Element(mColor));
	SetFont(SKIN.Font(mFont));

	sCurve = SKIN.Size(shFrameCurve);
	sOutline_BaseStaticBox = SKIN.Element(shGameUIOutline);

#ifndef __WIN32__
	if(mSecondLabelOverlay)
	{
		mSecondLabelOverlay->SetBackgroundColour(SKIN.Element(shBackground));
		mSecondLabelOverlay->SetForegroundColour(SKIN.Element(mColor));
		mSecondLabelOverlay->SetFont(SKIN.Font(mFont));
		mSecondLabelOverlay->SetLabel(mSecondLabel);
	}
#endif

	Refresh();
}
