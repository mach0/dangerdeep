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

// ptrvector - vector of ptrs, like std::unique_ptr, but with std::vector interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PTRVECTOR_H
#define PTRVECTOR_H

#include <vector>
#include <memory>
#include "helper.h"

/// same as std::vector regarding the interface, but handles pointers like std::unique_ptr.
template <typename T, class pointer = std::unique_ptr<T>>
class ptrvector
{
 protected:
	std::vector<pointer> data;

 private:
	ptrvector(const ptrvector& ) = delete;
	ptrvector& operator= (const ptrvector& ) = delete;

 public:
	ptrvector(size_t capacity = 0) : data(capacity) {}

	void resize(size_t newsize) { data.resize(newsize); }
	size_t size() const { return data.size(); }
	size_t capacity() const { return data.capacity(); }
	void clear() { data.clear(); }
	void swap(ptrvector& other) { data.swap(other.data); }

	/// push_back element.
	void push_back(pointer&& ptr) { data.push_back(std::move(ptr)); }

	/// push_back a pointer exception safe.
	void push_back(T* ptr) { data.push_back(pointer(ptr)); }

	T& operator[](size_t n) const { return *data[n]; }
	T& front() { return *data.front(); }
	T& back() { return *data.back(); }
	void set(size_t n, T* p) { data[n] = std::unique_ptr<T>(p); }
	void set(size_t n, pointer p) { data[n] = std::move(p); }
	bool is_valid(size_t n) const { return data[n].get() != nullptr; }

	void reset(size_t n, T* ptr = nullptr) { data[n] = pointer(ptr); }
	bool empty() const { return data.empty(); }

	T* release(unsigned n) { return data[n].release(); }

	void compact()
	{
		helper::erase_remove_if(data, [](const pointer& p) { return p == nullptr; });
	}
	
	class iterator
	{
	public:
		iterator(ptrvector<T, pointer>& myvec, size_t pos) : myvector(myvec), position(pos - 1) { ++(*this); }
		T& operator*() { return myvector[position]; }
		iterator& operator++() {
			for (++position; position < myvector.size() && !myvector.is_valid(position); ++position);
			return *this;
		}
		bool operator!= (const iterator& it) const { return position != it.position; }
	protected:
		ptrvector<T, pointer>& myvector;
		size_t position;
	};
	
	class const_iterator
	{
	public:
		const_iterator(const ptrvector<T, pointer>& myvec, size_t pos) : myvector(myvec), position(pos - 1) { ++(*this); }
		T& operator*() { return myvector[position]; } // note - Pointer is const, but not element itself!
		const_iterator& operator++() {
			for (++position; position < myvector.size() && !myvector.is_valid(position); ++position);
			return *this;
		}
		bool operator!= (const const_iterator& it) const { return position != it.position; }
	protected:
		const ptrvector<T, pointer>& myvector;
		size_t position;
	};
	
	iterator begin() { return iterator(*this, 0); }
	iterator end() { return iterator(*this, size()); }
	const_iterator begin() const { return const_iterator(*this, 0); }
	const_iterator end() const { return const_iterator(*this, size()); }
};

template<typename T> using sharedptrvector = ptrvector<T, std::shared_ptr<T>>;

#endif
