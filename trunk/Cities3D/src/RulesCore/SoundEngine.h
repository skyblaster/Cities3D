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

//---------------------------- USER INCLUDES    -----------------------------//
#include "RulesExport.h"
#include "ISoundEngine.h"

//---------------------------- DEFINES          -----------------------------//

//---------------------------- TYPEDEFS         -----------------------------//

//---------------------------- CLASSES          -----------------------------//

//---------------------------------------------------------------------------//
// Class: SoundEngine
// 
// The default implementation of the <ISoundEngine> interface.  
// Plays one-shot files from the sounds directory using miniaudio.
//
// Derived From:
//     <ISoundEngine>
//
// Project:
//     <RulesCore>
//
// Include:
//     SoundEngine.h
//
class RULES_EXPORT_SPEC SoundEngine : public ISoundEngine
{
	//-----------------------------------------------------------------------//
	// Section: Public
	//
public:
	//-----------------------------------------------------------------------//
	// Group: Constructors
	//

	//-----------------------------------------------------------------------//
	// Constructor: SoundEngine
	// 
	// The SoundEngine constructor.  The constructor
	// initializes miniaudio so that it is ready to play sounds.
	// 
	SoundEngine();

	//-----------------------------------------------------------------------//
	// Destructor: ~SoundEngine
	//
	// Releases the miniaudio engine.
	//
	~SoundEngine();

	//-----------------------------------------------------------------------//
	// Group: Functions
	//

	//-----------------------------------------------------------------------//
	// Function: PlaySound
	//
	// Plays a sound immediately.  If the given sound is
	// not valid, or does not exist, an error log message
	// is displayed in the main window with the appropriate
	// information.
	//
	// Parameters:
	//     sound - The name of the sound file in the sounds directory.
	//
	void PlaySound(const wxString &sound) const;

	//-----------------------------------------------------------------------//
	// Section: Private
	//
private:
	//-----------------------------------------------------------------------//
	// Group: Variables
	//

	struct Impl;
	Impl *mpImpl;
};

//---------------------------- PROTOTYPES       -----------------------------//
