// RenderingContext.cpp                                      Copyright (C) 2004 Thomas Jansen (jansen@caesar.de)
//                                                                     (C) 2004 research center caesar
//
// This file is part of OglExt, a free OpenGL extension library.
//
// This program is free software; you can redistribute it and/or modify it under the terms  of  the  GNU  Lesser
// General Public License as published by the Free Software Foundation; either version 2.1 of  the  License,  or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;  without  even  the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser  General  Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with  this  library;  if  not,
// write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// This file was automatically generated on June 15, 2004, 11:47 am

#include	"RenderingContext.hpp"
#include	"Macros.h"
#include	"OglExt.h"

#include	<stdio.h>

// this line was added by Thorsten Jordan to make the code compile on Unix/Linux systems.
// taken from glx.h
extern void (*glXGetProcAddressARB(const GLubyte *procName))( void );

// =============================================================================================================
// ===                                C L A S S   A D M I N I S T R A T I O N                                ===
// =============================================================================================================

//!	The constructor.

CRenderingContext::CRenderingContext()
{
	// 1: clear all uninitialized API function pointers...

	memset(this, 0, sizeof(CRenderingContext));

	// 2: init the extension strings...

	InitVersionString();
	InitExtensionString();

	// 3: init opengl version...

	INIT_GLVERSION(                              1, 2, 0,                    InitVersion12);
	INIT_GLVERSION(                              1, 3, 0,                    InitVersion13);
	INIT_GLVERSION(                              1, 4, 0,                    InitVersion14);
	INIT_GLVERSION(                              1, 5, 0,                    InitVersion15);

	// 4: init opengl extension...

	INIT_EXTENSION(                "GL_3DFX_multisample",              Init3dfxMultisample);
	INIT_EXTENSION(                    "GL_3DFX_tbuffer",                  Init3dfxTbuffer);
	INIT_EXTENSION(   "GL_3DFX_texture_compression_FXT1",   Init3dfxTextureCompressionFXT1);
	INIT_EXTENSION(            "GL_APPLE_client_storage",           InitAppleClientStorage);
	INIT_EXTENSION(             "GL_APPLE_element_array",            InitAppleElementArray);
	INIT_EXTENSION(                     "GL_APPLE_fence",                   InitAppleFence);
	INIT_EXTENSION(           "GL_APPLE_specular_vector",          InitAppleSpecularVector);
	INIT_EXTENSION(            "GL_APPLE_transform_hint",           InitAppleTransformHint);
	INIT_EXTENSION(       "GL_APPLE_vertex_array_object",       InitAppleVertexArrayObject);
	INIT_EXTENSION(        "GL_APPLE_vertex_array_range",        InitAppleVertexArrayRange);
	INIT_EXTENSION(                 "GL_APPLE_ycbcr_422",                InitAppleYcbcr422);
	INIT_EXTENSION(               "GL_ARB_depth_texture",              InitArbDepthTexture);
	INIT_EXTENSION(            "GL_ARB_fragment_program",           InitArbFragmentProgram);
	INIT_EXTENSION(              "GL_ARB_matrix_palette",             InitArbMatrixPalette);
	INIT_EXTENSION(                 "GL_ARB_multisample",               InitArbMultisample);
	INIT_EXTENSION(                "GL_ARB_multitexture",              InitArbMultitexture);
	INIT_EXTENSION(             "GL_ARB_occlusion_query",            InitArbOcclusionQuery);
	INIT_EXTENSION(            "GL_ARB_point_parameters",           InitArbPointParameters);
	INIT_EXTENSION(                "GL_ARB_point_sprite",               InitArbPointSprite);
	INIT_EXTENSION(              "GL_ARB_shader_objects",             InitArbShaderObjects);
	INIT_EXTENSION(        "GL_ARB_shading_language_100",        InitArbShadingLanguage100);
	INIT_EXTENSION(                      "GL_ARB_shadow",                    InitArbShadow);
	INIT_EXTENSION(              "GL_ARB_shadow_ambient",             InitArbShadowAmbient);
	INIT_EXTENSION(        "GL_ARB_texture_border_clamp",        InitArbTextureBorderClamp);
	INIT_EXTENSION(         "GL_ARB_texture_compression",        InitArbTextureCompression);
	INIT_EXTENSION(            "GL_ARB_texture_cube_map",            InitArbTextureCubeMap);
	INIT_EXTENSION(             "GL_ARB_texture_env_add",             InitArbTextureEnvAdd);
	INIT_EXTENSION(         "GL_ARB_texture_env_combine",         InitArbTextureEnvCombine);
	INIT_EXTENSION(        "GL_ARB_texture_env_crossbar",        InitArbTextureEnvCrossbar);
	INIT_EXTENSION(            "GL_ARB_texture_env_dot3",            InitArbTextureEnvDot3);
	INIT_EXTENSION(       "GL_ARB_texture_mirror_repeat",       InitArbTextureMirrorRepeat);
	INIT_EXTENSION(    "GL_ARB_texture_non_power_of_two",      InitArbTextureNonPowerOfTwo);
	INIT_EXTENSION(            "GL_ARB_transpose_matrix",           InitArbTransposeMatrix);
	INIT_EXTENSION(                "GL_ARB_vertex_blend",               InitArbVertexBlend);
	INIT_EXTENSION(        "GL_ARB_vertex_buffer_object",        InitArbVertexBufferObject);
	INIT_EXTENSION(              "GL_ARB_vertex_program",             InitArbVertexProgram);
	INIT_EXTENSION(                  "GL_ARB_window_pos",                 InitArbWindowPos);
	INIT_EXTENSION(                "GL_ATI_draw_buffers",               InitAtiDrawBuffers);
	INIT_EXTENSION(               "GL_ATI_element_array",              InitAtiElementArray);
	INIT_EXTENSION(              "GL_ATI_envmap_bumpmap",             InitAtiEnvmapBumpmap);
	INIT_EXTENSION(             "GL_ATI_fragment_shader",            InitAtiFragmentShader);
	INIT_EXTENSION(           "GL_ATI_map_object_buffer",           InitAtiMapObjectBuffer);
	INIT_EXTENSION(                "GL_ATI_pn_triangles",               InitAtiPnTriangles);
	INIT_EXTENSION(            "GL_ATI_separate_stencil",           InitAtiSeparateStencil);
	INIT_EXTENSION(        "GL_ATI_texture_env_combine3",        InitAtiTextureEnvCombine3);
	INIT_EXTENSION(               "GL_ATI_texture_float",              InitAtiTextureFloat);
	INIT_EXTENSION(         "GL_ATI_texture_mirror_once",         InitAtiTextureMirrorOnce);
	INIT_EXTENSION(        "GL_ATI_text_fragment_shader",        InitAtiTextFragmentShader);
	INIT_EXTENSION(         "GL_ATI_vertex_array_object",         InitAtiVertexArrayObject);
	INIT_EXTENSION(  "GL_ATI_vertex_attrib_array_object",   InitAtiVertexAttribArrayObject);
	INIT_EXTENSION(              "GL_ATI_vertex_streams",             InitAtiVertexStreams);
	INIT_EXTENSION(                  "GL_EXT_422_pixels",                 InitExt422Pixels);
	INIT_EXTENSION(                        "GL_EXT_abgr",                      InitExtAbgr);
	INIT_EXTENSION(                        "GL_EXT_bgra",                      InitExtBgra);
	INIT_EXTENSION(                 "GL_EXT_blend_color",                InitExtBlendColor);
	INIT_EXTENSION(         "GL_EXT_blend_func_separate",         InitExtBlendFuncSeparate);
	INIT_EXTENSION(              "GL_EXT_blend_logic_op",              InitExtBlendLogicOp);
	INIT_EXTENSION(                "GL_EXT_blend_minmax",               InitExtBlendMinmax);
	INIT_EXTENSION(              "GL_EXT_blend_subtract",             InitExtBlendSubtract);
	INIT_EXTENSION(            "GL_EXT_clip_volume_hint",            InitExtClipVolumeHint);
	INIT_EXTENSION(                       "GL_EXT_cmyka",                     InitExtCmyka);
	INIT_EXTENSION(                "GL_EXT_color_matrix",               InitExtColorMatrix);
	INIT_EXTENSION(              "GL_EXT_color_subtable",             InitExtColorSubtable);
	INIT_EXTENSION(       "GL_EXT_compiled_vertex_array",       InitExtCompiledVertexArray);
	INIT_EXTENSION(                 "GL_EXT_convolution",               InitExtConvolution);
	INIT_EXTENSION(            "GL_EXT_coordinate_frame",           InitExtCoordinateFrame);
	INIT_EXTENSION(                "GL_EXT_copy_texture",               InitExtCopyTexture);
	INIT_EXTENSION(                 "GL_EXT_cull_vertex",                InitExtCullVertex);
	INIT_EXTENSION(           "GL_EXT_depth_bounds_test",           InitExtDepthBoundsTest);
	INIT_EXTENSION(         "GL_EXT_draw_range_elements",         InitExtDrawRangeElements);
	INIT_EXTENSION(                   "GL_EXT_fog_coord",                  InitExtFogCoord);
	INIT_EXTENSION(           "GL_EXT_fragment_lighting",          InitExtFragmentLighting);
	INIT_EXTENSION(                   "GL_EXT_histogram",                 InitExtHistogram);
	INIT_EXTENSION(         "GL_EXT_index_array_formats",         InitExtIndexArrayFormats);
	INIT_EXTENSION(                  "GL_EXT_index_func",                 InitExtIndexFunc);
	INIT_EXTENSION(              "GL_EXT_index_material",             InitExtIndexMaterial);
	INIT_EXTENSION(               "GL_EXT_index_texture",              InitExtIndexTexture);
	INIT_EXTENSION(               "GL_EXT_light_texture",              InitExtLightTexture);
	INIT_EXTENSION(              "GL_EXT_misc_attribute",             InitExtMiscAttribute);
	INIT_EXTENSION(                 "GL_EXT_multisample",               InitExtMultisample);
	INIT_EXTENSION(                "GL_EXT_multitexture",              InitExtMultitexture);
	INIT_EXTENSION(           "GL_EXT_multi_draw_arrays",           InitExtMultiDrawArrays);
	INIT_EXTENSION(               "GL_EXT_packed_pixels",              InitExtPackedPixels);
	INIT_EXTENSION(            "GL_EXT_paletted_texture",           InitExtPalettedTexture);
	INIT_EXTENSION(             "GL_EXT_pixel_transform",            InitExtPixelTransform);
	INIT_EXTENSION( "GL_EXT_pixel_transform_color_table",  InitExtPixelTransformColorTable);
	INIT_EXTENSION(            "GL_EXT_point_parameters",           InitExtPointParameters);
	INIT_EXTENSION(              "GL_EXT_polygon_offset",             InitExtPolygonOffset);
	INIT_EXTENSION(              "GL_EXT_rescale_normal",             InitExtRescaleNormal);
	INIT_EXTENSION(             "GL_EXT_secondary_color",            InitExtSecondaryColor);
	INIT_EXTENSION(     "GL_EXT_separate_specular_color",     InitExtSeparateSpecularColor);
	INIT_EXTENSION(                "GL_EXT_shadow_funcs",               InitExtShadowFuncs);
	INIT_EXTENSION(      "GL_EXT_shared_texture_palette",      InitExtSharedTexturePalette);
	INIT_EXTENSION(            "GL_EXT_stencil_two_side",            InitExtStencilTwoSide);
	INIT_EXTENSION(                "GL_EXT_stencil_wrap",               InitExtStencilWrap);
	INIT_EXTENSION(                  "GL_EXT_subtexture",                InitExtSubtexture);
	INIT_EXTENSION(                     "GL_EXT_texture",                   InitExtTexture);
	INIT_EXTENSION(                   "GL_EXT_texture3D",                 InitExtTexture3D);
	INIT_EXTENSION(    "GL_EXT_texture_compression_s3tc",    InitExtTextureCompressionS3tc);
	INIT_EXTENSION(             "GL_EXT_texture_env_add",             InitExtTextureEnvAdd);
	INIT_EXTENSION(         "GL_EXT_texture_env_combine",         InitExtTextureEnvCombine);
	INIT_EXTENSION(            "GL_EXT_texture_env_dot3",            InitExtTextureEnvDot3);
	INIT_EXTENSION(  "GL_EXT_texture_filter_anisotropic",  InitExtTextureFilterAnisotropic);
	INIT_EXTENSION(            "GL_EXT_texture_lod_bias",            InitExtTextureLodBias);
	INIT_EXTENSION(        "GL_EXT_texture_mirror_clamp",        InitExtTextureMirrorClamp);
	INIT_EXTENSION(              "GL_EXT_texture_object",             InitExtTextureObject);
	INIT_EXTENSION(      "GL_EXT_texture_perturb_normal",      InitExtTexturePerturbNormal);
	INIT_EXTENSION(                "GL_EXT_vertex_array",               InitExtVertexArray);
	INIT_EXTENSION(               "GL_EXT_vertex_shader",              InitExtVertexShader);
	INIT_EXTENSION(            "GL_EXT_vertex_weighting",           InitExtVertexWeighting);
	INIT_EXTENSION(     "GL_HP_convolution_border_modes",     InitHpConvolutionBorderModes);
	INIT_EXTENSION(              "GL_HP_image_transform",             InitHpImageTransform);
	INIT_EXTENSION(               "GL_HP_occlusion_test",              InitHpOcclusionTest);
	INIT_EXTENSION(             "GL_HP_texture_lighting",            InitHpTextureLighting);
	INIT_EXTENSION(                 "GL_IBM_cull_vertex",                InitIbmCullVertex);
	INIT_EXTENSION(       "GL_IBM_multimode_draw_arrays",       InitIbmMultimodeDrawArrays);
	INIT_EXTENSION(              "GL_IBM_rasterpos_clip",             InitIbmRasterposClip);
	INIT_EXTENSION(                 "GL_IBM_static_data",                InitIbmStaticData);
	INIT_EXTENSION(     "GL_IBM_texture_mirrored_repeat",     InitIbmTextureMirroredRepeat);
	INIT_EXTENSION(          "GL_IBM_vertex_array_lists",          InitIbmVertexArrayLists);
	INIT_EXTENSION(        "GL_INGR_blend_func_separate",        InitIngrBlendFuncSeparate);
	INIT_EXTENSION(                "GL_INGR_color_clamp",               InitIngrColorClamp);
	INIT_EXTENSION(             "GL_INGR_interlace_read",            InitIngrInterlaceRead);
	INIT_EXTENSION(           "GL_INTEL_parallel_arrays",          InitIntelParallelArrays);
	INIT_EXTENSION(             "GL_MESA_resize_buffers",            InitMesaResizeBuffers);
	INIT_EXTENSION(                 "GL_MESA_window_pos",                InitMesaWindowPos);
	INIT_EXTENSION(                 "GL_NV_blend_square",                InitNvBlendSquare);
	INIT_EXTENSION(          "GL_NV_copy_depth_to_color",           InitNvCopyDepthToColor);
	INIT_EXTENSION(                  "GL_NV_depth_clamp",                 InitNvDepthClamp);
	INIT_EXTENSION(                   "GL_NV_evaluators",                 InitNvEvaluators);
	INIT_EXTENSION(                        "GL_NV_fence",                      InitNvFence);
	INIT_EXTENSION(                 "GL_NV_float_buffer",                InitNvFloatBuffer);
	INIT_EXTENSION(                 "GL_NV_fog_distance",                InitNvFogDistance);
	INIT_EXTENSION(             "GL_NV_fragment_program",            InitNvFragmentProgram);
	INIT_EXTENSION(                   "GL_NV_half_float",                  InitNvHalfFloat);
	INIT_EXTENSION(           "GL_NV_light_max_exponent",           InitNvLightMaxExponent);
	INIT_EXTENSION(      "GL_NV_multisample_filter_hint",      InitNvMultisampleFilterHint);
	INIT_EXTENSION(              "GL_NV_occlusion_query",             InitNvOcclusionQuery);
	INIT_EXTENSION(         "GL_NV_packed_depth_stencil",         InitNvPackedDepthStencil);
	INIT_EXTENSION(             "GL_NV_pixel_data_range",             InitNvPixelDataRange);
	INIT_EXTENSION(                 "GL_NV_point_sprite",                InitNvPointSprite);
	INIT_EXTENSION(            "GL_NV_primitive_restart",           InitNvPrimitiveRestart);
	INIT_EXTENSION(           "GL_NV_register_combiners",          InitNvRegisterCombiners);
	INIT_EXTENSION(          "GL_NV_register_combiners2",         InitNvRegisterCombiners2);
	INIT_EXTENSION(                "GL_NV_texgen_emboss",               InitNvTexgenEmboss);
	INIT_EXTENSION(            "GL_NV_texgen_reflection",           InitNvTexgenReflection);
	INIT_EXTENSION(      "GL_NV_texture_compression_vtc",      InitNvTextureCompressionVtc);
	INIT_EXTENSION(         "GL_NV_texture_env_combine4",         InitNvTextureEnvCombine4);
	INIT_EXTENSION(        "GL_NV_texture_expand_normal",        InitNvTextureExpandNormal);
	INIT_EXTENSION(            "GL_NV_texture_rectangle",           InitNvTextureRectangle);
	INIT_EXTENSION(               "GL_NV_texture_shader",              InitNvTextureShader);
	INIT_EXTENSION(              "GL_NV_texture_shader2",             InitNvTextureShader2);
	INIT_EXTENSION(              "GL_NV_texture_shader3",             InitNvTextureShader3);
	INIT_EXTENSION(           "GL_NV_vertex_array_range",           InitNvVertexArrayRange);
	INIT_EXTENSION(          "GL_NV_vertex_array_range2",          InitNvVertexArrayRange2);
	INIT_EXTENSION(               "GL_NV_vertex_program",              InitNvVertexProgram);
	INIT_EXTENSION(            "GL_NV_vertex_program1_1",            InitNvVertexProgram11);
	INIT_EXTENSION(              "GL_NV_vertex_program2",             InitNvVertexProgram2);
	INIT_EXTENSION(                   "GL_OML_interlace",                 InitOmlInterlace);
	INIT_EXTENSION(                    "GL_OML_resample",                  InitOmlResample);
	INIT_EXTENSION(                   "GL_OML_subsample",                 InitOmlSubsample);
	INIT_EXTENSION(                  "GL_PGI_misc_hints",                 InitPgiMiscHints);
	INIT_EXTENSION(                "GL_PGI_vertex_hints",               InitPgiVertexHints);
	INIT_EXTENSION(         "GL_REND_screen_coordinates",        InitRendScreenCoordinates);
	INIT_EXTENSION(                         "GL_S3_s3tc",                       InitS3S3tc);
	INIT_EXTENSION(             "GL_SGIS_detail_texture",            InitSgiSDetailTexture);
	INIT_EXTENSION(               "GL_SGIS_fog_function",              InitSgiSFogFunction);
	INIT_EXTENSION(            "GL_SGIS_generate_mipmap",           InitSgiSGenerateMipmap);
	INIT_EXTENSION(                "GL_SGIS_multisample",              InitSgiSMultisample);
	INIT_EXTENSION(               "GL_SGIS_multitexture",             InitSgisMultitexture);
	INIT_EXTENSION(              "GL_SGIS_pixel_texture",             InitSgiSPixelTexture);
	INIT_EXTENSION(          "GL_SGIS_point_line_texgen",          InitSgiSPointLineTexgen);
	INIT_EXTENSION(           "GL_SGIS_point_parameters",          InitSgiSPointParameters);
	INIT_EXTENSION(            "GL_SGIS_sharpen_texture",           InitSgiSSharpenTexture);
	INIT_EXTENSION(                  "GL_SGIS_texture4D",                InitSgiSTexture4D);
	INIT_EXTENSION(       "GL_SGIS_texture_border_clamp",       InitSgiSTextureBorderClamp);
	INIT_EXTENSION(         "GL_SGIS_texture_color_mask",         InitSgiSTextureColorMask);
	INIT_EXTENSION(         "GL_SGIS_texture_edge_clamp",         InitSgiSTextureEdgeClamp);
	INIT_EXTENSION(            "GL_SGIS_texture_filter4",           InitSgiSTextureFilter4);
	INIT_EXTENSION(                "GL_SGIS_texture_lod",               InitSgiSTextureLod);
	INIT_EXTENSION(                      "GL_SGIX_async",                    InitSgixAsync);
	INIT_EXTENSION(            "GL_SGIX_async_histogram",           InitSgixAsyncHistogram);
	INIT_EXTENSION(                "GL_SGIX_async_pixel",               InitSgixAsyncPixel);
	INIT_EXTENSION(         "GL_SGIX_blend_alpha_minmax",         InitSgixBlendAlphaMinmax);
	INIT_EXTENSION(      "GL_SGIX_calligraphic_fragment",     InitSgixCalligraphicFragment);
	INIT_EXTENSION(                    "GL_SGIX_clipmap",                  InitSgixClipmap);
	INIT_EXTENSION(       "GL_SGIX_convolution_accuracy",      InitSgixConvolutionAccuracy);
	INIT_EXTENSION(      "GL_SGIX_depth_pass_instrument",      InitSgixDepthPassInstrument);
	INIT_EXTENSION(              "GL_SGIX_depth_texture",             InitSgixDepthTexture);
	INIT_EXTENSION(               "GL_SGIX_flush_raster",              InitSgixFlushRaster);
	INIT_EXTENSION(                 "GL_SGIX_fog_offset",                InitSgixFogOffset);
	INIT_EXTENSION(                  "GL_SGIX_fog_scale",                 InitSgixFogScale);
	INIT_EXTENSION(          "GL_SGIX_fragment_lighting",         InitSgixFragmentLighting);
	INIT_EXTENSION(                  "GL_SGIX_framezoom",                InitSgixFramezoom);
	INIT_EXTENSION(            "GL_SGIX_igloo_interface",           InitSgixIglooInterface);
	INIT_EXTENSION(                "GL_SGIX_instruments",              InitSgixInstruments);
	INIT_EXTENSION(                  "GL_SGIX_interlace",                InitSgixInterlace);
	INIT_EXTENSION(             "GL_SGIX_ir_instrument1",            InitSgixIrInstrument1);
	INIT_EXTENSION(              "GL_SGIX_list_priority",             InitSgixListPriority);
	INIT_EXTENSION(              "GL_SGIX_pixel_texture",             InitSgixPixelTexture);
	INIT_EXTENSION(                "GL_SGIX_pixel_tiles",               InitSgixPixelTiles);
	INIT_EXTENSION(             "GL_SGIX_polynomial_ffd",            InitSgixPolynomialFfd);
	INIT_EXTENSION(            "GL_SGIX_reference_plane",           InitSgixReferencePlane);
	INIT_EXTENSION(                   "GL_SGIX_resample",                 InitSgixResample);
	INIT_EXTENSION(             "GL_SGIX_scalebias_hint",            InitSgixScalebiasHint);
	INIT_EXTENSION(                     "GL_SGIX_shadow",                   InitSgixShadow);
	INIT_EXTENSION(             "GL_SGIX_shadow_ambient",            InitSgixShadowAmbient);
	INIT_EXTENSION(                     "GL_SGIX_sprite",                   InitSgixSprite);
	INIT_EXTENSION(                  "GL_SGIX_subsample",                InitSgixSubsample);
	INIT_EXTENSION(          "GL_SGIX_tag_sample_buffer",          InitSgixTagSampleBuffer);
	INIT_EXTENSION(            "GL_SGIX_texture_add_env",            InitSgixTextureAddEnv);
	INIT_EXTENSION(   "GL_SGIX_texture_coordinate_clamp",   InitSgixTextureCoordinateClamp);
	INIT_EXTENSION(           "GL_SGIX_texture_lod_bias",           InitSgixTextureLodBias);
	INIT_EXTENSION(       "GL_SGIX_texture_multi_buffer",       InitSgixTextureMultiBuffer);
	INIT_EXTENSION(         "GL_SGIX_texture_scale_bias",         InitSgixTextureScaleBias);
	INIT_EXTENSION(             "GL_SGIX_texture_select",            InitSgixTextureSelect);
	INIT_EXTENSION(             "GL_SGIX_vertex_preclip",            InitSgixVertexPreclip);
	INIT_EXTENSION(                      "GL_SGIX_ycrcb",                    InitSgixYcrcb);
	INIT_EXTENSION(                     "GL_SGIX_ycrcba",                   InitSgixYcrcba);
	INIT_EXTENSION(            "GL_SGIX_ycrcb_subsample",           InitSgixYcrcbSubsample);
	INIT_EXTENSION(                "GL_SGI_color_matrix",               InitSgiColorMatrix);
	INIT_EXTENSION(                 "GL_SGI_color_table",                InitSgiColorTable);
	INIT_EXTENSION(         "GL_SGI_texture_color_table",         InitSgiTextureColorTable);
	INIT_EXTENSION(              "GL_SUNX_constant_data",             InitSunxConstantData);
	INIT_EXTENSION(    "GL_SUN_convolution_border_modes",    InitSunConvolutionBorderModes);
	INIT_EXTENSION(                "GL_SUN_global_alpha",               InitSunGlobalAlpha);
	INIT_EXTENSION(                  "GL_SUN_mesh_array",                 InitSunMeshArray);
	INIT_EXTENSION(                 "GL_SUN_slice_accum",                InitSunSliceAccum);
	INIT_EXTENSION(               "GL_SUN_triangle_list",              InitSunTriangleList);
	INIT_EXTENSION(                      "GL_SUN_vertex",                    InitSunvertex);
}



