Steps to adjust existing, working code base to code modernization state.
===================================================================================================
After each step check functionality.

1. Migrate SCons -> CMake, build all executables
	DONE, later split to finer libraries
2. Use clang to modernize code automatically
	DONE for modernizer checks (more error checks may be sensible)
3. Use C++11 thread/mutex/condition_variable
	DONE
4. Compare basic classes between branches for differences!
	use meld with directories to compare.
	See below for more details.
	DONE - remaining topics are GL related, except bivector
5. use new C++ features for XML reader etc.
	DONE
6. use key enum class!
	DONE - except in widget class
7. make use of new C++ features for widgets etc. (no plain new/delete)
	widgets now use unique_ptr and lambda/c++ caller instead of the old caller_arg templates.
	DONE in widgets, maybe there are more ways to modernize the widget classes
8. Removed SDL types and obsolete SDL.h includes
-------------WE ARE HERE-----------------	
9. Introduce newer system input event handling
	This means no more SDL includes in headers!
	Rather introduce own input_event classes from codemodernization
	Needs input_event_handler heirs and handling in system class.
	This modern way is not even included in the codemodernization branch.
	check_for_mouse_event must be translated as well.
------------ MOST GLOBAL CODE IMPROVEMENTS UP TO HERE, HERE COME GAMEPLAY/INTERNAL STRUCTURE IMPROVEMENTS ------------------------	
9. add new sensors (test if they work!!!)
10. update internal game classes (storage of sea_object, reference to them via ID!)
	later usage of rigid_body etc.
11. Copy new classes like gpu interface to master branch so they can be
	tested and used by standalone apps (copy DONE)
12. Divide code into separate libraries better (partly done)
------------ MODERN ADVANCED RENDERING AND I/O HERE --------------------------------------------------
13. Migrate to SDL2
14. Finally adjust rendering
	maybe we can adjust all the display classes to use the new kind of
	interface references but uses old rendering code...
	problem is a name conflict! model/mesh/texture are already used
	THIS IS THE TOUGHEST CHANGE, A CHANGE ALL OR NOTHING WILL WORK
	PROJECT!
15. or earlier: get rid of all the configuration options, we use SSE and all modern stuff automatically!
16. Turn on -Wall


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
- replace myclamp with std::clamp



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
There are/were 37 ::process_input methods in the code that all need to be split.
For TDC display with 2 screens we even can switch handlers when screen changes.
This all can be done with SDL1 also.
Translate_position doesn't need to be called any longer then, transformed positions
are already provided.
We could even provide the handler methods for all events in parallel to existing code!
user_display and user_interface etc. need to inherit from input_event_handler then.
But why not just use two different classes for the TDC display?
Translate-lambdas must not use hardcoded 1024,768 values but constants!
This means we have to look into the .cpp files what events are processed and need to
overload the matching handler methods, then duplicate the code from process_input
to the handler methods and adapt it.
Input handlers are: user_interface, all *display classes, user_popup, submarine_interface
and widget class.
This is a huge pile of work but rather a no-brainer.
And we can commit only changes to all of them, but we can test all changes!


	
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

