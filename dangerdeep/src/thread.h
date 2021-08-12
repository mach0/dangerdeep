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

// multithreading primitives: thread
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include "error.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

/// base class for threads.
///@note Each thread should be an instance of a class that inherits
///	from class thread. Overload some member functions to fill in code for the
/// thread, 	like init(), deinit(), loop() 	threads must be allocated with
/// new.\n 	Inherit from this class and implement init() / loop() / deinit()
class thread
{
  private:
    /// The state a thread is in
    enum class state
    {
        none,        // before start
        running,     // normal operation
        finished,    // after thread has exited (can't be restarted)
        init_failed, // when init has failed
        aborted      // when run/deinit has failed (internal error!)
    };

    std::thread thread_id;
    bool abort_request;
    state mystate;
    std::mutex state_mutex;
    std::condition_variable start_cond;
    std::string error_message; // to pass exception texts via thread boundaries

    const char* myname;

    void run();

  protected:
    virtual ~thread();

    virtual void init() { } ///< will be called once after thread starts
    virtual void loop() { } ///< will be called periodically in main thread loop

    virtual void deinit() {
    } ///< will be called once after main thread loop ends

    [[nodiscard]] bool abort_requested() const { return abort_request; }

  public:
    /// create a thread
    thread(const char* name);

    /// abort thread (do not force, just request)
    virtual void request_abort();

    /// start thread execution
    /// thread will run in a loop, calling loop() each time. It will
    /// automatically check the abort flag anything that needs to be done before
    /// or after the loop can be placed in redefined constructors or
    /// destructors.
    void start();

    /// caller thread waits for completion of this thread,
    /// object storage is freed after thread completion.
    void join();

    /// destroy thread, try to abort and join it or delete the object if it
    /// hasn't started yet.
    void destruct();

    /// let this thread sleep
    ///@param ms - sleep time in milliseconds
    static void sleep(unsigned ms);

    /// request if thread runs
    bool is_running();

    /// an unique_ptr similar class for threads
    template<class T>
    class ptr
    {
        ptr(const ptr&) = delete;
        ptr& operator=(const ptr&) = delete;

        T* p;

      public:
        /// construct thread auto pointer with thread pointer
        ///@note will throw error when t is not a thread
        ptr(T* t = nullptr) : p(nullptr) { reset(t); }

        /// destruct thread auto pointer (will destruct thread)
        ~ptr() { reset(nullptr); }

        /// reset pointer (destructs current thread)
        ///@note will throw error when t is not a thread
        ///@note seems bizarre, use with care!
        void reset(T* t = nullptr)
        {
            // extra paranoia test to ensure we handly only thread objects here
            if (t && (dynamic_cast<thread*>(t) == nullptr))
                THROW(error, "invalid pointer given to thread::ptr!");
            if (p)
                p->destruct();
            p = t;
        }

        /// use pointer like normal pointer
        T* operator->() const { return p; }

        /// get pointer
        [[nodiscard]] const T* get() const { return p; }
    };
};

/// Thread object that runs one function.
class thread_function : public ::thread
{
  public:
    thread_function(std::function<void()> func);
    void loop() override;

  protected:
    std::function<void()> myfunction;
};
