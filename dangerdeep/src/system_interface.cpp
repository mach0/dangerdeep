/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2016  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// SDL/OpenGL based system services
// (C)+(W) by Thorsten Jordan. See LICENSE
// Parts taken and adapted from Martin Butterwecks's OOML SDL code (wws.sourceforget.net/projects/ooml/)

#include "system_interface.h"
#include <SDL.h>
#include "gpu_interface.h"
#include "log.h"
#include <fstream>
#include <sstream>
#include <chrono>

static auto start_time = std::chrono::high_resolution_clock::now();

system_interface::parameters::parameters()
 :	resolution(1024, 768),
	window_caption("Danger from the Deep"),
	fullscreen(false),
	vertical_sync(true)
{
}



system_interface::system_interface(const parameters& params_) :
	params(params_),
	time_passed_while_sleeping(0),
	sleep_time(0),
	is_sleeping(false),
	maxfps(0),
	last_swap_time(0),
	screenshot_nr(0)
{
	// Initialize SDL first
	int err = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS|SDL_INIT_TIMER);
	if (err < 0)
		THROW(error, "video init failed");

	// request available screen modes (using display #0)
	const int display_nr = 0;
	int num_modes = SDL_GetNumDisplayModes(display_nr);
	if (num_modes > 0) {
		available_resolutions.reserve(num_modes);
		for (int i = 0; i < num_modes; ++i) {
			SDL_DisplayMode mode;
			if (SDL_GetDisplayMode(display_nr, i, &mode) == 0) {
				// we only need w/h
				available_resolutions.push_back(vector2i(mode.w, mode.h));
				log_info("Available resolution " << mode.w << "x" << mode.h << "\n");
			}
		}
	}

	// load default GL library
	SDL_GL_LoadLibrary(nullptr);
	// request GL 4.5 context.
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// Define depth buffer size, but we don't need it.
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#ifdef DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	// if we request a fullscreen mode that is not available, fall back to windowed mode
	if (params.fullscreen) {
		bool valid_mode = false;
		for (auto& r : available_resolutions) {
			if (r == params.resolution) {
				valid_mode = true;
				break;
			}
		}
		params.fullscreen = valid_mode;
	}

	SDL_Window* main_window = SDL_CreateWindow(params.window_caption.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		params.resolution.x, params.resolution.y,
		(params.fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_OPENGL);
	if (!main_window)
		THROW(error, "SDL Window creation failed");
	sdl_main_window = main_window;
	SDL_GLContext gl_context = SDL_GL_CreateContext(main_window);
	if (!gl_context) {
		THROW(error, "SDL GL Context creation failed");
	}
	sdl_gl_context = gl_context;
	// Enable V-Sync
	SDL_GL_SetSwapInterval(params.vertical_sync ? 1 : 0);

	// Call SDL_Quit any way at exit
	atexit(SDL_Quit);

	// Some SDL init.
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_JoystickEventState(SDL_IGNORE);
	SDL_ShowCursor(SDL_ENABLE);

	prepare_new_resolution();
}



system_interface::~system_interface()
{
	// deinit gpu_interface first
	gpu::interface::destroy_instance();
	SDL_GL_DeleteContext((SDL_GLContext)sdl_gl_context);
	SDL_DestroyWindow((SDL_Window*)sdl_main_window);
	SDL_Quit();
}