// =============================================================================================================
// ===                                    S T A T I C   F U N C T I O N S                                    ===
// =============================================================================================================

//!	Return address of a given OpenGL Function.

void * CRenderingContext::GetProcAddress(char const * szFunction)
{
	#if defined(_WIN32)
	
		// W: implementation for microsoft windows...

			// W.1: create a new function name...

			char * szNewFunction = new char [strlen(szFunction) + 3];
			if(!szNewFunction) {

				return NULL;
			}

			// W.2: create the new function name...
		
			szNewFunction[0]	= 'g';
			szNewFunction[1]	= 'l';
			strcpy(&szNewFunction[2], szFunction);

			// W.3: determine the address...

           // This conversion was modified by Thorsten Jordan to make the code compileable with mingw
			void * pProcAddress = (void*)(::wglGetProcAddress(szNewFunction));

			delete [] szNewFunction;

			return pProcAddress;

	#elif (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)

		// M: implementation for apple mac os x...

			// M.1: create a new function name...

			char * szNewFunction = new char [strlen(szFunction) + 4];
			if(!szNewFunction) {

				return NULL;
			}

			// M.2: create the new function name...

			szNewFunction[0]	= '_';
			szNewFunction[1]	= 'g';
			szNewFunction[2]	= 'l';
			strcpy(&szNewFunction[3], szFunction);

			// M.3: check if the symbol name is defined...

			if(!NSIsSymbolNameDefined(szNewFunction)) {

				delete [] szNewFunction;
				return NULL;
			}

			// M.4: look-up and bind the symbol...

			NSSymbol symProcedure = NSLookupAndBindSymbol(szNewFunction);
			if(!symProcedure) {

				delete [] szNewFunction;
				return NULL;
			}

			// M.5: determine the address...

			void * pProcAddress = NSAddressOfSymbol(symProcedure);

			delete [] szNewFunction;

			return pProcAddress;

	#else

		// U: implementation for broad range of UNIXes...

			// U.1: create a new function name...

			char * szNewFunction = new char [strlen(szFunction) + 3];
			if(!szNewFunction) {

				return NULL;
			}

			// U.2: create the new function name...
		
			szNewFunction[0]	= 'g';
			szNewFunction[1]	= 'l';
			strcpy(&szNewFunction[2], szFunction);

			// U.3: determine the address...

			void * pProcAddress = (void *) ::glXGetProcAddressARB((GLubyte const *) szNewFunction);

			delete [] szNewFunction;

			return pProcAddress;
	
	#endif
}



/* ========================================================================================================== */
/* ===                                    O P E N G L   V E R S I O N                                     === */
/* ========================================================================================================== */

// ---[ GL_VERSION_1_2 ]----------------------------------------------------------------------------------------

//!	Init GL_VERSION_1_2.

bool CRenderingContext::InitVersion12()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_2_OGLEXT

		GET_PROC_ADDRESS(BlendColor);
		GET_PROC_ADDRESS(BlendEquation);
		GET_PROC_ADDRESS(ColorSubTable);
		GET_PROC_ADDRESS(ColorTable);
		GET_PROC_ADDRESS(ColorTableParameterfv);
		GET_PROC_ADDRESS(ColorTableParameteriv);
		GET_PROC_ADDRESS(ConvolutionFilter1D);
		GET_PROC_ADDRESS(ConvolutionFilter2D);
		GET_PROC_ADDRESS(ConvolutionParameterf);
		GET_PROC_ADDRESS(ConvolutionParameterfv);
		GET_PROC_ADDRESS(ConvolutionParameteri);
		GET_PROC_ADDRESS(ConvolutionParameteriv);
		GET_PROC_ADDRESS(CopyColorSubTable);
		GET_PROC_ADDRESS(CopyColorTable);
		GET_PROC_ADDRESS(CopyConvolutionFilter1D);
		GET_PROC_ADDRESS(CopyConvolutionFilter2D);
		GET_PROC_ADDRESS(CopyTexSubImage3D);
		GET_PROC_ADDRESS(DrawRangeElements);
		GET_PROC_ADDRESS(GetColorTable);
		GET_PROC_ADDRESS(GetColorTableParameterfv);
		GET_PROC_ADDRESS(GetColorTableParameteriv);
		GET_PROC_ADDRESS(GetConvolutionFilter);
		GET_PROC_ADDRESS(GetConvolutionParameterfv);
		GET_PROC_ADDRESS(GetConvolutionParameteriv);
		GET_PROC_ADDRESS(GetHistogram);
		GET_PROC_ADDRESS(GetHistogramParameterfv);
		GET_PROC_ADDRESS(GetHistogramParameteriv);
		GET_PROC_ADDRESS(GetMinmax);
		GET_PROC_ADDRESS(GetMinmaxParameterfv);
		GET_PROC_ADDRESS(GetMinmaxParameteriv);
		GET_PROC_ADDRESS(GetSeparableFilter);
		GET_PROC_ADDRESS(Histogram);
		GET_PROC_ADDRESS(Minmax);
		GET_PROC_ADDRESS(ResetHistogram);
		GET_PROC_ADDRESS(ResetMinmax);
		GET_PROC_ADDRESS(SeparableFilter2D);
		GET_PROC_ADDRESS(TexImage3D);
		GET_PROC_ADDRESS(TexSubImage3D);

	#endif // GL_VERSION_1_2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_1_3 ]----------------------------------------------------------------------------------------

//!	Init GL_VERSION_1_3.

bool CRenderingContext::InitVersion13()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_3_OGLEXT

		GET_PROC_ADDRESS(ActiveTexture);
		GET_PROC_ADDRESS(ClientActiveTexture);
		GET_PROC_ADDRESS(CompressedTexImage1D);
		GET_PROC_ADDRESS(CompressedTexImage2D);
		GET_PROC_ADDRESS(CompressedTexImage3D);
		GET_PROC_ADDRESS(CompressedTexSubImage1D);
		GET_PROC_ADDRESS(CompressedTexSubImage2D);
		GET_PROC_ADDRESS(CompressedTexSubImage3D);
		GET_PROC_ADDRESS(GetCompressedTexImage);
		GET_PROC_ADDRESS(LoadTransposeMatrixd);
		GET_PROC_ADDRESS(LoadTransposeMatrixf);
		GET_PROC_ADDRESS(MultiTexCoord1d);
		GET_PROC_ADDRESS(MultiTexCoord1dv);
		GET_PROC_ADDRESS(MultiTexCoord1f);
		GET_PROC_ADDRESS(MultiTexCoord1fv);
		GET_PROC_ADDRESS(MultiTexCoord1i);
		GET_PROC_ADDRESS(MultiTexCoord1iv);
		GET_PROC_ADDRESS(MultiTexCoord1s);
		GET_PROC_ADDRESS(MultiTexCoord1sv);
		GET_PROC_ADDRESS(MultiTexCoord2d);
		GET_PROC_ADDRESS(MultiTexCoord2dv);
		GET_PROC_ADDRESS(MultiTexCoord2f);
		GET_PROC_ADDRESS(MultiTexCoord2fv);
		GET_PROC_ADDRESS(MultiTexCoord2i);
		GET_PROC_ADDRESS(MultiTexCoord2iv);
		GET_PROC_ADDRESS(MultiTexCoord2s);
		GET_PROC_ADDRESS(MultiTexCoord2sv);
		GET_PROC_ADDRESS(MultiTexCoord3d);
		GET_PROC_ADDRESS(MultiTexCoord3dv);
		GET_PROC_ADDRESS(MultiTexCoord3f);
		GET_PROC_ADDRESS(MultiTexCoord3fv);
		GET_PROC_ADDRESS(MultiTexCoord3i);
		GET_PROC_ADDRESS(MultiTexCoord3iv);
		GET_PROC_ADDRESS(MultiTexCoord3s);
		GET_PROC_ADDRESS(MultiTexCoord3sv);
		GET_PROC_ADDRESS(MultiTexCoord4d);
		GET_PROC_ADDRESS(MultiTexCoord4dv);
		GET_PROC_ADDRESS(MultiTexCoord4f);
		GET_PROC_ADDRESS(MultiTexCoord4fv);
		GET_PROC_ADDRESS(MultiTexCoord4i);
		GET_PROC_ADDRESS(MultiTexCoord4iv);
		GET_PROC_ADDRESS(MultiTexCoord4s);
		GET_PROC_ADDRESS(MultiTexCoord4sv);
		GET_PROC_ADDRESS(MultTransposeMatrixd);
		GET_PROC_ADDRESS(MultTransposeMatrixf);
		GET_PROC_ADDRESS(SampleCoverage);

	#endif // GL_VERSION_1_3_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_1_4 ]----------------------------------------------------------------------------------------

//!	Init GL_VERSION_1_4.

bool CRenderingContext::InitVersion14()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_4_OGLEXT

		GET_PROC_ADDRESS(BlendFuncSeparate);
		GET_PROC_ADDRESS(FogCoordd);
		GET_PROC_ADDRESS(FogCoorddv);
		GET_PROC_ADDRESS(FogCoordf);
		GET_PROC_ADDRESS(FogCoordfv);
		GET_PROC_ADDRESS(FogCoordPointer);
		GET_PROC_ADDRESS(MultiDrawArrays);
		GET_PROC_ADDRESS(MultiDrawElements);
		GET_PROC_ADDRESS(PointParameterf);
		GET_PROC_ADDRESS(PointParameterfv);
		GET_PROC_ADDRESS(PointParameteri);
		GET_PROC_ADDRESS(PointParameteriv);
		GET_PROC_ADDRESS(SecondaryColor3b);
		GET_PROC_ADDRESS(SecondaryColor3bv);
		GET_PROC_ADDRESS(SecondaryColor3d);
		GET_PROC_ADDRESS(SecondaryColor3dv);
		GET_PROC_ADDRESS(SecondaryColor3f);
		GET_PROC_ADDRESS(SecondaryColor3fv);
		GET_PROC_ADDRESS(SecondaryColor3i);
		GET_PROC_ADDRESS(SecondaryColor3iv);
		GET_PROC_ADDRESS(SecondaryColor3s);
		GET_PROC_ADDRESS(SecondaryColor3sv);
		GET_PROC_ADDRESS(SecondaryColor3ub);
		GET_PROC_ADDRESS(SecondaryColor3ubv);
		GET_PROC_ADDRESS(SecondaryColor3ui);
		GET_PROC_ADDRESS(SecondaryColor3uiv);
		GET_PROC_ADDRESS(SecondaryColor3us);
		GET_PROC_ADDRESS(SecondaryColor3usv);
		GET_PROC_ADDRESS(SecondaryColorPointer);
		GET_PROC_ADDRESS(WindowPos2d);
		GET_PROC_ADDRESS(WindowPos2dv);
		GET_PROC_ADDRESS(WindowPos2f);
		GET_PROC_ADDRESS(WindowPos2fv);
		GET_PROC_ADDRESS(WindowPos2i);
		GET_PROC_ADDRESS(WindowPos2iv);
		GET_PROC_ADDRESS(WindowPos2s);
		GET_PROC_ADDRESS(WindowPos2sv);
		GET_PROC_ADDRESS(WindowPos3d);
		GET_PROC_ADDRESS(WindowPos3dv);
		GET_PROC_ADDRESS(WindowPos3f);
		GET_PROC_ADDRESS(WindowPos3fv);
		GET_PROC_ADDRESS(WindowPos3i);
		GET_PROC_ADDRESS(WindowPos3iv);
		GET_PROC_ADDRESS(WindowPos3s);
		GET_PROC_ADDRESS(WindowPos3sv);

	#endif // GL_VERSION_1_4_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_VERSION_1_5 ]----------------------------------------------------------------------------------------

//!	Init GL_VERSION_1_5.

bool CRenderingContext::InitVersion15()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_VERSION_1_5_OGLEXT

		GET_PROC_ADDRESS(BeginQuery);
		GET_PROC_ADDRESS(BindBuffer);
		GET_PROC_ADDRESS(BufferData);
		GET_PROC_ADDRESS(BufferSubData);
		GET_PROC_ADDRESS(DeleteBuffers);
		GET_PROC_ADDRESS(DeleteQueries);
		GET_PROC_ADDRESS(EndQuery);
		GET_PROC_ADDRESS(GenBuffers);
		GET_PROC_ADDRESS(GenQueries);
		GET_PROC_ADDRESS(GetBufferParameteriv);
		GET_PROC_ADDRESS(GetBufferPointerv);
		GET_PROC_ADDRESS(GetBufferSubData);
		GET_PROC_ADDRESS(GetQueryiv);
		GET_PROC_ADDRESS(GetQueryObjectiv);
		GET_PROC_ADDRESS(GetQueryObjectuiv);
		GET_PROC_ADDRESS(IsBuffer);
		GET_PROC_ADDRESS(IsQuery);
		GET_PROC_ADDRESS(MapBuffer);
		GET_PROC_ADDRESS(UnmapBuffer);

	#endif // GL_VERSION_1_5_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


