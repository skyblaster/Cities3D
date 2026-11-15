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
: mBox(box)
, mTop(NULL)
, mBottom(NULL)
, mLeft(NULL)
, mRight(NULL)
, mDoubleBorder(false)
, mCurve(4)
{
	wxASSERT(NULL != box);
	wxWindow* parent = box->GetParent();
	wxASSERT(NULL != parent);

	mLeftColor = *wxWHITE;
	mRightColor = *wxWHITE;

	mTop = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxBORDER_NONE);
	mBottom = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxBORDER_NONE);
	mLeft = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxBORDER_NONE);
	mRight = new wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxBORDER_NONE);

	mTop->SetBackgroundStyle(wxBG_STYLE_PAINT);
	mBottom->SetBackgroundStyle(wxBG_STYLE_PAINT);
	mLeft->SetBackgroundStyle(wxBG_STYLE_PAINT);
	mRight->SetBackgroundStyle(wxBG_STYLE_PAINT);

	mTop->Bind(wxEVT_PAINT, &wxStaticBoxBorderOverlay::OnTopPaint, this);
	mBottom->Bind(wxEVT_PAINT, &wxStaticBoxBorderOverlay::OnBottomPaint, this);
	mLeft->Bind(wxEVT_PAINT, &wxStaticBoxBorderOverlay::OnTopPaint, this);
	mRight->Bind(wxEVT_PAINT, &wxStaticBoxBorderOverlay::OnBottomPaint, this);

	box->Bind(wxEVT_SIZE, &wxStaticBoxBorderOverlay::OnBoxSize, this);

	SyncToBox();
}

wxStaticBoxBorderOverlay::~wxStaticBoxBorderOverlay()
{
	if(mBox)
	{
		mBox->Unbind(wxEVT_SIZE, &wxStaticBoxBorderOverlay::OnBoxSize, this);
		mBox = NULL;
	}

	wxWindow* edges[4] = { mTop, mBottom, mLeft, mRight };
	for(int i = 0; i < 4; ++i)
	{
		if( (edges[i]) &&
			(false == edges[i]->IsBeingDeleted()))
		{
			edges[i]->Destroy();
		}
	}
	mTop = mBottom = mLeft = mRight = NULL;
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
	SyncToBox();
}

void wxStaticBoxBorderOverlay::SetCurve(wxInt32 curve)
{
	mCurve = curve;
	Refresh();
}

void wxStaticBoxBorderOverlay::Refresh()
{
	if(mTop) mTop->Refresh();
	if(mBottom) mBottom->Refresh();
	if(mLeft) mLeft->Refresh();
	if(mRight) mRight->Refresh();
}

void wxStaticBoxBorderOverlay::SyncToBox()
{
	if( (NULL == mBox) ||
		(NULL == mTop))
	{
		return;
	}

	const wxPoint pos = mBox->GetPosition();
	const wxSize size = mBox->GetSize();
	const int thickness = mDoubleBorder ? 3 : 2;

	if( (thickness * 2 >= size.GetWidth()) ||
		(thickness * 2 >= size.GetHeight()))
	{
		return;
	}

	mTop->SetSize(pos.x, pos.y, size.GetWidth(), thickness);
	mBottom->SetSize(pos.x, pos.y + size.GetHeight() - thickness,
		size.GetWidth(), thickness);
	mLeft->SetSize(pos.x, pos.y + thickness, thickness,
		size.GetHeight() - (thickness * 2));
	mRight->SetSize(pos.x + size.GetWidth() - thickness, pos.y + thickness,
		thickness, size.GetHeight() - (thickness * 2));

	Refresh();
}

void wxStaticBoxBorderOverlay::OnBoxSize(wxSizeEvent& event)
{
	SyncToBox();
	event.Skip();
}

void wxStaticBoxBorderOverlay::OnTopPaint(wxPaintEvent& event)
{
	wxWindow* win = wxDynamicCast(event.GetEventObject(), wxWindow);
	if(NULL == win)
	{
		return;
	}

	wxPaintDC dc(win);
	const wxSize size = win->GetClientSize();

	// Top edge and left edge share this handler: left is solid left color;
	// top is split left/right like FinalTradeBox.
	if(win == mLeft)
	{
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(mLeftColor));
		dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
		return;
	}

	const int halfX = size.GetWidth() / 2;
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(mLeftColor));
	dc.DrawRectangle(0, 0, halfX, size.GetHeight());
	dc.SetBrush(wxBrush(mRightColor));
	dc.DrawRectangle(halfX, 0, size.GetWidth() - halfX, size.GetHeight());
}

void wxStaticBoxBorderOverlay::OnBottomPaint(wxPaintEvent& event)
{
	wxWindow* win = wxDynamicCast(event.GetEventObject(), wxWindow);
	if(NULL == win)
	{
		return;
	}

	wxPaintDC dc(win);
	const wxSize size = win->GetClientSize();

	if(win == mRight)
	{
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(mRightColor));
		dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
		return;
	}

	const int halfX = size.GetWidth() / 2;
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(mLeftColor));
	dc.DrawRectangle(0, 0, halfX, size.GetHeight());
	dc.SetBrush(wxBrush(mRightColor));
	dc.DrawRectangle(halfX, 0, size.GetWidth() - halfX, size.GetHeight());
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
