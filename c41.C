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

#include "bcdisplayinfo.h"
#include "clip.h"
#include "bchash.h"
#include "filexml.h"
#include "guicast.h"
#include "language.h"
#include "picon_png.h"
#include "plugincolors.h"
#include "pluginvclient.h"
#include "vframe.h"

#include <stdint.h>
#include <string.h>


/* Class declarations */
class C41Effect;

class C41Config
{
	public:
		C41Config();

		void copy_from(C41Config &src);
		int equivalent(C41Config &src);
		void interpolate(C41Config &prev, 
				C41Config &next, 
				long prev_frame, 
				long next_frame, 
				long current_frame);

		int active;
		float min_r, min_g, min_b, magic4, magic5, magic6;
		float fix_min_r, fix_min_g, fix_min_b, fix_magic4, fix_magic5, fix_magic6;
};

class C41Enable : public BC_CheckBox
{
	public:
		C41Enable(C41Effect *plugin, int *output, int x, int y, char *text);
		int handle_event();
		C41Effect *plugin;
		int *output;
};

class C41TextBox : public BC_TextBox
{ 
	public:
		C41TextBox(C41Effect *plugin, float *value, int x, int y);
		int handle_event();
		C41Effect *plugin;
		float *boxValue;
};

class C41Button : public BC_GenericButton
{ 
	public:
		C41Button(C41Effect *plugin, int x, int y);
		int handle_event();
		C41Effect *plugin;
		float *boxValue;
};

class C41Window : public BC_Window
{
	public:
		C41Window(C41Effect *plugin, int x, int y);
		void create_objects();
		int close_event();
		C41Enable *active;
		C41TextBox *min_r, *min_g, *min_b, *magic4, *magic5, *magic6;
		C41TextBox *fix_min_r, *fix_min_g, *fix_min_b, *fix_magic4, *fix_magic5, *fix_magic6;
		C41Button *lock;
		C41Effect *plugin;
};

PLUGIN_THREAD_HEADER(C41Effect, C41Thread, C41Window)

class C41Effect : public PluginVClient
{
	public:
		C41Effect(PluginServer *server);
		~C41Effect();
		int process_buffer(VFrame *frame,
				int64_t start_position,
				double frame_rate);
		int is_realtime();
		char* plugin_title();
		VFrame* new_picon();
		int load_defaults();
		int save_defaults();
		void save_data(KeyFrame *keyframe);
		void read_data(KeyFrame *keyframe);
		void update_gui();
		void render_gui(void* data);
		int show_gui();
		void raise_window();
		int set_string();
		int load_configuration();
		int handle_opengl();
		void lock_parameters();

		C41Config config;
		C41Thread *thread;
		BC_Hash *defaults;
};

REGISTER_PLUGIN(C41Effect)


/* Methods decarations */

// C41Config
C41Config::C41Config()
{
	active = 0;
	
	min_r = min_g = min_b = magic4 = magic5 = magic6 = 0.;
	fix_min_r = fix_min_g = fix_min_b = fix_magic4 = fix_magic5 = fix_magic6 = 0.;
}
void C41Config::copy_from(C41Config &src)
{
	active = src.active;

	min_r = src.min_r;
	min_g = src.min_g;
	min_b = src.min_b;
	magic4 = src.magic4;
	magic5 = src.magic5;
	magic6 = src.magic6;

	fix_min_r = src.fix_min_r;
	fix_min_g = src.fix_min_g;
	fix_min_b = src.fix_min_b;
	fix_magic4 = src.fix_magic4;
	fix_magic5 = src.fix_magic5;
	fix_magic6 = src.fix_magic6;
}
int C41Config::equivalent(C41Config &src)
{
	return false;
}
void C41Config::interpolate(C41Config &prev, 
	C41Config &next, 
	long prev_frame, 
	long next_frame, 
	long current_frame)
{
	active = prev.active;
	
	min_r = prev.min_r;
	min_g = prev.min_g;
	min_b = prev.min_b;
	magic4 = prev.magic4;
	magic5 = prev.magic5;
	magic6 = prev.magic6;

	fix_min_r = prev.fix_min_r;
	fix_min_g = prev.fix_min_g;
	fix_min_b = prev.fix_min_b;
	fix_magic4 = prev.fix_magic4;
	fix_magic5 = prev.fix_magic5;
	fix_magic6 = prev.fix_magic6;

}