/* ========================================================================================================== */
/* ===                                  O P E N G L   E X T E N S I O N                                   === */
/* ========================================================================================================== */

// ---[ GL_3DFX_multisample ]-----------------------------------------------------------------------------------

//!	Init GL_3DFX_multisample.

bool CRenderingContext::Init3dfxMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_3DFX_multisample_OGLEXT


	#endif // GL_3DFX_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_3DFX_tbuffer ]---------------------------------------------------------------------------------------

//!	Init GL_3DFX_tbuffer.

bool CRenderingContext::Init3dfxTbuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_3DFX_tbuffer_OGLEXT

		GET_PROC_ADDRESS(TbufferMask3DFX);

	#endif // GL_3DFX_tbuffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_3DFX_texture_compression_FXT1 ]----------------------------------------------------------------------

//!	Init GL_3DFX_texture_compression_FXT1.

bool CRenderingContext::Init3dfxTextureCompressionFXT1()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_3DFX_texture_compression_FXT1_OGLEXT


	#endif // GL_3DFX_texture_compression_FXT1_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_client_storage ]-------------------------------------------------------------------------------

//!	Init GL_APPLE_client_storage.

bool CRenderingContext::InitAppleClientStorage()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_client_storage_OGLEXT


	#endif // GL_APPLE_client_storage_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_element_array ]--------------------------------------------------------------------------------

//!	Init GL_APPLE_element_array.

bool CRenderingContext::InitAppleElementArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_element_array_OGLEXT

		GET_PROC_ADDRESS(DrawElementArrayAPPLE);
		GET_PROC_ADDRESS(DrawRangeElementArrayAPPLE);
		GET_PROC_ADDRESS(ElementPointerAPPLE);
		GET_PROC_ADDRESS(MultiDrawElementArrayAPPLE);
		GET_PROC_ADDRESS(MultiDrawRangeElementArrayAPPLE);

	#endif // GL_APPLE_element_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_fence ]----------------------------------------------------------------------------------------

//!	Init GL_APPLE_fence.

bool CRenderingContext::InitAppleFence()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_fence_OGLEXT

		GET_PROC_ADDRESS(DeleteFencesAPPLE);
		GET_PROC_ADDRESS(FinishFenceAPPLE);
		GET_PROC_ADDRESS(FinishObjectAPPLE);
		GET_PROC_ADDRESS(GenFencesAPPLE);
		GET_PROC_ADDRESS(IsFenceAPPLE);
		GET_PROC_ADDRESS(SetFenceAPPLE);
		GET_PROC_ADDRESS(TestFenceAPPLE);
		GET_PROC_ADDRESS(TestObjectAPPLE);

	#endif // GL_APPLE_fence_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_specular_vector ]------------------------------------------------------------------------------

//!	Init GL_APPLE_specular_vector.

bool CRenderingContext::InitAppleSpecularVector()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_specular_vector_OGLEXT


	#endif // GL_APPLE_specular_vector_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_transform_hint ]-------------------------------------------------------------------------------

//!	Init GL_APPLE_transform_hint.

bool CRenderingContext::InitAppleTransformHint()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_transform_hint_OGLEXT


	#endif // GL_APPLE_transform_hint_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_vertex_array_object ]--------------------------------------------------------------------------

//!	Init GL_APPLE_vertex_array_object.

bool CRenderingContext::InitAppleVertexArrayObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_vertex_array_object_OGLEXT

		GET_PROC_ADDRESS(BindVertexArrayAPPLE);
		GET_PROC_ADDRESS(DeleteVertexArraysAPPLE);
		GET_PROC_ADDRESS(GenVertexArraysAPPLE);
		GET_PROC_ADDRESS(IsVertexArrayAPPLE);

	#endif // GL_APPLE_vertex_array_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_vertex_array_range ]---------------------------------------------------------------------------

//!	Init GL_APPLE_vertex_array_range.

bool CRenderingContext::InitAppleVertexArrayRange()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_vertex_array_range_OGLEXT

		GET_PROC_ADDRESS(FlushVertexArrayRangeAPPLE);
		GET_PROC_ADDRESS(VertexArrayParameteriAPPLE);
		GET_PROC_ADDRESS(VertexArrayRangeAPPLE);

	#endif // GL_APPLE_vertex_array_range_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_APPLE_ycbcr_422 ]------------------------------------------------------------------------------------

//!	Init GL_APPLE_ycbcr_422.

bool CRenderingContext::InitAppleYcbcr422()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_APPLE_ycbcr_422_OGLEXT


	#endif // GL_APPLE_ycbcr_422_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_depth_texture ]----------------------------------------------------------------------------------

//!	Init GL_ARB_depth_texture.

bool CRenderingContext::InitArbDepthTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_depth_texture_OGLEXT


	#endif // GL_ARB_depth_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_fragment_program ]-------------------------------------------------------------------------------

//!	Init GL_ARB_fragment_program.

bool CRenderingContext::InitArbFragmentProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_fragment_program_OGLEXT


	#endif // GL_ARB_fragment_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_matrix_palette ]---------------------------------------------------------------------------------

//!	Init GL_ARB_matrix_palette.

bool CRenderingContext::InitArbMatrixPalette()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_matrix_palette_OGLEXT

		GET_PROC_ADDRESS(CurrentPaletteMatrixARB);
		GET_PROC_ADDRESS(MatrixIndexPointerARB);
		GET_PROC_ADDRESS(MatrixIndexubvARB);
		GET_PROC_ADDRESS(MatrixIndexuivARB);
		GET_PROC_ADDRESS(MatrixIndexusvARB);

	#endif // GL_ARB_matrix_palette_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_multisample ]------------------------------------------------------------------------------------

//!	Init GL_ARB_multisample.

bool CRenderingContext::InitArbMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_multisample_OGLEXT

		GET_PROC_ADDRESS(SampleCoverageARB);

	#endif // GL_ARB_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_multitexture ]-----------------------------------------------------------------------------------

//!	Init GL_ARB_multitexture.

bool CRenderingContext::InitArbMultitexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_multitexture_OGLEXT

		GET_PROC_ADDRESS(ActiveTextureARB);
		GET_PROC_ADDRESS(ClientActiveTextureARB);
		GET_PROC_ADDRESS(MultiTexCoord1dARB);
		GET_PROC_ADDRESS(MultiTexCoord1dvARB);
		GET_PROC_ADDRESS(MultiTexCoord1fARB);
		GET_PROC_ADDRESS(MultiTexCoord1fvARB);
		GET_PROC_ADDRESS(MultiTexCoord1iARB);
		GET_PROC_ADDRESS(MultiTexCoord1ivARB);
		GET_PROC_ADDRESS(MultiTexCoord1sARB);
		GET_PROC_ADDRESS(MultiTexCoord1svARB);
		GET_PROC_ADDRESS(MultiTexCoord2dARB);
		GET_PROC_ADDRESS(MultiTexCoord2dvARB);
		GET_PROC_ADDRESS(MultiTexCoord2fARB);
		GET_PROC_ADDRESS(MultiTexCoord2fvARB);
		GET_PROC_ADDRESS(MultiTexCoord2iARB);
		GET_PROC_ADDRESS(MultiTexCoord2ivARB);
		GET_PROC_ADDRESS(MultiTexCoord2sARB);
		GET_PROC_ADDRESS(MultiTexCoord2svARB);
		GET_PROC_ADDRESS(MultiTexCoord3dARB);
		GET_PROC_ADDRESS(MultiTexCoord3dvARB);
		GET_PROC_ADDRESS(MultiTexCoord3fARB);
		GET_PROC_ADDRESS(MultiTexCoord3fvARB);
		GET_PROC_ADDRESS(MultiTexCoord3iARB);
		GET_PROC_ADDRESS(MultiTexCoord3ivARB);
		GET_PROC_ADDRESS(MultiTexCoord3sARB);
		GET_PROC_ADDRESS(MultiTexCoord3svARB);
		GET_PROC_ADDRESS(MultiTexCoord4dARB);
		GET_PROC_ADDRESS(MultiTexCoord4dvARB);
		GET_PROC_ADDRESS(MultiTexCoord4fARB);
		GET_PROC_ADDRESS(MultiTexCoord4fvARB);
		GET_PROC_ADDRESS(MultiTexCoord4iARB);
		GET_PROC_ADDRESS(MultiTexCoord4ivARB);
		GET_PROC_ADDRESS(MultiTexCoord4sARB);
		GET_PROC_ADDRESS(MultiTexCoord4svARB);

	#endif // GL_ARB_multitexture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_occlusion_query ]--------------------------------------------------------------------------------

//!	Init GL_ARB_occlusion_query.

bool CRenderingContext::InitArbOcclusionQuery()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_occlusion_query_OGLEXT

		GET_PROC_ADDRESS(BeginQueryARB);
		GET_PROC_ADDRESS(DeleteQueriesARB);
		GET_PROC_ADDRESS(EndQueryARB);
		GET_PROC_ADDRESS(GenQueriesARB);
		GET_PROC_ADDRESS(GetQueryivARB);
		GET_PROC_ADDRESS(GetQueryObjectivARB);
		GET_PROC_ADDRESS(GetQueryObjectuivARB);
		GET_PROC_ADDRESS(IsQueryARB);

	#endif // GL_ARB_occlusion_query_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_point_parameters ]-------------------------------------------------------------------------------

//!	Init GL_ARB_point_parameters.

bool CRenderingContext::InitArbPointParameters()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_point_parameters_OGLEXT

		GET_PROC_ADDRESS(PointParameterfARB);
		GET_PROC_ADDRESS(PointParameterfvARB);

	#endif // GL_ARB_point_parameters_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_point_sprite ]-----------------------------------------------------------------------------------

//!	Init GL_ARB_point_sprite.

bool CRenderingContext::InitArbPointSprite()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_point_sprite_OGLEXT


	#endif // GL_ARB_point_sprite_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_shader_objects ]---------------------------------------------------------------------------------

//!	Init GL_ARB_shader_objects.

bool CRenderingContext::InitArbShaderObjects()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_shader_objects_OGLEXT

		GET_PROC_ADDRESS(AttachObjectARB);
		GET_PROC_ADDRESS(CompileShaderARB);
		GET_PROC_ADDRESS(CreateProgramObjectARB);
		GET_PROC_ADDRESS(CreateShaderObjectARB);
		GET_PROC_ADDRESS(DeleteObjectARB);
		GET_PROC_ADDRESS(DetachObjectARB);
		GET_PROC_ADDRESS(GetActiveUniformARB);
		GET_PROC_ADDRESS(GetAttachedObjectsARB);
		GET_PROC_ADDRESS(GetHandleARB);
		GET_PROC_ADDRESS(GetInfoLogARB);
		GET_PROC_ADDRESS(GetObjectParameterfvARB);
		GET_PROC_ADDRESS(GetObjectParameterivARB);
		GET_PROC_ADDRESS(GetShaderSourceARB);
		GET_PROC_ADDRESS(GetUniformfvARB);
		GET_PROC_ADDRESS(GetUniformivARB);
		GET_PROC_ADDRESS(GetUniformLocationARB);
		GET_PROC_ADDRESS(LinkProgramARB);
		GET_PROC_ADDRESS(ShaderSourceARB);
		GET_PROC_ADDRESS(Uniform1fARB);
		GET_PROC_ADDRESS(Uniform1fvARB);
		GET_PROC_ADDRESS(Uniform1iARB);
		GET_PROC_ADDRESS(Uniform1ivARB);
		GET_PROC_ADDRESS(Uniform2fARB);
		GET_PROC_ADDRESS(Uniform2fvARB);
		GET_PROC_ADDRESS(Uniform2iARB);
		GET_PROC_ADDRESS(Uniform2ivARB);
		GET_PROC_ADDRESS(Uniform3fARB);
		GET_PROC_ADDRESS(Uniform3fvARB);
		GET_PROC_ADDRESS(Uniform3iARB);
		GET_PROC_ADDRESS(Uniform3ivARB);
		GET_PROC_ADDRESS(Uniform4fARB);
		GET_PROC_ADDRESS(Uniform4fvARB);
		GET_PROC_ADDRESS(Uniform4iARB);
		GET_PROC_ADDRESS(Uniform4ivARB);
		GET_PROC_ADDRESS(UniformMatrix2fvARB);
		GET_PROC_ADDRESS(UniformMatrix3fvARB);
		GET_PROC_ADDRESS(UniformMatrix4fvARB);
		GET_PROC_ADDRESS(UseProgramObjectARB);
		GET_PROC_ADDRESS(ValidateProgramARB);

	#endif // GL_ARB_shader_objects_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_shading_language_100 ]---------------------------------------------------------------------------

//!	Init GL_ARB_shading_language_100.

bool CRenderingContext::InitArbShadingLanguage100()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_shading_language_100_OGLEXT


	#endif // GL_ARB_shading_language_100_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_shadow ]-----------------------------------------------------------------------------------------

//!	Init GL_ARB_shadow.

bool CRenderingContext::InitArbShadow()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_shadow_OGLEXT


	#endif // GL_ARB_shadow_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_shadow_ambient ]---------------------------------------------------------------------------------

//!	Init GL_ARB_shadow_ambient.

bool CRenderingContext::InitArbShadowAmbient()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_shadow_ambient_OGLEXT


	#endif // GL_ARB_shadow_ambient_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_border_clamp ]---------------------------------------------------------------------------

//!	Init GL_ARB_texture_border_clamp.

bool CRenderingContext::InitArbTextureBorderClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_border_clamp_OGLEXT


	#endif // GL_ARB_texture_border_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_compression ]----------------------------------------------------------------------------

//!	Init GL_ARB_texture_compression.

bool CRenderingContext::InitArbTextureCompression()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_compression_OGLEXT

		GET_PROC_ADDRESS(CompressedTexImage1DARB);
		GET_PROC_ADDRESS(CompressedTexImage2DARB);
		GET_PROC_ADDRESS(CompressedTexImage3DARB);
		GET_PROC_ADDRESS(CompressedTexSubImage1DARB);
		GET_PROC_ADDRESS(CompressedTexSubImage2DARB);
		GET_PROC_ADDRESS(CompressedTexSubImage3DARB);
		GET_PROC_ADDRESS(GetCompressedTexImageARB);

	#endif // GL_ARB_texture_compression_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_cube_map ]-------------------------------------------------------------------------------

//!	Init GL_ARB_texture_cube_map.

bool CRenderingContext::InitArbTextureCubeMap()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_cube_map_OGLEXT


	#endif // GL_ARB_texture_cube_map_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_env_add ]--------------------------------------------------------------------------------

//!	Init GL_ARB_texture_env_add.

bool CRenderingContext::InitArbTextureEnvAdd()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_env_add_OGLEXT


	#endif // GL_ARB_texture_env_add_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_env_combine ]----------------------------------------------------------------------------

//!	Init GL_ARB_texture_env_combine.

bool CRenderingContext::InitArbTextureEnvCombine()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_env_combine_OGLEXT


	#endif // GL_ARB_texture_env_combine_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_env_crossbar ]---------------------------------------------------------------------------

//!	Init GL_ARB_texture_env_crossbar.

bool CRenderingContext::InitArbTextureEnvCrossbar()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_env_crossbar_OGLEXT


	#endif // GL_ARB_texture_env_crossbar_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_env_dot3 ]-------------------------------------------------------------------------------

//!	Init GL_ARB_texture_env_dot3.

bool CRenderingContext::InitArbTextureEnvDot3()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_env_dot3_OGLEXT


	#endif // GL_ARB_texture_env_dot3_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_mirror_repeat ]--------------------------------------------------------------------------

//!	Init GL_ARB_texture_mirror_repeat.

bool CRenderingContext::InitArbTextureMirrorRepeat()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_mirror_repeat_OGLEXT


	#endif // GL_ARB_texture_mirror_repeat_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_texture_non_power_of_two ]-----------------------------------------------------------------------

//!	Init GL_ARB_texture_non_power_of_two.

bool CRenderingContext::InitArbTextureNonPowerOfTwo()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_texture_non_power_of_two_OGLEXT


	#endif // GL_ARB_texture_non_power_of_two_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_transpose_matrix ]-------------------------------------------------------------------------------

//!	Init GL_ARB_transpose_matrix.

bool CRenderingContext::InitArbTransposeMatrix()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_transpose_matrix_OGLEXT

		GET_PROC_ADDRESS(LoadTransposeMatrixdARB);
		GET_PROC_ADDRESS(LoadTransposeMatrixfARB);
		GET_PROC_ADDRESS(MultTransposeMatrixdARB);
		GET_PROC_ADDRESS(MultTransposeMatrixfARB);

	#endif // GL_ARB_transpose_matrix_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_blend ]-----------------------------------------------------------------------------------

//!	Init GL_ARB_vertex_blend.

bool CRenderingContext::InitArbVertexBlend()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_blend_OGLEXT

		GET_PROC_ADDRESS(VertexBlendARB);
		GET_PROC_ADDRESS(WeightbvARB);
		GET_PROC_ADDRESS(WeightdvARB);
		GET_PROC_ADDRESS(WeightfvARB);
		GET_PROC_ADDRESS(WeightivARB);
		GET_PROC_ADDRESS(WeightPointerARB);
		GET_PROC_ADDRESS(WeightsvARB);
		GET_PROC_ADDRESS(WeightubvARB);
		GET_PROC_ADDRESS(WeightuivARB);
		GET_PROC_ADDRESS(WeightusvARB);

	#endif // GL_ARB_vertex_blend_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_buffer_object ]---------------------------------------------------------------------------

//!	Init GL_ARB_vertex_buffer_object.

bool CRenderingContext::InitArbVertexBufferObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_buffer_object_OGLEXT

		GET_PROC_ADDRESS(BindBufferARB);
		GET_PROC_ADDRESS(BufferDataARB);
		GET_PROC_ADDRESS(BufferSubDataARB);
		GET_PROC_ADDRESS(DeleteBuffersARB);
		GET_PROC_ADDRESS(GenBuffersARB);
		GET_PROC_ADDRESS(GetBufferParameterivARB);
		GET_PROC_ADDRESS(GetBufferPointervARB);
		GET_PROC_ADDRESS(GetBufferSubDataARB);
		GET_PROC_ADDRESS(IsBufferARB);
		GET_PROC_ADDRESS(MapBufferARB);
		GET_PROC_ADDRESS(UnmapBufferARB);

	#endif // GL_ARB_vertex_buffer_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_vertex_program ]---------------------------------------------------------------------------------

//!	Init GL_ARB_vertex_program.

