
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

#include "c41window.h"
#include "language.h"


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
