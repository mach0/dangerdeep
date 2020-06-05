# Main dangerdeep 

# Definitions for TinyXML and FFTW.
add_definitions ( -DTIXML_USE_STL -DWITH_FLOAT_FFTW )

# Also build library for tinyxml
add_subdirectory ( tinyxml )

# and for oglExt
add_subdirectory ( oglext )

set ( dftdall_src
	ai.cpp
	ai.h
	airplane.cpp
	airplane.h
	#airplane_interface.cpp
	#airplane_interface.h
	align16_allocator.h
	angle.h
	binstream.h
	bitstream.cpp
	bitstream.h
	bivector.h
	bspline.h
	bv_tree.cpp
	bv_tree.h
	bzip.cpp
	bzip.h
	caustics.cpp
	caustics.h
	cfg.cpp
	cfg.h
	coastmap.cpp
	coastmap.h
	color.h
	condvar.cpp
	condvar.h
	convoy.cpp
	convoy.h
	countrycodes.cpp
	countrycodes.h
	credits.cpp
	credits.h
	datadirs.cpp
	datadirs.h
	date.cpp
	date.h
	daysky.cpp
	daysky.h
	depth_charge.cpp
	depth_charge.h
	dmath.h
	error.cpp
	error.h
	event.cpp
	event.h
	faulthandler.h
	filehelper.cpp
	filehelper.h
	fixed.h
	font.cpp
	font.h
	fpsmeasure.cpp
	fpsmeasure.h
	fractal.h
	framebufferobject.cpp
	framebufferobject.h
	freeview_display.cpp
	freeview_display.h
	frustum.cpp
	frustum.h
	game.cpp
	game_editor.cpp
	game_editor.h
	game.h
	geoclipmap.cpp
	geoclipmap.h
	gldebug.h
	global_constants.h
	global_data.cpp
	global_data.h
	gun_shell.cpp
	gun_shell.h
	height_generator.h
	height_generator_map.cpp
	height_generator_map.h
	highscorelist.cpp
	highscorelist.h
	image.cpp
	image.h
	keys.cpp
	keys.h
	logbook.cpp
	logbook_display.cpp
	logbook_display.h
	logbook.h
	log.cpp
	log.h
	make_mesh.cpp
	make_mesh.h
	map_display.cpp
	map_display.h
	matrix3.h
	matrix4.h
	matrix.h
	message_queue.cpp
	message_queue.h
	model.cpp
	model.h
	#modelmeasure.cpp
	moon.cpp
	moon.h
	morton_bivector.h
	music.cpp
	music.h
	mutex.cpp
	mutex.h
	#mymain.cpp
	#network.cpp
	#network.h
	objcache.h
	ocean_wave_generator.h
	parser.cpp
	parser.h
	particle.cpp
	particle.h
	perlinnoise.cpp
	perlinnoise.h
	plane.h
	polygon.h
	#portal.cpp
	postprocessor.cpp
	postprocessor.h
	primitives.cpp
	primitives.h
	ptrlist.h
	ptrvector.h
	quaternion.h
	random_generator.h
	sea_object.cpp
	sea_object.h
	sensors.cpp
	sensors.h
	shader.cpp
	shader.h
	ship.cpp
	ship.h
	#ship_interface.cpp
	#ship_interface.h
	ships_sunk_display.cpp
	ships_sunk_display.h
	simplex_noise.cpp
	simplex_noise.h
	singleton.h
	sky.cpp
	sky.h
	sonar.cpp
	sonar.h
	sonar_operator.cpp
	sonar_operator.h
	sphere.h
	stars.cpp
	stars.h
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
	submarine.cpp
	submarine.h
	submarine_interface.cpp
	submarine_interface.h
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
	system.cpp
	system.h
	tdc.cpp
	tdc.h
	terrain.h
	texts.cpp
	texts.h
	texture.cpp
	texture.h
	thread.cpp
	thread.h
	tile_cache.h
	tile.h
	tone_reproductor.cpp
	tone_reproductor.h
	torpedo_camera_display.cpp
	torpedo_camera_display.h
	torpedo.cpp
	torpedo.h
	tree_generator.h
	triangle_intersection.h
	triangulate.cpp
	triangulate.h
	user_display.h
	user_interface.cpp
	user_interface.h
	user_popup.h
	vector2.h
	vector3.h
	vector4.h
	vertexbufferobject.cpp
	vertexbufferobject.h
	#viewmodel.cpp
	water.cpp
	water.h
	water_splash.cpp
	water_splash.h
	widget.cpp
	widget.h
	xml.cpp
	xml.h	
	dftdtester/tests.cpp
)

add_library ( dftdall SHARED ${dftdall_src} )
target_link_libraries ( dftdall ${LIBS} ${CMAKE_DL_LIBS} )

add_executable ( dangerdeep subsim.cpp ) 

target_link_libraries ( dangerdeep ${LIBS} ${CMAKE_DL_LIBS} dftdall tinyxml oglext )

if ( DFTD_BUILD_TOOLS )
	set ( viewmodel_src viewmodel.cpp )
	add_executable ( viewmodel      ${viewmodel_src} )
	target_link_libraries ( viewmodel dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )

	set ( modelmeasure_src modelmeasure.cpp )
	add_executable ( modelmeasure   ${modelmeasure_src} )
	target_link_libraries ( modelmeasure dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )

	set ( oceantest_src oceantest.cpp )
	add_executable ( oceantest      ${oceantest_src} )
	target_link_libraries ( oceantest dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )

	set ( bspline_src bspline_test.cpp )
	add_executable ( bsplinetest    ${bspline_src} )
	target_link_libraries ( bsplinetest dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )
	
	set ( portal_src portal.cpp )
	add_executable ( portal         ${portal_src} )
	target_link_libraries ( portal dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )  

	set ( treegen_src treegentest.cpp )
	add_executable ( treegentest    ${treegen_src} )
	target_link_libraries ( treegentest dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )   

	set ( bvtreetest_src bvtreeintersecttest.cpp )
	add_executable ( bvtreetest     ${bvtreetest_src} )
	target_link_libraries ( bvtreetest dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )  

	set ( mapprecompute_src         tools/map_precompute.cpp )
	add_executable ( map_precompute ${mapprecompute_src} )
	target_link_libraries ( map_precompute dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )

	if ( FFMPEG_FOUND AND DFTD_BUILD_VIDEO_TEST )
	    set ( videoplay_src videoplaytest.cpp )
	    add_executable ( videoplay  ${videoplay_src} )
	    target_link_libraries ( videoplay dftdall tinyxml oglext ${CMAKE_DL_LIBS} ${LIBS} )
	endif ()

endif ()