// C41Enable
C41Enable::C41Enable(C41Effect *plugin, int *output, int x, int y, char *text)
 : BC_CheckBox(x, y, *output, text)
{
	this->plugin = plugin;
	this->output = output;
}
int C41Enable::handle_event()
{
	*output = get_value();
	plugin->send_configure_change();
	return 1;
}

// C41TextBox
C41TextBox::C41TextBox(C41Effect *plugin, float *value, int x, int y)
 : BC_TextBox(x, y, 160, 1, *value)
{
	this->plugin = plugin;
	this->boxValue = value;
}
int C41TextBox::handle_event()
{
	*boxValue = atof(get_text());
	plugin->send_configure_change();
	return 1;
}


// C41Button
C41Button::C41Button(C41Effect *plugin, int x, int y)
 : BC_GenericButton(x, y, "Lock parameters")
{
	this->plugin = plugin;
}
int C41Button::handle_event()
{
	plugin->lock_parameters();
	plugin->send_configure_change();
	return 1;
}

// C41Window
C41Window::C41Window(C41Effect *plugin, int x, int y)
 : BC_Window(plugin->gui_string, x, y, 250, 560, 250, 560, 1, 0, 1)
{
	this->plugin = plugin;
}
void C41Window::create_objects()
{
	int x = 10, y = 10;

	add_subwindow(active = new C41Enable(plugin, &plugin->config.active, x, y, _("Activate plugin")));
	y += 40;

	add_subwindow(new BC_Title(x, y, _("Computed negfix values:")));
	y += 30;

	add_subwindow(new BC_Title(x, y, _("Min R:")));
	x += 60; add_subwindow(min_r = new C41TextBox(plugin, &plugin->config.min_r, x, y)); x -= 60; y += 30;

	add_subwindow(new BC_Title(x, y, _("Min G:")));
	x += 60; add_subwindow(min_g = new C41TextBox(plugin, &plugin->config.min_g, x, y)); x -= 60; y += 30;

	add_subwindow(new BC_Title(x, y, _("Min B:")));
	x += 60; add_subwindow(min_b = new C41TextBox(plugin, &plugin->config.min_b, x, y)); x -= 60; y += 30;
	
	add_subwindow(new BC_Title(x, y, _("Magic4:")));
	x += 60; add_subwindow(magic4 = new C41TextBox(plugin, &plugin->config.magic4, x, y)); x -= 60; y += 30;
	
	add_subwindow(new BC_Title(x, y, _("Magic5:")));
	x += 60; add_subwindow(magic5 = new C41TextBox(plugin, &plugin->config.magic5, x, y)); x -= 60; y += 30;
	
	add_subwindow(new BC_Title(x, y, _("Magic6:")));
	x += 60; add_subwindow(magic6 = new C41TextBox(plugin, &plugin->config.magic6, x, y)); x -= 60; y += 30;

	// The user shouldn't be able to change the computed values
	min_r->disable();
	min_g->disable();
	min_b->disable();
	magic5->disable();
	magic6->disable();


	y += 30;
	add_subwindow(lock = new C41Button(plugin, x, y));
	y += 30;
	
	y += 20;
	add_subwindow(new BC_Title(x, y, _("negfix values to apply:")));
	y += 30;

	add_subwindow(new BC_Title(x, y, _("Min R:")));
	x += 60; add_subwindow(fix_min_r = new C41TextBox(plugin, &plugin->config.fix_min_r, x, y)); x -= 60; y += 30;

	add_subwindow(new BC_Title(x, y, _("Min G:")));
	x += 60; add_subwindow(fix_min_g = new C41TextBox(plugin, &plugin->config.fix_min_g, x, y)); x -= 60; y += 30;

	add_subwindow(new BC_Title(x, y, _("Min B:")));
	x += 60; add_subwindow(fix_min_b = new C41TextBox(plugin, &plugin->config.fix_min_b, x, y)); x -= 60; y += 30;
		
	add_subwindow(new BC_Title(x, y, _("Magic4:")));
	x += 60; add_subwindow(fix_magic4 = new C41TextBox(plugin, &plugin->config.fix_magic4, x, y)); x -= 60; y += 30;

	add_subwindow(new BC_Title(x, y, _("Magic5:")));
	x += 60; add_subwindow(fix_magic5 = new C41TextBox(plugin, &plugin->config.fix_magic5, x, y)); x -= 60; y += 30;
	
	add_subwindow(new BC_Title(x, y, _("Magic6:")));
	x += 60; add_subwindow(fix_magic6 = new C41TextBox(plugin, &plugin->config.fix_magic6, x, y)); x -= 60; y += 30;


	show_window();
	flush();
}

