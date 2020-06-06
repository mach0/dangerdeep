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
	DONE - remaining topics are GL related
5. use key enum class!
	DONE - except in widget class
-------------WE ARE HERE-----------------	
6. make use of new C++ features for widgets etc.
	no more templates with object and method etc., just use lambda!
6. Introduce newer system input handling (needs SDL2?)
	rename system.* to system_interface.* to compare includes (no changes between codemodernization and master!)
	This means no more SDL includes in headers!!!
7. use new C++ features for XML reader etc.
8. add new sensors (test if they work!!!)
9. update internal game classes (storage of sea_object, reference to them via ID!)
	later usage of rigid_body etc.
10. Copy new classes like gpu interface to master branch so they can be
	tested and used by standalone apps (copy DONE)
11. Divide code into separate libraries better (partly done)
12. Migrate to SDL2
13. Finally adjust rendering
	maybe we can adjust all the display classes to use the new kind of
	interface references but uses old rendering code...
	problem is a name conflict! model/mesh/texture are already used
	THIS IS THE TOUGHEST CHANGE, A CHANGE ALL OR NOTHING WILL WORK
	PROJECT!
14. or earlier: get rid of all the configuration options, we use SSE and all modern stuff automatically!
15. Turn on -Wall


Changes that have been started in code comparison:
- Whole new GPU interface and rendering
- References to game objects via ID instead of pointers
- generic_rudder, should be used by airplane as well
- Change to SDL2, no more SDL includes, new input event handler classes
- New idea about coastmap/coastline rendering with ETopo data
- use std::vector everywhere instead of std::list
- unify all sensors, GHG etc. are also sensors and clean up that class mess
- do not track pointers but sensor contacts
- use lambdas for widgets or std::func so no mor templates or pointers to objects
- introduce damageable_part
- More documentation (Doxygen)
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
