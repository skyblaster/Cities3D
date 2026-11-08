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
#include <wx/snglinst.h>

//---------------------------- USER INCLUDES    -----------------------------//
#include "app.h"
#include "SettlersFrame.h"
#include "GameEngine.h"
#include "ImageEngine.h"
#include "ConfigDatabase.h"
#include "SelectLanguageDialog.h"
#include "SplashDialog.h"
#include "Controller.h"
#include "FilePath.h"
#include "OfficialMapDatabase.h"
#include "IKeyMapEngine.h"
#include <wx/ffile.h>
#include <wx/msgdlg.h>
#include <cstdio>

#ifndef __WIN32__
#include "cities3d.xpm"
#endif

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_APP(wxCities3DApp)

//---------------------------- STATICS          -----------------------------//

//---------------------------- PUBLIC           -----------------------------//
bool wxCities3DApp::OnInit()
{
	std::fprintf(stderr, "cities3d: OnInit begin\n");
	std::fflush(stderr);

	// If we're writing out maps, do it now.
#if MAPS_BUILD_MAPS
	OfficialMapDatabase::get().WriteMaps();
#endif	// MAPS_BUILD_MAPS

	// Set our running instance so CitiesOnline knows if we're up or not.
	mpInstanceChecker.reset(new wxSingleInstanceChecker(swCities3D));

	Controller::get().AddReceiver(shEventAddCatalog, 
		&wxCities3DApp::OnAddCatalog, this);
	Controller::get().AddReceiver(shEventResetLocale, 
		&wxCities3DApp::OnResetLocale, this);

	//turn on all image handlers
	::wxInitAllImageHandlers();
	::wxHandleFatalExceptions(true);

	// Cache the breadcrumb path while wx is healthy. OnFatalException must not
	// call into FilePath::UserDataDir()/Mkdir — that re-enters DatabaseCore/wx
	// from a signal handler and can SEGV again (see coredump __restore_rt).
	mLastRuleBreadcrumb = FilePath::LastRuleBreadcrumb();
	std::fprintf(stderr, "cities3d: breadcrumb=%s\n",
		(const char *)mLastRuleBreadcrumb.mb_str(wxConvUTF8));
	std::fflush(stderr);

	// See if they have a default language selected.
	wxString str = CONFIG.read<wxString>(swConfigSectionGeneral, swLanguage, swEmpty);

	// If they don't, run the selection dialog, which will force them to
	// make a selection.
	if(true == str.IsEmpty())
	{
#ifdef __WIN32__
		wxSelectLanguageDialog dlg(NULL);
		dlg.ShowModal();
#else
		// On non-Windows sytems, we just default to the system language
		const wxLanguageInfo *info(wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT));
		wxLogDebug(wxT("No language set, default %s"), info->Description.c_str());
		CONFIG.write<wxString>(swConfigSectionGeneral, swLanguage, info->Description);
#endif
	}

#ifdef __WXGTK__
	std::fprintf(stderr, "cities3d: loading gtkrc\n");
	std::fflush(stderr);
	const wxString nat_theme_path = FilePath::Skin(swGtkrc);
	if(false == (SetNativeTheme(nat_theme_path)))
	{
		wxLogDebug(wxT("Can't load gtkrc file: %s"), nat_theme_path);
	}
#endif // __WXGTK__

	// Set the locale.
	std::fprintf(stderr, "cities3d: resetting locale\n");
	std::fflush(stderr);
	OnResetLocale();
	
	// Splash screen.
	std::fprintf(stderr, "cities3d: creating splash\n");
	std::fflush(stderr);
	boost::scoped_ptr<wxSplashDialog> dialog(new wxSplashDialog);
	dialog->Create(NULL);
	std::fprintf(stderr, "cities3d: splash created\n");
	std::fflush(stderr);

	// Start up the game engine while we load.
	std::fprintf(stderr, "cities3d: starting engine\n");
	std::fflush(stderr);
	GameEngine::get().StartEngine();
	std::fprintf(stderr, "cities3d: engine started\n");
	std::fflush(stderr);

	wxBOOL max = CONFIG.read(swConfigSectionUI, swMainFrameMaximized, FALSE);

	std::fprintf(stderr, "cities3d: creating main frame\n");
	std::fflush(stderr);
	mFrame = new wxSettlersFrame(NULL, wxPoint(50, 50), wxSize(800, 600));
	mFrame->CentreOnScreen();
	SetTopWindow(mFrame);