bool CRenderingContext::InitArbVertexProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_vertex_program_OGLEXT

		GET_PROC_ADDRESS(BindProgramARB);
		GET_PROC_ADDRESS(DeleteProgramsARB);
		GET_PROC_ADDRESS(DisableVertexAttribArrayARB);
		GET_PROC_ADDRESS(EnableVertexAttribArrayARB);
		GET_PROC_ADDRESS(GenProgramsARB);
		GET_PROC_ADDRESS(GetProgramEnvParameterdvARB);
		GET_PROC_ADDRESS(GetProgramEnvParameterfvARB);
		GET_PROC_ADDRESS(GetProgramivARB);
		GET_PROC_ADDRESS(GetProgramLocalParameterdvARB);
		GET_PROC_ADDRESS(GetProgramLocalParameterfvARB);
		GET_PROC_ADDRESS(GetProgramStringARB);
		GET_PROC_ADDRESS(GetVertexAttribdvARB);
		GET_PROC_ADDRESS(GetVertexAttribfvARB);
		GET_PROC_ADDRESS(GetVertexAttribivARB);
		GET_PROC_ADDRESS(GetVertexAttribPointervARB);
		GET_PROC_ADDRESS(IsProgramARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4dARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4dvARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4fARB);
		GET_PROC_ADDRESS(ProgramEnvParameter4fvARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4dARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4dvARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4fARB);
		GET_PROC_ADDRESS(ProgramLocalParameter4fvARB);
		GET_PROC_ADDRESS(ProgramStringARB);
		GET_PROC_ADDRESS(VertexAttrib1dARB);
		GET_PROC_ADDRESS(VertexAttrib1dvARB);
		GET_PROC_ADDRESS(VertexAttrib1fARB);
		GET_PROC_ADDRESS(VertexAttrib1fvARB);
		GET_PROC_ADDRESS(VertexAttrib1sARB);
		GET_PROC_ADDRESS(VertexAttrib1svARB);
		GET_PROC_ADDRESS(VertexAttrib2dARB);
		GET_PROC_ADDRESS(VertexAttrib2dvARB);
		GET_PROC_ADDRESS(VertexAttrib2fARB);
		GET_PROC_ADDRESS(VertexAttrib2fvARB);
		GET_PROC_ADDRESS(VertexAttrib2sARB);
		GET_PROC_ADDRESS(VertexAttrib2svARB);
		GET_PROC_ADDRESS(VertexAttrib3dARB);
		GET_PROC_ADDRESS(VertexAttrib3dvARB);
		GET_PROC_ADDRESS(VertexAttrib3fARB);
		GET_PROC_ADDRESS(VertexAttrib3fvARB);
		GET_PROC_ADDRESS(VertexAttrib3sARB);
		GET_PROC_ADDRESS(VertexAttrib3svARB);
		GET_PROC_ADDRESS(VertexAttrib4bvARB);
		GET_PROC_ADDRESS(VertexAttrib4dARB);
		GET_PROC_ADDRESS(VertexAttrib4dvARB);
		GET_PROC_ADDRESS(VertexAttrib4fARB);
		GET_PROC_ADDRESS(VertexAttrib4fvARB);
		GET_PROC_ADDRESS(VertexAttrib4ivARB);
		GET_PROC_ADDRESS(VertexAttrib4NbvARB);
		GET_PROC_ADDRESS(VertexAttrib4NivARB);
		GET_PROC_ADDRESS(VertexAttrib4NsvARB);
		GET_PROC_ADDRESS(VertexAttrib4NubARB);
		GET_PROC_ADDRESS(VertexAttrib4NubvARB);
		GET_PROC_ADDRESS(VertexAttrib4NuivARB);
		GET_PROC_ADDRESS(VertexAttrib4NusvARB);
		GET_PROC_ADDRESS(VertexAttrib4sARB);
		GET_PROC_ADDRESS(VertexAttrib4svARB);
		GET_PROC_ADDRESS(VertexAttrib4ubvARB);
		GET_PROC_ADDRESS(VertexAttrib4uivARB);
		GET_PROC_ADDRESS(VertexAttrib4usvARB);
		GET_PROC_ADDRESS(VertexAttribPointerARB);

	#endif // GL_ARB_vertex_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ARB_window_pos ]-------------------------------------------------------------------------------------

//!	Init GL_ARB_window_pos.

bool CRenderingContext::InitArbWindowPos()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ARB_window_pos_OGLEXT

		GET_PROC_ADDRESS(WindowPos2dARB);
		GET_PROC_ADDRESS(WindowPos2dvARB);
		GET_PROC_ADDRESS(WindowPos2fARB);
		GET_PROC_ADDRESS(WindowPos2fvARB);
		GET_PROC_ADDRESS(WindowPos2iARB);
		GET_PROC_ADDRESS(WindowPos2ivARB);
		GET_PROC_ADDRESS(WindowPos2sARB);
		GET_PROC_ADDRESS(WindowPos2svARB);
		GET_PROC_ADDRESS(WindowPos3dARB);
		GET_PROC_ADDRESS(WindowPos3dvARB);
		GET_PROC_ADDRESS(WindowPos3fARB);
		GET_PROC_ADDRESS(WindowPos3fvARB);
		GET_PROC_ADDRESS(WindowPos3iARB);
		GET_PROC_ADDRESS(WindowPos3ivARB);
		GET_PROC_ADDRESS(WindowPos3sARB);
		GET_PROC_ADDRESS(WindowPos3svARB);

	#endif // GL_ARB_window_pos_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_draw_buffers ]-----------------------------------------------------------------------------------

//!	Init GL_ATI_draw_buffers.

bool CRenderingContext::InitAtiDrawBuffers()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_draw_buffers_OGLEXT

		GET_PROC_ADDRESS(DrawBuffersATI);

	#endif // GL_ATI_draw_buffers_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_element_array ]----------------------------------------------------------------------------------

//!	Init GL_ATI_element_array.

bool CRenderingContext::InitAtiElementArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_element_array_OGLEXT

		GET_PROC_ADDRESS(DrawElementArrayATI);
		GET_PROC_ADDRESS(DrawRangeElementArrayATI);
		GET_PROC_ADDRESS(ElementPointerATI);

	#endif // GL_ATI_element_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_envmap_bumpmap ]---------------------------------------------------------------------------------

//!	Init GL_ATI_envmap_bumpmap.

bool CRenderingContext::InitAtiEnvmapBumpmap()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_envmap_bumpmap_OGLEXT

		GET_PROC_ADDRESS(GetTexBumpParameterfvATI);
		GET_PROC_ADDRESS(GetTexBumpParameterivATI);
		GET_PROC_ADDRESS(TexBumpParameterfvATI);
		GET_PROC_ADDRESS(TexBumpParameterivATI);

	#endif // GL_ATI_envmap_bumpmap_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_fragment_shader ]--------------------------------------------------------------------------------

//!	Init GL_ATI_fragment_shader.

bool CRenderingContext::InitAtiFragmentShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_fragment_shader_OGLEXT

		GET_PROC_ADDRESS(AlphaFragmentOp1ATI);
		GET_PROC_ADDRESS(AlphaFragmentOp2ATI);
		GET_PROC_ADDRESS(AlphaFragmentOp3ATI);
		GET_PROC_ADDRESS(BeginFragmentShaderATI);
		GET_PROC_ADDRESS(BindFragmentShaderATI);
		GET_PROC_ADDRESS(ColorFragmentOp1ATI);
		GET_PROC_ADDRESS(ColorFragmentOp2ATI);
		GET_PROC_ADDRESS(ColorFragmentOp3ATI);
		GET_PROC_ADDRESS(DeleteFragmentShaderATI);
		GET_PROC_ADDRESS(EndFragmentShaderATI);
		GET_PROC_ADDRESS(GenFragmentShadersATI);
		GET_PROC_ADDRESS(PassTexCoordATI);
		GET_PROC_ADDRESS(SampleMapATI);
		GET_PROC_ADDRESS(SetFragmentShaderConstantATI);

	#endif // GL_ATI_fragment_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_map_object_buffer ]------------------------------------------------------------------------------

//!	Init GL_ATI_map_object_buffer.

bool CRenderingContext::InitAtiMapObjectBuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_map_object_buffer_OGLEXT

		GET_PROC_ADDRESS(MapObjectBufferATI);
		GET_PROC_ADDRESS(UnmapObjectBufferATI);

	#endif // GL_ATI_map_object_buffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_pn_triangles ]-----------------------------------------------------------------------------------

//!	Init GL_ATI_pn_triangles.

bool CRenderingContext::InitAtiPnTriangles()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_pn_triangles_OGLEXT

		GET_PROC_ADDRESS(PNTrianglesfATI);
		GET_PROC_ADDRESS(PNTrianglesiATI);

	#endif // GL_ATI_pn_triangles_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_separate_stencil ]-------------------------------------------------------------------------------

//!	Init GL_ATI_separate_stencil.

bool CRenderingContext::InitAtiSeparateStencil()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_separate_stencil_OGLEXT

		GET_PROC_ADDRESS(StencilFuncSeparateATI);
		GET_PROC_ADDRESS(StencilOpSeparateATI);

	#endif // GL_ATI_separate_stencil_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_texture_env_combine3 ]---------------------------------------------------------------------------

//!	Init GL_ATI_texture_env_combine3.

bool CRenderingContext::InitAtiTextureEnvCombine3()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_texture_env_combine3_OGLEXT


	#endif // GL_ATI_texture_env_combine3_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_texture_float ]----------------------------------------------------------------------------------

//!	Init GL_ATI_texture_float.

bool CRenderingContext::InitAtiTextureFloat()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_texture_float_OGLEXT


	#endif // GL_ATI_texture_float_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_texture_mirror_once ]----------------------------------------------------------------------------

//!	Init GL_ATI_texture_mirror_once.

bool CRenderingContext::InitAtiTextureMirrorOnce()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_texture_mirror_once_OGLEXT


	#endif // GL_ATI_texture_mirror_once_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_text_fragment_shader ]---------------------------------------------------------------------------

//!	Init GL_ATI_text_fragment_shader.

bool CRenderingContext::InitAtiTextFragmentShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_text_fragment_shader_OGLEXT


	#endif // GL_ATI_text_fragment_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_vertex_array_object ]----------------------------------------------------------------------------

//!	Init GL_ATI_vertex_array_object.

bool CRenderingContext::InitAtiVertexArrayObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_vertex_array_object_OGLEXT

		GET_PROC_ADDRESS(ArrayObjectATI);
		GET_PROC_ADDRESS(FreeObjectBufferATI);
		GET_PROC_ADDRESS(GetArrayObjectfvATI);
		GET_PROC_ADDRESS(GetArrayObjectivATI);
		GET_PROC_ADDRESS(GetObjectBufferfvATI);
		GET_PROC_ADDRESS(GetObjectBufferivATI);
		GET_PROC_ADDRESS(GetVariantArrayObjectfvATI);
		GET_PROC_ADDRESS(GetVariantArrayObjectivATI);
		GET_PROC_ADDRESS(IsObjectBufferATI);
		GET_PROC_ADDRESS(NewObjectBufferATI);
		GET_PROC_ADDRESS(UpdateObjectBufferATI);
		GET_PROC_ADDRESS(VariantArrayObjectATI);

	#endif // GL_ATI_vertex_array_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_vertex_attrib_array_object ]---------------------------------------------------------------------

//!	Init GL_ATI_vertex_attrib_array_object.

bool CRenderingContext::InitAtiVertexAttribArrayObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_vertex_attrib_array_object_OGLEXT

		GET_PROC_ADDRESS(GetVertexAttribArrayObjectfvATI);
		GET_PROC_ADDRESS(GetVertexAttribArrayObjectivATI);
		GET_PROC_ADDRESS(VertexAttribArrayObjectATI);

	#endif // GL_ATI_vertex_attrib_array_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_ATI_vertex_streams ]---------------------------------------------------------------------------------

//!	Init GL_ATI_vertex_streams.

bool CRenderingContext::InitAtiVertexStreams()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_ATI_vertex_streams_OGLEXT

		GET_PROC_ADDRESS(ClientActiveVertexStreamATI);
		GET_PROC_ADDRESS(NormalStream3bATI);
		GET_PROC_ADDRESS(NormalStream3bvATI);
		GET_PROC_ADDRESS(NormalStream3dATI);
		GET_PROC_ADDRESS(NormalStream3dvATI);
		GET_PROC_ADDRESS(NormalStream3fATI);
		GET_PROC_ADDRESS(NormalStream3fvATI);
		GET_PROC_ADDRESS(NormalStream3iATI);
		GET_PROC_ADDRESS(NormalStream3ivATI);
		GET_PROC_ADDRESS(NormalStream3sATI);
		GET_PROC_ADDRESS(NormalStream3svATI);
		GET_PROC_ADDRESS(VertexBlendEnvfATI);
		GET_PROC_ADDRESS(VertexBlendEnviATI);
		GET_PROC_ADDRESS(VertexStream1dATI);
		GET_PROC_ADDRESS(VertexStream1dvATI);
		GET_PROC_ADDRESS(VertexStream1fATI);
		GET_PROC_ADDRESS(VertexStream1fvATI);
		GET_PROC_ADDRESS(VertexStream1iATI);
		GET_PROC_ADDRESS(VertexStream1ivATI);
		GET_PROC_ADDRESS(VertexStream1sATI);
		GET_PROC_ADDRESS(VertexStream1svATI);
		GET_PROC_ADDRESS(VertexStream2dATI);
		GET_PROC_ADDRESS(VertexStream2dvATI);
		GET_PROC_ADDRESS(VertexStream2fATI);
		GET_PROC_ADDRESS(VertexStream2fvATI);
		GET_PROC_ADDRESS(VertexStream2iATI);
		GET_PROC_ADDRESS(VertexStream2ivATI);
		GET_PROC_ADDRESS(VertexStream2sATI);
		GET_PROC_ADDRESS(VertexStream2svATI);
		GET_PROC_ADDRESS(VertexStream3dATI);
		GET_PROC_ADDRESS(VertexStream3dvATI);
		GET_PROC_ADDRESS(VertexStream3fATI);
		GET_PROC_ADDRESS(VertexStream3fvATI);
		GET_PROC_ADDRESS(VertexStream3iATI);
		GET_PROC_ADDRESS(VertexStream3ivATI);
		GET_PROC_ADDRESS(VertexStream3sATI);
		GET_PROC_ADDRESS(VertexStream3svATI);
		GET_PROC_ADDRESS(VertexStream4dATI);
		GET_PROC_ADDRESS(VertexStream4dvATI);
		GET_PROC_ADDRESS(VertexStream4fATI);
		GET_PROC_ADDRESS(VertexStream4fvATI);
		GET_PROC_ADDRESS(VertexStream4iATI);
		GET_PROC_ADDRESS(VertexStream4ivATI);
		GET_PROC_ADDRESS(VertexStream4sATI);
		GET_PROC_ADDRESS(VertexStream4svATI);

	#endif // GL_ATI_vertex_streams_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_422_pixels ]-------------------------------------------------------------------------------------

//!	Init GL_EXT_422_pixels.

bool CRenderingContext::InitExt422Pixels()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_422_pixels_OGLEXT


	#endif // GL_EXT_422_pixels_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_abgr ]-------------------------------------------------------------------------------------------

//!	Init GL_EXT_abgr.

bool CRenderingContext::InitExtAbgr()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_abgr_OGLEXT


	#endif // GL_EXT_abgr_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_bgra ]-------------------------------------------------------------------------------------------

//!	Init GL_EXT_bgra.

bool CRenderingContext::InitExtBgra()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_bgra_OGLEXT


	#endif // GL_EXT_bgra_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_color ]------------------------------------------------------------------------------------

//!	Init GL_EXT_blend_color.

bool CRenderingContext::InitExtBlendColor()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_color_OGLEXT

		GET_PROC_ADDRESS(BlendColorEXT);

	#endif // GL_EXT_blend_color_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_func_separate ]----------------------------------------------------------------------------

//!	Init GL_EXT_blend_func_separate.

bool CRenderingContext::InitExtBlendFuncSeparate()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_func_separate_OGLEXT

		GET_PROC_ADDRESS(BlendFuncSeparateEXT);

	#endif // GL_EXT_blend_func_separate_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_logic_op ]---------------------------------------------------------------------------------

//!	Init GL_EXT_blend_logic_op.

bool CRenderingContext::InitExtBlendLogicOp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_logic_op_OGLEXT


	#endif // GL_EXT_blend_logic_op_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_minmax ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_blend_minmax.

bool CRenderingContext::InitExtBlendMinmax()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_minmax_OGLEXT

		GET_PROC_ADDRESS(BlendEquationEXT);

	#endif // GL_EXT_blend_minmax_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_blend_subtract ]---------------------------------------------------------------------------------

//!	Init GL_EXT_blend_subtract.

bool CRenderingContext::InitExtBlendSubtract()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_blend_subtract_OGLEXT


	#endif // GL_EXT_blend_subtract_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_clip_volume_hint ]-------------------------------------------------------------------------------

//!	Init GL_EXT_clip_volume_hint.

bool CRenderingContext::InitExtClipVolumeHint()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_clip_volume_hint_OGLEXT


	#endif // GL_EXT_clip_volume_hint_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_cmyka ]------------------------------------------------------------------------------------------

//!	Init GL_EXT_cmyka.

bool CRenderingContext::InitExtCmyka()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_cmyka_OGLEXT


	#endif // GL_EXT_cmyka_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_color_matrix ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_color_matrix.

bool CRenderingContext::InitExtColorMatrix()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_color_matrix_OGLEXT


	#endif // GL_EXT_color_matrix_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_color_subtable ]---------------------------------------------------------------------------------

//!	Init GL_EXT_color_subtable.

bool CRenderingContext::InitExtColorSubtable()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_color_subtable_OGLEXT

		GET_PROC_ADDRESS(ColorSubTableEXT);
		GET_PROC_ADDRESS(CopyColorSubTableEXT);

	#endif // GL_EXT_color_subtable_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_compiled_vertex_array ]--------------------------------------------------------------------------

//!	Init GL_EXT_compiled_vertex_array.

bool CRenderingContext::InitExtCompiledVertexArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_compiled_vertex_array_OGLEXT

		GET_PROC_ADDRESS(LockArraysEXT);
		GET_PROC_ADDRESS(UnlockArraysEXT);

	#endif // GL_EXT_compiled_vertex_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_convolution ]------------------------------------------------------------------------------------

//!	Init GL_EXT_convolution.

bool CRenderingContext::InitExtConvolution()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_convolution_OGLEXT

		GET_PROC_ADDRESS(ConvolutionFilter1DEXT);
		GET_PROC_ADDRESS(ConvolutionFilter2DEXT);
		GET_PROC_ADDRESS(ConvolutionParameterfEXT);
		GET_PROC_ADDRESS(ConvolutionParameterfvEXT);
		GET_PROC_ADDRESS(ConvolutionParameteriEXT);
		GET_PROC_ADDRESS(ConvolutionParameterivEXT);
		GET_PROC_ADDRESS(CopyConvolutionFilter1DEXT);
		GET_PROC_ADDRESS(CopyConvolutionFilter2DEXT);
		GET_PROC_ADDRESS(GetConvolutionFilterEXT);
		GET_PROC_ADDRESS(GetConvolutionParameterfvEXT);
		GET_PROC_ADDRESS(GetConvolutionParameterivEXT);
		GET_PROC_ADDRESS(GetSeparableFilterEXT);
		GET_PROC_ADDRESS(SeparableFilter2DEXT);

	#endif // GL_EXT_convolution_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_coordinate_frame ]-------------------------------------------------------------------------------

