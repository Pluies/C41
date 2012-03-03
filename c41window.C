/*
 * C41 plugin for Cinelerra
 * Copyright (C) 2011 Florent Delannoy <florent at plui dot es>
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

#include "bcbase.h"
#include "filexml.h"
#include "language.h"
#include "mutex.h"

class C41Thread;
class C41Toggle;
class C41Window;

class C41Toggle : public BC_CheckBox
{
public:
	C41Toggle(C41Main *client, int *output, int x, int y);
	~C41Toggle();
	int handle_event();

	C41Main *client;
	int *output;
};

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


C41Window::C41Window(C41Main *client)
 : BC_Window("", MEGREY, client->gui_string, 100, 60, 100, 60, 0, !client->show_initially)
{ this->client = client; }

C41Window::~C41Window()
{
	delete invert;
}

int C41Window::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, _("C41")));
	y += 20;
	add_tool(invert = new C41Toggle(client, &(client->invert), x, y));
}

WINDOW_CLOSE_EVENT(C41Window)

C41Toggle::C41Toggle(C41Main *client, int *output, int x, int y)
 : BC_CheckBox(x, y, 16, 16, *output)
{
	this->client = client;
	this->output = output;
}

C41Toggle::~C41Toggle()
{
}

int C41Toggle::handle_event()
{
	*output = get_value();
	client->send_configure_change();
}