bool system_interface::set_parameters(const parameters& params_)
{
	// if fullscreen mode is requested, check if it exists
	if (params_.fullscreen) {
		bool valid_mode = false;
		for (auto& r : available_resolutions) {
			if (r == params_.resolution) {
				valid_mode = true;
				break;
			}
		}
		if (!valid_mode) {
			return false;
		}
	}

	// If fullscreen state changes, toggle this
	if (params_.fullscreen != params.fullscreen) {
		SDL_SetWindowFullscreen((SDL_Window*)sdl_main_window, (params_.fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
	}

	// If Vsync changes, toggle it
	if (params_.vertical_sync != params.vertical_sync) {
		SDL_GL_SetSwapInterval(params_.vertical_sync ? 1 : 0);
	}

	// Now change window resolution or screen mode
	if (params_.fullscreen) {
		//SDL_SetWindowDisplayMode((SDL_Window*)sdl_main_window, // fixme needs an exact mode...
	} else {
		// fixme this maybe doesn't work for fullscreen modes...
		SDL_SetWindowSize((SDL_Window*)sdl_main_window, params_.resolution.x, params_.resolution.y);
	}

	// Parameters seem valid, so keep them
	params = params_;

	prepare_new_resolution();

	return true;
}



uint32_t system_interface::millisec()
{
	// SDL_GetTicks seems to have very crude timings, same for new C++ chrono stuff...
	auto current_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
//	std::cout << "duration " << duration << " tpws " << time_passed_while_sleeping << "\n";
	return uint32_t(duration) - time_passed_while_sleeping;
//	return SDL_GetTicks() - time_passed_while_sleeping;
}



void system_interface::swap_buffers()
{
	SDL_GL_SwapWindow((SDL_Window*)sdl_main_window);
	if (maxfps > 0) {
		unsigned tm = millisec();
		unsigned d = tm - last_swap_time;
		unsigned dmax = 1000/maxfps;
		if (d < dmax) {
			SDL_Delay(dmax - d);
			last_swap_time = tm + dmax - d;
		} else {
			last_swap_time = tm;
		}
	}
}



key_code get_key_code(SDL_Keycode sym)
{
	switch (sym) {
	case SDLK_BACKSPACE:	return key_code::BACKSPACE;
	case SDLK_COMMA:	return key_code::COMMA;
	case SDLK_DELETE:	return key_code::DELETE;
	case SDLK_DOWN:		return key_code::DOWN;
	case SDLK_END:		return key_code::END;
	case SDLK_ESCAPE:	return key_code::ESCAPE;
	case SDLK_HOME:		return key_code::HOME;
	case SDLK_LCTRL:	return key_code::LCTRL;
	case SDLK_LEFT:		return key_code::LEFT;
	case SDLK_LESS:		return key_code::LESS;
	case SDLK_LSHIFT:	return key_code::LSHIFT;
	case SDLK_MINUS:	return key_code::MINUS;
	case SDLK_PAGEDOWN:	return key_code::PAGEDOWN;
	case SDLK_PAGEUP:	return key_code::PAGEUP;
	case SDLK_PAUSE:	return key_code::PAUSE;
	case SDLK_PERIOD:	return key_code::PERIOD;
	case SDLK_PLUS:		return key_code::PLUS;
	case SDLK_PRINTSCREEN:	return key_code::PRINTSCREEN;
	case SDLK_RCTRL:	return key_code::RCTRL;
	case SDLK_RETURN:	return key_code::RETURN;
	case SDLK_RIGHT:	return key_code::RIGHT;
	case SDLK_RSHIFT:	return key_code::RSHIFT;
	case SDLK_SPACE:	return key_code::SPACE;
	case SDLK_TAB:		return key_code::TAB;
	case SDLK_UP:		return key_code::UP;
	case SDLK_0:		return key_code::_0;
	case SDLK_1:		return key_code::_1;
	case SDLK_2:		return key_code::_2;
	case SDLK_3:		return key_code::_3;
	case SDLK_4:		return key_code::_4;
	case SDLK_5:		return key_code::_5;
	case SDLK_6:		return key_code::_6;
	case SDLK_7:		return key_code::_7;
	case SDLK_8:		return key_code::_8;
	case SDLK_9:		return key_code::_9;
	case SDLK_a:		return key_code::a;
	case SDLK_b:		return key_code::b;
	case SDLK_c:		return key_code::c;
	case SDLK_d:		return key_code::d;
	case SDLK_e:		return key_code::e;
	case SDLK_f:		return key_code::f;
	case SDLK_g:		return key_code::g;
	case SDLK_h:		return key_code::h;
	case SDLK_i:		return key_code::i;
	case SDLK_j:		return key_code::j;
	case SDLK_k:		return key_code::k;
	case SDLK_l:		return key_code::l;
	case SDLK_m:		return key_code::m;
	case SDLK_n:		return key_code::n;
	case SDLK_o:		return key_code::o;
	case SDLK_p:		return key_code::p;
	case SDLK_q:		return key_code::q;
	case SDLK_r:		return key_code::r;
	case SDLK_s:		return key_code::s;
	case SDLK_t:		return key_code::t;
	case SDLK_u:		return key_code::u;
	case SDLK_v:		return key_code::v;
	case SDLK_w:		return key_code::w;
	case SDLK_x:		return key_code::x;
	case SDLK_y:		return key_code::y;
	case SDLK_z:		return key_code::z;
	case SDLK_F1:		return key_code::F1;
	case SDLK_F2:		return key_code::F2;
	case SDLK_F3:		return key_code::F3;
	case SDLK_F4:		return key_code::F4;
	case SDLK_F5:		return key_code::F5;
	case SDLK_F6:		return key_code::F6;
	case SDLK_F7:		return key_code::F7;
	case SDLK_F8:		return key_code::F8;
	case SDLK_F9:		return key_code::F9;
	case SDLK_F10:		return key_code::F10;
	case SDLK_F11:		return key_code::F11;
	case SDLK_F12:		return key_code::F12;
	case SDLK_KP_1:		return key_code::KP_1;
	case SDLK_KP_2:		return key_code::KP_2;
	case SDLK_KP_3:		return key_code::KP_3;
	case SDLK_KP_4:		return key_code::KP_4;
	case SDLK_KP_5:		return key_code::KP_5;
	case SDLK_KP_6:		return key_code::KP_6;
	case SDLK_KP_7:		return key_code::KP_7;
	case SDLK_KP_8:		return key_code::KP_8;
	case SDLK_KP_9:		return key_code::KP_9;
	case SDLK_KP_MINUS:	return key_code::KP_MINUS;
	case SDLK_KP_PLUS:	return key_code::KP_PLUS;
	default:		return key_code::UNKNOWN;
	}
}



key_mod get_key_mod(Uint16 mod)
{
	int km = int(key_mod::none);
	switch (mod) {
	case KMOD_LSHIFT:	km |= int(key_mod::lshift); break;
	case KMOD_RSHIFT:	km |= int(key_mod::rshift); break;
	case KMOD_LCTRL:	km |= int(key_mod::lctrl); break;
	case KMOD_RCTRL:	km |= int(key_mod::rctrl); break;
	case KMOD_LALT:		km |= int(key_mod::lalt); break;
	case KMOD_RALT:		km |= int(key_mod::ralt); break;
	}
	return key_mod(km);
}



std::string system_interface::get_key_name(key_code key, key_mod mod)
{
	std::string result;
	if (int(mod) & int(key_mod::shift)) result = "Shift + ";
	if (int(mod) & int(key_mod::alt)) result = "Alt + ";
	if (int(mod) & int(key_mod::ctrl)) result = "Ctrl + ";
	switch (key) {
	case key_code::BACKSPACE:	return result + SDL_GetKeyName(SDLK_BACKSPACE);
	case key_code::COMMA:	return result + SDL_GetKeyName(SDLK_COMMA);
	case key_code::DELETE:	return result + SDL_GetKeyName(SDLK_DELETE);
	case key_code::DOWN:	return result + SDL_GetKeyName(SDLK_DOWN);
	case key_code::END:	return result + SDL_GetKeyName(SDLK_END);
	case key_code::ESCAPE:	return result + SDL_GetKeyName(SDLK_ESCAPE);
	case key_code::HOME:	return result + SDL_GetKeyName(SDLK_HOME);
	case key_code::LCTRL:	return result + SDL_GetKeyName(SDLK_LCTRL);
	case key_code::LEFT:	return result + SDL_GetKeyName(SDLK_LEFT);
	case key_code::LESS:	return result + SDL_GetKeyName(SDLK_LESS);
	case key_code::LSHIFT:	return result + SDL_GetKeyName(SDLK_LSHIFT);
	case key_code::MINUS:	return result + SDL_GetKeyName(SDLK_MINUS);
	case key_code::PAGEDOWN:	return result + SDL_GetKeyName(SDLK_PAGEDOWN);
	case key_code::PAGEUP:	return result + SDL_GetKeyName(SDLK_PAGEUP);
	case key_code::PAUSE:	return result + SDL_GetKeyName(SDLK_PAUSE);
	case key_code::PERIOD:	return result + SDL_GetKeyName(SDLK_PERIOD);
	case key_code::PLUS:	return result + SDL_GetKeyName(SDLK_PLUS);
	case key_code::PRINTSCREEN:	return result + SDL_GetKeyName(SDLK_PRINTSCREEN);
	case key_code::RCTRL:	return result + SDL_GetKeyName(SDLK_RCTRL);
	case key_code::RETURN:	return result + SDL_GetKeyName(SDLK_RETURN);
	case key_code::RIGHT:	return result + SDL_GetKeyName(SDLK_RIGHT);
	case key_code::RSHIFT:	return result + SDL_GetKeyName(SDLK_RSHIFT);
	case key_code::SPACE:	return result + SDL_GetKeyName(SDLK_SPACE);
	case key_code::TAB:	return result + SDL_GetKeyName(SDLK_TAB);
	case key_code::UP:	return result + SDL_GetKeyName(SDLK_UP);
	case key_code::_0:	return result + SDL_GetKeyName(SDLK_0);
	case key_code::_1:	return result + SDL_GetKeyName(SDLK_1);
	case key_code::_2:	return result + SDL_GetKeyName(SDLK_2);
	case key_code::_3:	return result + SDL_GetKeyName(SDLK_3);
	case key_code::_4:	return result + SDL_GetKeyName(SDLK_4);
	case key_code::_5:	return result + SDL_GetKeyName(SDLK_5);
	case key_code::_6:	return result + SDL_GetKeyName(SDLK_6);
	case key_code::_7:	return result + SDL_GetKeyName(SDLK_7);
	case key_code::_8:	return result + SDL_GetKeyName(SDLK_8);
	case key_code::_9:	return result + SDL_GetKeyName(SDLK_9);
	case key_code::a:	return result + SDL_GetKeyName(SDLK_a);
	case key_code::b:	return result + SDL_GetKeyName(SDLK_b);
	case key_code::c:	return result + SDL_GetKeyName(SDLK_c);
	case key_code::d:	return result + SDL_GetKeyName(SDLK_d);
	case key_code::e:	return result + SDL_GetKeyName(SDLK_e);
	case key_code::f:	return result + SDL_GetKeyName(SDLK_f);
	case key_code::g:	return result + SDL_GetKeyName(SDLK_g);
	case key_code::h:	return result + SDL_GetKeyName(SDLK_h);
	case key_code::i:	return result + SDL_GetKeyName(SDLK_i);
	case key_code::j:	return result + SDL_GetKeyName(SDLK_j);
	case key_code::k:	return result + SDL_GetKeyName(SDLK_k);
	case key_code::l:	return result + SDL_GetKeyName(SDLK_l);
	case key_code::m:	return result + SDL_GetKeyName(SDLK_m);
	case key_code::n:	return result + SDL_GetKeyName(SDLK_n);
	case key_code::o:	return result + SDL_GetKeyName(SDLK_o);
	case key_code::p:	return result + SDL_GetKeyName(SDLK_p);
	case key_code::q:	return result + SDL_GetKeyName(SDLK_q);
	case key_code::r:	return result + SDL_GetKeyName(SDLK_r);
	case key_code::s:	return result + SDL_GetKeyName(SDLK_s);
	case key_code::t:	return result + SDL_GetKeyName(SDLK_t);
	case key_code::u:	return result + SDL_GetKeyName(SDLK_u);
	case key_code::v:	return result + SDL_GetKeyName(SDLK_v);
	case key_code::w:	return result + SDL_GetKeyName(SDLK_w);
	case key_code::x:	return result + SDL_GetKeyName(SDLK_x);
	case key_code::y:	return result + SDL_GetKeyName(SDLK_y);
	case key_code::z:	return result + SDL_GetKeyName(SDLK_z);
	case key_code::F1:	return result + SDL_GetKeyName(SDLK_F1);
	case key_code::F2:	return result + SDL_GetKeyName(SDLK_F2);
	case key_code::F3:	return result + SDL_GetKeyName(SDLK_F3);
	case key_code::F4:	return result + SDL_GetKeyName(SDLK_F4);
	case key_code::F5:	return result + SDL_GetKeyName(SDLK_F5);
	case key_code::F6:	return result + SDL_GetKeyName(SDLK_F6);
	case key_code::F7:	return result + SDL_GetKeyName(SDLK_F7);
	case key_code::F8:	return result + SDL_GetKeyName(SDLK_F8);
	case key_code::F9:	return result + SDL_GetKeyName(SDLK_F9);
	case key_code::F10:	return result + SDL_GetKeyName(SDLK_F10);
	case key_code::F11:	return result + SDL_GetKeyName(SDLK_F11);
	case key_code::F12:	return result + SDL_GetKeyName(SDLK_F12);
	case key_code::KP_1:	return result + SDL_GetKeyName(SDLK_KP_1);
	case key_code::KP_2:	return result + SDL_GetKeyName(SDLK_KP_2);
	case key_code::KP_3:	return result + SDL_GetKeyName(SDLK_KP_3);
	case key_code::KP_4:	return result + SDL_GetKeyName(SDLK_KP_4);
	case key_code::KP_5:	return result + SDL_GetKeyName(SDLK_KP_5);
	case key_code::KP_6:	return result + SDL_GetKeyName(SDLK_KP_6);
	case key_code::KP_7:	return result + SDL_GetKeyName(SDLK_KP_7);
	case key_code::KP_8:	return result + SDL_GetKeyName(SDLK_KP_8);
	case key_code::KP_9:	return result + SDL_GetKeyName(SDLK_KP_9);
	case key_code::KP_MINUS:	return result + SDL_GetKeyName(SDLK_KP_MINUS);
	case key_code::KP_PLUS:	return result + SDL_GetKeyName(SDLK_KP_PLUS);
	default:			return result + "UNKNOWN";
	}
}



bool system_interface::finish_frame()
{
	// Switch window frame buffers
	swap_buffers();

	// translate 2D motion/position to screen size -1...1, y axis up
	auto translate_p = [&](int x, int y) -> vector2f
	{
		return vector2f(2.f * x / params.resolution.x - 1.f,
				1.f - 2.f * y / params.resolution.y);
	};
	// translate 2D motion to screen size -1...1, y axis up
	auto translate_m = [&](int x, int y) -> vector2f
	{
		return vector2f(2.f * x / params.resolution.x,
				-2.f * y / params.resolution.y);
	};
	// translate 2d motion to pseudo 2d coordinates 1024x768, y axis down
	// Note that scaling depends on 2d screen size
	auto translate_m_2d = [&](int x, int y) -> vector2i
	{
		return vector2i(1024 * x / size_2D.x, 768 * y / size_2D.y);
	};
	// translate 2D position to pseudo 2d coordinates 1024x768, y axis down
	// coordinates inside area defined by offset_2D/size_2D should be mapped
	// to 1024x768. Note that this can give coordinates outside the 0,0...1023,767 area.
	auto translate_p_2d = [&](int x, int y) -> vector2i
	{
		// as first remove offset
		x -= offset_2D.x;
		y -= offset_2D.y;
		// then translate to pseudo 2d
		x = int(1024 * x / size_2D.x);
		y = int( 768 * y / size_2D.y);
		return vector2i(x, y);
	};

	// Handle all events
	SDL_Event event;
	do {
		unsigned nr_of_events = 0;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:			// Quit event
				log_info("---------- immediate exit ----------");
				log::instance().write(std::cerr, log::LOG_SYSINFO);
				{
					std::ofstream f("log.txt");
					log::instance().write(f, log::LOG_SYSINFO);
				}
				return true;

			case SDL_WINDOWEVENT:		// Application activation or focus event
				if (event.window.event == SDL_WINDOWEVENT_ENTER) {
					if (is_sleeping) {
						is_sleeping = false;
						time_passed_while_sleeping += SDL_GetTicks() - sleep_time;
					}
				} else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
					if (!is_sleeping) {
						is_sleeping = true;
						sleep_time = SDL_GetTicks();
					}
				}
				continue; // filter these events

			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					input_event_handler::key_data kd;
					kd.keycode = get_key_code(event.key.keysym.sym);
					kd.mod = get_key_mod(event.key.keysym.mod);
					if (kd.keycode != key_code::UNKNOWN) {
						kd.action = (event.type == SDL_KEYUP) ? input_action::up : input_action::down;
						for (unsigned i = unsigned(input_event_handlers.size()); i > 0; --i) {
							std::shared_ptr<input_event_handler> handler(input_event_handlers[i-1]);
							if (handler && handler->handle_key_event(kd)) {
								break;
							}
						}
					}
				}
				break;
			case SDL_TEXTINPUT:
				{
					const char* text = event.text.text;
					if (*text != 0x00) {	// not empty
						for (unsigned i = unsigned(input_event_handlers.size()); i > 0; --i) {
							std::shared_ptr<input_event_handler> handler(input_event_handlers[i-1]);
							if (handler && handler->handle_text_input_event(text)) {
								break;
							}
						}
					}
				}
				break;
			case SDL_MOUSEMOTION:
				{
					input_event_handler::mouse_motion_data md;
					// Translate motion/position to 2D coordinates
					md.relative_motion = translate_m(event.motion.xrel, event.motion.yrel);
					md.rel_motion_2d = translate_m_2d(event.motion.xrel, event.motion.yrel);
					md.position = translate_p(event.motion.x, event.motion.y);
					md.position_2d = translate_p_2d(event.motion.x, event.motion.y);
					md.buttons_pressed.fill(false);
					if (event.motion.state & SDL_BUTTON_LMASK) md.buttons_pressed[unsigned(mouse_button::left)] = true;
					if (event.motion.state & SDL_BUTTON_MMASK) md.buttons_pressed[unsigned(mouse_button::middle)] = true;
					if (event.motion.state & SDL_BUTTON_RMASK) md.buttons_pressed[unsigned(mouse_button::right)] = true;
					for (unsigned i = unsigned(input_event_handlers.size()); i > 0; --i) {
						std::shared_ptr<input_event_handler> handler(input_event_handlers[i-1]);
						if (handler && handler->handle_mouse_motion_event(md)) {
							break;
						}
					}
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				{
					input_event_handler::mouse_click_data md;
					md.position = translate_p(event.button.x, event.button.y);
					md.position_2d = translate_p_2d(event.button.x, event.button.y);
					// we get all mouse events here so we can track the button state and
					// offer that info also in button down/up events...
					// we would miss events outside the window then... but who cares,
					// however this is not needed so far
					if (event.button.button == SDL_BUTTON_LEFT) md.button = mouse_button::left;
					if (event.button.button == SDL_BUTTON_MIDDLE) md.button = mouse_button::middle;
					if (event.button.button == SDL_BUTTON_RIGHT) md.button = mouse_button::right;
					md.action = (event.type == SDL_MOUSEBUTTONUP) ? input_action::up : input_action::down;
					for (unsigned i = unsigned(input_event_handlers.size()); i > 0; --i) {
						std::shared_ptr<input_event_handler> handler(input_event_handlers[i-1]);
						if (handler && handler->handle_mouse_button_event(md)) {
							break;
						}
					}
				}
				break;
			case SDL_MOUSEWHEEL:
				{
					input_event_handler::mouse_wheel_data md;
					md.relative_motion = translate_m(event.wheel.x, event.wheel.y);
					md.rel_motion_2d = translate_m_2d(event.wheel.x, event.wheel.y);
					if (md.relative_motion.y < 0.f)	// fixme check that orientation is correct (up/down)
						md.action = input_action::up;
					else if (md.relative_motion.y > 0.f)	// fixme check that orientation is correct (up/down)
						md.action = input_action::down;
					for (unsigned i = unsigned(input_event_handlers.size()); i > 0; --i) {
						std::shared_ptr<input_event_handler> handler(input_event_handlers[i-1]);
						if (handler && handler->handle_mouse_wheel_event(md)) {
							break;
						}
					}
				}
				break;
			default: // we don't handle unknown events
					continue;
			}

			++nr_of_events;
		}
		// do not waste CPU time when sleeping
		if (nr_of_events == 0 && is_sleeping)
			SDL_Delay(25);
	} while (is_sleeping);
	return false;
}



