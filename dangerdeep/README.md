**This file is also way out of date (2003/07/03)**

## Danger from the Deep - WW2 german submarine simulation

Authors:
See "CREDITS".

Web:
http://dangerdeep.sourceforge.net

License:
See "LICENSE".

Install:
See "INSTALL".

### Technical Notes

If you are using a setup with more than one monitor you may experience
problems.

SDL 1.2.11 seems to have problems when running the game on
multimonitor/multihead displays. If you're running the game on a
computer-setup which utilizes more than one screen, you may encounter
problems, for example your X-Session dropping to the Console. In that
case pressing ALT+F7 or ALT+CTRL+F7 might help.

This is no problem with the game, but a problem of the SDL-Framwork
we're using. The SDL-people claim that this issue should be sorted out
in version 2.0 of SDL.

On another note, your graphic-card must at least support OpenGL 1.5 and
even better, OpenGL 2.0 for decent graphics.
Your graphic-card MUST support HARDWARE ACCELERATION.

The game won't run on machines with a Cirrus Logic 4MB card.

We like pie.

### Gameplay

The game recommends a resolution of 1024x768 strongly.
It can be played with different resoultions, but will
look ugly. Just type dangerdeep --help and check for
resolution switching. Using a higher resolution is not as
bad as using a smaller, though.
General rule: what does not work that is just not
implemented yet ;-)

### Keys:

#### displays:

F1     Gauges
F2    Periscope
F3    UZO
F4    Bridge
F5    Map
F6    Torpedo loading control
F7    Damage control
F8    Log book
F9    Success records
F10    Free view (for now)

#### control:

Cursor keys    Rudder left/right/up/down
Cursor k.+shift    Turn faster
Return        Center rudders
1,2,3,4,5,6    Throttle slow,half,full,flank,stop,reverse
SHIFT 1...6    Fire torpedo in tube 1...6 (bow/stern tube number position
        relation depends on sub type: as first all bow tubes are
        enumerated, then the stern tubes)
Space        Select target
0 (zero)    Scope up/down
c        Crash dive (to 150 meters)
d        Snorkel depth
f        Snorkel up/down
h        Set heading to view
i        identify target
p        Periscope depth
s        Surface
t        Fire torpedo (automatic selection of bow/stern tubes)
g + SHIFT    Man/unman deck gun
g        Fire deck gun
v        Set view to heading
z        Zoom view (glasses, bridge and periscope view)

/+-     Zoom/unzoom map

, .        turn view
; :        turn view fast
F11/F12        time scale faster/slower
Pause        (Un)pause game
ESC        quit (for now)
PRINT        Take screenshot (uncompressed ppm, for now)

#### special for now:

Numpad 8,4,6,2,1,3    step forward/left/right/backward/up/down in freeview mode

### Closing words

This game is made as tactical simulation that tries to be
as realistic (historical and physical) as possible.
It is NOT meant to glorify war in any form.
Many thousands of people died in the Atlantic during 1939-1945
on both sides.

