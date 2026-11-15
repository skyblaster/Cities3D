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
#pragma once

#include "style.h"	//READ THIS BEFORE MAKING ANY CHANGES TO THIS FILE!!!

//---------------------------- SYSTEM INCLUDES  -----------------------------//
#include <boost/scoped_ptr.hpp>

//---------------------------- USER INCLUDES    -----------------------------//
#include "UIExport.h"

//---------------------------- DEFINES          -----------------------------//

//---------------------------- TYPEDEFS         -----------------------------//
class wxStaticText;
class wxSizeEvent;
class wxPaintEvent;

//---------------------------- CLASSES          -----------------------------//

#ifndef __WIN32__
//---------------------------------------------------------------------------//
// Class: wxStaticBoxSecondLabel
//
// Right-aligned second title for a wxStaticBox on platforms that lack the
// MSW PaintForeground hook (wxGTK). Parent is the box's parent so the text
// sits on the frame edge instead of inside the content area.
//
class UI_EXPORT_SPEC wxStaticBoxSecondLabel
{
public:
	explicit wxStaticBoxSecondLabel(wxWindow* box);
	~wxStaticBoxSecondLabel();

	void SetLabel(const wxString& label);
	void SetForegroundColour(const wxColour& colour);
	void SetBackgroundColour(const wxColour& colour);
	void SetFont(const wxFont& font);

private:
	void OnBoxSize(wxSizeEvent& event);
	void Reposition();

	wxWindow* mBox;
	wxStaticText* mLabel;
};

//---------------------------------------------------------------------------//
// Class: wxStaticBoxBorderOverlay
//
// Colored trade-state border for wxStaticBox on non-Windows. MSW paints the
// border in PaintForeground; wxGTK cannot. This sibling window tracks the
// box, punches a shaped hole over the content, and strokes left/right colors
// (optionally doubled for accept flashes).
//
class UI_EXPORT_SPEC wxStaticBoxBorderOverlay : public wxWindow
{
public:
	explicit wxStaticBoxBorderOverlay(wxWindow* box);
	virtual ~wxStaticBoxBorderOverlay();

	void SetColors(const wxColour& left, const wxColour& right);
	void SetDoubleBorder(bool enabled);
	void SetCurve(wxInt32 curve);
	void SyncToBox();

private:
	void OnPaint(wxPaintEvent& event);
	void OnBoxSize(wxSizeEvent& event);
	void UpdateShape();

	wxWindow* mBox;
	wxColour mLeftColor;
	wxColour mRightColor;
	bool mDoubleBorder;
	wxInt32 mCurve;
};
#endif

//---------------------------------------------------------------------------//
// Class: wxBaseStaticBox
// 
// A basic static box.  A static box is drawn on the screen
// as a rectangle with an optional title text across the top
// of the rectangle starting in the upper-left corner.  The class 
// wxBaseStaticBox provides a simple base class for all static boxes
// in the game, providing a common look and feel across UI objects.  
// It is in all ways like a wxStaticBox, except that it sets its 
// background color to the background color of its parent window, 
// sets its foreground to the given color, and allows for 
// a user-defined font.
//
// Derived From:
//     wxStaticBox (see <http://www.wxwindows.org/manuals/2.4.2/wx359.htm>)
//
// Project:
//     <UICore>
//
// Include:
//     BaseStaticBox.h
//
class UI_EXPORT_SPEC wxBaseStaticBox : public wxStaticBox
{
	//-----------------------------------------------------------------------//
	// Section: Public
	//
public:
	//-----------------------------------------------------------------------//
	// Group: Constructors
	//

	//-----------------------------------------------------------------------//
	// Constructor: wxBaseStaticBox
	//
	// The wxBaseStaticBox constructor.  Creates the static box with the
	// given text color, font, label, etc.
	//
	// Parameters:
	//     parent - The parent window.  Must not be NULL.
	//     id - The message handling ID.  Should be a unique (to the parent 
	//          window) ID, if the parent window wants to receive messages from
	//          the control.
	//     label - The title text.
	//     color - The title text color.
	//     font - The title text font.
	//     pos - The starting position of the static box.
	//     size - The starting size of the static box.
	//
	wxBaseStaticBox(wxWindow* parent, wxWindowID id, const wxString& label, 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, 
		const HashString &color = shDefaultText,
		const HashString &font = shDefaultFont);

	//-----------------------------------------------------------------------//
	// Group: Destructor
	//

	//-----------------------------------------------------------------------//
	// Destructor: wxBaseStaticBox
	//
	// The wxBaseStaticBox destructor.
	//
	~wxBaseStaticBox();

	//-----------------------------------------------------------------------//
	// Group: Functions
	//

	//-----------------------------------------------------------------------//
	// Function: SetSecondLabel
	//
	// Sets the second, right-side label for the box.  (Optional)
	//
	void SetSecondLabel(const wxString& label);

#ifdef __WIN32__
protected:
	// paint the given rectangle with our background brush/colour
    virtual void PaintBackground(wxDC& dc, const struct tagRECT& rc);
    // paint the foreground of the static box
    virtual void PaintForeground(wxDC& dc, const struct tagRECT& rc);
#endif

	//-----------------------------------------------------------------------//
	// Section: Private
	//
private:
	//-----------------------------------------------------------------------//
	// Group: Game Event Functions
	//

	//-----------------------------------------------------------------------//
	// Function: OnSkin
	//
	// Handles a new skin being selected.
	//
	void OnSkin(wxInt32 = 0);

	//-----------------------------------------------------------------------//
	// Group: Variables
	//

	//-----------------------------------------------------------------------//
	// String: mColor
	//
	// The color of the text.
	//
	HashString mColor;

	//-----------------------------------------------------------------------//
	// String: mFont
	//
	// The font of the text.
	//
	HashString mFont;

	//-----------------------------------------------------------------------//
	// String: mSecondLabel
	//
	// The second label for the box, if any.
	//
	wxString mSecondLabel;

#ifndef __WIN32__
	boost::scoped_ptr<wxStaticBoxSecondLabel> mSecondLabelOverlay;
#endif
};

//---------------------------- PROTOTYPES       -----------------------------//