//!	Init GL_EXT_coordinate_frame.

bool CRenderingContext::InitExtCoordinateFrame()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_coordinate_frame_OGLEXT

		GET_PROC_ADDRESS(Binormal3bEXT);
		GET_PROC_ADDRESS(Binormal3bvEXT);
		GET_PROC_ADDRESS(Binormal3dEXT);
		GET_PROC_ADDRESS(Binormal3dvEXT);
		GET_PROC_ADDRESS(Binormal3fEXT);
		GET_PROC_ADDRESS(Binormal3fvEXT);
		GET_PROC_ADDRESS(Binormal3iEXT);
		GET_PROC_ADDRESS(Binormal3ivEXT);
		GET_PROC_ADDRESS(Binormal3sEXT);
		GET_PROC_ADDRESS(Binormal3svEXT);
		GET_PROC_ADDRESS(BinormalPointerEXT);
		GET_PROC_ADDRESS(Tangent3bEXT);
		GET_PROC_ADDRESS(Tangent3bvEXT);
		GET_PROC_ADDRESS(Tangent3dEXT);
		GET_PROC_ADDRESS(Tangent3dvEXT);
		GET_PROC_ADDRESS(Tangent3fEXT);
		GET_PROC_ADDRESS(Tangent3fvEXT);
		GET_PROC_ADDRESS(Tangent3iEXT);
		GET_PROC_ADDRESS(Tangent3ivEXT);
		GET_PROC_ADDRESS(Tangent3sEXT);
		GET_PROC_ADDRESS(Tangent3svEXT);
		GET_PROC_ADDRESS(TangentPointerEXT);

	#endif // GL_EXT_coordinate_frame_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_copy_texture ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_copy_texture.

bool CRenderingContext::InitExtCopyTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_copy_texture_OGLEXT

		GET_PROC_ADDRESS(CopyTexImage1DEXT);
		GET_PROC_ADDRESS(CopyTexImage2DEXT);
		GET_PROC_ADDRESS(CopyTexSubImage1DEXT);
		GET_PROC_ADDRESS(CopyTexSubImage2DEXT);
		GET_PROC_ADDRESS(CopyTexSubImage3DEXT);

	#endif // GL_EXT_copy_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_cull_vertex ]------------------------------------------------------------------------------------

//!	Init GL_EXT_cull_vertex.

bool CRenderingContext::InitExtCullVertex()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_cull_vertex_OGLEXT

		GET_PROC_ADDRESS(CullParameterdvEXT);
		GET_PROC_ADDRESS(CullParameterfvEXT);

	#endif // GL_EXT_cull_vertex_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_depth_bounds_test ]------------------------------------------------------------------------------

//!	Init GL_EXT_depth_bounds_test.

bool CRenderingContext::InitExtDepthBoundsTest()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_depth_bounds_test_OGLEXT


	#endif // GL_EXT_depth_bounds_test_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_draw_range_elements ]----------------------------------------------------------------------------

//!	Init GL_EXT_draw_range_elements.

bool CRenderingContext::InitExtDrawRangeElements()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_draw_range_elements_OGLEXT

		GET_PROC_ADDRESS(DrawRangeElementsEXT);

	#endif // GL_EXT_draw_range_elements_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_fog_coord ]--------------------------------------------------------------------------------------

//!	Init GL_EXT_fog_coord.

bool CRenderingContext::InitExtFogCoord()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_fog_coord_OGLEXT

		GET_PROC_ADDRESS(FogCoorddEXT);
		GET_PROC_ADDRESS(FogCoorddvEXT);
		GET_PROC_ADDRESS(FogCoordfEXT);
		GET_PROC_ADDRESS(FogCoordfvEXT);
		GET_PROC_ADDRESS(FogCoordPointerEXT);

	#endif // GL_EXT_fog_coord_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_fragment_lighting ]------------------------------------------------------------------------------

//!	Init GL_EXT_fragment_lighting.

bool CRenderingContext::InitExtFragmentLighting()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_fragment_lighting_OGLEXT


	#endif // GL_EXT_fragment_lighting_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_histogram ]--------------------------------------------------------------------------------------

//!	Init GL_EXT_histogram.

bool CRenderingContext::InitExtHistogram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_histogram_OGLEXT

		GET_PROC_ADDRESS(GetHistogramEXT);
		GET_PROC_ADDRESS(GetHistogramParameterfvEXT);
		GET_PROC_ADDRESS(GetHistogramParameterivEXT);
		GET_PROC_ADDRESS(GetMinmaxEXT);
		GET_PROC_ADDRESS(GetMinmaxParameterfvEXT);
		GET_PROC_ADDRESS(GetMinmaxParameterivEXT);
		GET_PROC_ADDRESS(HistogramEXT);
		GET_PROC_ADDRESS(MinmaxEXT);
		GET_PROC_ADDRESS(ResetHistogramEXT);
		GET_PROC_ADDRESS(ResetMinmaxEXT);

	#endif // GL_EXT_histogram_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_index_array_formats ]----------------------------------------------------------------------------

//!	Init GL_EXT_index_array_formats.

bool CRenderingContext::InitExtIndexArrayFormats()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_index_array_formats_OGLEXT


	#endif // GL_EXT_index_array_formats_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_index_func ]-------------------------------------------------------------------------------------

//!	Init GL_EXT_index_func.

bool CRenderingContext::InitExtIndexFunc()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_index_func_OGLEXT

		GET_PROC_ADDRESS(IndexFuncEXT);

	#endif // GL_EXT_index_func_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_index_material ]---------------------------------------------------------------------------------

//!	Init GL_EXT_index_material.

bool CRenderingContext::InitExtIndexMaterial()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_index_material_OGLEXT

		GET_PROC_ADDRESS(IndexMaterialEXT);

	#endif // GL_EXT_index_material_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_index_texture ]----------------------------------------------------------------------------------

//!	Init GL_EXT_index_texture.

bool CRenderingContext::InitExtIndexTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_index_texture_OGLEXT


	#endif // GL_EXT_index_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_light_texture ]----------------------------------------------------------------------------------

//!	Init GL_EXT_light_texture.

bool CRenderingContext::InitExtLightTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_light_texture_OGLEXT

		GET_PROC_ADDRESS(ApplyTextureEXT);
		GET_PROC_ADDRESS(TextureLightEXT);
		GET_PROC_ADDRESS(TextureMaterialEXT);

	#endif // GL_EXT_light_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_misc_attribute ]---------------------------------------------------------------------------------

//!	Init GL_EXT_misc_attribute.

bool CRenderingContext::InitExtMiscAttribute()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_misc_attribute_OGLEXT


	#endif // GL_EXT_misc_attribute_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_multisample ]------------------------------------------------------------------------------------

//!	Init GL_EXT_multisample.

bool CRenderingContext::InitExtMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_multisample_OGLEXT

		GET_PROC_ADDRESS(SampleMaskEXT);
		GET_PROC_ADDRESS(SamplePatternEXT);

	#endif // GL_EXT_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_multitexture ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_multitexture.

bool CRenderingContext::InitExtMultitexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_multitexture_OGLEXT

		GET_PROC_ADDRESS(InterleavedTextureCoordSetsEXT);
		GET_PROC_ADDRESS(MultiTexCoord1dEXT);
		GET_PROC_ADDRESS(MultiTexCoord1dvEXT);
		GET_PROC_ADDRESS(MultiTexCoord1fEXT);
		GET_PROC_ADDRESS(MultiTexCoord1fvEXT);
		GET_PROC_ADDRESS(MultiTexCoord1iEXT);
		GET_PROC_ADDRESS(MultiTexCoord1ivEXT);
		GET_PROC_ADDRESS(MultiTexCoord1sEXT);
		GET_PROC_ADDRESS(MultiTexCoord1svEXT);
		GET_PROC_ADDRESS(MultiTexCoord2dEXT);
		GET_PROC_ADDRESS(MultiTexCoord2dvEXT);
		GET_PROC_ADDRESS(MultiTexCoord2fEXT);
		GET_PROC_ADDRESS(MultiTexCoord2fvEXT);
		GET_PROC_ADDRESS(MultiTexCoord2iEXT);
		GET_PROC_ADDRESS(MultiTexCoord2ivEXT);
		GET_PROC_ADDRESS(MultiTexCoord2sEXT);
		GET_PROC_ADDRESS(MultiTexCoord2svEXT);
		GET_PROC_ADDRESS(MultiTexCoord3dEXT);
		GET_PROC_ADDRESS(MultiTexCoord3dvEXT);
		GET_PROC_ADDRESS(MultiTexCoord3fEXT);
		GET_PROC_ADDRESS(MultiTexCoord3fvEXT);
		GET_PROC_ADDRESS(MultiTexCoord3iEXT);
		GET_PROC_ADDRESS(MultiTexCoord3ivEXT);
		GET_PROC_ADDRESS(MultiTexCoord3sEXT);
		GET_PROC_ADDRESS(MultiTexCoord3svEXT);
		GET_PROC_ADDRESS(MultiTexCoord4dEXT);
		GET_PROC_ADDRESS(MultiTexCoord4dvEXT);
		GET_PROC_ADDRESS(MultiTexCoord4fEXT);
		GET_PROC_ADDRESS(MultiTexCoord4fvEXT);
		GET_PROC_ADDRESS(MultiTexCoord4iEXT);
		GET_PROC_ADDRESS(MultiTexCoord4ivEXT);
		GET_PROC_ADDRESS(MultiTexCoord4sEXT);
		GET_PROC_ADDRESS(MultiTexCoord4svEXT);
		GET_PROC_ADDRESS(SelectTextureCoordSetEXT);
		GET_PROC_ADDRESS(SelectTextureEXT);
		GET_PROC_ADDRESS(SelectTextureTransformEXT);

	#endif // GL_EXT_multitexture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_multi_draw_arrays ]------------------------------------------------------------------------------

//!	Init GL_EXT_multi_draw_arrays.

bool CRenderingContext::InitExtMultiDrawArrays()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_multi_draw_arrays_OGLEXT

		GET_PROC_ADDRESS(MultiDrawArraysEXT);
		GET_PROC_ADDRESS(MultiDrawElementsEXT);

	#endif // GL_EXT_multi_draw_arrays_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_packed_pixels ]----------------------------------------------------------------------------------

//!	Init GL_EXT_packed_pixels.

bool CRenderingContext::InitExtPackedPixels()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_packed_pixels_OGLEXT


	#endif // GL_EXT_packed_pixels_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_paletted_texture ]-------------------------------------------------------------------------------

//!	Init GL_EXT_paletted_texture.

bool CRenderingContext::InitExtPalettedTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_paletted_texture_OGLEXT

		GET_PROC_ADDRESS(ColorTableEXT);
		GET_PROC_ADDRESS(GetColorTableEXT);
		GET_PROC_ADDRESS(GetColorTableParameterfvEXT);
		GET_PROC_ADDRESS(GetColorTableParameterivEXT);

	#endif // GL_EXT_paletted_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_pixel_transform ]--------------------------------------------------------------------------------

//!	Init GL_EXT_pixel_transform.

bool CRenderingContext::InitExtPixelTransform()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_pixel_transform_OGLEXT

		GET_PROC_ADDRESS(PixelTransformParameterfEXT);
		GET_PROC_ADDRESS(PixelTransformParameterfvEXT);
		GET_PROC_ADDRESS(PixelTransformParameteriEXT);
		GET_PROC_ADDRESS(PixelTransformParameterivEXT);

	#endif // GL_EXT_pixel_transform_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_pixel_transform_color_table ]--------------------------------------------------------------------

//!	Init GL_EXT_pixel_transform_color_table.

bool CRenderingContext::InitExtPixelTransformColorTable()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_pixel_transform_color_table_OGLEXT


	#endif // GL_EXT_pixel_transform_color_table_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_point_parameters ]-------------------------------------------------------------------------------

//!	Init GL_EXT_point_parameters.

bool CRenderingContext::InitExtPointParameters()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_point_parameters_OGLEXT

		GET_PROC_ADDRESS(PointParameterfEXT);
		GET_PROC_ADDRESS(PointParameterfvEXT);

	#endif // GL_EXT_point_parameters_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_polygon_offset ]---------------------------------------------------------------------------------

//!	Init GL_EXT_polygon_offset.

bool CRenderingContext::InitExtPolygonOffset()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_polygon_offset_OGLEXT

		GET_PROC_ADDRESS(PolygonOffsetEXT);

	#endif // GL_EXT_polygon_offset_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_rescale_normal ]---------------------------------------------------------------------------------

//!	Init GL_EXT_rescale_normal.

bool CRenderingContext::InitExtRescaleNormal()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_rescale_normal_OGLEXT


	#endif // GL_EXT_rescale_normal_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_secondary_color ]--------------------------------------------------------------------------------

//!	Init GL_EXT_secondary_color.

bool CRenderingContext::InitExtSecondaryColor()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_secondary_color_OGLEXT

		GET_PROC_ADDRESS(SecondaryColor3bEXT);
		GET_PROC_ADDRESS(SecondaryColor3bvEXT);
		GET_PROC_ADDRESS(SecondaryColor3dEXT);
		GET_PROC_ADDRESS(SecondaryColor3dvEXT);
		GET_PROC_ADDRESS(SecondaryColor3fEXT);
		GET_PROC_ADDRESS(SecondaryColor3fvEXT);
		GET_PROC_ADDRESS(SecondaryColor3iEXT);
		GET_PROC_ADDRESS(SecondaryColor3ivEXT);
		GET_PROC_ADDRESS(SecondaryColor3sEXT);
		GET_PROC_ADDRESS(SecondaryColor3svEXT);
		GET_PROC_ADDRESS(SecondaryColor3ubEXT);
		GET_PROC_ADDRESS(SecondaryColor3ubvEXT);
		GET_PROC_ADDRESS(SecondaryColor3uiEXT);
		GET_PROC_ADDRESS(SecondaryColor3uivEXT);
		GET_PROC_ADDRESS(SecondaryColor3usEXT);
		GET_PROC_ADDRESS(SecondaryColor3usvEXT);
		GET_PROC_ADDRESS(SecondaryColorPointerEXT);

	#endif // GL_EXT_secondary_color_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_separate_specular_color ]------------------------------------------------------------------------

//!	Init GL_EXT_separate_specular_color.

bool CRenderingContext::InitExtSeparateSpecularColor()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_separate_specular_color_OGLEXT


	#endif // GL_EXT_separate_specular_color_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_shadow_funcs ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_shadow_funcs.

bool CRenderingContext::InitExtShadowFuncs()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_shadow_funcs_OGLEXT


	#endif // GL_EXT_shadow_funcs_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_shared_texture_palette ]-------------------------------------------------------------------------

//!	Init GL_EXT_shared_texture_palette.

bool CRenderingContext::InitExtSharedTexturePalette()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_shared_texture_palette_OGLEXT


	#endif // GL_EXT_shared_texture_palette_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_stencil_two_side ]-------------------------------------------------------------------------------

//!	Init GL_EXT_stencil_two_side.

bool CRenderingContext::InitExtStencilTwoSide()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_stencil_two_side_OGLEXT

		GET_PROC_ADDRESS(ActiveStencilFaceEXT);

	#endif // GL_EXT_stencil_two_side_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_stencil_wrap ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_stencil_wrap.

bool CRenderingContext::InitExtStencilWrap()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_stencil_wrap_OGLEXT


	#endif // GL_EXT_stencil_wrap_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_subtexture ]-------------------------------------------------------------------------------------

//!	Init GL_EXT_subtexture.

bool CRenderingContext::InitExtSubtexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_subtexture_OGLEXT

		GET_PROC_ADDRESS(TexSubImage1DEXT);
		GET_PROC_ADDRESS(TexSubImage2DEXT);

	#endif // GL_EXT_subtexture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture ]----------------------------------------------------------------------------------------

//!	Init GL_EXT_texture.

bool CRenderingContext::InitExtTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_OGLEXT


	#endif // GL_EXT_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture3D ]--------------------------------------------------------------------------------------

//!	Init GL_EXT_texture3D.

bool CRenderingContext::InitExtTexture3D()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture3D_OGLEXT

		GET_PROC_ADDRESS(TexImage3DEXT);
		GET_PROC_ADDRESS(TexSubImage3DEXT);

	#endif // GL_EXT_texture3D_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_compression_s3tc ]-----------------------------------------------------------------------

//!	Init GL_EXT_texture_compression_s3tc.

bool CRenderingContext::InitExtTextureCompressionS3tc()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_compression_s3tc_OGLEXT


	#endif // GL_EXT_texture_compression_s3tc_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_env_add ]--------------------------------------------------------------------------------

//!	Init GL_EXT_texture_env_add.

bool CRenderingContext::InitExtTextureEnvAdd()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_env_add_OGLEXT


	#endif // GL_EXT_texture_env_add_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_env_combine ]----------------------------------------------------------------------------

//!	Init GL_EXT_texture_env_combine.

bool CRenderingContext::InitExtTextureEnvCombine()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_env_combine_OGLEXT


	#endif // GL_EXT_texture_env_combine_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_env_dot3 ]-------------------------------------------------------------------------------

//!	Init GL_EXT_texture_env_dot3.

bool CRenderingContext::InitExtTextureEnvDot3()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_env_dot3_OGLEXT


	#endif // GL_EXT_texture_env_dot3_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_filter_anisotropic ]---------------------------------------------------------------------

//!	Init GL_EXT_texture_filter_anisotropic.

bool CRenderingContext::InitExtTextureFilterAnisotropic()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_filter_anisotropic_OGLEXT


	#endif // GL_EXT_texture_filter_anisotropic_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_lod_bias ]-------------------------------------------------------------------------------

//!	Init GL_EXT_texture_lod_bias.

bool CRenderingContext::InitExtTextureLodBias()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_lod_bias_OGLEXT


	#endif // GL_EXT_texture_lod_bias_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_mirror_clamp ]---------------------------------------------------------------------------

//!	Init GL_EXT_texture_mirror_clamp.

bool CRenderingContext::InitExtTextureMirrorClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_mirror_clamp_OGLEXT


	#endif // GL_EXT_texture_mirror_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_object ]---------------------------------------------------------------------------------

//!	Init GL_EXT_texture_object.

bool CRenderingContext::InitExtTextureObject()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_object_OGLEXT

		GET_PROC_ADDRESS(AreTexturesResidentEXT);
		GET_PROC_ADDRESS(BindTextureEXT);
		GET_PROC_ADDRESS(DeleteTexturesEXT);
		GET_PROC_ADDRESS(GenTexturesEXT);
		GET_PROC_ADDRESS(IsTextureEXT);
		GET_PROC_ADDRESS(PrioritizeTexturesEXT);

	#endif // GL_EXT_texture_object_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_texture_perturb_normal ]-------------------------------------------------------------------------

//!	Init GL_EXT_texture_perturb_normal.

