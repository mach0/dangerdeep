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

// music
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

/*
   class music doesn't deal with SDL audio initialization since It assumes it
   was already inited by sound class fixme: later handle this in sound system
   class
*/

#pragma once

#include "angle.h"
#include "message_queue.h"
#include "random_generator.h"
#include "ressource_ptr.h"
#include "singleton.h"
#include "thread.h"
#include "vector3.h"

#include <SDL_mixer.h> // for Mix_Music/Mix_Chunk - forward declaration doesn't work
#include <map>
#include <string>
#include <utility>
#include <vector>

// definitions for sound file categories
#define SFX_MACHINE_SUB_DIESEL   "sub-diesel"
#define SFX_MACHINE_ESCORT       "escort"
#define SFX_BIG_GUN_FIRE         "big-gun-fire"
#define SFX_MEDIUM_GUN_FIRE      "medium-gun-fire"
#define SFX_DECK_GUN_FIRE        "deck-gun-fire"
#define SFX_TUBE_LAUNCH          "tube-launch"
#define SFX_PING                 "ping"
#define SFX_SHIP_BELL            "ship-bell"
#define SFX_SHELL_EXPLODE        "shell-explode"
#define SFX_SHELL_SPLASH         "shell-splash"
#define SFX_DEPTH_CHARGE_LAUNCH  "depth-charge-launch"
#define SFX_DEPTH_CHARGE_EXPLODE "depth-charge-explode"

///\brief Handles music and background songs.
class music : public singleton<class music>, public ::thread
{
  public:
    /// which mode to use when playing tracks from a play list
    enum class playback_mode
    {
        loop_list,
        loop_track,
        shuffle_track
    };

    /// create music handler
    music(bool use_music = true, unsigned sample_rate = 44100);

    // ----------- command interface --------------------

    /// append entry to play list
    ///@param filename - filename of track
    ///@returns true if command was successful
    bool append_track(const std::string& filename);

    /// set playback mode
    ///@param pbm - either loop list, loop track or shuffle tracks
    ///@returns true if command was successful
    bool set_playback_mode(playback_mode pbm);

    /// start playing music
    ///@param fadein - fadein time
    ///@returns true if command was successful
    bool play(unsigned fadein = 0);

    /// stop playing music
    ///@param fadeout - fadeout time
    ///@returns true if command was successful
    bool stop(unsigned fadeout = 0);

    /// pause music play
    ///@returns true if command was successful
    bool pause();

    /// resume music play
    ///@returns true if command was successful
    bool resume();

    /// set music position
    bool set_music_position(float pos);

    /// switch playback to track
    ///@param nr - number of track in playlist
    ///@param fadeouttime - time to fadeout current playback
    ///@param fadeintime - time to fadein new playback
    ///@returns true if command was successful
    bool
    play_track(unsigned nr, unsigned fadeouttime = 0, unsigned fadeintime = 0);

    /// get a copy of current playlist
    ///@returns copy of playlist, empty list on error
    std::vector<std::string> get_playlist();

    /// get number of currently played track
    ///@returns track or 0 on error
    unsigned get_current_track();

    /// request if music plays
    ///@returns state or false on error
    bool is_playing();

    /// play event sfx
    ///@param category - name of category (what event)
    ///@param listener - position of listener
    ///@param listener_dir - angle that listener is facing (around z-axis)
    ///@param noise_pos - position of noise source
    ///@returns true if command was successful
    bool play_sfx(
        const std::string& category,
        const vector3& listener,
        angle listener_dir,
        const vector3& noise_pos);

    /// play machine (environmental) sfx
    ///@param name - name of machine
    ///@param throttle - throttle level, can be 0...100, 0 stops play
    ///@returns true if command was successful
    bool play_sfx_machine(const std::string& name, unsigned throttle);

    /// Pause/Resume all sound effects
    ///@param on - true to pause, false to resume
    ///@returns true if command was successful
    bool pause_sfx(bool on);

