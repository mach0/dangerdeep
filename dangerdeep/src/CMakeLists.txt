# Main dangerdeep

add_compile_definitions (TIXML_USE_STL)


# Also build library for tinyxml
add_subdirectory (tinyxml)


# and for oglExt
add_subdirectory (oglext)


# DFTD basic
add_library (dftdbasic STATIC
	angle.h
	angular_table.h
	area.h
	binstream.h
	bitstream.cpp
	bitstream.h
	bivector.h
	box.h
	bspline.h
	bv_tree.cpp
	bv_tree.h
	cfg.cpp
	cfg.h
	circle.h
	constant.h
	cylinder.h
	datadirs.cpp
	datadirs.h
	dmath.h
	error.cpp
	error.h
	faulthandler.h
	filehelper.cpp
	filehelper.h
	graph.h
	helper.h
	input_event_handler.h
	keys.cpp
	keys.h
	log.cpp
	log.h
	matrix.h
	matrix3.h
	matrix4.h
	#mesh.cpp
	#mesh.h
	message_queue.cpp
	message_queue.h
	#model_state.cpp
	#model_state.h
	objcache.h
	object_store.h
	parser.cpp
	parser.h
	plane.h
	polygon.h
	polyhedron.h
	quaternion.h
	random_generator.h
	rectangle.h
	ressource_ptr.h
	rigid_body.cpp
	rigid_body.h
	singleton.h
	sphere.h
	thread.cpp
	thread.h
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
)
target_link_libraries (dftdbasic ${LIBS} ${CMAKE_DL_LIBS} tinyxml)


# DFTD game core
add_library (dftdgamecore STATIC
	ai.cpp
	ai.h
	airplane.cpp
	airplane.h
	convoy.cpp
	convoy.h
	countrycodes.cpp
	countrycodes.h
	date.cpp
	date.h
	depth_charge.cpp
	depth_charge.h
	event.cpp
	event.h
	game.cpp
	game.h
	game_editor.cpp
	game_editor.h
	gun_shell.cpp
	gun_shell.h
	height_generator.h
	logbook.h
	ocean_wave_generator.h
	particle.cpp
	particle.h
	sea_object.cpp
	sea_object.h
	sea_object_id.h
	sensors.cpp
	sensors.h
	ship.cpp
	ship.h
	sonar.cpp
	sonar.h
	sonar_operator.cpp
	sonar_operator.h
	submarine.cpp
	submarine.h
	terrain.h
	tile.h
	tile_cache.h
	texts.cpp
	texts.h
	tdc.cpp
	tdc.h
	torpedo.cpp
	torpedo.h
	water_splash.cpp
	water_splash.h
)
target_link_libraries ( dftdgamecore dftdbasic )


# DFTD media, ffmpeg
add_library (dftdmedia STATIC
	bzip.cpp		# only for terrain, remove later
	bzip.h			# only for terrain, remove later
	caustics.cpp
	caustics.h
	color.h
	daysky.cpp
	daysky.h
	fixed.h
	font.cpp
	font.h
	fpsmeasure.cpp
	fpsmeasure.h
	fractal.h
	framebufferobject.cpp
	framebufferobject.h
	frustum.cpp
	frustum.h
	geoclipmap.cpp
	geoclipmap.h
	gldebug.h
	image.cpp
	image.h
	make_mesh.cpp
	make_mesh.h
	model.cpp
	model.h
	moon.cpp
	moon.h
	morton_bivector.h
	music.cpp
	music.h
	perlinnoise.cpp
	perlinnoise.h
	postprocessor.cpp
	postprocessor.h
	primitives.cpp
	primitives.h
	shader.cpp
	shader.h
	sky.cpp
	sky.h
	stars.cpp
	stars.h
	simplex_noise.cpp
	simplex_noise.h
	system_interface.cpp
	system_interface.h
	texture.cpp
	texture.h
	tone_reproductor.cpp
	tone_reproductor.h
	vertexbufferobject.cpp
	vertexbufferobject.h
	water.cpp
	water.h
)
target_link_libraries (dftdmedia dftdbasic oglext)


