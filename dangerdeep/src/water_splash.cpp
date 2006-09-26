/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

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

// water splash (C)+(W) 2006 Thorsten Jordan

#include "water_splash.h"
#include "global_data.h"
#include "texture.h"

void water_splash::render_cylinder(double radius_bottom, double radius_top, double height,
				   double alpha, double u_scal, unsigned nr_segs)
{
	glBegin(GL_QUAD_STRIP);
	glColor4f(1, 1, 1, alpha);
	double us = u_scal / nr_segs;
	//fixme: use different alpha for bottom? like always full alpha?
	for (unsigned i = 0; i <= nr_segs; ++i) {
		double a = -2*M_PI*i/nr_segs;
		double sa = sin(a);
		double ca = cos(a);
		glColor4f(1,1,1,0.5+0.5*alpha);
		glTexCoord2f(i * us, 1);
		glVertex3f(radius_bottom * ca, radius_bottom * sa, -5.0); // compensate tide!
		glColor4f(1, 1, 1, alpha);
		glTexCoord2f(i * us, 0);
		glVertex3f(radius_top * ca, radius_top * sa, height);
	}
	glEnd();
}



void water_splash::compute_values(double risetime, double riseheight)
{
	double falltime = sqrt(riseheight * 2.0 / GRAVITY);
	lifetime = risetime + falltime;
}



water_splash::water_splash(const vector3& pos, double tm)
{
	std::vector<double> p;
	p.push_back(0);		// 0s:	0m
	p.push_back(20);	// 0.5s:20m
	p.push_back(18.75);	// 1.0s:18.75
	p.push_back(15.0);	// 1.5s:15
	p.push_back(8.75);	// 2.0s:8.75
	p.push_back(0.0);	// 2.5s:0
	bheight.reset(new bspline(3, p));
	p[0] = 5.0;
	p[1] = 5.0;
	p[2] = 6.0;
	p[3] = 7.0;
	p[4] = 8.0;
	p[5] = 9.0;
	bradius_top.reset(new bspline(3, p));
	p[0] = 5.0;
	p[1] = 5.0;
	p[2] = 5.2;
	p[3] = 5.4;
	p[4] = 5.6;
	p[5] = 5.8;
	bradius_bottom.reset(new bspline(3, p));
	p[0] = 1.0;
	p[1] = 1.0;
	p[2] = 0.75;
	p[3] = 0.5;
	p[4] = 0.25;
	p[5] = 0.0;
	balpha.reset(new bspline(3, p));

	starttime = tm;
	compute_values(0.4, 25.0);	// reach 25m in 0.4secs.
}



void water_splash::display(double tm) const
{
	if (tm - starttime > lifetime + 0.5)
		return;

	texturecache.ref("splashring.png")->set_gl_texture();

	glDisable(GL_LIGHTING);
	//glTranslate
	//render two cylinders...
	//alpha 0% at end, increase radius on fade, widen more
	if (tm - starttime >= 0.5) {
		double t = (tm - starttime - 0.5)/lifetime;
		double rt = bradius_top->value(t) * 0.8;
		double rb = bradius_bottom->value(t) * 0.8;
		double a = balpha->value(t);
		render_cylinder(rb, rt, bheight->value(t) * 1.2, a);
	}
	if (tm - starttime <= lifetime) {
		double t = (tm - starttime)/lifetime;
		double rt = bradius_top->value(t);
		double rb = bradius_bottom->value(t);
		double a = balpha->value(t);
		render_cylinder(rb, rt, bheight->value(t), a);
	}
	glEnable(GL_LIGHTING);
}
