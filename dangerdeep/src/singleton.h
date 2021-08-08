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

// this class holds the game's configuration
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SINGLETON_H
#define SINGLETON_H

/// Implementation of the singleton pattern
template <typename D> class singleton
{
  private:
    static D*& instance_ptr()
    {
        static D* myinstanceptr = nullptr;
        return myinstanceptr;
    }

  public:
    /// get the one and only instance
    /// @remarks since D is constructed not before first call, it avoids the
    /// static initialization order fiasco.
    static D& instance()
    {
        D*& p = instance_ptr();
        if (!p)
            p = new D();
        return *p;
    }

    static void create_instance(D* ptr)
    {
        D*& p = instance_ptr();
        delete p;
        p = ptr;
    }

    static void destroy_instance()
    {
        D*& p = instance_ptr();
        delete p;
        p = nullptr;
    }

    static D* release_instance()
    {
        D*& p  = instance_ptr();
        D* ptr = p;
        p      = nullptr;
        return ptr;
    }

  protected:
    singleton() = default;

  private:
    singleton(const singleton&) = delete;
    singleton(singleton&&)      = delete;
    singleton& operator=(const singleton&) = delete;
    singleton& operator=(singleton&&) = delete;
};

#endif