# DFTD all
# TODO: distribute correctly,cfg/keys need global_data and that needs lot more
add_library (dftdall STATIC
	global_data.cpp
	global_data.h
	highscorelist.cpp
	highscorelist.h
	tree_generator.h
	dftdtester/tests.cpp
)
target_link_libraries ( dftdall dftdmedia )


# DFTD game UI
add_library (dftdgameui STATIC
	coastmap.cpp
	coastmap.h
	freeview_display.cpp
	freeview_display.h
	logbook_display.cpp
	logbook_display.h
	map_display.cpp
	map_display.h
	ships_sunk_display.cpp
	ships_sunk_display.h
	sub_bg_display.cpp
	sub_bg_display.h
	sub_bridge_display.cpp
	sub_bridge_display.h
	sub_captainscabin_display.cpp
	sub_captainscabin_display.h
	sub_control_popup.cpp
	sub_control_popup.h
	sub_damage_display.cpp
	sub_damage_display.h
	sub_ecard_popup.cpp
	sub_ecard_popup.h
	sub_gauges_display.cpp
	sub_gauges_display.h
	sub_ghg_display.cpp
	sub_ghg_display.h
	sub_kdb_display.cpp
	sub_kdb_display.h
	sub_periscope_display.cpp
	sub_periscope_display.h
	sub_recogmanual_display.cpp
	sub_recogmanual_display.h
	sub_recogmanual_popup.cpp
	sub_recogmanual_popup.h
	sub_soldbuch_display.cpp
	sub_soldbuch_display.h
	sub_tdc_display.cpp
	sub_tdc_display.h
	sub_tdc2_display.cpp
	sub_tdc2_display.h
	sub_tdc_popup.cpp
	sub_tdc_popup.h
	sub_torpedo_display.cpp
	sub_torpedo_display.h
	sub_torpsetup_display.cpp
	sub_torpsetup_display.h
	sub_uzo_display.cpp
	sub_uzo_display.h
	sub_valves_display.cpp
	sub_valves_display.h
	submarine_interface.cpp
	submarine_interface.h
	torpedo_camera_display.cpp
	torpedo_camera_display.h
	user_display.cpp
	user_display.h
	user_interface.cpp
	user_interface.h
	user_popup.cpp
	user_popup.h
	widget.cpp	# fixme needs text, is not really part of the game ui
	widget.h
)
target_link_libraries (dftdgameui dftdmedia dftdgamecore dftdall)
# fixme need to link dftdall for add_loading_screen in global_data etc.


set (dangerdeep_source
	subsim.cpp
	credits.cpp
	credits.h
)
add_executable (dangerdeep ${dangerdeep_source})

target_link_libraries (dangerdeep ${LIBS} ${CMAKE_DL_LIBS} dftdgameui)


# Extra tools, 3d model, XML files, font conversion...
if (DFTD_BUILD_TOOLS)

	add_executable (viewmodel      viewmodel.cpp)
	target_link_libraries (viewmodel dftdgameui)

    # load exported model, dump hierarchy as XML
	add_executable (modelmeasure   modelmeasure.cpp)
	target_link_libraries (modelmeasure dftdmedia)

	add_executable (oceantest      oceantest.cpp)
	target_link_libraries (oceantest dftdmedia)

	add_executable (bsplinetest    bspline_test.cpp)
	target_link_libraries (bsplinetest dftdbasic)

	add_executable (portal         portal.cpp)
	target_link_libraries (portal dftdmedia)

	add_executable (treegentest    treegentest.cpp)
	target_link_libraries (treegentest dftdall)

	add_executable (bvtreetest     bvtreeintersecttest.cpp)
	target_link_libraries (bvtreetest dftdmedia)

	add_executable (map_precompute tools/map_precompute.cpp)
	target_link_libraries (map_precompute dftdall)

	add_executable (test_display test_display.cpp)
	target_link_libraries (test_display dftdgameui)

	if (FFMPEG_FOUND AND DFTD_BUILD_VIDEO_TEST)
	    add_executable (videoplay  videoplaytest.cpp)
	    target_link_libraries (videoplay dftdmedia)
	endif ()
endif ()