    // ---------------------------------------------

    /// set to false if you don't want music.  fixme - rather ugly approach!
    static bool use_music;

    /// Set sound directory
    void set_sound_dir(const std::string& sd) { sound_dir = sd; }

  protected:
    typedef ressource_ptr<Mix_Music> mix_music_ptr;
    typedef ressource_ptr<Mix_Chunk> mix_chunk_ptr;
    const unsigned nr_reserved_channels{1};
    const unsigned sample_rate;
    unsigned current_track{0};
    int usersel_next_track{-1};
    unsigned usersel_fadein{0};
    playback_mode pbm{playback_mode::loop_list};
    bool stopped{true};
    std::vector<std::string> playlist;
    std::vector<mix_music_ptr> musiclist;
    message_queue command_queue;
    std::map<std::string, std::vector<mix_chunk_ptr>> sfx_events;
    std::map<std::string, std::vector<mix_chunk_ptr>> sfx_machines;
    Mix_Chunk* current_machine_sfx{nullptr};
    std::string sound_dir; ///< Set from outside
    random_generator rndgen;
    void destructor();

    void start_play_track(unsigned nr, unsigned fadeintime = 0);
    static void callback_track_finished();

    void init() override;
    void loop() override;
    void deinit() override;

    void request_abort() override;

    // internal command(s)
    bool track_finished();

    void exec_append_track(const std::string& filename);
    void exec_set_playback_mode(playback_mode pbm);
    void exec_play(unsigned fadein);
    void exec_stop(unsigned fadeout);
    void exec_pause();
    void exec_resume();
    void exec_set_music_position(float pos);
    void
    exec_play_track(unsigned nr, unsigned fadeouttime, unsigned fadeintime);
    void exec_track_finished();
    void exec_get_playlist(std::vector<std::string>& playlist);
    void exec_get_current_track(unsigned& track);
    void exec_is_playing(bool& isply);
    void exec_play_sfx(
        const std::string& category,
        const vector3& listener,
        angle listener_dir,
        const vector3& noise_pos);
    void exec_play_sfx_machine(const std::string& name, unsigned throttle);
    void exec_pause_sfx(bool on);

    class command_append_track : public message
    {
        music& my_music;
        std::string filename;
        void eval() const override { my_music.exec_append_track(filename); }

      public:
        command_append_track(music& my_music_, std::string filename_) :
            my_music(my_music_), filename(std::move(filename_))
        {
        }
    };

    class command_set_playback_mode : public message
    {
        music& my_music;
        playback_mode pbm;
        void eval() const override { my_music.exec_set_playback_mode(pbm); }

      public:
        command_set_playback_mode(music& my_music_, playback_mode pbm_) :
            my_music(my_music_), pbm(pbm_)
        {
        }
    };

    class command_play : public message
    {
        music& my_music;
        unsigned fadein;
        void eval() const override { my_music.exec_play(fadein); }

      public:
        command_play(music& my_music_, unsigned fadein_) :
            my_music(my_music_), fadein(fadein_)
        {
        }
    };

    class command_stop : public message
    {
        music& my_music;
        unsigned fadeout;
        void eval() const override { my_music.exec_stop(fadeout); }

      public:
        command_stop(music& my_music_, unsigned fadeout_) :
            my_music(my_music_), fadeout(fadeout_)
        {
        }
    };

    class command_pause : public message
    {
        music& my_music;
        void eval() const override { my_music.exec_pause(); }

      public:
        command_pause(music& my_music_) : my_music(my_music_) { }
    };

    class command_resume : public message
    {
        music& my_music;
        void eval() const override { my_music.exec_resume(); }

      public:
        command_resume(music& my_music_) : my_music(my_music_) { }
    };

    class command_set_music_position : public message
    {
        music& my_music;
        float pos;
        void eval() const override { my_music.exec_set_music_position(pos); }

