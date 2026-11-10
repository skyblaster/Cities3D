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
#include "CardsDialog.h"
#include "ISizer.h"
#include "CardListCtrl.h"
#include "BaseButton.h"
#include "BaseStaticBox.h"
#include "FittedStaticText.h"
#include "Controller.h"
#include "ImageEngine.h"
#include "SkinManager.h"
#include "DataObject.h"
#include "IRuleEngine.h"
#include <cstdio>

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	enum
	{
		ID_PLAY_CARD_BUTTON			= 0,
		ID_CARD_LIST				,
	};

	// Paints PlayerCards art directly. wxStaticBitmap::SetBitmap and
	// wxImageCenteredBitmap (DrawBitmap with mask) both left a black hole for
	// these PNGs on wxGTK3 even when ImageEngine loaded them successfully.
	class wxCardImagePanel : public wxPanel
	{
	public:
		wxCardImagePanel(wxWindow* parent, const wxSize& size)
		: wxPanel(parent, wxID_ANY, wxDefaultPosition, size,
			wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE)
		{
			SetBackgroundStyle(wxBG_STYLE_PAINT);
			SetMinSize(size);
			SetBackgroundColour(SKIN.Element(shBackground));
		}

		void SetCardImage(const wxImage& image)
		{
			if( (true == image.IsOk()) &&
				(0 < image.GetWidth()) &&
				(0 < image.GetHeight()))
			{
				mImage = image.Copy();
				const wxSize size(mImage.GetWidth(), mImage.GetHeight());
				SetMinSize(size);
				SetSize(size);
				std::fprintf(stderr,
					"cities3d: card panel set %dx%d ok=%d\n",
					size.x, size.y, mImage.IsOk() ? 1 : 0);
			}
			else
			{
				mImage = wxImage();
				std::fprintf(stderr, "cities3d: card panel cleared (bad image)\n");
			}
			std::fflush(stderr);
			Refresh();
			Update();
		}

	private:
		void OnPaint(wxPaintEvent&)
		{
			wxPaintDC dc(this);
			const wxSize size = GetClientSize();
			const wxColour bg = GetBackgroundColour();
			dc.SetBrush(wxBrush(bg));
			dc.SetPen(wxPen(bg));
			dc.DrawRectangle(0, 0, size.x, size.y);

			if(false == mImage.IsOk())
			{
				return;
			}

			wxImage draw = mImage.Copy();
			if( (size.x > 0) &&
				(size.y > 0) &&
				( (draw.GetWidth() != size.x) ||
				  (draw.GetHeight() != size.y) ))
			{
				draw = mImage.Scale(size.x, size.y, wxIMAGE_QUALITY_HIGH);
			}

			// Drop mask/alpha so GTK cannot treat opaque card RGB as
			// fully transparent when composing the paint.
			if(true == draw.HasMask())
			{
				draw.SetMask(false);
			}
			if(true == draw.HasAlpha())
			{
				draw.ClearAlpha();
			}

			wxBitmap bitmap(draw, 24);
			if(false == bitmap.IsOk())
			{
				bitmap = wxBitmap(draw);
			}
			if(false == bitmap.IsOk())
			{
				std::fprintf(stderr,
					"cities3d: card panel wxBitmap failed from %dx%d image\n",
					draw.GetWidth(), draw.GetHeight());
				std::fflush(stderr);
				return;
			}

			dc.DrawBitmap(bitmap, 0, 0, false);
		}

		wxImage mImage;

		DECLARE_EVENT_TABLE()
	};

	BEGIN_EVENT_TABLE(wxCardImagePanel, wxPanel)
		EVT_PAINT(wxCardImagePanel::OnPaint)
	END_EVENT_TABLE()

	wxCardListCtrl *spCardList = NULL;
	wxBaseButton* spButton = NULL;
	wxCardImagePanel *spCardBitmap = NULL;
	wxBaseStaticBox *spBox = NULL;
	wxFittedStaticText *spText = NULL;
	wxSize sCardImageSize(162, 171);

	class Sizer_CardsDialog : public ISizer
	{
	public:
		Sizer_CardsDialog(wxWindow *window, boost::weak_ptr<Game> game, 
			boost::shared_ptr<wxCardsDialog::CardListCreator> creator,
			const wxString& text)
			: ISizer(window)
			, mGame(game)
			, mCreator(creator)
			, mText(text)
		{}

		void CreateLayout(wxBoxSizer *sizer)
		{
			wxBoxSizer *horz = new wxBoxSizer(wxHORIZONTAL);
			
			wxBoxSizer* vert1 = new wxBoxSizer(wxVERTICAL);
			spCardList = mCreator->CreateCardList(mpWindow, ID_CARD_LIST,
				mGame, wxDefaultPosition, wxSize(150, 200));

			spButton = new wxBaseButton(mpWindow, ID_PLAY_CARD_BUTTON, mText);
			spButton->Disable();

			wxBoxSizer* horz1 = new wxBoxSizer(wxHORIZONTAL);
			horz1->AddStretchSpacer();
			horz1->Add(spButton, 0);
			horz1->AddStretchSpacer();

			vert1->Add(spCardList, 1, wxEXPAND | wxRIGHT | wxLEFT | wxTOP, 4);
			vert1->Add(horz1, 0, wxEXPAND | wxALL, 2);

			wxASSERT(0 < spCardList->GetItemCount());
			const HashString& card = spCardList->GetCardName(0);

			wxImage image = IMAGE.GetImage(shPlayerCards, card);
			if( (false == image.IsOk()) ||
				(0 >= image.GetWidth()) ||
				(0 >= image.GetHeight()))
			{
				std::fprintf(stderr,
					"cities3d: PlayerCards image missing for %s\n",
					card.c_str());
				std::fflush(stderr);
			}
			else
			{
				sCardImageSize = wxSize(image.GetWidth(), image.GetHeight());
				std::fprintf(stderr,
					"cities3d: PlayerCards %s %dx%d\n",
					card.c_str(), sCardImageSize.x, sCardImageSize.y);
				std::fflush(stderr);
			}

			spCardBitmap = new wxCardImagePanel(mpWindow, sCardImageSize);
			if(true == image.IsOk())
			{
				spCardBitmap->SetCardImage(image);
			}

			wxBoxSizer* vert2 = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer* horzBox = new wxBoxSizer(wxHORIZONTAL);
			
			spBox = new wxBaseStaticBox(mpWindow, 0, swEmpty, wxDefaultPosition, 
				wxDefaultSize, shDefaultText, shCardHeaderFont);
			wxStaticBoxSizer* boxSizer = new wxStaticBoxSizer(spBox, wxVERTICAL);
			spText = new wxFittedStaticText(mpWindow, 0, swEmpty, shDefaultText, 
				shInfoFont, wxSize(sCardImageSize.GetWidth(), 0),
				wxST_NO_AUTORESIZE | wxALIGN_CENTER);
			// Fixed size — do not wxEXPAND|wxSHAPED or the panel grows into a
			// black frame around an unloaded / unscaled bitmap.
			boxSizer->Add(spCardBitmap, 0, wxALIGN_CENTER | wxALL, 2);
			boxSizer->Add(spText, 0, wxEXPAND | wxALL, 2);
			horzBox->Add(boxSizer, 0, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 4);

			vert2->Add(horzBox, 0, wxEXPAND);
			vert2->AddStretchSpacer();

			horz->Add(vert1, 1, wxEXPAND);
			horz->Add(vert2, 0, wxEXPAND);

			sizer->Add(horz, 1, wxEXPAND);
		}

	private:
		boost::weak_ptr<Game> mGame;
		boost::shared_ptr<wxCardsDialog::CardListCreator> mCreator;
		wxString mText;
	};
}

