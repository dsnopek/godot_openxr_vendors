/**************************************************************************/
/*  openxr_android_light_estimation.h                                     */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/sky.hpp>

namespace godot {
class DirectionalLight3D;
class WorldEnvironment;
class Timer;
}; // namespace godot

using namespace godot;

// @todo Use timer to update estimate. Need update_frequency property, and should start when in tree and visible; stop, otherwise
// @todo Start with updating directional light (maybe visualize a cylinder attached to the camera?)
// @todo Then try a sky shader with spherical harmonics

class OpenXRAndroidLightEstimation : public Node3D {
	GDCLASS(OpenXRAndroidLightEstimation, Node3D)

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	enum SphericalHarmonicsType {
		SPHERICAL_HARMONICS_TYPE_AMBIENT,
		SPHERICAL_HARMONICS_TYPE_TOTAL,
	};

	enum SphericalHarmonicsDegree {
		SPHERICAL_HARMONICS_DEGREE_L0,
		SPHERICAL_HARMONICS_DEGREE_L1,
		SPHERICAL_HARMONICS_DEGREE_L2,
		SPHERICAL_HARMONICS_DEGREE_MAX,
	};

	void set_directional_light(DirectionalLight3D *p_directional_light);
	DirectionalLight3D *get_directional_light() const;

	void set_world_environment(WorldEnvironment *p_world_environment);
	WorldEnvironment *get_world_environment() const;

	void set_update_frequency(double p_seconds);
	double get_update_frequency() const;

	void set_directional_light_color(const Color &p_color);
	Color get_directional_light_color() const;

	void set_spherical_harmonics_type(SphericalHarmonicsType p_sh_type);
	SphericalHarmonicsType get_spherical_harmonics_type() const;

	void set_spherical_harmonics_degree(SphericalHarmonicsDegree p_sh_degree);
	SphericalHarmonicsDegree get_spherical_harmonics_degree() const;

	~OpenXRAndroidLightEstimation();

private:
	Timer *timer = nullptr;

	ObjectID directional_light_id;
	ObjectID world_environment_id;
	int64_t last_update_time = 0;

	Color directional_light_color = Color(1.0, 1.0, 1.0, 1.0);

	SphericalHarmonicsType sh_type = SPHERICAL_HARMONICS_TYPE_AMBIENT;
	SphericalHarmonicsDegree sh_degree = SPHERICAL_HARMONICS_DEGREE_L1;

	Ref<Shader> sky_shaders[SPHERICAL_HARMONICS_DEGREE_MAX];
	Ref<ShaderMaterial> sky_material;
	Ref<Sky> sky;
	Ref<Sky> old_sky;

	void start_or_stop();
	void update_light_estimate();
	void reset_sky();

	Ref<Shader> get_shader(SphericalHarmonicsDegree p_sh_degree);
};

VARIANT_ENUM_CAST(OpenXRAndroidLightEstimation::SphericalHarmonicsType);
VARIANT_ENUM_CAST(OpenXRAndroidLightEstimation::SphericalHarmonicsDegree);
