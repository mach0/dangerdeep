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

// a generic object cache
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>

///\brief Handles and caches instances of globally used objects.
// fixme: to make it useable as *cache* we need to delay destruction. when an
// object reaches refcount zero, do not delete it immediatly. Check periodically
// or when the next object is deleted, so at least 1-2 objects can be hold with
// refcount zero, like a delete-queue. This avoids permanent reload when e.g.
// user switches between two menes and both use images of the image-cache.
// fixme2: maybe add special handler-class, like an auto_ptr, c'tor ref's an
// object, d'tor unrefs it. Thus objcache usage is easier.

// fixme 2: add "reference" class, that is auto_ptr like reference handler. Do
// NOT return plain pointers from the cache. Because if we generate resources by
// ref'ing the cache in some code and an exception is thrown, the ref'd objects
// won't get unref'd again, leading to memory waste (though NOT memory leaks)
template<class T>
class objcachet
{
    std::map<std::string, std::pair<unsigned, T*>> cache;
    std::string basedir;
    objcachet()           = delete;
    objcachet<T>& operator=(const objcachet<T>&) = delete;
    objcachet(const objcachet<T>&)               = delete;

  public:
    objcachet(std::string basedir_) : basedir(std::move(basedir_)) { }
    ~objcachet() { clear(); }

    // call to deinit cache
    void clear()
    {
        for (auto it = cache.begin(); it != cache.end(); ++it)
            delete it->second.second;
        cache.clear();
    }

    T* find(const std::string& objname)
    {
        if (objname.empty())
            return (T*) nullptr;
        auto it = cache.find(objname);
        if (it == cache.end())
            return nullptr;
        return it->second.second;
    }

    T* ref(const std::string& objname)
    {
        if (objname.empty())
            return (T*) nullptr;
        auto it = cache.find(objname);
        if (it == cache.end())
        {
            it =
                cache
                    .insert(std::make_pair(
                        objname,
                        std::make_pair((unsigned) 1, new T(basedir + objname))))
                    .first;
        }
        else
        {
            ++(it->second.first);
        }
        return it->second.second;
    }

    bool ref(const std::string& objname, T* obj)
    {
        if (objname.empty())
            return false; // no valid name
        auto it = cache.find(objname);
        if (it == cache.end())
        {
            it = cache
                     .insert(std::make_pair(
                         objname, std::make_pair((unsigned) 1, obj)))
                     .first;
        }
        else
        {
            return false; // already exists
        }
        return true;
    }

    void unref(const std::string& objname)
    {
        if (objname.empty())
            return;
        auto it = cache.find(objname);
        if (it != cache.end())
        {
            if (it->second.first == 0)
            {
                // error, unref'd too much...
                return;
            }
            --(it->second.first);
            if (it->second.first == 0)
            {
                delete it->second.second;
                cache.erase(it);
            }
        }
    }

    void unref(T* obj)
    {
        for (auto it = cache.begin(); it != cache.end(); ++it)
        {
            if (it->second.second == obj)
            {
                if (it->second.first == 0)
                {
                    // error, unref'd too much...
                    return;
                }
                --(it->second.first);
                if (it->second.first == 0)
                {
                    delete it->second.second;
                    cache.erase(it);
                }
                break;
            }
        }
    }

    void print() const
    {
        std::cout << "objcache: " << cache.size() << " entries.\n";
        for (typename std::map<std::string, std::pair<unsigned, T*>>::
                 const_iterator it = cache.begin();
             it != cache.end();
             ++it)
            std::cout << "key=\"" << it->first << "\" ref=" << it->second.first
                      << " addr=" << it->second.second << "\n";
    }

    class reference
    {
        objcachet<T>& mycache;
        T* myobj;

      public:
        reference(objcachet<T>& cache, const std::string& objname) :
            mycache(cache), myobj(cache.ref(objname))
        {
        }
        ~reference() { mycache.unref(myobj); }
        T* get() { return myobj; }
        const T* get() const { return myobj; }
    };
};

/// handle class to use as reference
template<class C, typename Key = std::string>
class object_handle
{
  public:
    object_handle() = default;
    ~object_handle() { unref(); }
    C& get() { return *storage; }
    const C& get() const { return *storage; }
    bool is_valid() const { return mystore != nullptr; }
    C* operator->() { return storage; }
    const C* operator->() const { return storage; }
    object_handle(objcachet<C>& store, Key key_) :
        mystore(&store), key(std::move(key_)), storage(store.ref(key))
    {
    }
    object_handle(object_handle&& o) :
        mystore(o.mystore), key(std::move(o.key)), storage(o.storage)
    {
        o.storage = nullptr;
    }
    object_handle& operator=(object_handle&& o)
    {
        if (&o != this)
        {
            mystore   = o.mystore;
            key       = std::move(o.key);
            storage   = o.storage;
            o.storage = nullptr;
        }
        return *this;
    }

  protected:
    object_handle(const object_handle&) = delete;
    object_handle& operator=(const object_handle&) = delete;
    void unref()
    {
        if (mystore != nullptr && storage != nullptr)
        {
            mystore->unref(key);
            mystore = nullptr;
            storage = nullptr;
        }
    }
    objcachet<C>* mystore{nullptr};
    Key key;
    C* storage{nullptr};
};
