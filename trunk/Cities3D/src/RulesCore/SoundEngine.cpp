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
#include "SoundEngine.h"
#include "FilePath.h"
#include "ConfigDatabase.h"

#ifdef CITIES3D_USE_MINIAUDIO
#ifdef new
#undef new
#endif
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#endif

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

struct SoundEngine::Impl
{
#ifdef CITIES3D_USE_MINIAUDIO
	ma_engine engine;
	bool ready;
#endif
};

//---------------------------- PUBLIC           -----------------------------//
SoundEngine::SoundEngine()
: mpImpl(new Impl)
{
#ifdef CITIES3D_USE_MINIAUDIO
	mpImpl->ready = false;

	ma_result result = ma_engine_init(NULL, &mpImpl->engine);
	if(MA_SUCCESS != result)
	{
		wxLogError(wxT("Can't initialize sound engine: %s"),
			wxString::FromUTF8(ma_result_description(result)).c_str());
		return;
	}

	mpImpl->ready = true;
#endif
}

SoundEngine::~SoundEngine()
{
#ifdef CITIES3D_USE_MINIAUDIO
	if((NULL != mpImpl) && (true == mpImpl->ready))
	{
		ma_engine_uninit(&mpImpl->engine);
	}
#endif
	delete mpImpl;
}

void SoundEngine::PlaySound(const wxString &sound) const
{
	//see if we even need to play a sound
	if(FALSE == CONFIG.read<wxInt32>(swConfigSectionGeneral, swPlaySounds, TRUE))
	{
		return;
	}

#ifdef CITIES3D_USE_MINIAUDIO
	if((NULL == mpImpl) || (false == mpImpl->ready))
	{
		return;
	}

	wxASSERT(false == sound.IsEmpty());

	const wxString path = FilePath::Sound(sound);
	if(true == path.IsEmpty())
	{
		wxLogError(wxT("Sound Error: %s, file not found"), sound.c_str());
		return;
	}

	const wxScopedCharBuffer utf8 = path.ToUTF8();
	const ma_result result = ma_engine_play_sound(&mpImpl->engine, utf8.data(),
		NULL);
	if(MA_SUCCESS != result)
	{
		wxLogError(wxT("Sound Error: %s, %s"), path.c_str(),
			wxString::FromUTF8(ma_result_description(result)).c_str());
	}
#else
	wxUnusedVar(sound);
#endif
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//