#ifdef __WIN32__
	wxIcon appIcon(swACities3D, wxBITMAP_TYPE_ICO_RESOURCE);
	if(true == appIcon.IsOk())
	{
		mFrame->SetIcon(appIcon);
	}
#else
	mFrame->SetIcon(wxICON(a_cities3d));
#endif

	if(TRUE == max)
	{
		mFrame->Maximize();
	}

	mFrame->Show(true);

	// Now that we have started the engine and created the frame, send the
	// startup signal.
	Controller::get().Transmit(shEventStartup, 0);

	std::fprintf(stderr, "cities3d: OnInit complete\n");
	std::fflush(stderr);
    return true;

}

void wxCities3DApp::OnFatalException()
{
	wxString last = wxT("(unknown)");
	// Use only the path cached in OnInit — do not call FilePath helpers here.
	if(false == mLastRuleBreadcrumb.IsEmpty())
	{
		wxFFile breadcrumb(mLastRuleBreadcrumb, wxT("r"));
		if(true == breadcrumb.IsOpened())
		{
			breadcrumb.ReadAll(&last);
		}
	}

	// Prefer stderr inside Flatpak/gdb; avoid wxMessageBox here because it
	// re-enters GTK/fontconfig from a fatal signal path and can SEGV again.
	std::fprintf(stderr,
		"cities3d: fatal exception; last rule: %s\n",
		(const char *)last.mb_str(wxConvUTF8));
	std::fflush(stderr);
}

int wxCities3DApp::OnExit()
{
	//clean up all memory before exiting
	GameEngine::get().StopEngine(true);
	
	Controller::get().RemoveReceiver(shEventAddCatalog, 
		&wxCities3DApp::OnAddCatalog, this);
	Controller::get().RemoveReceiver(shEventResetLocale, 
		&wxCities3DApp::OnResetLocale, this);

	return wxApp::OnExit();
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
BEGIN_EVENT_TABLE(wxCities3DApp, wxApp)
	EVT_KEY_DOWN(wxCities3DApp::OnKeyDown)
END_EVENT_TABLE()

void wxCities3DApp::OnResetLocale(wxInt32)
{
	wxString str = CONFIG.read<wxString>(swConfigSectionGeneral, swLanguage, 
		swEmpty);

	if(false == str.IsEmpty())
	{
		// Find the language info for this string and set it as the locale.
		const wxLanguageInfo *info = wxLocale::FindLanguageInfo(str);
		wxASSERT(NULL != info);

		if(NULL != info)
		{
			mpLocale.reset(new wxLocale(info->Language));

			// Immediately add the correct lookup paths.
			wxArrayString localePaths = FilePath::EnumLocaleDirs(info->CanonicalName);
			for(size_t i = 0; i < localePaths.GetCount(); ++i)
				mpLocale->AddCatalogLookupPathPrefix(localePaths[i]);

			// Finally, add the Cities3D domain, which covers Cities3D and all 
			// Core DLL files.  Individual Rules files will have their domains 
			// added as they are loaded.
			mpLocale->AddCatalog(swCities3D);

			// CoreRules is special.  Because it is basically permanently 
			// loaded, it needs to be added so that strings in static memory
			// within the class will be able to look in the right location.
			// The other plugins are loaded and immediately unloaded when they
			// are discovered, so they are added as catalogs before they are
			// actually used.
			mpLocale->AddCatalog(swCoreRules);
		}
	}
}

void wxCities3DApp::OnAddCatalog(const wxString &str)
{
	wxASSERT(mpLocale);

	if(mpLocale)
	{
		mpLocale->AddCatalog(str);
	}
}

void wxCities3DApp::OnKeyDown(wxKeyEvent &event)
{
	// Intercept all keys and pass them to the key map engine.
	KEYMAP.ProcessKeyEvent(event);
}
