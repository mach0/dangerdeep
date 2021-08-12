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

// pointer to any ressource with user defined freeing
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

template<typename T>
void free_ressource(T*)
{
}

template<typename T>
class ressource_ptr
{
  public:
    ressource_ptr() : ptr(nullptr) { }
    ressource_ptr(T* p) : ptr(p) { }

    ~ressource_ptr() { free_ressource(ptr); }
    ressource_ptr(ressource_ptr&& m) : ptr(m.ptr) { m.ptr = nullptr; }

    ressource_ptr& operator=(ressource_ptr&& m)
    {
        auto tmp = m.ptr;
        m.ptr    = nullptr;
        ptr      = tmp;
    }

    [[nodiscard]] T* get() const { return ptr; }

  protected:
    T* ptr;
    ressource_ptr(const ressource_ptr&);
    ressource_ptr& operator=(const ressource_ptr&);
};
