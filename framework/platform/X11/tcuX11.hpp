#ifndef _TCUX11_HPP
#define _TCUX11_HPP
/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief X11 utilities.
 *//*--------------------------------------------------------------------*/

#include "tcuDefs.hpp"
#include "gluRenderConfig.hpp"
#include "gluPlatform.hpp"
#include "deMutex.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

namespace tcu
{
namespace x11
{
enum
{
	DEFAULT_WINDOW_WIDTH	= 400,
	DEFAULT_WINDOW_HEIGHT	= 300
};

class EventState
{
public:
				EventState	(void);
	virtual		~EventState	(void);
	void		setQuitFlag	(bool quit);
	bool		getQuitFlag	(void);

protected:
	de::Mutex	m_mutex;
	bool		m_quit;

private:
				EventState	(const EventState&);
	EventState&	operator=	(const EventState&);
};

class DisplayBase
{
public:
					DisplayBase		(EventState& platform);
	virtual			~DisplayBase	(void);
	virtual void	processEvents	(void) = 0;

protected:
	EventState&		m_eventState;

private:
					DisplayBase		(const DisplayBase&);
	DisplayBase&	operator=		(const DisplayBase&);
};

class WindowBase
{
public:
							WindowBase		(void);
	virtual					~WindowBase		(void);

	virtual void			setVisibility	(bool visible) = 0;

	virtual void			processEvents	(void) = 0;
	virtual DisplayBase&	getDisplay		(void) = 0;

	virtual void			getDimensions	(int* width, int* height) const = 0;
	virtual void			setDimensions	(int width, int height) = 0;

protected:
	bool					m_visible;

private:
							WindowBase		(const WindowBase&);
	WindowBase&				operator=		(const WindowBase&);
};

class XlibDisplay : public DisplayBase
{
public:
					XlibDisplay		(EventState& platform, const char* name);
	virtual			~XlibDisplay	(void);

	::Display*		getXDisplay		(void) { return m_display;		}
	Atom			getDeleteAtom	(void) { return m_deleteAtom;	}

	::Visual*		getVisual		(VisualID visualID);
	bool			getVisualInfo	(VisualID visualID, XVisualInfo& dst);
	void			processEvents	(void);
	void			processEvent	(XEvent& event);

protected:
	::Display*		m_display;
	Atom			m_deleteAtom;

private:
					XlibDisplay		(const XlibDisplay&);
	XlibDisplay&	operator=		(const XlibDisplay&);
};

class XlibWindow : public WindowBase
{
public:
					XlibWindow			(XlibDisplay& display, int width, int height,
										::Visual* visual);
					~XlibWindow			(void);

	void			setVisibility	(bool visible);

	void			processEvents	(void);
	DisplayBase&	getDisplay		(void) { return (DisplayBase&)m_display; }
	::Window&		getXID			(void) { return m_window; }

	void			getDimensions	(int* width, int* height) const;
	void			setDimensions	(int width, int height);

protected:
	XlibDisplay&	m_display;
	::Colormap		m_colormap;
	::Window		m_window;

private:
					XlibWindow		(const XlibWindow&);
	XlibWindow&		operator=		(const XlibWindow&);
};

} // x11
} // tcu

#endif // _TCUX11_HPP
