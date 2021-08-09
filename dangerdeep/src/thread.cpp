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

#include "thread.h"

#include "error.h"
#include "log.h"
#include "system_interface.h"

#include <utility>

/// Constructor
thread::thread(const char* name) :
    abort_request(false), mystate(state::none), myname(name)
{
}

/// main thread run method, catches all exceptions
void thread::run()
{
    try
    {
        log::instance().new_thread(myname);
        init();
    }
    catch (std::exception& e)
    {
        // failed to initialize, report that
        std::unique_lock<std::mutex> ml(state_mutex);
        error_message = e.what();
        mystate       = state::init_failed;
        start_cond.notify_all();
        return;
    }
    catch (...)
    {
        // failed to initialize, report that
        std::unique_lock<std::mutex> ml(state_mutex);
        error_message = "UNKNOWN";
        mystate       = state::init_failed;
        start_cond.notify_all();
        return;
    }
    // initialization was successfully, report that
    {
        std::unique_lock<std::mutex> ml(state_mutex);
        mystate = state::running;
        start_cond.notify_all();
    }
    try
    {
        while (!abort_requested())
        {
            loop();
        }
        deinit();
        log::instance().end_thread();
    }
    catch (std::exception& e)
    {
        // thread execution failed
        std::unique_lock<std::mutex> ml(state_mutex);
        error_message = e.what();
        mystate       = state::aborted;
        return;
    }
    catch (...)
    {
        // thread execution failed
        std::unique_lock<std::mutex> ml(state_mutex);
        error_message = "UNKNOWN";
        mystate       = state::aborted;
        return;
    }
    // normal execution finished
    std::unique_lock<std::mutex> ml(state_mutex);
    mystate = state::finished;
}

/// Destructor, needed because of virtual
thread::~thread() = default;

/// Request abort of thread
void thread::request_abort()
{
    abort_request = true;
}

/// Start execution of thread
void thread::start()
{
    if (abort_request)
        THROW(error, "thread abort requested, but start() called");
    std::unique_lock<std::mutex> ml(state_mutex);
    if (mystate != state::none)
        THROW(error, "thread already started, but start() called again");
    thread_id = std::thread(&thread::run, this);
    // we could wait with timeout, but how long? init could take any time...
    start_cond.wait(ml);
    // now check if thread has started
    if (mystate == state::init_failed)
        THROW(error, ("thread start failed: ") + error_message);
    // very rare, but possible
    else if (mystate == state::aborted)
        THROW(error, ("thread run failed: ") + error_message);
}

/// Wait for thread to finish
void thread::join()
{
    thread_id.join();
    auto mystate_copy       = std::move(mystate);
    auto error_message_copy = std::move(error_message);
    delete this;
    if (mystate_copy != thread::state::finished)
        THROW(
            error,
            std::string("thread aborted with error: ") + error_message_copy);
}

/// destroy thread, try to abort and join it or delete the object if it hasn't
/// started yet.
void thread::destruct()
{
    state ts = state::none;
    {
        std::unique_lock<std::mutex> ml(state_mutex);
        ts = mystate;
    }
    // request if thread runs, in that case send abort request
    if (ts == state::running)
        request_abort();
    // request if thread has ever run, in that case we need to join
    if (mystate != state::none)
        join();
    else
        delete this;
}

/// let caller sleep
void thread::sleep(unsigned ms)
{
    std::this_thread::sleep_for(std::chrono::microseconds(ms));
}

/// request if thread runs
bool thread::is_running()
{
    // only reading is normally safe, but not for multi-core architectures.
    std::unique_lock<std::mutex> ml(state_mutex);
    return mystate == state::running;
}

thread_function::thread_function(std::function<void()> func) :
    thread("function-caller"), myfunction(std::move(func))
{
}

void thread_function::loop()
{
    myfunction();
    request_abort();
}
