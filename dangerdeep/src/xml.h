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

#ifndef XML_H
#define XML_H

#include "angle.h"
#include "error.h"
#include "quaternion.h"
#include "vector3.h"

#include <memory>
#include <string>

class TiXmlElement;
class TiXmlDocument;

///\brief General exception for an error while using the XML interface
class xml_error : public error
{
  public:
    xml_error(
        const std::string& location, const std::string& name,
        const std::string& fn) :
        error(
            location,
            std::string("xml error: ") + name + std::string(", file: ") + fn)
    {
    }
};

///\brief XML element specific exception
class xml_elem_error : public xml_error
{
  public:
    xml_elem_error(
        const std::string& location, const std::string& name,
        const std::string& fn) :
        xml_error(location, std::string("failed to get element ") + name, fn)
    {
    }
};

///\brief A XML element representation with interface for handling of elements
///like adding or requesting children or data.
class xml_elem
{
  private:
    xml_elem() = delete;

  protected:
    TiXmlElement* elem;
    xml_elem(TiXmlElement* e) : elem(e) { }

    friend class xml_doc;

  public:
    bool has_attr(const std::string& name = "value") const;
    std::string attr(const std::string& name = "value") const;
    int attri(const std::string& name = "value") const;
    unsigned attru(const std::string& name = "value") const;
    double attrf(const std::string& name = "value") const;
    vector3 attrv3() const;
    vector2 attrv2() const;
    vector2i attrv2i() const;
    quaternion attrq() const;
    angle attra() const;
    bool attrb(const std::string& name = "value") const;
    xml_elem child(const std::string& name) const;
    bool has_child(const std::string& name) const;
    xml_elem add_child(const std::string& name);
    void set_attr(const std::string& val, const std::string& name = "value");
    void set_attr(unsigned u, const std::string& name = "value");
    void set_attr(int i, const std::string& name = "value");
    void set_attr(double f, const std::string& name = "value");
    void set_attr(const vector3& v);
    void set_attr(const vector2& v);
    void set_attr(const quaternion& q);
    void set_attr(angle a);
    void set_attr(bool b, const std::string& name = "value");
    void get_attr(std::string& val, const std::string& name = "value") const
    {
        val = attr(name);
    }
    void get_attr(unsigned& val, const std::string& name = "value") const
    {
        val = attru(name);
    }
    void get_attr(int& val, const std::string& name = "value") const
    {
        val = attri(name);
    }
    void get_attr(double& val, const std::string& name = "value") const
    {
        val = attrf(name);
    }
    void get_attr(vector3& val) const { val = attrv3(); }
    void get_attr(vector2& val) const { val = attrv2(); }
    void get_attr(quaternion& val) const { val = attrq(); }
    void get_attr(angle& val) const { val = attra(); }
    void get_attr(bool& val, const std::string& name = "value") const
    {
        val = attrb(name);
    }
    const std::string& get_name() const;
    void add_child_text(const std::string& txt); // add text child
    const std::string& child_text()
        const; // returns value of text child, throws error if there is none

    // get name of document
    const std::string& doc_name() const;

    /// An iterator to iterate over children of a XML node. Chose children with
    /// defined and same name or any child.
    class iterator
    {
      private:
        iterator() = delete;

      protected:
        const xml_elem& parent;
        TiXmlElement* e;
        bool samename; // iterate over any children or only over children with
                       // same name
      public:
        iterator(
            const xml_elem& parent_, TiXmlElement* elem_ = nullptr,
            bool samename_ = true) :
            parent(parent_),
            e(elem_), samename(samename_)
        {
        }
        xml_elem operator*() const;
        iterator& operator++();
        bool operator!=(const iterator& it) const { return e != it.e; }
    };

    /// Matching iterator range class. Note that the childname MUST NOT be
    /// std::string& because when giving const char* strings, the std::string
    /// would be a temporary and get lost, iteration will then fail later!
    class iterator_range_samename
    {
      public:
        iterator_range_samename(
            const xml_elem& parent_, const char* childname_) :
            parent(parent_),
            childname(childname_)
        {
        }
        iterator begin() const;
        iterator end() const { return parent.end(); }

      protected:
        const xml_elem& parent;
        const char* childname;
    };

    friend class iterator_range_samename;

    iterator begin() const;
    iterator end() const { return iterator(*this, nullptr, false); }
    iterator_range_samename iterate(const char* childname) const
    {
        return iterator_range_samename(*this, childname);
    }
};

///\brief A XML document representation with interface for handling of
///documents.
class xml_doc
{
  private:
    xml_doc()               = delete;
    xml_doc(const xml_doc&) = delete;
    xml_doc& operator=(const xml_doc&) = delete;

  protected:
    std::unique_ptr<TiXmlDocument> doc;

  public:
    xml_doc(std::string fn);
    ~xml_doc();
    void load();
    void save();
    xml_elem first_child();
    xml_elem child(const std::string& name);
    xml_elem add_child(const std::string& name);
    const std::string& get_filename() const;
};

#endif // XML_H
