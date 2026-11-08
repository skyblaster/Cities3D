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


#include "BuildUI.h"
#include "BuildData.h"
#include "Controller.h"
#include "DataObject.h"
#include "ImageEngine.h"
#include "Game.h"
#include "IGameObject.h"
#include "SettlersGLCanvas.h"
#include "SkinManager.h"
#include "ConfigDatabase.h"
#include "BaseBitmapButton.h"
#include "IRuleEngine.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

class wxBuildBitmap
	: public wxBaseBitmapButton
	, public BuildPopup
{
public:
#ifdef __WIN32__
	//disables the 'this' : used in base member initializer list warning
	#pragma warning(disable:4355)
#endif
	wxBuildBitmap(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap)
	: wxBaseBitmapButton(parent, id, wxPoint(1, 1), 
		wxSize(bitmap.GetWidth(), bitmap.GetHeight()))
	, BuildPopup(this)
	{
		SetBitmaps(bitmap, bitmap);
	}

	void SetBitmaps(const wxBitmap& enabled, const wxBitmap& disabled)
	{
		wxBaseBitmapButton::SetBitmaps(enabled, disabled);
	}
};

//encapsulates the retrieval of the bitmap for the wxBuildUI
class BuildBitmap
{
public:
	virtual wxBitmap GetBitmap(const wxSize &size) const=0;
	virtual void SetPlayerColor(ColorType) {}
	virtual ColorType GetPlayerColor() const
	{
		return CR_WHITE;
	}

	void SetIconCount(wxInt32 icons)
	{
		mNumIcons = icons;
	}

	wxInt32 GetIconCount() const
	{
		return mNumIcons;
	}

	const HashString& GetIconName() const
	{
		return mIconName;
	}

	void OverlayIcons(wxBitmap& bitmap) const
	{
		if( (0 >= mNumIcons) ||
			(false == bitmap.IsOk()) ||
			(false == mIconImage.IsOk()))
		{
			return;
		}

		const wxInt32 width = bitmap.GetWidth();
		const wxInt32 height = bitmap.GetHeight();
		if( (4 > width) ||
			(4 > height))
		{
			return;
		}

		// 0.9.7 draws the 16px trade art at full size in the bottom-right
		// corner. Shrinking it to a quarter of a short pane turns the mark
		// into a speck. Only shrink when the art is taller than the cell.
		const wxInt32 cell = std::min(width, height);
		wxImage scaled = mIconImage;
		if( (scaled.GetWidth() > cell) ||
			(scaled.GetHeight() > cell))
		{
			const double fit = std::min(
				static_cast<double>(cell) / scaled.GetWidth(),
				static_cast<double>(cell) / scaled.GetHeight());
			const wxInt32 badgeW = std::max(1, static_cast<wxInt32>(
				scaled.GetWidth() * fit));
			const wxInt32 badgeH = std::max(1, static_cast<wxInt32>(
				scaled.GetHeight() * fit));
			scaled = mIconImage.Scale(badgeW, badgeH, wxIMAGE_QUALITY_NEAREST);
		}

		wxBitmap icon(scaled);
		icon.SetMask(new wxMask(icon, SKIN.Element(shIconTransparent)));

		wxMemoryDC dc;
		dc.SelectObject(bitmap);

		const wxInt32 w = icon.GetWidth();
		const wxInt32 h = icon.GetHeight();
		wxInt32 x = width - w;
		wxInt32 y = height - h;

		for(wxInt32 i = 0; i < mNumIcons; ++i)
		{
			if( (0 <= x) &&
				(0 <= y))
			{
				dc.DrawBitmap(icon, x, y, true);
			}

			y -= h;
			if(0 > y)
			{
				y = height - h;
				x -= w;
			}
		}

		dc.SelectObject(wxNullBitmap);
	}

protected:
	BuildBitmap(const HashString& iconName) 
		: mNumIcons(0)
		, mIconName(iconName)
	{
		mIconImage = IMAGE.GetImage(shBuildUIIcons, iconName);
	}