bool CRenderingContext::InitExtTexturePerturbNormal()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_texture_perturb_normal_OGLEXT

		GET_PROC_ADDRESS(TextureNormalEXT);

	#endif // GL_EXT_texture_perturb_normal_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_vertex_array ]-----------------------------------------------------------------------------------

//!	Init GL_EXT_vertex_array.

bool CRenderingContext::InitExtVertexArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_vertex_array_OGLEXT

		GET_PROC_ADDRESS(ArrayElementEXT);
		GET_PROC_ADDRESS(ColorPointerEXT);
		GET_PROC_ADDRESS(DrawArraysEXT);
		GET_PROC_ADDRESS(EdgeFlagPointerEXT);
		GET_PROC_ADDRESS(GetPointervEXT);
		GET_PROC_ADDRESS(IndexPointerEXT);
		GET_PROC_ADDRESS(NormalPointerEXT);
		GET_PROC_ADDRESS(TexCoordPointerEXT);
		GET_PROC_ADDRESS(VertexPointerEXT);

	#endif // GL_EXT_vertex_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_vertex_shader ]----------------------------------------------------------------------------------

//!	Init GL_EXT_vertex_shader.

bool CRenderingContext::InitExtVertexShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_vertex_shader_OGLEXT

		GET_PROC_ADDRESS(BeginVertexShaderEXT);
		GET_PROC_ADDRESS(BindLightParameterEXT);
		GET_PROC_ADDRESS(BindMaterialParameterEXT);
		GET_PROC_ADDRESS(BindParameterEXT);
		GET_PROC_ADDRESS(BindTexGenParameterEXT);
		GET_PROC_ADDRESS(BindTextureUnitParameterEXT);
		GET_PROC_ADDRESS(BindVertexShaderEXT);
		GET_PROC_ADDRESS(DeleteVertexShaderEXT);
		GET_PROC_ADDRESS(DisableVariantClientStateEXT);
		GET_PROC_ADDRESS(EnableVariantClientStateEXT);
		GET_PROC_ADDRESS(EndVertexShaderEXT);
		GET_PROC_ADDRESS(ExtractComponentEXT);
		GET_PROC_ADDRESS(GenSymbolsEXT);
		GET_PROC_ADDRESS(GenVertexShadersEXT);
		GET_PROC_ADDRESS(GetInvariantBooleanvEXT);
		GET_PROC_ADDRESS(GetInvariantFloatvEXT);
		GET_PROC_ADDRESS(GetInvariantIntegervEXT);
		GET_PROC_ADDRESS(GetLocalConstantBooleanvEXT);
		GET_PROC_ADDRESS(GetLocalConstantFloatvEXT);
		GET_PROC_ADDRESS(GetLocalConstantIntegervEXT);
		GET_PROC_ADDRESS(GetVariantBooleanvEXT);
		GET_PROC_ADDRESS(GetVariantFloatvEXT);
		GET_PROC_ADDRESS(GetVariantIntegervEXT);
		GET_PROC_ADDRESS(GetVariantPointervEXT);
		GET_PROC_ADDRESS(InsertComponentEXT);
		GET_PROC_ADDRESS(IsVariantEnabledEXT);
		GET_PROC_ADDRESS(SetInvariantEXT);
		GET_PROC_ADDRESS(SetLocalConstantEXT);
		GET_PROC_ADDRESS(ShaderOp1EXT);
		GET_PROC_ADDRESS(ShaderOp2EXT);
		GET_PROC_ADDRESS(ShaderOp3EXT);
		GET_PROC_ADDRESS(SwizzleEXT);
		GET_PROC_ADDRESS(VariantbvEXT);
		GET_PROC_ADDRESS(VariantdvEXT);
		GET_PROC_ADDRESS(VariantfvEXT);
		GET_PROC_ADDRESS(VariantivEXT);
		GET_PROC_ADDRESS(VariantPointerEXT);
		GET_PROC_ADDRESS(VariantsvEXT);
		GET_PROC_ADDRESS(VariantubvEXT);
		GET_PROC_ADDRESS(VariantuivEXT);
		GET_PROC_ADDRESS(VariantusvEXT);
		GET_PROC_ADDRESS(WriteMaskEXT);

	#endif // GL_EXT_vertex_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_EXT_vertex_weighting ]-------------------------------------------------------------------------------

//!	Init GL_EXT_vertex_weighting.

bool CRenderingContext::InitExtVertexWeighting()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_EXT_vertex_weighting_OGLEXT

		GET_PROC_ADDRESS(VertexWeightfEXT);
		GET_PROC_ADDRESS(VertexWeightfvEXT);
		GET_PROC_ADDRESS(VertexWeightPointerEXT);

	#endif // GL_EXT_vertex_weighting_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_HP_convolution_border_modes ]------------------------------------------------------------------------

//!	Init GL_HP_convolution_border_modes.

bool CRenderingContext::InitHpConvolutionBorderModes()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_HP_convolution_border_modes_OGLEXT


	#endif // GL_HP_convolution_border_modes_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_HP_image_transform ]---------------------------------------------------------------------------------

//!	Init GL_HP_image_transform.

bool CRenderingContext::InitHpImageTransform()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_HP_image_transform_OGLEXT

		GET_PROC_ADDRESS(GetImageTransformParameterfvHP);
		GET_PROC_ADDRESS(GetImageTransformParameterivHP);
		GET_PROC_ADDRESS(ImageTransformParameterfHP);
		GET_PROC_ADDRESS(ImageTransformParameterfvHP);
		GET_PROC_ADDRESS(ImageTransformParameteriHP);
		GET_PROC_ADDRESS(ImageTransformParameterivHP);

	#endif // GL_HP_image_transform_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_HP_occlusion_test ]----------------------------------------------------------------------------------

//!	Init GL_HP_occlusion_test.

bool CRenderingContext::InitHpOcclusionTest()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_HP_occlusion_test_OGLEXT


	#endif // GL_HP_occlusion_test_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_HP_texture_lighting ]--------------------------------------------------------------------------------

//!	Init GL_HP_texture_lighting.

bool CRenderingContext::InitHpTextureLighting()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_HP_texture_lighting_OGLEXT


	#endif // GL_HP_texture_lighting_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_cull_vertex ]------------------------------------------------------------------------------------

//!	Init GL_IBM_cull_vertex.

bool CRenderingContext::InitIbmCullVertex()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_cull_vertex_OGLEXT


	#endif // GL_IBM_cull_vertex_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_multimode_draw_arrays ]--------------------------------------------------------------------------

//!	Init GL_IBM_multimode_draw_arrays.

bool CRenderingContext::InitIbmMultimodeDrawArrays()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_multimode_draw_arrays_OGLEXT

		GET_PROC_ADDRESS(MultiModeDrawArraysIBM);
		GET_PROC_ADDRESS(MultiModeDrawElementsIBM);

	#endif // GL_IBM_multimode_draw_arrays_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_rasterpos_clip ]---------------------------------------------------------------------------------

//!	Init GL_IBM_rasterpos_clip.

bool CRenderingContext::InitIbmRasterposClip()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_rasterpos_clip_OGLEXT


	#endif // GL_IBM_rasterpos_clip_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_static_data ]------------------------------------------------------------------------------------

//!	Init GL_IBM_static_data.

bool CRenderingContext::InitIbmStaticData()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_static_data_OGLEXT


	#endif // GL_IBM_static_data_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_texture_mirrored_repeat ]------------------------------------------------------------------------

//!	Init GL_IBM_texture_mirrored_repeat.

bool CRenderingContext::InitIbmTextureMirroredRepeat()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_texture_mirrored_repeat_OGLEXT


	#endif // GL_IBM_texture_mirrored_repeat_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_IBM_vertex_array_lists ]-----------------------------------------------------------------------------

//!	Init GL_IBM_vertex_array_lists.

bool CRenderingContext::InitIbmVertexArrayLists()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_IBM_vertex_array_lists_OGLEXT

		GET_PROC_ADDRESS(ColorPointerListIBM);
		GET_PROC_ADDRESS(EdgeFlagPointerListIBM);
		GET_PROC_ADDRESS(FogCoordPointerListIBM);
		GET_PROC_ADDRESS(IndexPointerListIBM);
		GET_PROC_ADDRESS(NormalPointerListIBM);
		GET_PROC_ADDRESS(SecondaryColorPointerListIBM);
		GET_PROC_ADDRESS(TexCoordPointerListIBM);
		GET_PROC_ADDRESS(VertexPointerListIBM);

	#endif // GL_IBM_vertex_array_lists_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_INGR_blend_func_separate ]---------------------------------------------------------------------------

//!	Init GL_INGR_blend_func_separate.

bool CRenderingContext::InitIngrBlendFuncSeparate()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_INGR_blend_func_separate_OGLEXT

		GET_PROC_ADDRESS(BlendFuncSeparateINGR);

	#endif // GL_INGR_blend_func_separate_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_INGR_color_clamp ]-----------------------------------------------------------------------------------

//!	Init GL_INGR_color_clamp.

bool CRenderingContext::InitIngrColorClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_INGR_color_clamp_OGLEXT


	#endif // GL_INGR_color_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_INGR_interlace_read ]--------------------------------------------------------------------------------

//!	Init GL_INGR_interlace_read.

bool CRenderingContext::InitIngrInterlaceRead()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_INGR_interlace_read_OGLEXT


	#endif // GL_INGR_interlace_read_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_INTEL_parallel_arrays ]------------------------------------------------------------------------------

//!	Init GL_INTEL_parallel_arrays.

bool CRenderingContext::InitIntelParallelArrays()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_INTEL_parallel_arrays_OGLEXT

		GET_PROC_ADDRESS(ColorPointervINTEL);
		GET_PROC_ADDRESS(NormalPointervINTEL);
		GET_PROC_ADDRESS(TexCoordPointervINTEL);
		GET_PROC_ADDRESS(VertexPointervINTEL);

	#endif // GL_INTEL_parallel_arrays_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_MESA_resize_buffers ]--------------------------------------------------------------------------------

//!	Init GL_MESA_resize_buffers.

bool CRenderingContext::InitMesaResizeBuffers()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_MESA_resize_buffers_OGLEXT

		GET_PROC_ADDRESS(ResizeBuffersMESA);

	#endif // GL_MESA_resize_buffers_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_MESA_window_pos ]------------------------------------------------------------------------------------

//!	Init GL_MESA_window_pos.

bool CRenderingContext::InitMesaWindowPos()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_MESA_window_pos_OGLEXT

		GET_PROC_ADDRESS(WindowPos2dMESA);
		GET_PROC_ADDRESS(WindowPos2dvMESA);
		GET_PROC_ADDRESS(WindowPos2fMESA);
		GET_PROC_ADDRESS(WindowPos2fvMESA);
		GET_PROC_ADDRESS(WindowPos2iMESA);
		GET_PROC_ADDRESS(WindowPos2ivMESA);
		GET_PROC_ADDRESS(WindowPos2sMESA);
		GET_PROC_ADDRESS(WindowPos2svMESA);
		GET_PROC_ADDRESS(WindowPos3dMESA);
		GET_PROC_ADDRESS(WindowPos3dvMESA);
		GET_PROC_ADDRESS(WindowPos3fMESA);
		GET_PROC_ADDRESS(WindowPos3fvMESA);
		GET_PROC_ADDRESS(WindowPos3iMESA);
		GET_PROC_ADDRESS(WindowPos3ivMESA);
		GET_PROC_ADDRESS(WindowPos3sMESA);
		GET_PROC_ADDRESS(WindowPos3svMESA);
		GET_PROC_ADDRESS(WindowPos4dMESA);
		GET_PROC_ADDRESS(WindowPos4dvMESA);
		GET_PROC_ADDRESS(WindowPos4fMESA);
		GET_PROC_ADDRESS(WindowPos4fvMESA);
		GET_PROC_ADDRESS(WindowPos4iMESA);
		GET_PROC_ADDRESS(WindowPos4ivMESA);
		GET_PROC_ADDRESS(WindowPos4sMESA);
		GET_PROC_ADDRESS(WindowPos4svMESA);

	#endif // GL_MESA_window_pos_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_blend_square ]------------------------------------------------------------------------------------

//!	Init GL_NV_blend_square.

bool CRenderingContext::InitNvBlendSquare()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_blend_square_OGLEXT


	#endif // GL_NV_blend_square_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_copy_depth_to_color ]-----------------------------------------------------------------------------

//!	Init GL_NV_copy_depth_to_color.

bool CRenderingContext::InitNvCopyDepthToColor()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_copy_depth_to_color_OGLEXT


	#endif // GL_NV_copy_depth_to_color_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_depth_clamp ]-------------------------------------------------------------------------------------

//!	Init GL_NV_depth_clamp.

bool CRenderingContext::InitNvDepthClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_depth_clamp_OGLEXT


	#endif // GL_NV_depth_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_evaluators ]--------------------------------------------------------------------------------------

//!	Init GL_NV_evaluators.

bool CRenderingContext::InitNvEvaluators()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_evaluators_OGLEXT

		GET_PROC_ADDRESS(EvalMapsNV);
		GET_PROC_ADDRESS(GetMapAttribParameterfvNV);
		GET_PROC_ADDRESS(GetMapAttribParameterivNV);
		GET_PROC_ADDRESS(GetMapControlPointsNV);
		GET_PROC_ADDRESS(GetMapParameterfvNV);
		GET_PROC_ADDRESS(GetMapParameterivNV);
		GET_PROC_ADDRESS(MapControlPointsNV);
		GET_PROC_ADDRESS(MapParameterfvNV);
		GET_PROC_ADDRESS(MapParameterivNV);

	#endif // GL_NV_evaluators_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_fence ]-------------------------------------------------------------------------------------------

//!	Init GL_NV_fence.

bool CRenderingContext::InitNvFence()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_fence_OGLEXT

		GET_PROC_ADDRESS(DeleteFencesNV);
		GET_PROC_ADDRESS(FinishFenceNV);
		GET_PROC_ADDRESS(GenFencesNV);
		GET_PROC_ADDRESS(GetFenceivNV);
		GET_PROC_ADDRESS(IsFenceNV);
		GET_PROC_ADDRESS(SetFenceNV);
		GET_PROC_ADDRESS(TestFenceNV);

	#endif // GL_NV_fence_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_float_buffer ]------------------------------------------------------------------------------------

//!	Init GL_NV_float_buffer.

bool CRenderingContext::InitNvFloatBuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_float_buffer_OGLEXT


	#endif // GL_NV_float_buffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_fog_distance ]------------------------------------------------------------------------------------

//!	Init GL_NV_fog_distance.

bool CRenderingContext::InitNvFogDistance()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_fog_distance_OGLEXT


	#endif // GL_NV_fog_distance_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_fragment_program ]--------------------------------------------------------------------------------

//!	Init GL_NV_fragment_program.

bool CRenderingContext::InitNvFragmentProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_fragment_program_OGLEXT

		GET_PROC_ADDRESS(GetProgramNamedParameterdvNV);
		GET_PROC_ADDRESS(GetProgramNamedParameterfvNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4dNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4dvNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4fNV);
		GET_PROC_ADDRESS(ProgramNamedParameter4fvNV);

	#endif // GL_NV_fragment_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_half_float ]--------------------------------------------------------------------------------------

//!	Init GL_NV_half_float.

bool CRenderingContext::InitNvHalfFloat()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_half_float_OGLEXT

		GET_PROC_ADDRESS(Color3hNV);
		GET_PROC_ADDRESS(Color3hvNV);
		GET_PROC_ADDRESS(Color4hNV);
		GET_PROC_ADDRESS(Color4hvNV);
		GET_PROC_ADDRESS(FogCoordhNV);
		GET_PROC_ADDRESS(FogCoordhvNV);
		GET_PROC_ADDRESS(MultiTexCoord1hNV);
		GET_PROC_ADDRESS(MultiTexCoord1hvNV);
		GET_PROC_ADDRESS(MultiTexCoord2hNV);
		GET_PROC_ADDRESS(MultiTexCoord2hvNV);
		GET_PROC_ADDRESS(MultiTexCoord3hNV);
		GET_PROC_ADDRESS(MultiTexCoord3hvNV);
		GET_PROC_ADDRESS(MultiTexCoord4hNV);
		GET_PROC_ADDRESS(MultiTexCoord4hvNV);
		GET_PROC_ADDRESS(Normal3hNV);
		GET_PROC_ADDRESS(Normal3hvNV);
		GET_PROC_ADDRESS(SecondaryColor3hNV);
		GET_PROC_ADDRESS(SecondaryColor3hvNV);
		GET_PROC_ADDRESS(TexCoord1hNV);
		GET_PROC_ADDRESS(TexCoord1hvNV);
		GET_PROC_ADDRESS(TexCoord2hNV);
		GET_PROC_ADDRESS(TexCoord2hvNV);
		GET_PROC_ADDRESS(TexCoord3hNV);
		GET_PROC_ADDRESS(TexCoord3hvNV);
		GET_PROC_ADDRESS(TexCoord4hNV);
		GET_PROC_ADDRESS(TexCoord4hvNV);
		GET_PROC_ADDRESS(Vertex2hNV);
		GET_PROC_ADDRESS(Vertex2hvNV);
		GET_PROC_ADDRESS(Vertex3hNV);
		GET_PROC_ADDRESS(Vertex3hvNV);
		GET_PROC_ADDRESS(Vertex4hNV);
		GET_PROC_ADDRESS(Vertex4hvNV);
		GET_PROC_ADDRESS(VertexAttrib1hNV);
		GET_PROC_ADDRESS(VertexAttrib1hvNV);
		GET_PROC_ADDRESS(VertexAttrib2hNV);
		GET_PROC_ADDRESS(VertexAttrib2hvNV);
		GET_PROC_ADDRESS(VertexAttrib3hNV);
		GET_PROC_ADDRESS(VertexAttrib3hvNV);
		GET_PROC_ADDRESS(VertexAttrib4hNV);
		GET_PROC_ADDRESS(VertexAttrib4hvNV);
		GET_PROC_ADDRESS(VertexAttribs1hvNV);
		GET_PROC_ADDRESS(VertexAttribs2hvNV);
		GET_PROC_ADDRESS(VertexAttribs3hvNV);
		GET_PROC_ADDRESS(VertexAttribs4hvNV);
		GET_PROC_ADDRESS(VertexWeighthNV);
		GET_PROC_ADDRESS(VertexWeighthvNV);

	#endif // GL_NV_half_float_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_light_max_exponent ]------------------------------------------------------------------------------

//!	Init GL_NV_light_max_exponent.

bool CRenderingContext::InitNvLightMaxExponent()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_light_max_exponent_OGLEXT


	#endif // GL_NV_light_max_exponent_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_multisample_filter_hint ]-------------------------------------------------------------------------

//!	Init GL_NV_multisample_filter_hint.

bool CRenderingContext::InitNvMultisampleFilterHint()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_multisample_filter_hint_OGLEXT


	#endif // GL_NV_multisample_filter_hint_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_occlusion_query ]---------------------------------------------------------------------------------

//!	Init GL_NV_occlusion_query.

