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

//---------------------------- DEFINES          -----------------------------//

// Define once in the .cpp that owns Type (the shared library that exports it).
// DLL_EXPORT keeps one process-wide instance visible to plugins under
// -fvisibility=hidden. An inline Meyers singleton in this header would give
// each .so its own copy, so plugin UI registration never reaches the app.
#define IMPLEMENT_SINGLETON(Type) \
	template<> \
	DLL_EXPORT Type &Singleton<Type>::get() \
	{ \
		static Type theSingleton; \
		return theSingleton; \
	}

//---------------------------- TYPEDEFS         -----------------------------//

//---------------------------- CLASSES          -----------------------------//

//---------------------------------------------------------------------------//
// Class: Singleton
// 
// A template class that provides an implementation of the
// Singleton pattern.  Any class may derive from Singleton to
// become a Singleton.  This pattern should not be abused.
//
// Singletons are globally visible across all modules (plugins).
// Use <LocalSingleton> for per-plugin state (e.g. RuleSetDatabase).
//
// Project:
//     <DatabaseCore>
//
// Include:
//     Singleton.h
//
// Usage:
// (start example)
//     class MySingleton : public Singleton<MySingleton>
//     {
//     public:
//         /* ... */
//     private:
//         MySingleton();
//
//         //since the constructor is private, Singleton must be a friend
//         //so that this class can be instantiated.
//         friend class Singleton<MySingleton>;
//     };
//
//     // In MySingleton.cpp (the exporting shared library):
//     IMPLEMENT_SINGLETON(MySingleton)
// ·
// (end)
//
template <typename T>
class Singleton
{
	//-----------------------------------------------------------------------//
	// Section: Public
	//
public:
	//-----------------------------------------------------------------------//
	// Group: Static Functions
	//

	//-----------------------------------------------------------------------//
	// Function: get
	//
	// The one and only one method of accessing a Singleton. Defined out of
	// line via IMPLEMENT_SINGLETON(T) in the exporting shared library.
	//
	// Returns:
	//     The Singleton object.
	//
	static T &get();

	//-----------------------------------------------------------------------//
	// Section: Protected
	//
protected:
	//-----------------------------------------------------------------------//
	// Group: Constructors
	//

	//-----------------------------------------------------------------------//
	// Constructor: Singleton
	// 
	// The Singleton constructor.  It is protected so that the Singleton cannot 
	// be instantiated unless it is derived from.
	//
	Singleton()
	{
	}

	//-----------------------------------------------------------------------//
	// Group: Destructor
	//

	//-----------------------------------------------------------------------//
	// Destructor: ~Singleton
	// 
	// The Singleton destructor.  It is protected so that the Singleton cannot 
	// be instantiated unless it is derived from.
	//
	virtual ~Singleton()
	{
	}

	//-----------------------------------------------------------------------//
	// Section: Private
	//
private:
	//-----------------------------------------------------------------------//
	// Group: Copy Constructor
	//

	//-----------------------------------------------------------------------//
	// Constructor: Singleton
	// 
	// The Singleton copy constructor.  It is private and never instantiated so 
	// that any developer trying to make a copy of a singleton object will get 
	// a compiler error.
	//
	Singleton(const Singleton &);

	//-----------------------------------------------------------------------//
	// Group: Operators
	//

	//-----------------------------------------------------------------------//
	// Function: operator=
	// 
	// The Singleton assignment operator.  It is private and never instantiated
	// so that any developer trying to create a Singleton by assigning it will 
	// get a compiler error.
	//
	Singleton &operator=(const Singleton &);
};

//---------------------------- PROTOTYPES       -----------------------------//