//---------------------------- PUBLIC           -----------------------------//
#ifdef __WIN32__
	//disables the 'this' : used in base member initializer list warning
	#pragma warning(disable:4355)
#endif
wxCardsDialog::wxCardsDialog(wxWindow *parent, boost::weak_ptr<Game> game,
	const wxString& buttonText, const wxString& titleText, 
	boost::shared_ptr<CardListCreator> creator, const HashString& rule,
	wxInt32 index)
: wxBaseCloseableDialog(parent, ISizerPtr(new Sizer_CardsDialog(this, game, 
	creator, buttonText)), titleText, swYourCards)
, mRule(rule)
, mIndex(index)
{
	Controller::get().AddReceiver(shEventImageUpdate, 
		&wxCardsDialog::OnImageUpdate, this);

	// Select the first item as a nicety.
	spCardList->SetSelectedItem(0);
}

wxCardsDialog::~wxCardsDialog()
{
	Controller::get().RemoveReceiver(shEventImageUpdate, 
		&wxCardsDialog::OnImageUpdate, this);
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
BEGIN_EVENT_TABLE(wxCardsDialog, wxBaseCloseableDialog)
	EVT_LIST_ITEM_SELECTED(ID_CARD_LIST, wxCardsDialog::OnCardSelected)
	// Do not handle DESELECTED: SetSelectedItem/PopulateList fire deselection
	// with GetSelectedItem()==-1 (or empty card), which cleared a just-loaded
	// PlayerCards preview on wxGTK (see cardimg-3 logs: set ok then cleared).
	EVT_BUTTON(ID_PLAY_CARD_BUTTON, wxCardsDialog::OnPlayCardButton)
END_EVENT_TABLE()

void wxCardsDialog::ResetImage(const HashString& card)
{
	wxString displayText;
	wxString description;

	if(false == card.empty())
	{
		wxInt32 index = spCardList->GetSelectedItem();

		displayText = spCardList->GetItemText(index, swCard);

		DataObject input(card), output;
		RULE.Decide(shLogicCardText, input, output);
		description = output.read<wxString>();
        description += sw10Spaces;

		wxImage image = IMAGE.GetImage(shPlayerCards, card);
		if( (true == image.IsOk()) &&
			(0 < image.GetWidth()) &&
			(0 < image.GetHeight()))
		{
			sCardImageSize = wxSize(image.GetWidth(), image.GetHeight());
			spCardBitmap->SetCardImage(image);
			spCardBitmap->Show(true);
		}
		else
		{
			std::fprintf(stderr,
				"cities3d: PlayerCards image missing for %s\n",
				card.c_str());
			std::fflush(stderr);
			spCardBitmap->SetCardImage(wxImage());
			spCardBitmap->Show(false);
		}
	}
	else
	{
		// Keep the last good preview; empty selection is transient during
		// list rebuilds and must not wipe PlayerCards art.
		std::fprintf(stderr,
			"cities3d: card preview keep (empty selection)\n");
		std::fflush(stderr);
		return;
	}

	spBox->Freeze();
	spBox->SetLabel(displayText);
	spBox->Thaw();

	// Here we have to do some chicanery.
	wxSizer* sizer = spText->GetContainingSizer();
	sizer->Detach(spText);
	spText->Destroy();
	spText = new wxFittedStaticText(this, 0, description, 
		shDefaultText, shInfoFont, 
		wxSize(sCardImageSize.GetWidth(), 0), wxST_NO_AUTORESIZE | 
		wxALIGN_CENTER);
	sizer->Add(spText, 0, wxEXPAND | wxALL, 2);
	
	sizer = GetSizer();
	wxSize minSize = sizer->GetMinSize();
	wxSize size = GetClientSize();
	if(size.y < minSize.y)
	{
		wxSize min = GetMinSize();
		min.y += (minSize.y - size.y);
		SetMinSize(min);

		SetClientSize(wxSize(size.x, minSize.y));
	}

	sizer->Layout();
	spCardBitmap->Refresh();
}

void wxCardsDialog::Close()
{
	Controller::get().Transmit(shEventCards, DataObject(GamePtr(), false));
}

void wxCardsDialog::OnImageUpdate(bool)
{
	// If they update the images, we need to regrab.
	ResetImage(mCurrentCard);
}

void wxCardsDialog::OnCardSelected(wxListEvent &)
{
	wxInt32 index = spCardList->GetSelectedItem();
	if(-1 == index)
	{
		spButton->Enable(false);
		return;
	}

	HashString card = spCardList->GetCardName(index);

	bool enable = false;
	// See if the card can be played.
	if(TRUE == spCardList->GetItemData(index))
	{
		// They have to be the current player as well.
		DataObject input(mIndex), output;
		RULE.Decide(shLogicIsThisPlayer, input, output);
		enable = output.read<bool>();
	}

	spButton->Enable(enable);

	if(mCurrentCard != card)
	{
		ResetImage(card);

		mCurrentCard = card;
	}
}

void wxCardsDialog::OnPlayCardButton(wxCommandEvent &)
{
	// Close things down and play the card.
	Controller::get().Transmit(shEventCards, DataObject(GamePtr(), false));

	wxASSERT(false == mCurrentCard.empty());
	RULE.Execute(mRule, DataObject(mCurrentCard, mIndex));

	OnPlayCard();
}
