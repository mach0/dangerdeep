/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2020  Thorsten Jordan, Luis Barrancos and others.

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

// Base interface for user screens.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#include "user_display.h"
#include "user_interface.h"
#include "system_interface.h"
#include "xml.h"

user_display::user_display(class user_interface& ui_, const char* display_name)
 :	ui(ui_)
{
	// if displays use no elements they give nullptr as definition
	if (display_name != nullptr) {
		auto display_dir = get_display_dir() + display_name + "/";
		xml_doc display_config(display_dir + "layout.xml");
		display_config.load();
		for (auto elem : display_config.child("dftd-display").iterate("element")) {
			auto pos = elem.attrv2i();
			auto id = -1;
			if (elem.has_attr("id")) {
				id = elem.attri("id");
			}
			std::string filename_day;
			if (!elem.has_child("day")) {
				THROW(error, std::string(display_name) + ", invalid display def xml file, day node missing");
			}
			filename_day = elem.child("day").child_text();
			std::string filename_night;
			if (elem.has_child("night")) {
				filename_night = elem.child("night").child_text();
			}
			angle start_angle;
			angle end_angle;
			double start_value{0.0};
			double end_value{360.0};
			angle rotation_offset;
			if (elem.has_child("scale")) {
				// If a scale tag is given the value is limited and scaled
				auto elem_scale = elem.child("scale");
				auto elem_start = elem_scale.child("start");
				auto elem_end = elem_scale.child("end");
				rotation_offset = -angle(elem_scale.attrf("pointer"));
				start_angle = angle(elem_start.attrf("angle"));
				start_value = elem_start.attrf("value");
				end_angle = angle(elem_end.attrf("angle"));
				end_value = elem_end.attrf("value");
			}
			vector2i centerpos;
			bool has_center{false};
			if (elem.has_child("center")) {
				auto elem_center = elem.child("center");
				centerpos = elem_center.attrv2i();
				has_center = true;
			} else if (elem.has_child("size")) {
				auto elem_size = elem.child("size");
				centerpos = pos + elem_size.attrv2i() / 2;
				has_center = true;
			}
			if (has_center) {
				elements.emplace_back(pos, centerpos, start_angle, start_value, end_angle, end_value, rotation_offset, display_dir + filename_day, display_dir + filename_night);
			} else if (elem.has_child("phases")) {
				auto elem_phases = elem.child("phases");
				auto nr = elem_phases.attru("nr");
				auto offset = elem_phases.attru("offset");
				elements.emplace_back(pos, nr, offset, display_dir + filename_day, display_dir + filename_night);
			} else {
				// normal static element
				elements.emplace_back(pos, display_dir + filename_day, display_dir + filename_night);
			}
			if (id != -1) {
				id_to_element[id] = unsigned(elements.size() - 1);
			}
		}
	}
}


user_display::elem2D::elem2D(const vector2i& pos, const std::string& filename_day, const std::string& filename_night)
 :	position(pos)
 ,	has_night(!filename_night.empty())
{
	filenames_day.push_back(filename_day);
	filenames_night.push_back(filename_night);
	tex.resize(1);
}



user_display::elem2D::elem2D(const vector2i& pos, const vector2i& ctr, angle start_angle_, double start_value_, angle end_angle_, double end_value_, angle rotation_offset_, const std::string& filename_day, const std::string& filename_night)
 :	position(pos)
 ,	center(ctr)
 ,	rotateable(true)
 ,	has_night(!filename_night.empty())
 ,	start_angle(start_angle_)
 ,	start_value(start_value_)
 ,	end_angle(end_angle_)
 ,	end_value(end_value_)
 ,	rotation_offset(rotation_offset_)
{
	filenames_day.push_back(filename_day);
	filenames_night.push_back(filename_night);
	tex.resize(1);
}



