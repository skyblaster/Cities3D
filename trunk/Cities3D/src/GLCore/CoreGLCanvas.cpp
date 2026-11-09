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
#include <GL/glew.h>
#ifdef __WIN32__
#include <GL/wglew.h>
#endif
#include <cstdio>

//---------------------------- USER INCLUDES    -----------------------------//
#include "CoreGLCanvas.h"

//---------------------------- TYPEDEFS         -----------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------- STATICS          -----------------------------//
namespace
{
	void InitGL(wxCoreGLCanvas *canvas)
	{
		wxASSERT(NULL != canvas);

		if(NULL != canvas->GetContext())
		{
			//set shading model to smooth
			glShadeModel(GL_SMOOTH);

			//adjust the perspective hwxInt32
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

			//set the clear color
			glClearColor(0., 0., 0., 0.);

			//set up depth testing
			glClearDepth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);

			//set up lighting
			GLfloat amb[] = {1.0f, 1.0f, 1.0f, 1.0f};
			GLfloat dif[] = {1.0f, 1.0f, 1.0f, 1.0f};
			GLfloat pos[] = {0.0f, 1.0f, 4.0f, 1.0f};

			glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
			glLightfv(GL_LIGHT0, GL_POSITION, pos);
			glEnable(GL_LIGHT0);

			//turn on lighting
			glEnable(GL_LIGHTING);
		}
	}
}

//---------------------------- PUBLIC           -----------------------------//
const int* wxCoreGLCanvas::ChooseDisplayAttribList(int requestedDepth)
{
	static int attrib[8];

	int depths[4];
	int count = 0;
	if(0 < requestedDepth)
	{
		depths[count++] = requestedDepth;
	}
	if(24 != requestedDepth)
	{
		depths[count++] = 24;
	}
	if(16 != requestedDepth)
	{
		depths[count++] = 16;
	}
	depths[count++] = 0;

	for(int i = 0; i < count; ++i)
	{
		int a = 0;
		attrib[a++] = WX_GL_RGBA;
		attrib[a++] = WX_GL_DOUBLEBUFFER;
		if(0 < depths[i])
		{
			attrib[a++] = WX_GL_DEPTH_SIZE;
			attrib[a++] = depths[i];
		}
		attrib[a] = 0;

		if(true == wxGLCanvas::IsDisplaySupported(attrib))
		{
			return attrib;
		}
	}

	return NULL;
}

//---------------------------- PROTECTED        -----------------------------//
wxCoreGLCanvas::wxCoreGLCanvas(wxWindow *parent, wxWindowID id, 
							   const wxPoint& pos, const wxSize& size, 
							   long style, const wxString& name, 
							   wxInt32 *attrib)
#if wxCHECK_VERSION(2, 9, 0)
: wxGLCanvas(parent, id, attrib, pos, size, style, name)
#else
: wxGLCanvas(parent, id, pos, size, style, name, attrib)
#endif
, mpTopLevelWindow(dynamic_cast<wxTopLevelWindow *>(parent))
, mDirty(true)
, m_glContext(NULL)
, mGLInitialized(false)
#ifdef __WXGTK__
, mPainted(false)
#endif
{
}

wxCoreGLCanvas::~wxCoreGLCanvas()
{
#if wxCHECK_VERSION(2, 9, 0)
	delete m_glContext;
	m_glContext = NULL;
#endif
}

wxGLContext* wxCoreGLCanvas::GetContext()
{
	#ifdef __WXGTK__
	if(false == IsShown())
	{
		return NULL;
	}
	#endif

#if wxCHECK_VERSION(2, 9, 0)
	if( (NULL == m_glContext) && (NULL != GetHandle()) )
	{
		m_glContext = new wxGLContext(this);
	}
	return m_glContext;
#else
	return wxGLCanvas::GetContext();
#endif
}

