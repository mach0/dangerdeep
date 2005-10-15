// user display: submarine's tdc
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TDC_DISPLAY_H
#define SUB_TDC_DISPLAY_H

#include "user_display.h"
#include "image.h"
#include <vector>

class sub_tdc_display : public user_display
{
	class scheme {
	public:
		auto_ptr<image> background;
		rotat_tex clockbig;
		rotat_tex clocksml;
		rotat_tex targetcourse;
		rotat_tex targetrange;
		rotat_tex targetspeed;
		rotat_tex spreadangle;
		rotat_tex targetpos;
		// everything that does not rotate could also be an "image"...
		// but only when this doesn't trash the image cache
		texture::ptr tubelight[6];
		texture::ptr tubeswitch[6];
		texture::ptr firebutton;
		texture::ptr automode[2];	// on/off
		rotat_tex gyro360;
		rotat_tex gyro10;
		texture::ptr firesolutionquality;
		rotat_tex torpspeed;
		scheme() {}
	protected:
		scheme(const scheme& );
		scheme& operator= (const scheme& );
	};

	scheme normallight, nightlight;

	unsigned selected_mode;	// 0-1 (automatic on / off)

public:
	sub_tdc_display(class user_interface& ui_);

	//overload for zoom key handling ('y') and TDC input
	virtual void process_input(class game& gm, const SDL_Event& event);
	virtual void display(class game& gm) const;
};

#endif
