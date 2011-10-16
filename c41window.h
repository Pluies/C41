
/*
 * CINELERRA
 * Copyright (C) 2008 Adam Williams <broadcast at earthling dot net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

#ifndef C41WINDOW_H
#define C41WINDOW_H

#include "bcbase.h"

class C41Thread;
class C41Window;

#include "filexml.h"
#include "mutex.h"
#include "c41.h"

class C41Thread : public Thread
{
public:
	C41Thread(C41Main *client);
	~C41Thread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	C41Main *client;
	C41Window *window;
};

class C41Toggle;

class C41Window : public BC_Window
{
public:
	C41Window(C41Main *client);
	~C41Window();
	
	int create_objects();
	int close_event();
	
	C41Main *client;
	C41Toggle *invert;
};

class C41Toggle : public BC_CheckBox
{
public:
	C41Toggle(C41Main *client, int *output, int x, int y);
	~C41Toggle();
	int handle_event();

	C41Main *client;
	int *output;
};


#endif