void system_interface::screenshot(const std::string& filename)
{
	// We need to use SDL to get window buffer data, OpenGL ReadPixels is obsolete with GL3+
	SDL_Surface* main_window_surface = SDL_GetWindowSurface((SDL_Window*)sdl_main_window);
	if (main_window_surface != nullptr) {
		std::string fn;
		if (filename.empty()) {
			std::ostringstream os;
			os << screenshot_dir << "screenshot" << screenshot_nr++ << ".bmp";
			fn = os.str();
		} else {
			fn = filename + ".bmp";
		}
		SDL_SaveBMP(main_window_surface, fn.c_str());
		log_info("screenshot taken as " << fn);
	}
}



void system_interface::add_event_handler(std::weak_ptr<input_event_handler> new_handler)
{
	input_event_handlers.push_back(new_handler);
}



void system_interface::remove_event_handler()
{
	if (input_event_handlers.empty()) {
		THROW(error, "remove_event_handler with already empty stack");
	}
	input_event_handlers.pop_back();
}



void system_interface::prepare_new_resolution()
{
	// compute 2D data and area and resolution. it must be 4:3 always.
	if (params.resolution.x * 3 >= params.resolution.y * 4) {
		// screen is wider than high
		size_2D.x = params.resolution.y * 4 / 3;
		size_2D.y = params.resolution.y;
		offset_2D.x = (params.resolution.x - size_2D.x) / 2;
		offset_2D.y = 0;
		offset_pseudo_2D.x = 2.f * offset_2D.x / params.resolution.x - 1.f;
		offset_pseudo_2D.y = 1.f;
		scale_pseudo_2D.x = (2.f * size_2D.x) / (params.resolution.x * 1024);
		scale_pseudo_2D.y = -2.f / 768;
	} else {
		// screen is higher than wide
		size_2D.x = params.resolution.x;
		size_2D.y = params.resolution.x * 3 / 4;
		offset_2D.x = 0;
		offset_2D.y = (params.resolution.y - size_2D.y) / 2;
		offset_pseudo_2D.x = -1.f;
		offset_pseudo_2D.y = 1.f - 2.f * offset_2D.y / params.resolution.y;
		scale_pseudo_2D.x = 2.f / 1024;
		scale_pseudo_2D.y = -(2.f * size_2D.y) / (params.resolution.y * 768);
	}

	// reinit frame buffer
	GPU().init_frame_buffer(params.resolution.x, params.resolution.y);
}