bool CRenderingContext::InitNvOcclusionQuery()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_occlusion_query_OGLEXT

		GET_PROC_ADDRESS(BeginOcclusionQueryNV);
		GET_PROC_ADDRESS(DeleteOcclusionQueriesNV);
		GET_PROC_ADDRESS(EndOcclusionQueryNV);
		GET_PROC_ADDRESS(GenOcclusionQueriesNV);
		GET_PROC_ADDRESS(GetOcclusionQueryivNV);
		GET_PROC_ADDRESS(GetOcclusionQueryuivNV);
		GET_PROC_ADDRESS(IsOcclusionQueryNV);

	#endif // GL_NV_occlusion_query_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_packed_depth_stencil ]----------------------------------------------------------------------------

//!	Init GL_NV_packed_depth_stencil.

bool CRenderingContext::InitNvPackedDepthStencil()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_packed_depth_stencil_OGLEXT


	#endif // GL_NV_packed_depth_stencil_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_pixel_data_range ]--------------------------------------------------------------------------------

//!	Init GL_NV_pixel_data_range.

bool CRenderingContext::InitNvPixelDataRange()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_pixel_data_range_OGLEXT

		GET_PROC_ADDRESS(FlushPixelDataRangeNV);
		GET_PROC_ADDRESS(PixelDataRangeNV);

	#endif // GL_NV_pixel_data_range_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_point_sprite ]------------------------------------------------------------------------------------

//!	Init GL_NV_point_sprite.

bool CRenderingContext::InitNvPointSprite()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_point_sprite_OGLEXT

		GET_PROC_ADDRESS(PointParameteriNV);
		GET_PROC_ADDRESS(PointParameterivNV);

	#endif // GL_NV_point_sprite_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_primitive_restart ]-------------------------------------------------------------------------------

//!	Init GL_NV_primitive_restart.

bool CRenderingContext::InitNvPrimitiveRestart()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_primitive_restart_OGLEXT

		GET_PROC_ADDRESS(PrimitiveRestartIndexNV);
		GET_PROC_ADDRESS(PrimitiveRestartNV);

	#endif // GL_NV_primitive_restart_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_register_combiners ]------------------------------------------------------------------------------

//!	Init GL_NV_register_combiners.

bool CRenderingContext::InitNvRegisterCombiners()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_register_combiners_OGLEXT

		GET_PROC_ADDRESS(CombinerInputNV);
		GET_PROC_ADDRESS(CombinerOutputNV);
		GET_PROC_ADDRESS(CombinerParameterfNV);
		GET_PROC_ADDRESS(CombinerParameterfvNV);
		GET_PROC_ADDRESS(CombinerParameteriNV);
		GET_PROC_ADDRESS(CombinerParameterivNV);
		GET_PROC_ADDRESS(FinalCombinerInputNV);
		GET_PROC_ADDRESS(GetCombinerInputParameterfvNV);
		GET_PROC_ADDRESS(GetCombinerInputParameterivNV);
		GET_PROC_ADDRESS(GetCombinerOutputParameterfvNV);
		GET_PROC_ADDRESS(GetCombinerOutputParameterivNV);
		GET_PROC_ADDRESS(GetFinalCombinerInputParameterfvNV);
		GET_PROC_ADDRESS(GetFinalCombinerInputParameterivNV);

	#endif // GL_NV_register_combiners_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_register_combiners2 ]-----------------------------------------------------------------------------

//!	Init GL_NV_register_combiners2.

bool CRenderingContext::InitNvRegisterCombiners2()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_register_combiners2_OGLEXT

		GET_PROC_ADDRESS(CombinerStageParameterfvNV);
		GET_PROC_ADDRESS(GetCombinerStageParameterfvNV);

	#endif // GL_NV_register_combiners2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texgen_emboss ]-----------------------------------------------------------------------------------

//!	Init GL_NV_texgen_emboss.

bool CRenderingContext::InitNvTexgenEmboss()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texgen_emboss_OGLEXT


	#endif // GL_NV_texgen_emboss_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texgen_reflection ]-------------------------------------------------------------------------------

//!	Init GL_NV_texgen_reflection.

bool CRenderingContext::InitNvTexgenReflection()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texgen_reflection_OGLEXT


	#endif // GL_NV_texgen_reflection_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_compression_vtc ]-------------------------------------------------------------------------

//!	Init GL_NV_texture_compression_vtc.

bool CRenderingContext::InitNvTextureCompressionVtc()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_compression_vtc_OGLEXT


	#endif // GL_NV_texture_compression_vtc_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_env_combine4 ]----------------------------------------------------------------------------

//!	Init GL_NV_texture_env_combine4.

bool CRenderingContext::InitNvTextureEnvCombine4()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_env_combine4_OGLEXT


	#endif // GL_NV_texture_env_combine4_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_expand_normal ]---------------------------------------------------------------------------

//!	Init GL_NV_texture_expand_normal.

bool CRenderingContext::InitNvTextureExpandNormal()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_expand_normal_OGLEXT


	#endif // GL_NV_texture_expand_normal_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_rectangle ]-------------------------------------------------------------------------------

//!	Init GL_NV_texture_rectangle.

bool CRenderingContext::InitNvTextureRectangle()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_rectangle_OGLEXT


	#endif // GL_NV_texture_rectangle_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_shader ]----------------------------------------------------------------------------------

//!	Init GL_NV_texture_shader.

bool CRenderingContext::InitNvTextureShader()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_shader_OGLEXT


	#endif // GL_NV_texture_shader_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_shader2 ]---------------------------------------------------------------------------------

//!	Init GL_NV_texture_shader2.

bool CRenderingContext::InitNvTextureShader2()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_shader2_OGLEXT


	#endif // GL_NV_texture_shader2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_texture_shader3 ]---------------------------------------------------------------------------------

//!	Init GL_NV_texture_shader3.

bool CRenderingContext::InitNvTextureShader3()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_texture_shader3_OGLEXT


	#endif // GL_NV_texture_shader3_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_array_range ]------------------------------------------------------------------------------

//!	Init GL_NV_vertex_array_range.

bool CRenderingContext::InitNvVertexArrayRange()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_array_range_OGLEXT

		GET_PROC_ADDRESS(FlushVertexArrayRangeNV);
		GET_PROC_ADDRESS(VertexArrayRangeNV);

	#endif // GL_NV_vertex_array_range_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_array_range2 ]-----------------------------------------------------------------------------

//!	Init GL_NV_vertex_array_range2.

bool CRenderingContext::InitNvVertexArrayRange2()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_array_range2_OGLEXT


	#endif // GL_NV_vertex_array_range2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_program ]----------------------------------------------------------------------------------

//!	Init GL_NV_vertex_program.

bool CRenderingContext::InitNvVertexProgram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_program_OGLEXT

		GET_PROC_ADDRESS(AreProgramsResidentNV);
		GET_PROC_ADDRESS(BindProgramNV);
		GET_PROC_ADDRESS(DeleteProgramsNV);
		GET_PROC_ADDRESS(ExecuteProgramNV);
		GET_PROC_ADDRESS(GenProgramsNV);
		GET_PROC_ADDRESS(GetProgramivNV);
		GET_PROC_ADDRESS(GetProgramParameterdvNV);
		GET_PROC_ADDRESS(GetProgramParameterfvNV);
		GET_PROC_ADDRESS(GetProgramStringNV);
		GET_PROC_ADDRESS(GetTrackMatrixivNV);
		GET_PROC_ADDRESS(GetVertexAttribdvNV);
		GET_PROC_ADDRESS(GetVertexAttribfvNV);
		GET_PROC_ADDRESS(GetVertexAttribivNV);
		GET_PROC_ADDRESS(GetVertexAttribPointervNV);
		GET_PROC_ADDRESS(IsProgramNV);
		GET_PROC_ADDRESS(LoadProgramNV);
		GET_PROC_ADDRESS(ProgramParameter4dNV);
		GET_PROC_ADDRESS(ProgramParameter4dvNV);
		GET_PROC_ADDRESS(ProgramParameter4fNV);
		GET_PROC_ADDRESS(ProgramParameter4fvNV);
		GET_PROC_ADDRESS(ProgramParameters4dvNV);
		GET_PROC_ADDRESS(ProgramParameters4fvNV);
		GET_PROC_ADDRESS(RequestResidentProgramsNV);
		GET_PROC_ADDRESS(TrackMatrixNV);
		GET_PROC_ADDRESS(VertexAttrib1dNV);
		GET_PROC_ADDRESS(VertexAttrib1dvNV);
		GET_PROC_ADDRESS(VertexAttrib1fNV);
		GET_PROC_ADDRESS(VertexAttrib1fvNV);
		GET_PROC_ADDRESS(VertexAttrib1sNV);
		GET_PROC_ADDRESS(VertexAttrib1svNV);
		GET_PROC_ADDRESS(VertexAttrib2dNV);
		GET_PROC_ADDRESS(VertexAttrib2dvNV);
		GET_PROC_ADDRESS(VertexAttrib2fNV);
		GET_PROC_ADDRESS(VertexAttrib2fvNV);
		GET_PROC_ADDRESS(VertexAttrib2sNV);
		GET_PROC_ADDRESS(VertexAttrib2svNV);
		GET_PROC_ADDRESS(VertexAttrib3dNV);
		GET_PROC_ADDRESS(VertexAttrib3dvNV);
		GET_PROC_ADDRESS(VertexAttrib3fNV);
		GET_PROC_ADDRESS(VertexAttrib3fvNV);
		GET_PROC_ADDRESS(VertexAttrib3sNV);
		GET_PROC_ADDRESS(VertexAttrib3svNV);
		GET_PROC_ADDRESS(VertexAttrib4dNV);
		GET_PROC_ADDRESS(VertexAttrib4dvNV);
		GET_PROC_ADDRESS(VertexAttrib4fNV);
		GET_PROC_ADDRESS(VertexAttrib4fvNV);
		GET_PROC_ADDRESS(VertexAttrib4sNV);
		GET_PROC_ADDRESS(VertexAttrib4svNV);
		GET_PROC_ADDRESS(VertexAttrib4ubNV);
		GET_PROC_ADDRESS(VertexAttrib4ubvNV);
		GET_PROC_ADDRESS(VertexAttribPointerNV);
		GET_PROC_ADDRESS(VertexAttribs1dvNV);
		GET_PROC_ADDRESS(VertexAttribs1fvNV);
		GET_PROC_ADDRESS(VertexAttribs1svNV);
		GET_PROC_ADDRESS(VertexAttribs2dvNV);
		GET_PROC_ADDRESS(VertexAttribs2fvNV);
		GET_PROC_ADDRESS(VertexAttribs2svNV);
		GET_PROC_ADDRESS(VertexAttribs3dvNV);
		GET_PROC_ADDRESS(VertexAttribs3fvNV);
		GET_PROC_ADDRESS(VertexAttribs3svNV);
		GET_PROC_ADDRESS(VertexAttribs4dvNV);
		GET_PROC_ADDRESS(VertexAttribs4fvNV);
		GET_PROC_ADDRESS(VertexAttribs4svNV);
		GET_PROC_ADDRESS(VertexAttribs4ubvNV);

	#endif // GL_NV_vertex_program_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_program1_1 ]-------------------------------------------------------------------------------

//!	Init GL_NV_vertex_program1_1.

bool CRenderingContext::InitNvVertexProgram11()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_program1_1_OGLEXT


	#endif // GL_NV_vertex_program1_1_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_NV_vertex_program2 ]---------------------------------------------------------------------------------

//!	Init GL_NV_vertex_program2.

bool CRenderingContext::InitNvVertexProgram2()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_NV_vertex_program2_OGLEXT


	#endif // GL_NV_vertex_program2_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_OML_interlace ]--------------------------------------------------------------------------------------

//!	Init GL_OML_interlace.

bool CRenderingContext::InitOmlInterlace()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_OML_interlace_OGLEXT


	#endif // GL_OML_interlace_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_OML_resample ]---------------------------------------------------------------------------------------

//!	Init GL_OML_resample.

bool CRenderingContext::InitOmlResample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_OML_resample_OGLEXT


	#endif // GL_OML_resample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_OML_subsample ]--------------------------------------------------------------------------------------

//!	Init GL_OML_subsample.

bool CRenderingContext::InitOmlSubsample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_OML_subsample_OGLEXT


	#endif // GL_OML_subsample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_PGI_misc_hints ]-------------------------------------------------------------------------------------

//!	Init GL_PGI_misc_hints.

bool CRenderingContext::InitPgiMiscHints()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_PGI_misc_hints_OGLEXT

		GET_PROC_ADDRESS(HintPGI);

	#endif // GL_PGI_misc_hints_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_PGI_vertex_hints ]-----------------------------------------------------------------------------------

//!	Init GL_PGI_vertex_hints.

bool CRenderingContext::InitPgiVertexHints()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_PGI_vertex_hints_OGLEXT


	#endif // GL_PGI_vertex_hints_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_REND_screen_coordinates ]----------------------------------------------------------------------------

//!	Init GL_REND_screen_coordinates.

bool CRenderingContext::InitRendScreenCoordinates()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_REND_screen_coordinates_OGLEXT


	#endif // GL_REND_screen_coordinates_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_S3_s3tc ]--------------------------------------------------------------------------------------------

//!	Init GL_S3_s3tc.

bool CRenderingContext::InitS3S3tc()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_S3_s3tc_OGLEXT


	#endif // GL_S3_s3tc_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_detail_texture ]--------------------------------------------------------------------------------

//!	Init GL_SGIS_detail_texture.

bool CRenderingContext::InitSgiSDetailTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_detail_texture_OGLEXT

		GET_PROC_ADDRESS(DetailTexFuncSGIS);
		GET_PROC_ADDRESS(GetDetailTexFuncSGIS);

	#endif // GL_SGIS_detail_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_fog_function ]----------------------------------------------------------------------------------

//!	Init GL_SGIS_fog_function.

bool CRenderingContext::InitSgiSFogFunction()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_fog_function_OGLEXT

		GET_PROC_ADDRESS(FogFuncSGIS);
		GET_PROC_ADDRESS(GetFogFuncSGIS);

	#endif // GL_SGIS_fog_function_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_generate_mipmap ]-------------------------------------------------------------------------------

//!	Init GL_SGIS_generate_mipmap.

bool CRenderingContext::InitSgiSGenerateMipmap()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_generate_mipmap_OGLEXT


	#endif // GL_SGIS_generate_mipmap_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_multisample ]-----------------------------------------------------------------------------------

//!	Init GL_SGIS_multisample.

bool CRenderingContext::InitSgiSMultisample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_multisample_OGLEXT

		GET_PROC_ADDRESS(SampleMaskSGIS);
		GET_PROC_ADDRESS(SamplePatternSGIS);

	#endif // GL_SGIS_multisample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_multitexture ]----------------------------------------------------------------------------------

//!	Init GL_SGIS_multitexture.

bool CRenderingContext::InitSgisMultitexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_multitexture_OGLEXT

		GET_PROC_ADDRESS(InterleavedTextureCoordSetsSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1dSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1dvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1fSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1fvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1iSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1ivSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1sSGIS);
		GET_PROC_ADDRESS(MultiTexCoord1svSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2dSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2dvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2fSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2fvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2iSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2ivSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2sSGIS);
		GET_PROC_ADDRESS(MultiTexCoord2svSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3dSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3dvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3fSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3fvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3iSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3ivSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3sSGIS);
		GET_PROC_ADDRESS(MultiTexCoord3svSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4dSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4dvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4fSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4fvSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4iSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4ivSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4sSGIS);
		GET_PROC_ADDRESS(MultiTexCoord4svSGIS);
		GET_PROC_ADDRESS(SelectTextureCoordSetSGIS);
		GET_PROC_ADDRESS(SelectTextureSGIS);
		GET_PROC_ADDRESS(SelectTextureTransformSGIS);

	#endif // GL_SGIS_multitexture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_pixel_texture ]---------------------------------------------------------------------------------

//!	Init GL_SGIS_pixel_texture.

bool CRenderingContext::InitSgiSPixelTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_pixel_texture_OGLEXT

		GET_PROC_ADDRESS(GetPixelTexGenParameterfvSGIS);
		GET_PROC_ADDRESS(GetPixelTexGenParameterivSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameterfSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameterfvSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameteriSGIS);
		GET_PROC_ADDRESS(PixelTexGenParameterivSGIS);

	#endif // GL_SGIS_pixel_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_point_line_texgen ]-----------------------------------------------------------------------------

//!	Init GL_SGIS_point_line_texgen.

bool CRenderingContext::InitSgiSPointLineTexgen()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_point_line_texgen_OGLEXT


	#endif // GL_SGIS_point_line_texgen_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_point_parameters ]------------------------------------------------------------------------------

//!	Init GL_SGIS_point_parameters.

bool CRenderingContext::InitSgiSPointParameters()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_point_parameters_OGLEXT

		GET_PROC_ADDRESS(PointParameterfSGIS);
		GET_PROC_ADDRESS(PointParameterfvSGIS);

	#endif // GL_SGIS_point_parameters_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_sharpen_texture ]-------------------------------------------------------------------------------

//!	Init GL_SGIS_sharpen_texture.

bool CRenderingContext::InitSgiSSharpenTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_sharpen_texture_OGLEXT

		GET_PROC_ADDRESS(GetSharpenTexFuncSGIS);
		GET_PROC_ADDRESS(SharpenTexFuncSGIS);

	#endif // GL_SGIS_sharpen_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture4D ]-------------------------------------------------------------------------------------

//!	Init GL_SGIS_texture4D.

bool CRenderingContext::InitSgiSTexture4D()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture4D_OGLEXT

		GET_PROC_ADDRESS(TexImage4DSGIS);
		GET_PROC_ADDRESS(TexSubImage4DSGIS);

	#endif // GL_SGIS_texture4D_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_border_clamp ]--------------------------------------------------------------------------

//!	Init GL_SGIS_texture_border_clamp.

bool CRenderingContext::InitSgiSTextureBorderClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_border_clamp_OGLEXT


	#endif // GL_SGIS_texture_border_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_color_mask ]----------------------------------------------------------------------------

//!	Init GL_SGIS_texture_color_mask.

bool CRenderingContext::InitSgiSTextureColorMask()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_color_mask_OGLEXT

		GET_PROC_ADDRESS(TextureColorMaskSGIS);

	#endif // GL_SGIS_texture_color_mask_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_edge_clamp ]----------------------------------------------------------------------------

//!	Init GL_SGIS_texture_edge_clamp.

bool CRenderingContext::InitSgiSTextureEdgeClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_edge_clamp_OGLEXT


	#endif // GL_SGIS_texture_edge_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_filter4 ]-------------------------------------------------------------------------------

//!	Init GL_SGIS_texture_filter4.

bool CRenderingContext::InitSgiSTextureFilter4()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_filter4_OGLEXT

		GET_PROC_ADDRESS(GetTexFilterFuncSGIS);
		GET_PROC_ADDRESS(TexFilterFuncSGIS);

	#endif // GL_SGIS_texture_filter4_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIS_texture_lod ]-----------------------------------------------------------------------------------

//!	Init GL_SGIS_texture_lod.

