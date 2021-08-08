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

// A generic store for named objects with reference counter.
// (C)+(W) by Thorsten Jordan. See LICENSE

#include <memory>
#include <string>
#include <unordered_map>

/// Manages storage of named objects
template <class C, typename Key = std::string> class object_store
{
  public:
    /// handle class to use as reference
    class handle
    {
      public:
        handle() : mystore(nullptr), storage(nullptr) { }
        ~handle() { unref(); }
        C& get() { return *storage; }
        const C& get() const { return *storage; }
        bool is_valid() const { return mystore != nullptr; }

      protected:
        handle(object_store& store, Key key_, C& obj)
        {
            mystore = &store;
            key     = std::move(key_);
            storage = &store.ref(key);
        }
        void unref()
        {
            if (mystore)
            {
                mystore->unref(key);
                mystore = nullptr;
                storage = nullptr;
            }
        }
        object_store* mystore;
        Key key;
        C* storage;
    };

    /// Get handle to named object
    handle get_handle(const Key& name)
    {
        auto& obj = ref(name);
        return handle(*this, name, obj);
    }

    /// Get reference to named object - give funtion to construct it as
    /// parameter, so any parameters for construction can be passed.
    C& ref(
        const Key& name,
        std::function<std::unique_ptr<C>(const Key&)> maker = [](const Key& k) {
            return std::make_unique<C>(k);
        })
    {
        auto it = storage
                      .insert(std::make_pair(
                          name, std::make_pair(0, std::unique_ptr<C>())))
                      .first;
        if (it->second.first == 0)
        {
            it->second.second = maker(name);
        }
        ++it->second.first;
        return *it->second.second;
    }

    /// Give back reference/used object
    void unref(const Key& name)
    {
        auto it = storage.find(name);
        if (it == storage.end())
        {
            THROW(error, "tried to unref unknown object");
        }
        if (it->second.first == 0)
        {
            THROW(error, "unref with already empty object");
        }
        --it->second.first;
        if (it->second.first == 0)
        {
            it->second.second.reset(nullptr);
        }
    }

  protected:
    /// Data storage
    std::unordered_map<Key, std::pair<unsigned, std::unique_ptr<C>>> storage;
};