user_display::elem2D::elem2D(const vector2i& pos, unsigned phases, unsigned offset, const std::string& filename_day, const std::string& filename_night)
 :	position(pos)
 ,	has_night(!filename_night.empty())
{
	filenames_day.resize(phases);
	filenames_night.resize(phases);
	tex.resize(phases);
	for (unsigned i = 0; i < phases; ++i) {
		filenames_day[i] = helper::replace_first(filename_day, "%u", helper::str(offset + i));
		if (has_night) {
			filenames_night[i] = helper::replace_first(filename_night, "%u", helper::str(offset + i));
		}
	}
}



void user_display::elem2D::set_phase(float phase_) const
{
	phase = std::min(unsigned(tex.size() * phase_), unsigned(tex.size() - 1));
}



void user_display::elem2D::draw() const
{
	if (rotateable) {
		// rotation around pixel center (offset +0.5) could be sensible but it looks correct that way.
		const auto display_angle = rotation_offset + start_angle + (get_angle_range() * (value - start_value) / (end_value - start_value));
		tex[phase]->draw_rot(center.x, center.y, display_angle.value(), center.x - position.x, center.y - position.y);
	} else {
		tex[phase]->draw(position.x, position.y);
	}
}



bool user_display::elem2D::is_mouse_over(const vector2i& pos, int tolerance) const
{
	return (pos.x + tolerance >= position.x && pos.y + tolerance >= position.y
		&& pos.x - tolerance < position.x + int(tex[phase]->get_width())
		&& pos.y - tolerance < position.y + int(tex[phase]->get_height()));
}



void user_display::elem2D::set_draw_size(const vector2i& sz)
{
	//fixme?! needed by bridge and damage - but why?
}



void user_display::elem2D::init(bool is_day)
{
	// init all textures
	for (auto i = 0U; i < unsigned(tex.size()); ++i) {
		tex[i] = std::make_unique<texture>((is_day || !has_night || filenames_night[i].empty()) ? filenames_day[i] : filenames_night[i]);
	}
}



void user_display::elem2D::deinit()
{
	// delete all textures
	for (auto i = 0U; i < unsigned(tex.size()); ++i) {
		tex[i] = nullptr;
	}
}



void user_display::elem2D::set_value(double v) const
{
	value = std::clamp(v, std::min(start_value, end_value), std::max(start_value, end_value));
}



std::optional<double> user_display::elem2D::get_value(const vector2i& pos) const
{
	// need to negate y, because onscreen y is down
	const auto a = angle(vector2(pos.x - center.x, center.y - pos.y)) - start_angle;
	if (a.value() > get_angle_range()) {
		return std::nullopt;
	}
	return helper::interpolate(start_value, end_value, std::min(1.0, a.value() / get_angle_range()));
}



std::optional<unsigned> user_display::elem2D::get_value_uint(const vector2i& pos) const
{
	const auto v = get_value(pos);
	if (!v.has_value() || v.value() < 0.0 || v.value() >= end_value) {
		return std::nullopt;
	}
	return unsigned(std::floor(v.value()));
}



double user_display::elem2D::get_angle_range() const
{
	auto range = end_angle - start_angle;
	// if start and end angle match use full range
	if (range.value() < 1.0) {
		return 360.0;
	}
	return range.value();
}



const user_display::elem2D& user_display::element_for_id(unsigned id) const
{
	auto it = id_to_element.find(id);
	if (it == id_to_element.end()) {
		THROW(error, "invalid display definition, id not found");
	}
	return elements[it->second];
}



void user_display::draw_elements() const
{
	sys().prepare_2d_drawing();
	for (auto& e : elements) {
		e.draw();
	}
	//fixme every display?
	ui.draw_infopanel();
	sys().unprepare_2d_drawing();
}



void user_display::enter(bool is_day)
{
	// default: call init on all elements
	for (auto& e : elements) {
		e.init(is_day);
	}
}



void user_display::leave()
{
	// default: call deinit on all elements
	for (auto& e : elements) {
		e.deinit();
	}
}