WINDOW_CLOSE_EVENT(C41Window)
PLUGIN_THREAD_OBJECT(C41Effect, C41Thread, C41Window)

// C41Effect
C41Effect::C41Effect(PluginServer *server)
 : PluginVClient(server)
{
	PLUGIN_CONSTRUCTOR_MACRO
}
C41Effect::~C41Effect()
{
	PLUGIN_DESTRUCTOR_MACRO
}
char* C41Effect::plugin_title() { return N_("C41"); }
int C41Effect::is_realtime() { return 1; }

NEW_PICON_MACRO(C41Effect)
SHOW_GUI_MACRO(C41Effect, C41Thread)
RAISE_WINDOW_MACRO(C41Effect)
SET_STRING_MACRO(C41Effect)
LOAD_CONFIGURATION_MACRO(C41Effect, C41Config)

void C41Effect::lock_parameters()
{
	config.fix_min_r = config.min_r;
	config.fix_min_g = config.min_g;
	config.fix_min_b = config.min_b;
	config.fix_magic4 = config.magic4;
	config.fix_magic5 = config.magic5;
	config.fix_magic6 = config.magic6;
}


void C41Effect::update_gui()
{
// We don't use update_gui, but rather render_gui.
// However, the method is still needed to instantiate the plugin
	
}


void C41Effect::render_gui(void* data)
{
	if(thread)
	{
		if(load_configuration()){
			thread->window->lock_window();

			// Updating values computed by process_buffer
			float* nf_vals = (float*) data;
			config.min_r = nf_vals[0];
			config.min_g = nf_vals[1];
			config.min_b = nf_vals[2];
			config.magic4 = nf_vals[3];
			config.magic5 = nf_vals[4];
			config.magic6 = nf_vals[5];

			// Updating the GUI itself
			thread->window->active->update(config.active);

			thread->window->min_r->update(config.min_r);
			thread->window->min_g->update(config.min_g);
			thread->window->min_b->update(config.min_b);
			thread->window->magic4->update(config.magic4);
			thread->window->magic5->update(config.magic5);
			thread->window->magic6->update(config.magic6);
			
			thread->window->fix_min_r->update(config.fix_min_r);
			thread->window->fix_min_g->update(config.fix_min_g);
			thread->window->fix_min_b->update(config.fix_min_b);
			thread->window->fix_magic4->update(config.fix_magic4);
			thread->window->fix_magic5->update(config.fix_magic5);
			thread->window->fix_magic6->update(config.fix_magic6);
	
			// DEBUG
			// printf("UPDATING GUI with RENDER and values %f %f %f\n", config.min_r, config.min_g, config.min_b);
			free(data);

			thread->window->unlock_window();
		}
	}

}

