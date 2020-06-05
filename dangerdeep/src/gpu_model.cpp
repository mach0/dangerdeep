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

// A 3d model displayer
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "datadirs.h"
#include "gpu_model.h"
#include "model.h"
#include "model_state.h"

// class wide statics.
unsigned gpu::model::init_count = 0;
std::unordered_map<gpu::basic_shader_feature, gpu::program> gpu::model::default_programs;
gpu::texture_array gpu::model::caustics;



/// Constructor to display given model
gpu::model::model(const ::model& m, const scene& myscene_, const model_state& ms)
	: mymodel(m)
	, myscene(myscene_)
{
	if (init_count++ == 0) {
		render_init();
	}

	// Initialize all necessary programs to render materials and also the UBOs for material data
	materials.resize(m.get_nr_of_materials());
	for (unsigned i = 0; i < m.get_nr_of_materials(); ++i) {
		const auto& mat = m.get_material(i);
		if (!mat.get_shader_base_filename().empty()) {
			materials[i].myprogram = gpu::program(mat.get_shader_base_filename());
		}
		material_data md;
		md.common_color = mat.get_diffuse_color();
		md.shininess = mat.get_shininess();
		md.specular_color = mat.get_specular_color().vec3();
		materials[i].data_ubo.init(buffer::usage_type::static_draw, md);
	}

	// depending on model state's layout prepare textures,
	// so they are not created on first display call but a bit earlier
	prepare_textures_and_samplers(ms.get_layout());

	// Initialize meshes from model
	const unsigned nr_of_meshes = m.get_nr_of_meshes();
	meshes.reserve(nr_of_meshes);
	for (unsigned i = 0; i < nr_of_meshes; ++i) {
		auto material_index = mymodel.get_mesh(i).get_material_id();
		if (material_index == 0) {
			THROW(error, "mesh without material index!");
		}
		--material_index;
		// custom shader is given, use it for all kinds of rendering
		const auto& mymaterial = materials[material_index];
		const auto& custom_program = mymaterial.myprogram;
		if (!custom_program.empty()) {
			meshes.push_back(gpu::mesh(m.get_mesh(i), custom_program, mymaterial.data_ubo, myscene));
		} else {
			// determine shader programs for mesh, normal, underwater, mirrorclip
			const auto& modelmaterial = mymodel.get_material(material_index);
			gpu::program* mat_prog = nullptr;
			gpu::program* mat_uw_prog = nullptr;
			gpu::program* mat_mc_prog = nullptr;
			auto add = [](gpu::basic_shader_feature b0, gpu::basic_shader_feature b1) {
				return gpu::basic_shader_feature(int(b0) | int(b1));
			};
			auto bb = gpu::basic_shader_feature::lighting;
			if (modelmaterial.has_map(::model::map_type::diffuse)) {
				auto bc = add(bb, gpu::basic_shader_feature::colormap);
				if (modelmaterial.has_map(::model::map_type::normal)) {
					auto bn = add(bc, gpu::basic_shader_feature::normalmap);
					if (modelmaterial.has_map(::model::map_type::specular)) {
						auto bs = add(bn, gpu::basic_shader_feature::specularmap);
						mat_prog = &get_default_program(add(bs, gpu::basic_shader_feature::fog));
						mat_uw_prog = &get_default_program(add(bs, gpu::basic_shader_feature::underwater));
						mat_mc_prog = &get_default_program(add(bc, gpu::basic_shader_feature::clipplane));
					} else {
						mat_prog = &get_default_program(add(bn, gpu::basic_shader_feature::fog));
						mat_uw_prog = &get_default_program(add(bn, gpu::basic_shader_feature::underwater));
						mat_mc_prog = &get_default_program(add(bc, gpu::basic_shader_feature::clipplane));
					}
				} else {
					mat_prog = &get_default_program(add(bc, gpu::basic_shader_feature::fog));
					mat_uw_prog = &get_default_program(add(bc, gpu::basic_shader_feature::underwater));
					mat_mc_prog = &get_default_program(add(bc, gpu::basic_shader_feature::clipplane));
				}
			} else {
				mat_prog = &get_default_program(add(bb, gpu::basic_shader_feature::fog));
				mat_uw_prog = &get_default_program(add(bb, gpu::basic_shader_feature::underwater));
				mat_mc_prog = &get_default_program(add(bb, gpu::basic_shader_feature::clipplane));
			}
			meshes.push_back(gpu::mesh(m.get_mesh(i), *mat_prog, *mat_uw_prog, *mat_mc_prog, mymaterial.data_ubo, myscene));
		}
	}
}