bool CRenderingContext::InitSgiSTextureLod()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIS_texture_lod_OGLEXT


	#endif // GL_SGIS_texture_lod_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_async ]-----------------------------------------------------------------------------------------

//!	Init GL_SGIX_async.

bool CRenderingContext::InitSgixAsync()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_async_OGLEXT

		GET_PROC_ADDRESS(AsyncMarkerSGIX);
		GET_PROC_ADDRESS(DeleteAsyncMarkersSGIX);
		GET_PROC_ADDRESS(FinishAsyncSGIX);
		GET_PROC_ADDRESS(GenAsyncMarkersSGIX);
		GET_PROC_ADDRESS(IsAsyncMarkerSGIX);
		GET_PROC_ADDRESS(PollAsyncSGIX);

	#endif // GL_SGIX_async_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_async_histogram ]-------------------------------------------------------------------------------

//!	Init GL_SGIX_async_histogram.

bool CRenderingContext::InitSgixAsyncHistogram()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_async_histogram_OGLEXT


	#endif // GL_SGIX_async_histogram_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_async_pixel ]-----------------------------------------------------------------------------------

//!	Init GL_SGIX_async_pixel.

bool CRenderingContext::InitSgixAsyncPixel()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_async_pixel_OGLEXT


	#endif // GL_SGIX_async_pixel_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_blend_alpha_minmax ]----------------------------------------------------------------------------

//!	Init GL_SGIX_blend_alpha_minmax.

bool CRenderingContext::InitSgixBlendAlphaMinmax()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_blend_alpha_minmax_OGLEXT


	#endif // GL_SGIX_blend_alpha_minmax_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_calligraphic_fragment ]-------------------------------------------------------------------------

//!	Init GL_SGIX_calligraphic_fragment.

bool CRenderingContext::InitSgixCalligraphicFragment()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_calligraphic_fragment_OGLEXT


	#endif // GL_SGIX_calligraphic_fragment_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_clipmap ]---------------------------------------------------------------------------------------

//!	Init GL_SGIX_clipmap.

bool CRenderingContext::InitSgixClipmap()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_clipmap_OGLEXT


	#endif // GL_SGIX_clipmap_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_convolution_accuracy ]--------------------------------------------------------------------------

//!	Init GL_SGIX_convolution_accuracy.

bool CRenderingContext::InitSgixConvolutionAccuracy()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_convolution_accuracy_OGLEXT


	#endif // GL_SGIX_convolution_accuracy_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_depth_pass_instrument ]-------------------------------------------------------------------------

//!	Init GL_SGIX_depth_pass_instrument.

bool CRenderingContext::InitSgixDepthPassInstrument()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_depth_pass_instrument_OGLEXT


	#endif // GL_SGIX_depth_pass_instrument_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_depth_texture ]---------------------------------------------------------------------------------

//!	Init GL_SGIX_depth_texture.

bool CRenderingContext::InitSgixDepthTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_depth_texture_OGLEXT


	#endif // GL_SGIX_depth_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_flush_raster ]----------------------------------------------------------------------------------

//!	Init GL_SGIX_flush_raster.

bool CRenderingContext::InitSgixFlushRaster()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_flush_raster_OGLEXT

		GET_PROC_ADDRESS(FlushRasterSGIX);

	#endif // GL_SGIX_flush_raster_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_fog_offset ]------------------------------------------------------------------------------------

//!	Init GL_SGIX_fog_offset.

bool CRenderingContext::InitSgixFogOffset()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_fog_offset_OGLEXT


	#endif // GL_SGIX_fog_offset_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_fog_scale ]-------------------------------------------------------------------------------------

//!	Init GL_SGIX_fog_scale.

bool CRenderingContext::InitSgixFogScale()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_fog_scale_OGLEXT


	#endif // GL_SGIX_fog_scale_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_fragment_lighting ]-----------------------------------------------------------------------------

//!	Init GL_SGIX_fragment_lighting.

bool CRenderingContext::InitSgixFragmentLighting()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_fragment_lighting_OGLEXT

		GET_PROC_ADDRESS(FragmentColorMaterialSGIX);
		GET_PROC_ADDRESS(FragmentLightfSGIX);
		GET_PROC_ADDRESS(FragmentLightfvSGIX);
		GET_PROC_ADDRESS(FragmentLightiSGIX);
		GET_PROC_ADDRESS(FragmentLightivSGIX);
		GET_PROC_ADDRESS(FragmentLightModelfSGIX);
		GET_PROC_ADDRESS(FragmentLightModelfvSGIX);
		GET_PROC_ADDRESS(FragmentLightModeliSGIX);
		GET_PROC_ADDRESS(FragmentLightModelivSGIX);
		GET_PROC_ADDRESS(FragmentMaterialfSGIX);
		GET_PROC_ADDRESS(FragmentMaterialfvSGIX);
		GET_PROC_ADDRESS(FragmentMaterialiSGIX);
		GET_PROC_ADDRESS(FragmentMaterialivSGIX);
		GET_PROC_ADDRESS(GetFragmentLightfvSGIX);
		GET_PROC_ADDRESS(GetFragmentLightivSGIX);
		GET_PROC_ADDRESS(GetFragmentMaterialfvSGIX);
		GET_PROC_ADDRESS(GetFragmentMaterialivSGIX);
		GET_PROC_ADDRESS(LightEnviSGIX);

	#endif // GL_SGIX_fragment_lighting_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_framezoom ]-------------------------------------------------------------------------------------

//!	Init GL_SGIX_framezoom.

bool CRenderingContext::InitSgixFramezoom()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_framezoom_OGLEXT

		GET_PROC_ADDRESS(FrameZoomSGIX);

	#endif // GL_SGIX_framezoom_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_igloo_interface ]-------------------------------------------------------------------------------

//!	Init GL_SGIX_igloo_interface.

bool CRenderingContext::InitSgixIglooInterface()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_igloo_interface_OGLEXT

		GET_PROC_ADDRESS(IglooInterfaceSGIX);

	#endif // GL_SGIX_igloo_interface_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_instruments ]-----------------------------------------------------------------------------------

//!	Init GL_SGIX_instruments.

bool CRenderingContext::InitSgixInstruments()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_instruments_OGLEXT

		GET_PROC_ADDRESS(GetInstrumentsSGIX);
		GET_PROC_ADDRESS(InstrumentsBufferSGIX);
		GET_PROC_ADDRESS(PollInstrumentsSGIX);
		GET_PROC_ADDRESS(ReadInstrumentsSGIX);
		GET_PROC_ADDRESS(StartInstrumentsSGIX);
		GET_PROC_ADDRESS(StopInstrumentsSGIX);

	#endif // GL_SGIX_instruments_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_interlace ]-------------------------------------------------------------------------------------

//!	Init GL_SGIX_interlace.

bool CRenderingContext::InitSgixInterlace()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_interlace_OGLEXT


	#endif // GL_SGIX_interlace_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_ir_instrument1 ]--------------------------------------------------------------------------------

//!	Init GL_SGIX_ir_instrument1.

bool CRenderingContext::InitSgixIrInstrument1()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_ir_instrument1_OGLEXT


	#endif // GL_SGIX_ir_instrument1_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_list_priority ]---------------------------------------------------------------------------------

//!	Init GL_SGIX_list_priority.

bool CRenderingContext::InitSgixListPriority()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_list_priority_OGLEXT

		GET_PROC_ADDRESS(GetListParameterfvSGIX);
		GET_PROC_ADDRESS(GetListParameterivSGIX);
		GET_PROC_ADDRESS(ListParameterfSGIX);
		GET_PROC_ADDRESS(ListParameterfvSGIX);
		GET_PROC_ADDRESS(ListParameteriSGIX);
		GET_PROC_ADDRESS(ListParameterivSGIX);

	#endif // GL_SGIX_list_priority_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_pixel_texture ]---------------------------------------------------------------------------------

//!	Init GL_SGIX_pixel_texture.

bool CRenderingContext::InitSgixPixelTexture()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_pixel_texture_OGLEXT

		GET_PROC_ADDRESS(PixelTexGenSGIX);

	#endif // GL_SGIX_pixel_texture_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_pixel_tiles ]-----------------------------------------------------------------------------------

//!	Init GL_SGIX_pixel_tiles.

bool CRenderingContext::InitSgixPixelTiles()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_pixel_tiles_OGLEXT


	#endif // GL_SGIX_pixel_tiles_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_polynomial_ffd ]--------------------------------------------------------------------------------

//!	Init GL_SGIX_polynomial_ffd.

bool CRenderingContext::InitSgixPolynomialFfd()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_polynomial_ffd_OGLEXT

		GET_PROC_ADDRESS(DeformationMap3dSGIX);
		GET_PROC_ADDRESS(DeformationMap3fSGIX);
		GET_PROC_ADDRESS(DeformSGIX);
		GET_PROC_ADDRESS(LoadIdentityDeformationMapSGIX);

	#endif // GL_SGIX_polynomial_ffd_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_reference_plane ]-------------------------------------------------------------------------------

//!	Init GL_SGIX_reference_plane.

bool CRenderingContext::InitSgixReferencePlane()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_reference_plane_OGLEXT

		GET_PROC_ADDRESS(ReferencePlaneSGIX);

	#endif // GL_SGIX_reference_plane_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_resample ]--------------------------------------------------------------------------------------

//!	Init GL_SGIX_resample.

bool CRenderingContext::InitSgixResample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_resample_OGLEXT


	#endif // GL_SGIX_resample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_scalebias_hint ]--------------------------------------------------------------------------------

//!	Init GL_SGIX_scalebias_hint.

bool CRenderingContext::InitSgixScalebiasHint()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_scalebias_hint_OGLEXT


	#endif // GL_SGIX_scalebias_hint_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_shadow ]----------------------------------------------------------------------------------------

//!	Init GL_SGIX_shadow.

bool CRenderingContext::InitSgixShadow()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_shadow_OGLEXT


	#endif // GL_SGIX_shadow_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_shadow_ambient ]--------------------------------------------------------------------------------

//!	Init GL_SGIX_shadow_ambient.

bool CRenderingContext::InitSgixShadowAmbient()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_shadow_ambient_OGLEXT


	#endif // GL_SGIX_shadow_ambient_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_sprite ]----------------------------------------------------------------------------------------

//!	Init GL_SGIX_sprite.

bool CRenderingContext::InitSgixSprite()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_sprite_OGLEXT

		GET_PROC_ADDRESS(SpriteParameterfSGIX);
		GET_PROC_ADDRESS(SpriteParameterfvSGIX);
		GET_PROC_ADDRESS(SpriteParameteriSGIX);
		GET_PROC_ADDRESS(SpriteParameterivSGIX);

	#endif // GL_SGIX_sprite_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_subsample ]-------------------------------------------------------------------------------------

//!	Init GL_SGIX_subsample.

bool CRenderingContext::InitSgixSubsample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_subsample_OGLEXT


	#endif // GL_SGIX_subsample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_tag_sample_buffer ]-----------------------------------------------------------------------------

//!	Init GL_SGIX_tag_sample_buffer.

bool CRenderingContext::InitSgixTagSampleBuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_tag_sample_buffer_OGLEXT

		GET_PROC_ADDRESS(TagSampleBufferSGIX);

	#endif // GL_SGIX_tag_sample_buffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_texture_add_env ]-------------------------------------------------------------------------------

//!	Init GL_SGIX_texture_add_env.

bool CRenderingContext::InitSgixTextureAddEnv()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_texture_add_env_OGLEXT


	#endif // GL_SGIX_texture_add_env_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_texture_coordinate_clamp ]----------------------------------------------------------------------

//!	Init GL_SGIX_texture_coordinate_clamp.

bool CRenderingContext::InitSgixTextureCoordinateClamp()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_texture_coordinate_clamp_OGLEXT


	#endif // GL_SGIX_texture_coordinate_clamp_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_texture_lod_bias ]------------------------------------------------------------------------------

//!	Init GL_SGIX_texture_lod_bias.

bool CRenderingContext::InitSgixTextureLodBias()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_texture_lod_bias_OGLEXT


	#endif // GL_SGIX_texture_lod_bias_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_texture_multi_buffer ]--------------------------------------------------------------------------

//!	Init GL_SGIX_texture_multi_buffer.

bool CRenderingContext::InitSgixTextureMultiBuffer()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_texture_multi_buffer_OGLEXT


	#endif // GL_SGIX_texture_multi_buffer_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_texture_scale_bias ]----------------------------------------------------------------------------

//!	Init GL_SGIX_texture_scale_bias.

bool CRenderingContext::InitSgixTextureScaleBias()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_texture_scale_bias_OGLEXT


	#endif // GL_SGIX_texture_scale_bias_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_texture_select ]--------------------------------------------------------------------------------

//!	Init GL_SGIX_texture_select.

bool CRenderingContext::InitSgixTextureSelect()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_texture_select_OGLEXT


	#endif // GL_SGIX_texture_select_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_vertex_preclip ]--------------------------------------------------------------------------------

//!	Init GL_SGIX_vertex_preclip.

bool CRenderingContext::InitSgixVertexPreclip()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_vertex_preclip_OGLEXT


	#endif // GL_SGIX_vertex_preclip_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_ycrcb ]-----------------------------------------------------------------------------------------

//!	Init GL_SGIX_ycrcb.

bool CRenderingContext::InitSgixYcrcb()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_ycrcb_OGLEXT


	#endif // GL_SGIX_ycrcb_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_ycrcba ]----------------------------------------------------------------------------------------

//!	Init GL_SGIX_ycrcba.

bool CRenderingContext::InitSgixYcrcba()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_ycrcba_OGLEXT


	#endif // GL_SGIX_ycrcba_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGIX_ycrcb_subsample ]-------------------------------------------------------------------------------

//!	Init GL_SGIX_ycrcb_subsample.

bool CRenderingContext::InitSgixYcrcbSubsample()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGIX_ycrcb_subsample_OGLEXT


	#endif // GL_SGIX_ycrcb_subsample_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGI_color_matrix ]-----------------------------------------------------------------------------------

//!	Init GL_SGI_color_matrix.

bool CRenderingContext::InitSgiColorMatrix()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGI_color_matrix_OGLEXT


	#endif // GL_SGI_color_matrix_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGI_color_table ]------------------------------------------------------------------------------------

//!	Init GL_SGI_color_table.

bool CRenderingContext::InitSgiColorTable()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGI_color_table_OGLEXT

		GET_PROC_ADDRESS(ColorTableParameterfvSGI);
		GET_PROC_ADDRESS(ColorTableParameterivSGI);
		GET_PROC_ADDRESS(ColorTableSGI);
		GET_PROC_ADDRESS(CopyColorTableSGI);
		GET_PROC_ADDRESS(GetColorTableParameterfvSGI);
		GET_PROC_ADDRESS(GetColorTableParameterivSGI);
		GET_PROC_ADDRESS(GetColorTableSGI);

	#endif // GL_SGI_color_table_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SGI_texture_color_table ]----------------------------------------------------------------------------

//!	Init GL_SGI_texture_color_table.

bool CRenderingContext::InitSgiTextureColorTable()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SGI_texture_color_table_OGLEXT


	#endif // GL_SGI_texture_color_table_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUNX_constant_data ]---------------------------------------------------------------------------------

//!	Init GL_SUNX_constant_data.

bool CRenderingContext::InitSunxConstantData()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUNX_constant_data_OGLEXT

		GET_PROC_ADDRESS(FinishTextureSUNX);

	#endif // GL_SUNX_constant_data_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_convolution_border_modes ]-----------------------------------------------------------------------

//!	Init GL_SUN_convolution_border_modes.

bool CRenderingContext::InitSunConvolutionBorderModes()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_convolution_border_modes_OGLEXT


	#endif // GL_SUN_convolution_border_modes_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_global_alpha ]-----------------------------------------------------------------------------------

//!	Init GL_SUN_global_alpha.

bool CRenderingContext::InitSunGlobalAlpha()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_global_alpha_OGLEXT

		GET_PROC_ADDRESS(GlobalAlphaFactorbSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactordSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorfSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactoriSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorsSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorubSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactoruiSUN);
		GET_PROC_ADDRESS(GlobalAlphaFactorusSUN);

	#endif // GL_SUN_global_alpha_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_mesh_array ]-------------------------------------------------------------------------------------

//!	Init GL_SUN_mesh_array.

bool CRenderingContext::InitSunMeshArray()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_mesh_array_OGLEXT

		GET_PROC_ADDRESS(DrawMeshArraysSUN);

	#endif // GL_SUN_mesh_array_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_slice_accum ]------------------------------------------------------------------------------------

//!	Init GL_SUN_slice_accum.

bool CRenderingContext::InitSunSliceAccum()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_slice_accum_OGLEXT


	#endif // GL_SUN_slice_accum_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_triangle_list ]----------------------------------------------------------------------------------

//!	Init GL_SUN_triangle_list.

bool CRenderingContext::InitSunTriangleList()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_triangle_list_OGLEXT

		GET_PROC_ADDRESS(ReplacementCodePointerSUN);
		GET_PROC_ADDRESS(ReplacementCodeubSUN);
		GET_PROC_ADDRESS(ReplacementCodeubvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiSUN);
		GET_PROC_ADDRESS(ReplacementCodeuivSUN);
		GET_PROC_ADDRESS(ReplacementCodeusSUN);
		GET_PROC_ADDRESS(ReplacementCodeusvSUN);

	#endif // GL_SUN_triangle_list_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


// ---[ GL_SUN_vertex ]-----------------------------------------------------------------------------------------

//!	Init GL_SUN_vertex.

bool CRenderingContext::InitSunvertex()
{
	bool bReturn = true;

	// 1: get all function pointers...

	#ifdef GL_SUN_vertex_OGLEXT

		GET_PROC_ADDRESS(Color3fVertex3fSUN);
		GET_PROC_ADDRESS(Color3fVertex3fvSUN);
		GET_PROC_ADDRESS(Color4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(Color4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(Color4ubVertex2fSUN);
		GET_PROC_ADDRESS(Color4ubVertex2fvSUN);
		GET_PROC_ADDRESS(Color4ubVertex3fSUN);
		GET_PROC_ADDRESS(Color4ubVertex3fvSUN);
		GET_PROC_ADDRESS(Normal3fVertex3fSUN);
		GET_PROC_ADDRESS(Normal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4ubVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiColor4ubVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiTexCoord2fVertex3fvSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiVertex3fSUN);
		GET_PROC_ADDRESS(ReplacementCodeuiVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fColor3fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fColor3fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4ubVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fColor4ubVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fNormal3fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fNormal3fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord2fVertex3fSUN);
		GET_PROC_ADDRESS(TexCoord2fVertex3fvSUN);
		GET_PROC_ADDRESS(TexCoord4fColor4fNormal3fVertex4fSUN);
		GET_PROC_ADDRESS(TexCoord4fColor4fNormal3fVertex4fvSUN);
		GET_PROC_ADDRESS(TexCoord4fVertex4fSUN);
		GET_PROC_ADDRESS(TexCoord4fVertex4fvSUN);

	#endif // GL_SUN_vertex_OGLEXT

	// 2: return 'true' if everything went fine...

	return bReturn;
}


