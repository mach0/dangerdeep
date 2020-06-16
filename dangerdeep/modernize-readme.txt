Steps to adjust existing, working code base to code modernization state.
===================================================================================================
After each step check functionality.

----------- STEPS DONE ------------------------
1. Migrate SCons -> CMake, build all executables
2. Use clang to modernize code automatically
3. Use C++11 thread/mutex/condition_variable
4. Compare basic classes between branches for differences!
5. use new C++ features for XML reader etc.
6. use key enum class!
7. make use of new C++ features for widgets etc. (no plain new/delete)
	widgets now use unique_ptr and lambda/c++ caller instead of the old caller_arg templates.
	maybe there are more ways to modernize the widget classes
8. Removed SDL types and obsolete SDL.h includes
9. Introduce newer system input event handling and migrate to SDL2
10. Store sea_objects directly with move semantics, no more pointers, use
    unique_ptr everywhere possible, const sea_object pointers
11. Introduced new and faster BV-Tree
12. Split tdc display in two classes (two displays) - need to extend display
  list by one display and special handling to switch displays, for easier
  transition to new elem2d class
13. Introduce elem2D class for popups (cleaner shorter code, easier
    transition to new GPU interface)
14. Split to more libraries
-------------WE ARE HERE-----------------
. Introduce elem2d helper for displays and convert code to them (makes
  transition to new gpu code easier)
  Much boring work but can be tested in master easily.
  We can add elem2d class and elements and convert display for display
  with checking between each of them!
. Fix viewmodel app (no model can be seen, no background)
------------ MOST GLOBAL CODE IMPROVEMENTS UP TO HERE, HERE COME GAMEPLAY/INTERNAL STRUCTURE IMPROVEMENTS ------------------------
. add new sensors (test if they work!!!) needs test program.
. Divide code into separate libraries better (partly done)
------------ MODERN ADVANCED RENDERING AND I/O HERE --------------------------------------------------
. Finally adjust rendering
	maybe we can adjust all the display classes to use the new kind of
	interface references but uses old rendering code...
	problem is a name conflict! model/mesh/texture are already used
	THIS IS THE TOUGHEST CHANGE, A CHANGE ALL OR NOTHING WILL WORK
	PROJECT!
. or earlier: get rid of all the configuration options, we use SSE and all modern stuff automatically!
. Turn on -Wall
. Sound sfx are not loaded (wav expected, ogg provided)
. Fix portal rendering
. Use physical units!
. xml reader could use std::optional
. Popup elements could be data files as well


Changes that have been started in code comparison:
- use std::vector everywhere instead of std::list
- do not use new/delete but make_unique/unique_ptr
- unify all sensors, GHG etc. are also sensors and clean up that class mess
- do not track pointers but sensor contacts
- More documentation (Doxygen)
- Whole new GPU interface and rendering
- References to game objects via ID instead of pointers (torpedo_camera, convoy, ai)
- generic_rudder, should be used by airplane as well
- Change to SDL2, no more SDL includes, new input event handler classes
- New idea about coastmap/coastline rendering with ETopo data
- introduce damageable_part
- Less warnings due to type specifiers
- Loading/Saving more generic (not bound to xml)
- model with state, class mesh separated
- rework event class as std::variant
- remove macros
- remove game::job
- mymain transition list to vector
- break inheritance, torpodo is NO ship, maybe we don't need inheritance at all
- if we could make use of system interface etc. we could at least convert the standalone apps. However the name collision of
  model/mesh/image inhibit this.
- maybe we can rename the image class so we can have the old and new one. Or use a new name for the new one.
- maybe we can use SDL2 with old OpenGL as possible transition!
- translate bivector
- sub_captains_cabin, use std::function instead of old C/C++ functions, maybe lambda also help
- remove obsolete classes like morton_bivector etc. (what doesn't exist in codemodernization branch)
- using namespace std in cpp should be avoided?
- replace myclamp with std::clamp, myfmod etc.
- all display classes is_mouse_over should use vector2i instead of separeted x,y ints.
- display classes need no game& for display(), can get that from user_interface, same for freeview_display methods
- Provide new input_event data in system!
  widgets don't work correctly, some displays don't work right.
  we can't enter bridge/uzo even near the surface!
- make every declared variable const that is not changed!
- check clang tidy for readability-qualified-auto, readability-non-const-parameter
- But why not just use two different classes for the TDC display?
- BV Tree should use reserve for nodes, it would be better to give a mesh
  directly to bvtree and it reserves space for nodes (twice input) and then
  copies it!

don't store target with every sea_object, only the user interface or player
needs it!

later usage of rigid_body, generic_rudder etc.
A generic pointer to sea_object may be unnecessary, the objects can report their models for display.

BV-Tree collision checks: the number of iterations here is insanely high.
Maybe there is a faster algorithm or by converting recursion to iteration it
is faster?

rudder hard left seems not to work!


Notes
===================================================================================================

2.)
===
clang tidy modernizers
DONE
    modernize-use-override
    modernize-replace-auto-ptr
    modernize-avoid-bind
    modernize-concat-nested-namespaces
    modernize-deprecated-headers
    modernize-deprecated-ios-base-aliases
    modernize-replace-random-shuffle
    modernize-raw-string-literal
    modernize-use-transparent-functors
    modernize-use-uncaught-exceptions
    modernize-make-shared
    modernize-redundant-void-arg
    modernize-use-bool-literals
    modernize-shrink-to-fit
    modernize-unary-static-assert
    modernize-use-nodiscard
    modernize-use-noexcept
    modernize-avoid-c-arrays
    modernize-use-emplace
    modernize-use-equals-default
    modernize-use-equals-delete
    modernize-use-using
    modernize-make-unique
    modernize-use-nullptr
    modernize-use-default-member-init
    modernize-return-braced-init-list
    modernize-pass-by-value
    modernize-loop-convert
    llvm-include-order
    modernize-use-auto