/// Destructor to free stuff when last model is gone
gpu::model::~model()
{
	if (--init_count == 0) {
		render_deinit();
	}
}



/// Prepare texture and sampler values for all materials
void gpu::model::prepare_textures_and_samplers(const std::string& layout)
{
	if (layout != current_layout) {
		// reference all texture maps of new layout and unref all of old layout.
		// Iterate over all materials and get names of maps for new layout.
		// per material build texture and sampler lists
		for (unsigned material_index = 0; material_index < mymodel.get_nr_of_materials(); ++material_index) {
			const auto& mat = mymodel.get_material(material_index);
			bool is_default_program = mat.get_shader_base_filename().empty();
			auto& mymat = materials[material_index];
			mymat.textures_and_samplers.resize(mat.get_maps().size());
			for (unsigned map_index = 0; map_index < unsigned(mat.get_maps().size()); ++map_index) {
				const auto& mmap = mat.get_maps()[map_index];
				if (mmap.empty()) {
					// clear texture/sampler
					mymat.textures_and_samplers[map_index] = { nullptr, sampler::type::number };
				} else {
					const auto new_filename = mmap.get_filename_for_layout(layout);
					const auto old_filename = current_layout.empty() ? std::string() : mmap.get_filename_for_layout(current_layout);
					if (new_filename != old_filename) {
						float bump_height = -1.f;
						std::string filename_for_ref = new_filename;
						if (map_index == unsigned(::model::map_type::normal) && mmap.has_bump_height()) {
							// create normal map from bump map!
							bump_height = mmap.get_bump_height();
							filename_for_ref += "/bump/" + helper::str(bump_height);
						}
						// texture construction parameters and sampler type depends on map type.
						bool use_mipmaps = is_default_program;
						bool use_compression = false;	// fixme when to use?
						// get texture reference and store it
						mymat.textures_and_samplers[map_index].first = &texture_store.ref(filename_for_ref, [&](const std::string& /*name*/) {
							return std::make_unique<gpu::texture>(mymodel.get_filesystem_path() + new_filename, data_type::ubyte, use_mipmaps, use_compression, bump_height);
						});
						auto dst = use_mipmaps ? sampler::type::trilinear_clamp : sampler::type::bilinear_clamp;
						mymat.textures_and_samplers[map_index].second = dst;

						// unref old filename so it is potentially no longer used
						if (!old_filename.empty()) {
							texture_store.unref(old_filename);
						}
					}
				}
			}
		}
		current_layout = layout;
	}
}



/// Generic display method
void gpu::model::display_generic(const model_state& ms, mesh_display_method mdm)
{
	prepare_textures_and_samplers(ms.get_layout());
	mymodel.iterate_objects(0, ms.get_transformation(), [&](unsigned object_index, const matrix4& parent_transformation) {
		const auto object_transformation = parent_transformation * ms.get_object_local_transformation(object_index);
		if (mymodel.has_object_a_mesh(object_index)) {
			const auto mesh_index = mymodel.get_mesh_index_of_object(object_index);
			const auto material_index = mymodel.get_mesh(mesh_index).get_material_id();
			if (material_index == 0) {
				THROW(error, "no material for mesh set!");
			}
			meshes[mesh_index].set_textures_and_samplers(materials[material_index - 1].textures_and_samplers);
			(meshes[mesh_index].*mdm)(object_transformation);
		}
		return object_transformation;
	});
}



/// Display the whole model with transformation accumulated so far (camera)
void gpu::model::display(const model_state& ms)
{
	display_generic(ms, &gpu::mesh::display);
}



/// Display the whole model with transformation accumulated so far (camera)
void gpu::model::display_under_water(const model_state& ms)
{
	display_generic(ms, &gpu::mesh::display_under_water);
}



/// Display a whole model clipped and mirrored and clipped at z=0 plane with transformation accumulated so far (camera)
void gpu::model::display_mirror_clip(const model_state& ms)
{
	display_generic(ms, &gpu::mesh::display_mirror_clip);
}



/// Initialize global render data
void gpu::model::render_init()
{
	// uniform locations are the same for all shaders.
	// programs are created on demand.
	// fixme create caustics texture array here!
}



/// Deinitialize global render data
void gpu::model::render_deinit()
{
	default_programs.clear();
	caustics = gpu::texture_array();
}



gpu::program& gpu::model::get_default_program(gpu::basic_shader_feature bsf)
{
	auto pib = default_programs.insert(std::make_pair(bsf, gpu::program()));
	if (pib.second) {
		pib.first->second = gpu::make(gpu::generate_basic_shader_source(bsf));
	}
	return pib.first->second;
}