int C41Effect::load_defaults()
{
	char directory[BCTEXTLEN];
	sprintf(directory, "%sC41.rc", BCASTDIR);
	defaults = new BC_Hash(directory);
	defaults->load();
	config.active = defaults->get("ACTIVE", config.active);

	config.min_r = defaults->get("MIN_R", config.min_r);
	config.min_g = defaults->get("MIN_G", config.min_g);
	config.min_b = defaults->get("MIN_B", config.min_b);
	config.magic4 = defaults->get("MAGIC4", config.magic4);
	config.magic5 = defaults->get("MAGIC5", config.magic5);
	config.magic6 = defaults->get("MAGIC6", config.magic6);

	config.fix_min_r = defaults->get("FIX_MIN_R", config.fix_min_r);
	config.fix_min_g = defaults->get("FIX_MIN_G", config.fix_min_g);
	config.fix_min_b = defaults->get("FIX_MIN_B", config.fix_min_b);
	config.fix_magic4 = defaults->get("FIX_MAGIC4", config.fix_magic4);
	config.fix_magic5 = defaults->get("FIX_MAGIC5", config.fix_magic5);
	config.fix_magic6 = defaults->get("FIX_MAGIC6", config.fix_magic6);

	return 0;
}
int C41Effect::save_defaults()
{
	defaults->update("ACTIVE", config.active);
	defaults->update("MIN_R", config.min_r);
	defaults->update("MIN_G", config.min_g);
	defaults->update("MIN_B", config.min_b);
	defaults->save();
	return 0;
}
void C41Effect::save_data(KeyFrame *keyframe)
{
	FileXML output;
	output.set_shared_string(keyframe->data, MESSAGESIZE);
	output.tag.set_title("C41");
	output.tag.set_property("ACTIVE", config.active);

	output.tag.set_property("MIN_R", config.min_r);
	output.tag.set_property("MIN_G", config.min_g);
	output.tag.set_property("MIN_B", config.min_b);
	output.tag.set_property("MAGIC4", config.magic4);
	output.tag.set_property("MAGIC5", config.magic5);
	output.tag.set_property("MAGIC6", config.magic6);

	output.tag.set_property("FIX_MIN_R", config.fix_min_r);
	output.tag.set_property("FIX_MIN_G", config.fix_min_g);
	output.tag.set_property("FIX_MIN_B", config.fix_min_b);
	output.tag.set_property("FIX_MAGIC4", config.fix_magic4);
	output.tag.set_property("FIX_MAGIC5", config.fix_magic5);
	output.tag.set_property("FIX_MAGIC6", config.fix_magic6);
	
	output.append_tag();
	output.tag.set_title("/C41");
	output.append_tag();
	output.terminate_string();
}
void C41Effect::read_data(KeyFrame *keyframe)
{
	FileXML input;
	input.set_shared_string(keyframe->data, strlen(keyframe->data));
	while(!input.read_tag())
	{
		if(input.tag.title_is("C41"))
		{
			config.active = input.tag.get_property("ACTIVE", config.active);

			config.min_r = input.tag.get_property("MIN_R", config.min_r);
			config.min_g = input.tag.get_property("MIN_G", config.min_g);
			config.min_b = input.tag.get_property("MIN_B", config.min_b);
			config.magic4 = input.tag.get_property("MAGIC4", config.magic5);
			config.magic5 = input.tag.get_property("MAGIC5", config.magic5);
			config.magic6 = input.tag.get_property("MAGIC6", config.magic6);

			config.fix_min_r = input.tag.get_property("FIX_MIN_R", config.fix_min_r);
			config.fix_min_g = input.tag.get_property("FIX_MIN_G", config.fix_min_g);
			config.fix_min_b = input.tag.get_property("FIX_MIN_B", config.fix_min_b);
			config.fix_magic4 = input.tag.get_property("FIX_MAGIC4", config.fix_magic5);
			config.fix_magic5 = input.tag.get_property("FIX_MAGIC5", config.fix_magic5);
			config.fix_magic6 = input.tag.get_property("FIX_MAGIC6", config.fix_magic6);
		}
	}
}