DO NOT USE
    modernize-use-trailing-return-type
CHECK
    There is a long list of clang tidy checks that could also be applied!
TRIED BUT NO EFFECT
	performance-move-const-arg
	performance-unnecessary-copy-initialization
CALL
	./run-clang-tidy.py -header-filter='.*' -checks='-*,FILTERNAME' -fix



4.)
===
FILES TAKEN FROM CODEMODERNIZATION BRANCH / SYNCHRONIZED
	angle.h
	angular_table.h
	area.h
	binstream.h
	bitstream.cpp
	bitstream.h
	box.h
	bspline.h
	bspline_test.cpp
	bv_tree.cpp
	bv_tree.h
	bzip.cpp
	bzip.h
	circle.h
	constant.h
	countrycodes.cpp
	countrycodes.h
	date.cpp	use std::chrono??
	date.h
	daysky.cpp
	daysky.h
	dmath.h
	error.cpp
	faulthandler.h
	filehelper.cpp
	filehelper.h
	fpsmeasure.h
	game_editor.cpp
	game_editor.h
	generic_rudder.cpp
	generic_rudder.h
	gpu_helper.cpp
	gpu_helper.h
	gpu_interface.cpp
	gpu_interface.h
	gpu_mesh.cpp
	gpu_mesh.h
	gpu_model.cpp
	gpu_model.h
	height_generator_map.cpp
	height_generator_map.h
	helper.h
	highscorelist.cpp
	highscorelist.h
	log.cpp
	log.h
	logbook.h
	matrix.h
	matrix3.h
	mesh.cpp
	mesh.h
	message_queue.cpp
	message_queue.h
	model_state.cpp
	model_state.h
	object_store.h
	oceantest.cpp
	parser.cpp
	parser.h
	plane.h
	polygon.h
	polyhedron.h
	ptrvector.h
	quaternion.h
	rectangle.h
	ressource_ptr.h
	rigid_body.cpp
	rigid_body.h
	sphere.h
	system_interface.cpp
	system_interface.h
	tdc.cpp
	tdc.h
	texts.cpp
	texts.h
	thread.cpp
	thread.h
	tone_reproductor.cpp
	tone_reproductor.h
	triangle_intersection.h
	triangulate.cpp
	triangulate.h
	units.h
	vector2.h
	vector3.h
	vector4.h
	voxel.cpp
	voxel.h
	xml.cpp
	xml.h

OPEN TOPICS
	color.h		ambigious double ctor
	datadirs	data_file_handler missing
	error.h		sdl_error for backwards compatibility
	fpsmeasure.cpp	needs new system interface
	frustum.cpp	opengl code removed
	frustum.h	opengl code removed
	height_generator.h	opengl issues
	matrix4.h	opengl only in old code
	random_generator.cpp	old code kept additionally yet
	random_generator.h	old code kept additionally yet
	bivector.h



9.)
===

Input event handler concept:
Instead of putting all events in one structure (or even union as SDL does
it) register input_event_handler classes at the system interface. The input
event handler classes offer overloads for the various events. So when a
certain event is triggered (i.e. a key is pressed) the event is directly
fetched to the handlers by calling their handler methods with the data.
This means entering a user_display registers it as handler to the system
interface and leaving the display unregisters it.
This makes large switch/case code blocks obsolete that iterate over all
events and call code by event type.
Event handlers dont need to be registered when display change, as user interface fetches data.



10.)
====

Rendering: what needs to be changed
- model rendering	DONE (mostly)
- sky			DONE
- moon			DONE
- water		DONE but visual bugs
- geoclipmap terrain	DONE
- widgets
- 2d drawing		DONE
- displays