	virtual ~BuildBitmap(){}

private:
	wxInt32 mNumIcons;
	wxImage mIconImage;
	HashString mIconName;
};

namespace
{
	enum
	{
		//stupid wxWidgets doesn't allow the same button ID
		//in Release mode--we should never have more than 100 wxBuildUIs
		//on the screen at once :P
		ID_BUTTON0		= 0,
		ID_BUTTON100	= 100
	};

	struct no_delete_build
	{
		void operator()(void *){}
	};

	wxInt32 sButtonOffset = 0;

	wxColour sBackgroundBuild;
	wxColour sOutline;
	wxColour sClickable;

	void PunchBuildBackground(wxImage& image)
	{
		if( (false == image.IsOk()) ||
			(NULL == image.GetData()))
		{
			return;
		}

		const wxColour trans = SKIN.Element(shIconTransparent);
		const wxColour skinBg = SKIN.Element(shBackground);
		unsigned char* p = image.GetData();
		const size_t pixels = static_cast<size_t>(image.GetWidth()) *
			image.GetHeight();
		for(size_t i = 0; i < pixels; ++i)
		{
			const bool punch =
				( (12 > p[0]) && (12 > p[1]) && (12 > p[2]) ) ||
				( (p[0] == trans.Red()) &&
				  (p[1] == trans.Green()) &&
				  (p[2] == trans.Blue()) ) ||
				( (p[0] == skinBg.Red()) &&
				  (p[1] == skinBg.Green()) &&
				  (p[2] == skinBg.Blue()) );
			if(true == punch)
			{
				p[0] = p[1] = p[2] = 0;
			}
			p += 3;
		}
	}
	
	class wxBuildUIBitmap : public BuildBitmap
	{
	public:
		wxBuildUIBitmap(const HashString &imageKey, const HashString &imageName,
			const HashString& iconName)
			: BuildBitmap(iconName)
			, mImageKey(imageKey)
			, mImageName(imageName) {}

		virtual wxBitmap GetBitmap(const wxSize &size) const
		{
			wxImage image = IMAGE.GetImage(mImageKey, mImageName, size);

			wxBitmap bitmap(image);

			OverlayIcons(bitmap);

			return bitmap;
		}

	private:
		HashString mImageKey;
		HashString mImageName;
	};

	class BuildGLBitmap : public BuildBitmap
	{
	public:
		BuildGLBitmap(wxSettlersGLCanvas *canvas, 
			const GameObjectPtr &gameObject, const DrawObjectPtr &drawObject,
			const HashString &image, const HashString &mask, 
			const HashString& iconName)
			: BuildBitmap(iconName)
			, mPlayerColor(CR_WHITE)
			, mCanvas(canvas)
			, mGameObject(gameObject)
			, mDrawObject(drawObject)
			, mImage(image)
			, mMask(mask) {}

		wxBitmap GetBitmap(const wxSize &size) const
		{
			wxBitmap bitmap;

			if( (TRUE == CONFIG.read<wxInt32>(swConfigSectionOpenGL,
				swRenderBitmaps, TRUE)) &&
				(NULL != mCanvas) &&
				(NULL != mDrawObject))
			{
				if(mGameObject)
				{
					mGameObject->color(mPlayerColor);
				}

				Vector vector(10.0f, 10.0f, 10.0f);
				bitmap = mCanvas->ConvertGLtoWX(mDrawObject, size.GetWidth(),
					size.GetHeight(), vector);

				// Keep the lit model and drop the capture's black backdrop so
				// the icon background stays the skin, as in 0.9.7.
				if(true == bitmap.IsOk())
				{
					wxImage image = bitmap.ConvertToImage();
					PunchBuildBackground(image);
					bitmap = wxBitmap(image);
				}
			}

			if(false == bitmap.IsOk())
			{
				bitmap = wxBitmap(
					IMAGE.Colorize(mImage, mMask, mPlayerColor, size, true));

				if(true == bitmap.IsOk())
				{
					wxImage image = bitmap.ConvertToImage();
					PunchBuildBackground(image);
					bitmap = wxBitmap(image);
				}
			}

			// Draw the bank mark before masking. wx 3 clips a draw to the
			// bitmap's mask, so a mark painted on the punched corner is
			// thrown away and the road shows no trade indicator.
			OverlayIcons(bitmap);
			if(true == bitmap.IsOk())
			{
				bitmap.SetMask(new wxMask(bitmap, *wxBLACK));
			}

			return bitmap;
		}

