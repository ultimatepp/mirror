#include "SDL2GL.h"

#define LLOG(x)

namespace Upp {

extern dword fbKEYtoK(dword chr);

dword lastbdowntime[8] = {0};
dword isdblclick[8] = {0};

dword mouseb;
dword modkeys;
bool  sdlMouseIsIn;

bool SDL2GUI::IsMouseIn()
{
	return sdlMouseIsIn;
}

dword SDL2GUI::GetMouseButtons()
{
	return mouseb;
}

dword SDL2GUI::GetModKeys()
{
	return modkeys;
}

void SDL2GUI::HandleSDLEvent(SDL_Event* event)
{
	LLOG("HandleSDLEvent " << event->type);
	SDL_Event next_event;
	dword keycode;
	switch(event->type) {
//		case SDL_ACTIVEEVENT: //SDL_ActiveEvent
//			break;
	case SDL_TEXTINPUT: {
			//send respective keyup things as char events as well
		WString text = FromUtf8(event->text.text);
		for(int i = 0; i < text.GetCount(); i++) {
			int c = text[i];
			if(c != 127)
				Ctrl::DoKeyFB(c, 1);
		}
		break;
	}
	case SDL_KEYDOWN:
		switch(event->key.keysym.sym) {
			case SDLK_LSHIFT: modkeys |= KM_LSHIFT; break;
			case SDLK_RSHIFT: modkeys |= KM_RSHIFT; break;
			case SDLK_LCTRL: modkeys |= KM_LCTRL; break;
			case SDLK_RCTRL: modkeys |= KM_RCTRL; break;
			case SDLK_LALT: modkeys |= KM_LALT; break;
			case SDLK_RALT: modkeys |= KM_RALT; break;
		}
		
		keycode = fbKEYtoK((dword)event->key.keysym.sym);
		
		if(keycode != K_SPACE) { //dont send space on keydown
			static int repeat_count;
			SDL_PumpEvents();
			if(SDL_PeepEvents(&next_event, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN) &&
			   next_event.key.keysym.sym == event->key.keysym.sym) {
				repeat_count++; // Keyboard repeat compression
				break;
			}
			Ctrl::DoKeyFB(keycode, 1 + repeat_count);
			repeat_count = 0;
		}
		break;
	case SDL_KEYUP: //SDL_KeyboardEvent
		switch(event->key.keysym.sym) {
			case SDLK_LSHIFT: modkeys &= ~KM_LSHIFT; break;
			case SDLK_RSHIFT: modkeys &= ~KM_RSHIFT; break;
			case SDLK_LCTRL: modkeys &= ~KM_LCTRL; break;
			case SDLK_RCTRL: modkeys &= ~KM_RCTRL; break;
			case SDLK_LALT: modkeys &= ~KM_LALT; break;
			case SDLK_RALT: modkeys &= ~KM_RALT; break;
		}

		Ctrl::DoKeyFB(fbKEYtoK((dword)event->key.keysym.sym) | K_KEYUP, 1);
		break;
	case SDL_MOUSEMOTION:
		SDL_PumpEvents();
		if(SDL_PeepEvents(&next_event, 1, SDL_PEEKEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0)
			break; // MouseMove compression
		Ctrl::DoMouseFB(Ctrl::MOUSEMOVE, Point(event->motion.x, event->motion.y));
		break;
	case SDL_MOUSEWHEEL:
		Ctrl::DoMouseFB(Ctrl::MOUSEWHEEL, GetMousePos(), sgn(event->wheel.y) * 120);
		break;
	case SDL_MOUSEBUTTONDOWN: {
			Point p(event->button.x, event->button.y);
			int bi = event->button.button;
			dword ct = SDL_GetTicks();
			if(isdblclick[bi] && (abs(int(ct) - int(lastbdowntime[bi])) < 400))
			{
				switch(bi)
				{
					case SDL_BUTTON_LEFT: Ctrl::DoMouseFB(Ctrl::LEFTDOUBLE, p); break;
					case SDL_BUTTON_RIGHT: Ctrl::DoMouseFB(Ctrl::RIGHTDOUBLE, p); break;
					case SDL_BUTTON_MIDDLE: Ctrl::DoMouseFB(Ctrl::MIDDLEDOUBLE, p); break;
				}
				isdblclick[bi] = 0; //reset, to go ahead sending repeats
			}
			else
			{
				lastbdowntime[bi] = ct;
				isdblclick[bi] = 0; //prepare for repeat
				switch(bi)
				{
					case SDL_BUTTON_LEFT: mouseb |= (1<<0); Ctrl::DoMouseFB(Ctrl::LEFTDOWN, p); break;
					case SDL_BUTTON_RIGHT: mouseb |= (1<<1); Ctrl::DoMouseFB(Ctrl::RIGHTDOWN, p); break;
					case SDL_BUTTON_MIDDLE: mouseb |= (1<<2); Ctrl::DoMouseFB(Ctrl::MIDDLEDOWN, p); break;
				}
			}
		}
		break;
	case SDL_MOUSEBUTTONUP: {
			int bi = event->button.button;
			isdblclick[bi] = 1; //indicate maybe a dblclick
	
			Point p(event->button.x, event->button.y);
			switch(bi)
			{
				case SDL_BUTTON_LEFT: mouseb &= ~(1<<0); Ctrl::DoMouseFB(Ctrl::LEFTUP, p); break;
				case SDL_BUTTON_RIGHT: mouseb &= ~(1<<1); Ctrl::DoMouseFB(Ctrl::RIGHTUP, p); break;
				case SDL_BUTTON_MIDDLE: mouseb &= ~(1<<2); Ctrl::DoMouseFB(Ctrl::MIDDLEUP, p); break;
			}
		}
		break;
/*		case SDL_VIDEORESIZE: //SDL_ResizeEvent
		{
			width = event->resize.w;
			height = event->resize.h;
	
			SDL_FreeSurface(screen);
			screen = CreateScreen(width, height, bpp, videoflags);
			ASSERT(screen);
			Ctrl::SetFramebufferSize(Size(width, height));
		}
			break;
		case SDL_VIDEOEXPOSE: //SDL_ExposeEvent
			break;*/
	case SDL_WINDOWEVENT:
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            break;
        case SDL_WINDOWEVENT_MOVED:
            break;
//		case SDL_WINDOWEVENT_SIZE_CHANGED:
//			SDLwidth = event->window.data1;
//			SDLheight = event->window.data2;
//      	break;
        case SDL_WINDOWEVENT_RESIZED:
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            break;
        case SDL_WINDOWEVENT_RESTORED:
            break;
        case SDL_WINDOWEVENT_ENTER:
			sdlMouseIsIn = true;
			Ctrl::PaintAll();
            break;
        case SDL_WINDOWEVENT_LEAVE:
			sdlMouseIsIn = false;
			Ctrl::PaintAll();
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            break;
        case SDL_WINDOWEVENT_CLOSE:
            break;
        }
		break;
	case SDL_QUIT: //SDL_QuitEvent
		Ctrl::EndSession();
		break;
	}
}

bool SDL2GUI::ProcessEvent(bool *quit)
{
	bool ret = false;
	SDL_Event event;
	if(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT && quit)
			*quit = true;
		HandleSDLEvent(&event);
		ret = true;
	}
	return ret;
}


void SDL2GUI::WaitEvent(int ms)
{
	SDL_WaitEventTimeout(NULL, ms);
}

bool SDL2GUI::IsWaitingEvent()
{
	SDL_PumpEvents();
	SDL_Event events;
	return SDL_PeepEvents(&events, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0;
}

SDL_TimerID waketimer_id = 0;
Uint32 WakeCb(Uint32 interval, void *param)
{
	//wake up message que, FIXME maybe it can be done better?
	SDL_Event event;
	event.type=SDL_USEREVENT;
	SDL_PushEvent(&event);
	return 0;
}

void SDL2GUI::WakeUpGuiThread()
{
	waketimer_id = SDL_AddTimer(20, WakeCb, NULL);
}

}