int C41Effect::process_buffer(VFrame *frame,
		int64_t start_position,
		double frame_rate)
{
	load_configuration();

	read_frame(frame, 
			0, 
			start_position, 
			frame_rate,
			get_use_opengl());


	switch(frame->get_color_model())
	{
		case BC_RGB888:
		case BC_YUV888:
		case BC_RGBA_FLOAT:
		case BC_RGBA8888:
		case BC_YUVA8888:
		case BC_RGB161616:
		case BC_YUV161616:
		case BC_RGBA16161616:
		case BC_YUVA16161616:
			return 0; // Unsupported
			break;
		case BC_RGB_FLOAT:
			break;
	}

	// Compute magic negfix values
	float minima_r = 50., minima_g = 50., minima_b = 50.;
	float maxima_r = 0., maxima_g = 0., maxima_b = 0.;
	int frame_w = frame->get_w();
	int frame_h = frame->get_h();

	// Shave the image in order to avoid black borders
	// Tolerance default: 5%, i.e. 0.05
	#define TOLERANCE 0.05
	#define SKIP_ROW if (i<(TOLERANCE * frame_h) || i>((1-TOLERANCE)*frame_h)) continue
	#define SKIP_COL if (j<(TOLERANCE * frame_w) || j>((1-TOLERANCE)*frame_w)) continue

	for(int i = 0; i < frame_h; i++) 
	{
		SKIP_ROW;
		float *row = (float*)frame->get_rows()[i];
		for(int j = 0; j < frame_w; j++, row += 3) {
			SKIP_COL;

			if(row[0]<minima_r) minima_r = row[0];
			if(row[0]>maxima_r) maxima_r = row[0];

			if(row[1]<minima_g) minima_g = row[1];
			if(row[1]>maxima_g) maxima_g = row[1];

			if(row[2]<minima_b) minima_b = row[2];
			if(row[2]>maxima_b) maxima_b = row[2];
		}
	}

	// DEBUG
	//printf("Minima: %f %f %f\n", minima_r, minima_g, minima_b);
	//printf("Maxima: %f %f %f\n", maxima_r, maxima_g, maxima_b);
	float magic1 = minima_r;
	float magic2 = minima_g;
	float magic3 = minima_b;
	float magic4 = (minima_r/maxima_r)*0.95;
	float magic5 = log(maxima_g/minima_g) / log(maxima_r/minima_r);
	float magic6 = log(maxima_b/minima_b) / log(maxima_r/minima_r);
	// DEBUG
	// printf("Magic values: %f %f %f\n", magic4, magic5, magic6);

	// Update GUI
	float *nf_vals = (float*) malloc(6*sizeof(float));
	nf_vals[0] = magic1; nf_vals[1] = magic2; nf_vals[2] = magic3;
	nf_vals[3] = magic4; nf_vals[4] = magic5; nf_vals[5] = magic6;
	send_render_gui(nf_vals);
	
	// Apply the transformation
	if(config.active){

		// Get the values from the config instead of the computed ones
		magic1 = config.fix_min_r;
		magic2 = config.fix_min_g;
		magic3 = config.fix_min_b;
		magic4 = config.fix_magic4;
		magic5 = config.fix_magic5;
		magic6 = config.fix_magic6;

		for(int i = 0; i < frame_h; i++){
			// DEBUG: will show the parts of the image skipped to compute the minima
			// SKIP_ROW;
			float *row = (float*)frame->get_rows()[i];
			for(int j = 0; j < frame_w; j++, row += 3) {
				// DEBUG
				// SKIP_COL;
				row[0] =  (magic1 / row[0]) - magic4;

				//row[1] = (magic2 / row[1]);
				row[1] = pow( (magic2 / row[1]) ,1/magic5) - magic4;

				//row[2] = (magic3 / row[2]);
				row[2] = pow((magic3 / row[2]),1/magic6) - magic4;

			}
		}
	}
	/* END NEGFIX */

	return 0;
}
int C41Effect::handle_opengl()
{
#ifdef HAVE_GL
	/* TODO */
	/* NOT IMPLEMENTED */
	/* TODO */
#endif
}