		virtual void SetPlayerColor(ColorType color)
		{
			mPlayerColor = color;
		}

		virtual ColorType GetPlayerColor() const
		{
			return mPlayerColor;
		}

	private:
		ColorType mPlayerColor;
		wxSettlersGLCanvas *mCanvas;
		GameObjectPtr mGameObject;
		DrawObjectPtr mDrawObject;
		HashString mImage;
		HashString mMask;
	};

	const wxSize sMinSizeBuild = wxSize(50, 50);
}

//---------------------------- PUBLIC           -----------------------------//
wxBuildUI::wxBuildUI(wxWindow* parent, const BuildDataPtr &build, 
	wxSettlersGLCanvas* canvas)
: wxBaseUI<wxOwnerDrawWindow>(parent, sMinSizeBuild)
, BuildPopup(this)
, mLastIndex(-1)
, mClickable(false)
{
	SetMinSize(sMinSizeBuild);

	//create the build bitmap we'll use later on
	if(build->mGameObject)
	{
		wxASSERT(NULL != canvas);

		mBuildBitmap = BuildBitmapPtr(
			new BuildGLBitmap(canvas, build->mGameObject, build->mDrawObject, 
			build->mImageKey, build->mImageName, build->mIconName));
	}
	else
	{
		mBuildBitmap = BuildBitmapPtr(new wxBuildUIBitmap(build->mImageKey, 
			build->mImageName, build->mIconName));
	}

	mRule = build->mRule;
	mLogic = build->mLogic;
	mLegend = build->mLegend;
	mLogicCost = build->mLogicCost;
	mKeyEvent = shEvent;
	mKeyEvent += build->mKeyEvent;

	// Set the button ID properly.
	mButtonID = ID_BUTTON0 + sButtonOffset++;

	//create the bitmap window
	mBitmap = boost::shared_ptr<wxBuildBitmap>(
			new wxBuildBitmap(this, mButtonID, 
			wxBitmap(1, 1)), 
			no_delete_build());

	mBitmap->SetText(mLegend);
	mBitmap->SetLogicCost(mLogicCost);
	mBitmap->SetKeyEvent(mKeyEvent);

	SetText(mLegend);
	SetLogicCost(mLogicCost);
	SetKeyEvent(mKeyEvent);

	sBackgroundBuild = SKIN.Element(shBackground);
	sOutline = SKIN.Element(shGameUIOutline);
	sClickable = SKIN.Element(shGameUIClickable);

	wxSizeEvent event(sMinSizeBuild);
	Size(event);

	mBitmap->Enable(false);
	mBitmap->Show(false);

	Enable(false);
	Show(true);
	
	Controller::get().AddReceiver(shEventBuildUI, &wxBuildUI::OnUpdate, 
		this);
	Controller::get().AddReceiver(shEventShutdownUI, &wxBuildUI::OnShutdownUI, 
		this);
	Controller::get().AddReceiver(shEventResetBitmaps,
		&wxBuildUI::OnResetBitmaps, this);
	Controller::get().AddReceiver(mKeyEvent, &wxBuildUI::OnKeyEvent, this);
}