void wxCoreGLCanvas::SetCurrent()
{
	wxGLContext* context = GetContext();
	if(NULL == context)
	{
		return;
	}

#if wxCHECK_VERSION(2, 9, 0)
	wxGLCanvas::SetCurrent(*context);
#else
	wxGLCanvas::SetCurrent();
#endif

	if(false == mGLInitialized)
	{
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if(GLEW_OK != err)
		{
			// GLEW built for GLX reports NO_GLX_DISPLAY when the current
			// context is EGL (wxGTK default since 3.1.5). Core GL entry
			// points from glewContextInit are still valid; ignore that
			// specific code so hotseat is not blocked by a false popup.
			const bool ignoreNoGlx =
#ifdef GLEW_ERROR_NO_GLX_DISPLAY
				(GLEW_ERROR_NO_GLX_DISPLAY == err);
#else
				false;
#endif
			const char* glewMsg = reinterpret_cast<const char*>(
				glewGetErrorString(err));
			std::fprintf(stderr, "cities3d: glewInit err=%u (%s)%s\n",
				static_cast<unsigned>(err),
				glewMsg ? glewMsg : "?",
				ignoreNoGlx ? " (ignored)" : "");
			std::fflush(stderr);
			if(false == ignoreNoGlx)
			{
				wxLogError(
					wxT("No OpenGL extensions are available on this hardware."));
			}
		}
		else
		{
			std::fprintf(stderr, "cities3d: glewInit ok\n");
			std::fflush(stderr);
		}
		{
			const char* vendor = reinterpret_cast<const char*>(
				glGetString(GL_VENDOR));
			const char* renderer = reinterpret_cast<const char*>(
				glGetString(GL_RENDERER));
			const char* version = reinterpret_cast<const char*>(
				glGetString(GL_VERSION));
			std::fprintf(stderr, "cities3d: GL vendor=%s renderer=%s version=%s\n",
				vendor ? vendor : "?",
				renderer ? renderer : "?",
				version ? version : "?");
			std::fflush(stderr);
		}
#ifdef __WIN32__
		if(WGLEW_EXT_swap_control)
		{
			wglSwapIntervalEXT(0);
		}
#endif
		InitGL(this);
		mGLInitialized = true;
	}
}

void wxCoreGLCanvas::ResizeGL(void)
{
    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
    wxInt32 w, h;
    GetClientSize(&w, &h);

    if (GetContext())
    {
        SetCurrent();
        glViewport(0, 0, (GLint) w, (GLint) h);

		//select the projection matrix and reset it
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//calculate the aspect ratio
		GLfloat dAspect;
		dAspect = (GLfloat) w / (GLfloat) h;
		gluPerspective(45.0f, dAspect, 0.1, 250.);

		//reset the modelview matrix
		glMatrixMode(GL_MODELVIEW);
    }
}

void wxCoreGLCanvas::SetDirty(bool dirty)
{
	mDirty = dirty;
	
	if(true == dirty)
	{
		Refresh(false);
	}
}

//---------------------------- PRIVATE          -----------------------------//
BEGIN_EVENT_TABLE(wxCoreGLCanvas, wxGLCanvas)
    EVT_SIZE(wxCoreGLCanvas::OnSize)
	EVT_PAINT(wxCoreGLCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(wxCoreGLCanvas::OnEraseBackground)
END_EVENT_TABLE()

void wxCoreGLCanvas::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);

	// Only paint if we're in the background.
	if( (NULL != mpTopLevelWindow) &&
		(false == mpTopLevelWindow->IsActive()))
	{
		mDirty = true;
	}
	
	if(true == GetParent()->IsShown())
	{
		Paint();
	}

#	ifdef __WXGTK__
	mPainted = true;
#	endif
}

void wxCoreGLCanvas::OnSize(wxSizeEvent& event)
{
#	ifdef __WXGTK__
	if(false == mPainted)
	{
		return;
	}
#	endif

    //this is also necessary to update the context on some platforms
    event.Skip();

	ResizeGL();

	SetDirty();
}

void wxCoreGLCanvas::OnEraseBackground(wxEraseEvent&)
{
    //do nothing, which avoids flashing in windows
}
