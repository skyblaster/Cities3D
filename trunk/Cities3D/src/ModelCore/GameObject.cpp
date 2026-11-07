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
#include "GameObject.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//

//---------------------------- PUBLIC           -----------------------------//
GameObject::GameObject()
: mDisplayFlags(0)
, mID(0)
, mCoords(0.0, 0.0, 0.0)
, mRotation(0.0, 0.0, 0.0)
{
}

bool GameObject::isset(const ObjectFlag flag) const
{
	const unsigned value = static_cast<unsigned>(flag);
	if(16U <= value)
	{
		return false;
	}

	return (mDisplayFlags & (1 << value)) || false;
}

void GameObject::set(const ObjectFlag flag)
{
	const unsigned value = static_cast<unsigned>(flag);
	if(16U <= value)
	{
		return;
	}

	mDisplayFlags |= (1 << value);
}

bool GameObject::isset(const wxInt32 flag) const
{
	if( (0 > flag) ||
		(16 <= flag))
	{
		return false;
	}

	return (mDisplayFlags & (1 << (16 + flag))) || false;
}

void GameObject::set(const wxInt32 flag)
{
	if( (0 > flag) ||
		(16 <= flag))
	{
		return;
	}

	mDisplayFlags |= (1 << (16 + flag));
}

void GameObject::reapplyFlags(const ObjectFlag *flags, size_t count,
	bool allowDim)
{
	const wxInt32 hiddenBit = (1 << Hidden);
	const bool hidden = (0 != (mDisplayFlags & hiddenBit));

	mDisplayFlags = 0;

	if(NULL != flags)
	{
		for(size_t i = 0; i < count; ++i)
		{
			const unsigned value = static_cast<unsigned>(flags[i]);
			if(16U <= value)
			{
				continue;
			}
			if((false == allowDim) && (Dim == flags[i]))
			{
				continue;
			}

			mDisplayFlags |= (1 << value);
		}
	}

	if(true == hidden)
	{
		mDisplayFlags |= hiddenBit;
	}
}

void GameObject::load(wxDataInputStream &stream)
{
	stream >> mDisplayFlags;
	stream >> mID;
	stream >> mCoords.x;
	stream >> mCoords.y;
	stream >> mCoords.z;
	stream >> mRotation.x;
	stream >> mRotation.y;
	stream >> mRotation.z;
}

void GameObject::save(wxDataOutputStream &stream) const
{
	stream << mDisplayFlags;
	stream << mID;
	stream << mCoords.x;
	stream << mCoords.y;
	stream << mCoords.z;
	stream << mRotation.x;
	stream << mRotation.y;
	stream << mRotation.z;
}

//---------------------------- PROTECTED        -----------------------------//

//---------------------------- PRIVATE          -----------------------------//