wxBuildUI::~wxBuildUI()
{
	Controller::get().RemoveReceiver(shEventBuildUI, &wxBuildUI::OnUpdate, 
		this);
	Controller::get().RemoveReceiver(shEventShutdownUI, &wxBuildUI::OnShutdownUI, 
		this);
	Controller::get().RemoveReceiver(shEventResetBitmaps,
		&wxBuildUI::OnResetBitmaps, this);
	Controller::get().RemoveReceiver(mKeyEvent, &wxBuildUI::OnKeyEvent, this);
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
BEGIN_EVENT_TABLE(wxBuildUI, wxBaseUI<wxOwnerDrawWindow>)
	EVT_COMMAND_RANGE(ID_BUTTON0, ID_BUTTON100, wxEVT_COMMAND_BUTTON_CLICKED, 
	wxBuildUI::OnButton)
END_EVENT_TABLE()

void wxBuildUI::CreateDisabledBitmap()
{
	wxBitmap temp = mEnabledBitmap;

	if(shTrade != mBuildBitmap->GetIconName())
	{
		// This makes me ill.
		wxInt32 icons = mBuildBitmap->GetIconCount();
		if(0 == icons)
		{
			mBuildBitmap->SetIconCount(1);
		}

		mBuildBitmap->OverlayIcons(temp);
		mBuildBitmap->SetIconCount(0);
	}

	if(false == temp.IsOk())
	{
		return;
	}

	wxImage image = temp.ConvertToImage();
	if( (false == image.IsOk()) || (NULL == image.GetData()) )
	{
		return;
	}

	wxInt32 size = image.GetHeight() * image.GetWidth();

	wxColour trans = SKIN.Element(shIconTransparent);
	unsigned char r = trans.Red();
	unsigned char g = trans.Green();
	unsigned char b = trans.Blue();

	// Convert the image to grayscale.
	unsigned char* data = image.GetData();
	unsigned char* p = data;
	unsigned char luma = 0;

	for(wxInt32 i = 0; i < size; ++i)
	{
		// Keep the icon-transparent colour, and near-black backdrop, see-through
		// so the diagonal hatch shows behind every piece. The model itself is
		// half of standard luma, matching the 0.9.7 grey pieces.
		const bool backdrop =
			( (p[0] == r) && (p[1] == g) && (p[2] == b) ) ||
			( (12 > p[0]) && (12 > p[1]) && (12 > p[2]) );
		if(true == backdrop)
		{
			p[0] = r;
			p[1] = g;
			p[2] = b;
		}
		else
		{
			float shade = (p[0] * 0.299f) + (p[1] * 0.587f) + (p[2] * 0.114f);
			shade *= 0.50f;
			if(255.0f < shade)
			{
				shade = 255.0f;
			}
			luma = static_cast<unsigned char>(shade);
			if(0 < luma)
			{
				p[0] = p[1] = p[2] = luma;
			}
			else
			{
				p[0] = r;
				p[1] = g;
				p[2] = b;
			}
		}

		p += 3;
	}

	mDisabledBitmap = wxBitmap(image);

	// Create the mask for it.
	mDisabledBitmap.SetMask(new wxMask(mDisabledBitmap, trans));
}

void wxBuildUI::RebuildBitmaps(const wxSize& windowSize)
{
	wxSize size = windowSize;
	if( (2 >= size.x) ||
		(2 >= size.y))
	{
		size = GetSize();
	}
	if( (2 >= size.x) ||
		(2 >= size.y))
	{
		size = sMinSizeBuild;
	}

	wxSize bitmapSize = size - wxSize(2, 2);
	bitmapSize.x = std::max(bitmapSize.x, 1);
	bitmapSize.y = std::max(bitmapSize.y, 1);

	// A non-square cell stretches the model. Always capture a square.
	const wxInt32 side = std::min(bitmapSize.x, bitmapSize.y);
	bitmapSize = wxSize(side, side);

	// Grey the unaffordable icon from an uncoloured model, then capture the
	// player-coloured model used when the piece can be built.
	const ColorType color = mBuildBitmap->GetPlayerColor();
	mBuildBitmap->SetPlayerColor(CR_WHITE);
	mEnabledBitmap = mBuildBitmap->GetBitmap(bitmapSize);
	CreateDisabledBitmap();

	if(CR_WHITE != color)
	{
		mBuildBitmap->SetPlayerColor(color);
	}

	mEnabledBitmap = mBuildBitmap->GetBitmap(bitmapSize);
	mBitmap->SetSize(1, 1, bitmapSize.x, bitmapSize.y);
	mBitmap->SetBitmaps(mEnabledBitmap, mDisabledBitmap);
	Refresh();
}

void wxBuildUI::Paint(wxMemoryDC &dc)
{
	wxSize size = GetSize();

	dc.SetBrush(wxBrush(sBackgroundBuild, wxSOLID));
	dc.SetPen(wxPen(mClickable ? sClickable : sOutline, 1, wxSOLID));
	dc.DrawRectangle(0, 0, size.x, size.y);

	// Hatch marks a piece that cannot be built. The model itself carries the
	// player colour when it can.
	if(false == mClickable)
	{
		wxBrush brush(sOutline);
		brush.SetStyle(wxBDIAGONAL_HATCH);
		dc.SetBrush(brush);
		dc.DrawRectangle(0, 0, size.x, size.y);

		if(true == mDisabledBitmap.IsOk())
		{
			dc.DrawBitmap(mDisabledBitmap, 1, 1, true);
		}
	}
}

void wxBuildUI::Size(wxSizeEvent& event)
{
	if(wxSize(0, 0) != event.GetSize())
	{
		RebuildBitmaps(event.GetSize());
	}
}

void wxBuildUI::OnUpdate(const DataObject &object)
{
	wxASSERT(2 == object.numItems());

	bool enable = object.read<bool>();
	const GamePtr &game = object.read<GamePtr>(1);

	// Before doing anything else, we have to check to see if it is the player
	// at this computer.
	DataObject input(-1), output;
	RULE.Decide(shLogicIsThisPlayer, input, output);

	bool thisPlayer = output.read<bool>();
	bool afford = false;
	wxInt32 icons = 0;

	wxInt32 numIcons = mBuildBitmap->GetIconCount();

	if( (true == thisPlayer) &&
		(true == enable))
	{
		DataObject input(0), output;
		RULE.Decide(mLogic, input, output);

		if(0 == output.numItems())
		{
			afford = false;
		}
		else
		{
			afford = output.read<bool>();

			if(1 < output.numItems())
			{
				icons = output.read<wxInt32>(1);
			}
		}
	}

	mBuildBitmap->SetIconCount(icons);
	mClickable = enable && thisPlayer && afford;

	wxInt32 index = -1;
	if(game)
	{
		index = game->read<wxInt32>(shCurrentPlayer);
	}

	if( (NULL != game) &&
		(0 <= index) &&
		(game->numPlayers() > index))
	{
		const ColorType playerColor = game->playerGame(index).color();
		mLastIndex = index;
		mBuildBitmap->SetPlayerColor(playerColor);
		RebuildBitmaps(GetSize());
		CallAfter([this]() { RebuildBitmaps(GetSize()); });
	}

	mBitmap->SetBitmaps(mEnabledBitmap, mDisabledBitmap);
	mBitmap->Enable(mClickable);
	mBitmap->Show(mClickable);
	Enable(mClickable);

	Refresh();
}

void wxBuildUI::OnShutdownUI(wxInt32)
{
	mClickable = false;
	mBitmap->Enable(false);
	Enable(false);

	Refresh();
}

void wxBuildUI::OnResetBitmaps(wxInt32)
{
	RebuildBitmaps(GetSize());
}

void wxBuildUI::OnKeyEvent(wxInt32)
{
	// If we're enabled, click.
	if(true == IsEnabled())
	{
		wxCommandEvent event;
		event.SetId(mButtonID);
		OnButton(event);
	}
}

void wxBuildUI::OnButton(wxCommandEvent &event)
{
	if(event.GetId() == mButtonID)
	{
		// Disable the wxBuildUI to prevent double-clicks.
		Enable(false);

		// Clear any current undo state.  Any time they buy a new item or
		// initiate a new action, it prevents going back.
		RULE.Execute(shRuleBreakUndo, DataObject());

		RULE.Execute(mRule, DataObject());
	}
}
