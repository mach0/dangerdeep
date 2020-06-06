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

// xml access interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "xml.h"
#include "tinyxml/tinyxml.h"
#include <cstdio>

#ifdef WIN32
#ifdef _MSC_VER
#define snprintf sprintf_s
#endif
#endif

using std::string;

xml_elem xml_elem::child(const std::string& name) const
{
	TiXmlElement* e = elem->FirstChildElement(name);
	if (!e) THROW(xml_elem_error, name, doc_name());
	return xml_elem(e);
}



bool xml_elem::has_child(const std::string& name) const
{
	TiXmlElement* e = elem->FirstChildElement(name);
	return e != nullptr;
}



xml_elem xml_elem::add_child(const std::string& name)
{
	auto* e = new TiXmlElement(name);
	elem->LinkEndChild(e);
	return {e};
}



const std::string& xml_elem::doc_name() const
{
	TiXmlDocument* doc = elem->GetDocument();
	// extra-Paranoia... should never happen
	if (!doc) THROW(xml_error, std::string("can't get document name for node ") + elem->ValueStr(), "???");
	return doc->ValueStr();
}



bool xml_elem::has_attr(const std::string& name) const
{
	return elem->Attribute(name) != nullptr;
}



std::string xml_elem::attr(const std::string& name) const
{
	const std::string* tmp = elem->Attribute(name);
	if (tmp) return *tmp;
	return std::string();
}



int xml_elem::attri(const std::string& name) const
{
	const std::string* tmp = elem->Attribute(name);
	if (tmp) return atoi(tmp->c_str());
	return 0;
}



unsigned xml_elem::attru(const std::string& name) const
{
	return unsigned(attri(name));
}



double xml_elem::attrf(const std::string& name) const
{
	const std::string* tmp = elem->Attribute(name);
	if (tmp) return atof(tmp->c_str());
	return 0.0;
}



vector3 xml_elem::attrv3() const
{
	return {attrf("x"), attrf("y"), attrf("z")};
}



vector2 xml_elem::attrv2() const
{
	return {attrf("x"), attrf("y")};
}



quaternion xml_elem::attrq() const
{
	return quaternion(attrf("s"), attrv3());
}



angle xml_elem::attra() const
{
	return {attrf("angle")};
}



bool xml_elem::attrb(const std::string& name) const
{
	return attru(name) != 0;
}



void xml_elem::set_attr(const std::string& val, const std::string& name)
{
	elem->SetAttribute(name, val);
}



void xml_elem::set_attr(unsigned u, const std::string& name)
{
	set_attr(int(u), name);
}



void xml_elem::set_attr(int i, const std::string& name)
{
	elem->SetAttribute(name, i);
}



void xml_elem::set_attr(double f, const std::string& name)
{
	// note! DO NOT USE std::ostringstream HERE!
	// its format is different to sprintf(), it has less precision!
	// we could change ostringstream's format, but for what? this is easier...
	char tmp[64];
	int l = snprintf(tmp, 64, "%f", f);
	// strip unneeded zeros at end.
	for (int i = l-1; i >= 0; --i) {
		if (tmp[i] == '0') {
			tmp[i] = 0;
		} else {
			// strip dot at end, if it remains
			if (tmp[i] == '.') {
				tmp[i] = 0;
			}
			break;
		}
	}
	set_attr(std::string(tmp), name);
}



void xml_elem::set_attr(const vector3& v)
{
	set_attr(v.x, "x");
	set_attr(v.y, "y");
	set_attr(v.z, "z");
}



void xml_elem::set_attr(const vector2& v)
{
	set_attr(v.x, "x");
	set_attr(v.y, "y");
}



void xml_elem::set_attr(const quaternion& q)
{
	set_attr(q.s, "s");
	set_attr(q.v);
}



void xml_elem::set_attr(angle a)
{
	set_attr(a.value(), "angle");
}



void xml_elem::set_attr(bool b, const std::string& name)
{
	set_attr(unsigned(b), name);
}



const std::string& xml_elem::get_name() const
{
	return elem->ValueStr();
}



void xml_elem::add_child_text(const std::string& txt)
{
	elem->LinkEndChild(new TiXmlText(txt));
}



const std::string& xml_elem::child_text() const
{
	TiXmlNode* ntext = elem->FirstChild();
	if (!ntext)
		THROW(xml_error, std::string("child of ") + get_name() + std::string(" is no text node"), doc_name());
	return ntext->ValueStr();
}



xml_elem::iterator xml_elem::begin() const
{
	return iterator(*this, elem->FirstChildElement(), false);
}



xml_elem::iterator xml_elem::iterator_range_samename::begin() const
{
	return iterator(parent, parent.elem->FirstChildElement(childname), true);
}



xml_elem xml_elem::iterator::operator*() const
{
	if (!e) THROW(xml_error, "elem() on empty iterator", parent.doc_name());
	return xml_elem(e);
}



xml_elem::iterator& xml_elem::iterator::operator++()
{
	if (!e) THROW(xml_error, "next() on empty iterator", parent.doc_name());
	if (samename)
		e = e->NextSiblingElement(e->ValueStr());
	else
		e = e->NextSiblingElement();
	return *this;
}



xml_doc::xml_doc(std::string fn)
	: doc(new TiXmlDocument(fn))
{
}



xml_doc::~xml_doc()
{
	// needed to make unique_ptr compile
}



void xml_doc::load()
{
	if (!doc->LoadFile()) {
		THROW(xml_error, std::string("can't load: ") + doc->ErrorDesc(), doc->ValueStr());
	}
}



void xml_doc::save()
{
	if (!doc->SaveFile()) {
		THROW(xml_error, std::string("can't save: ") + doc->ErrorDesc(), doc->ValueStr());
	}
}



xml_elem xml_doc::first_child()
{
	TiXmlElement* e = doc->FirstChildElement();
	if (!e) THROW(xml_elem_error, "<first-child>", doc->ValueStr());
	return {e};
}



xml_elem xml_doc::child(const std::string& name)
{
	TiXmlElement* e = doc->FirstChildElement(name);
	if (!e) THROW(xml_elem_error, name, doc->ValueStr());
	return {e};
}



xml_elem xml_doc::add_child(const std::string& name)
{
	auto* e = new TiXmlElement(name);
	doc->LinkEndChild(e);
	return {e};
}



const std::string& xml_doc::get_filename() const
{
	return doc->ValueStr();
}