      public:
        command_set_music_position(music& my_music_, float pos_) :
            my_music(my_music_), pos(pos_)
        {
        }
    };

    class command_play_track : public message
    {
        music& my_music;
        unsigned nr;
        unsigned fadeouttime;
        unsigned fadeintime;
        void eval() const override
        {
            my_music.exec_play_track(nr, fadeouttime, fadeintime);
        }

      public:
        command_play_track(
            music& my_music_,
            unsigned nr_,
            unsigned fadeouttime_,
            unsigned fadeintime_) :
            my_music(my_music_),
            nr(nr_), fadeouttime(fadeouttime_), fadeintime(fadeintime_)
        {
        }
    };

    class command_track_finished : public message
    {
        music& my_music;
        void eval() const override { my_music.exec_track_finished(); }

      public:
        command_track_finished(music& my_music_) : my_music(my_music_) { }
    };

    class command_get_playlist : public message
    {
        music& my_music;
        std::vector<std::string>& playlist;
        void eval() const override { my_music.exec_get_playlist(playlist); }

      public:
        command_get_playlist(
            music& my_music_,
            std::vector<std::string>& playlist_) :
            my_music(my_music_),
            playlist(playlist_)
        {
        }
    };

    class command_get_current_track : public message
    {
        music& my_music;
        unsigned& track;
        void eval() const override { my_music.exec_get_current_track(track); }

      public:
        command_get_current_track(music& my_music_, unsigned& track_) :
            my_music(my_music_), track(track_)
        {
        }
    };

    class command_is_playing : public message
    {
        music& my_music;
        bool& isply;
        void eval() const override { my_music.exec_is_playing(isply); }

      public:
        command_is_playing(music& my_music_, bool& isply_) :
            my_music(my_music_), isply(isply_)
        {
        }
    };

    class command_play_sfx : public message
    {
        music& my_music;
        std::string category;
        vector3 listener;
        angle listener_dir;
        vector3 noise_pos;
        void eval() const override
        {
            my_music.exec_play_sfx(category, listener, listener_dir, noise_pos);
        }

      public:
        command_play_sfx(
            music& my_music_,
            std::string category_,
            vector3 listener_,
            angle listener_dir_,
            vector3 noise_pos_) :
            my_music(my_music_),
            category(std::move(category_)), listener(std::move(listener_)),
            listener_dir(std::move(listener_dir_)),
            noise_pos(std::move(noise_pos_))
        {
        }
    };

    class command_play_sfx_machine : public message
    {
        music& my_music;
        std::string name;
        unsigned throttle;
        void eval() const override
        {
            my_music.exec_play_sfx_machine(name, throttle);
        }

      public:
        command_play_sfx_machine(
            music& my_music_,
            std::string name_,
            unsigned throttle_) :
            my_music(my_music_),
            name(std::move(name_)), throttle(throttle_)
        {
        }
    };

    class command_pause_sfx : public message
    {
        music& my_music;
        bool on;
        void eval() const override { my_music.exec_pause_sfx(on); }

      public:
        command_pause_sfx(music& my_music_, bool on_) :
            my_music(my_music_), on(on_)
        {
        }
    };
};

template<>
inline void free_ressource(Mix_Chunk* p)
{
    Mix_FreeChunk(p);
}
template<>
inline void free_ressource(Mix_Music* p)
{
    Mix_FreeMusic(p);
}

/* data for command generator, save to file --- snip ---
music
append_track std::string filename
set_playback_mode playback_mode pbm
play unsigned fadein=0
stop unsigned fadeout=0
pause
resume
play_track unsigned nr unsigned fadeouttime=0 unsigned fadeintime=0
track_finished
get_playlist std::vector<std::string>& playlist
get_current_track unsigned& track
is_playing bool& isply
play_sfx std::string category vector3 listener angle listener_dir vector3
noise_pos play_sfx_machine std::string name unsigned throttle pause_sfx bool on
--- snap --- */
