/**************************************************************************/
/*  openxr_android_light_estimation.cpp                                   */
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

#include "extensions/openxr_android_light_estimation_extension_wrapper.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *ANDROID_LIGHT_ESTIMATION_DEPTH_DIRECTIONAL_LIGHT_VALID_NAME = "ANDROID_LIGHT_ESTIMATION_DIRECTIONAL_LIGHT_VALID";
static const char *ANDROID_LIGHT_ESTIMATION_DEPTH_DIRECTIONAL_LIGHT_DIRECTION_NAME = "ANDROID_LIGHT_ESTIMATION_DIRECTIONAL_LIGHT_DIRECTION";
static const char *ANDROID_LIGHT_ESTIMATION_DEPTH_DIRECTIONAL_LIGHT_INTENSITY_NAME = "ANDROID_LIGHT_ESTIMATION_DIRECTIONAL_LIGHT_INTENSITY";
static const char *ANDROID_LIGHT_ESTIMATION_AMBIENT_LIGHT_VALID_NAME = "ANDROID_LIGHT_ESTIMATION_AMBIENT_LIGHT_VALID";
static const char *ANDROID_LIGHT_ESTIMATION_AMBIENT_LIGHT_INTENSITY_NAME = "ANDROID_LIGHT_ESTIMATION_AMBIENT_LIGHT_INTENSITY";
static const char *ANDROID_LIGHT_ESTIMATION_AMBIENT_LIGHT_COLOR_CORRECTION_NAME = "ANDROID_LIGHT_ESTIMATION_AMBIENT_LIGHT_COLOR_CORRECTION";
static const char *ANDROID_LIGHT_ESTIMATION_SH_AMBIENT_VALID_NAME = "ANDROID_LIGHT_ESTIMATION_SH_AMBIENT_VALID";
static const char *ANDROID_LIGHT_ESTIMATION_SH_AMBIENT_COEFFICIENTS_NAME = "ANDROID_LIGHT_ESTIMATION_SH_AMBIENT_COEFFICIENTS";
static const char *ANDROID_LIGHT_ESTIMATION_SH_TOTAL_VALID_NAME = "ANDROID_LIGHT_ESTIMATION_SH_TOTAL_VALID";
static const char *ANDROID_LIGHT_ESTIMATION_SH_TOTAL_COEFFICIENTS_NAME = "ANDROID_LIGHT_ESTIMATION_SH_TOTAL_COEFFICIENTS";

OpenXRAndroidLightEstimationExtensionWrapper *OpenXRAndroidLightEstimationExtensionWrapper::singleton = nullptr;

OpenXRAndroidLightEstimationExtensionWrapper *OpenXRAndroidLightEstimationExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidLightEstimationExtensionWrapper());
	}
	return singleton;
}

OpenXRAndroidLightEstimationExtensionWrapper::OpenXRAndroidLightEstimationExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidLightEstimationExtensionWrapper singleton already exists.");

	request_extensions[XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME] = &android_light_estimation_ext;
	singleton = this;
}

OpenXRAndroidLightEstimationExtensionWrapper::~OpenXRAndroidLightEstimationExtensionWrapper() {
	cleanup();
}

void OpenXRAndroidLightEstimationExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_light_estimation_supported"), &OpenXRAndroidLightEstimationExtensionWrapper::is_light_estimation_started);

	ClassDB::bind_method(D_METHOD("start_light_estimation"), &OpenXRAndroidLightEstimationExtensionWrapper::start_light_estimation);
	ClassDB::bind_method(D_METHOD("stop_light_estimation"), &OpenXRAndroidLightEstimationExtensionWrapper::stop_light_estimation);
	ClassDB::bind_method(D_METHOD("is_light_estimation_started"), &OpenXRAndroidLightEstimationExtensionWrapper::is_light_estimation_supported);

	ClassDB::bind_method(D_METHOD("set_light_estimate_types", "estimate_types"), &OpenXRAndroidLightEstimationExtensionWrapper::set_light_estimate_types);
	ClassDB::bind_method(D_METHOD("get_light_estimate_types"), &OpenXRAndroidLightEstimationExtensionWrapper::get_light_estimate_types);

	ClassDB::bind_method(D_METHOD("is_estimate_valid"), &OpenXRAndroidLightEstimationExtensionWrapper::is_estimate_valid);
	ClassDB::bind_method(D_METHOD("get_last_updated_time"), &OpenXRAndroidLightEstimationExtensionWrapper::get_last_updated_time);

	ClassDB::bind_method(D_METHOD("is_directional_light_valid"), &OpenXRAndroidLightEstimationExtensionWrapper::is_directional_light_valid);
	ClassDB::bind_method(D_METHOD("get_directional_light_intensity"), &OpenXRAndroidLightEstimationExtensionWrapper::get_directional_light_intensity);
	ClassDB::bind_method(D_METHOD("get_directional_light_direction"), &OpenXRAndroidLightEstimationExtensionWrapper::get_directional_light_direction);

	ClassDB::bind_method(D_METHOD("is_ambient_light_valid"), &OpenXRAndroidLightEstimationExtensionWrapper::is_ambient_light_valid);
	ClassDB::bind_method(D_METHOD("get_ambient_light_intensity"), &OpenXRAndroidLightEstimationExtensionWrapper::get_ambient_light_intensity);
	ClassDB::bind_method(D_METHOD("get_ambient_light_color_correction"), &OpenXRAndroidLightEstimationExtensionWrapper::get_ambient_light_color_correction);

	ClassDB::bind_method(D_METHOD("is_spherical_harmonics_ambient_valid"), &OpenXRAndroidLightEstimationExtensionWrapper::is_spherical_harmonics_ambient_valid);
	ClassDB::bind_method(D_METHOD("get_spherical_harmonics_ambient_coefficients"), &OpenXRAndroidLightEstimationExtensionWrapper::get_spherical_harmonics_ambient_coefficients);

	ClassDB::bind_method(D_METHOD("is_spherical_harmonics_total_valid"), &OpenXRAndroidLightEstimationExtensionWrapper::is_spherical_harmonics_total_valid);
	ClassDB::bind_method(D_METHOD("get_spherical_harmonics_total_coefficients"), &OpenXRAndroidLightEstimationExtensionWrapper::get_spherical_harmonics_total_coefficients);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "light_estimate_types", PROPERTY_HINT_FLAGS, "Directional Light,Ambient,Spherical Harmonics (Ambient),Spherical Harmonics (Total),Cubemap"), "set_light_estimate_types", "get_light_estimate_types");

	BIND_ENUM_CONSTANT(LIGHT_ESTIMATE_TYPE_DIRECTIONAL_LIGHT);
	BIND_ENUM_CONSTANT(LIGHT_ESTIMATE_TYPE_AMBIENT);
	BIND_ENUM_CONSTANT(LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_AMBIENT);
	BIND_ENUM_CONSTANT(LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_TOTAL);
	BIND_ENUM_CONSTANT(LIGHT_ESTIMATE_TYPE_CUBEMAP);
	BIND_ENUM_CONSTANT(LIGHT_ESTIMATE_TYPE_ALL);
}

void OpenXRAndroidLightEstimationExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (android_light_estimation_ext) {
		bool result = initialize_android_light_estimation_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize XR_ANDROID_light_estimation extension");
			android_light_estimation_ext = false;
		}
	}
}

void OpenXRAndroidLightEstimationExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

uint64_t OpenXRAndroidLightEstimationExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (android_light_estimation_ext) {
		system_light_estimation_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_light_estimation_properties);
	}
	return reinterpret_cast<uint64_t>(p_next_pointer);
}

bool OpenXRAndroidLightEstimationExtensionWrapper::initialize_android_light_estimation_extension(const XrInstance &instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateLightEstimatorANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyLightEstimatorANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetLightEstimateANDROID);

	return true;
}

void OpenXRAndroidLightEstimationExtensionWrapper::cleanup() {
	android_light_estimation_ext = false;
}

Dictionary OpenXRAndroidLightEstimationExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

bool OpenXRAndroidLightEstimationExtensionWrapper::start_light_estimation() {
	if (light_estimator != XR_NULL_HANDLE) {
		return true;
	}

	XrLightEstimatorCreateInfoANDROID create_info = {
		XR_TYPE_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID, // type
		nullptr, // next
	};

	XrResult result = xrCreateLightEstimatorANDROID(SESSION, &create_info, &light_estimator);
	if (XR_FAILED(result)) {
		light_estimator = XR_NULL_HANDLE;
		UtilityFunctions::printerr("Failed to start light estimation: ", get_openxr_api()->get_error_string(result));
		return false;
	}

	return true;
}

void OpenXRAndroidLightEstimationExtensionWrapper::stop_light_estimation() {
	if (light_estimator == XR_NULL_HANDLE) {
		return;
	}

	XrResult result = xrDestroyLightEstimatorANDROID(light_estimator);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to stop light estimation: ", get_openxr_api()->get_error_string(result));
		return;
	}

	clear_light_info();
	light_estimator = XR_NULL_HANDLE;
}

bool OpenXRAndroidLightEstimationExtensionWrapper::is_light_estimation_started() {
	return light_estimator != XR_NULL_HANDLE;
}

void OpenXRAndroidLightEstimationExtensionWrapper::clear_light_info() {
	estimate_info.state = XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID;
	directional_light_info.state = XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID;
	ambient_light_info.state = XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID;
	spherical_harmonics_ambient_info.state = XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID;
	spherical_harmonics_total_info.state = XR_LIGHT_ESTIMATE_STATE_INVALID_ANDROID;
}

void OpenXRAndroidLightEstimationExtensionWrapper::_on_process() {
	if (!is_light_estimation_started()) {
		return;
	}

	Ref<OpenXRAPIExtension> openxr_api = get_openxr_api();
	ERR_FAIL_COND(openxr_api.is_null());

	clear_light_info();

	XrLightEstimateGetInfoANDROID info = {
		XR_TYPE_LIGHT_ESTIMATE_GET_INFO_ANDROID, // type
		nullptr, // next
		(XrSpace)openxr_api->get_play_space(), // space
		openxr_api->get_predicted_display_time(), // time
	};

	void *next_pointer = nullptr;
	if (estimate_types.has_flag(LIGHT_ESTIMATE_TYPE_DIRECTIONAL_LIGHT)) {
		directional_light_info.next = next_pointer;
		next_pointer = &directional_light_info;
	}
	if (estimate_types.has_flag(LIGHT_ESTIMATE_TYPE_AMBIENT)) {
		ambient_light_info.next = next_pointer;
		next_pointer = &ambient_light_info;
	}
	if (estimate_types.has_flag(LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_AMBIENT)) {
		spherical_harmonics_ambient_info.next = next_pointer;
		next_pointer = &spherical_harmonics_ambient_info;
	}
	if (estimate_types.has_flag(LIGHT_ESTIMATE_TYPE_SPHERICAL_HARMONICS_TOTAL)) {
		spherical_harmonics_total_info.next = next_pointer;
		next_pointer = &spherical_harmonics_total_info;
	}
	estimate_info.next = next_pointer;

	XrResult result = xrGetLightEstimateANDROID(light_estimator, &info, &estimate_info);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to get light estimate: ", get_openxr_api()->get_error_string(result));
		return;
	}

	if (estimate_info.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID && estimate_info.lastUpdatedTime > last_update_uniforms_time) {
		last_update_uniforms_time == estimate_info.lastUpdatedTime;

		// @todo Update all the global uniforms
	}
}

void OpenXRAndroidLightEstimationExtensionWrapper::set_light_estimate_types(BitField<LightEstimateType> p_estimate_types) {
	estimate_types = p_estimate_types;
}

BitField<OpenXRAndroidLightEstimationExtensionWrapper::LightEstimateType> OpenXRAndroidLightEstimationExtensionWrapper::get_light_estimate_types() const {
	return estimate_types;
}

bool OpenXRAndroidLightEstimationExtensionWrapper::is_estimate_valid() const {
	return estimate_info.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID;
}

int64_t OpenXRAndroidLightEstimationExtensionWrapper::get_last_updated_time() const {
	return estimate_info.lastUpdatedTime;
}

bool OpenXRAndroidLightEstimationExtensionWrapper::is_directional_light_valid() const {
	return directional_light_info.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID;
}

Color OpenXRAndroidLightEstimationExtensionWrapper::get_directional_light_intensity() const {
	return Color(
			directional_light_info.intensity.x,
			directional_light_info.intensity.y,
			directional_light_info.intensity.z,
			1.0);
}

Vector3 OpenXRAndroidLightEstimationExtensionWrapper::get_directional_light_direction() const {
	return Vector3(
			directional_light_info.direction.x,
			directional_light_info.direction.y,
			directional_light_info.direction.z);
}

bool OpenXRAndroidLightEstimationExtensionWrapper::is_ambient_light_valid() const {
	return ambient_light_info.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID;
}

Color OpenXRAndroidLightEstimationExtensionWrapper::get_ambient_light_intensity() const {
	return Color(
			ambient_light_info.intensity.x,
			ambient_light_info.intensity.y,
			ambient_light_info.intensity.z,
			1.0);
}

Color OpenXRAndroidLightEstimationExtensionWrapper::get_ambient_light_color_correction() const {
	return Color(
			ambient_light_info.colorCorrection.x,
			ambient_light_info.colorCorrection.y,
			ambient_light_info.colorCorrection.z,
			1.0);
}

bool OpenXRAndroidLightEstimationExtensionWrapper::is_spherical_harmonics_ambient_valid() const {
	return spherical_harmonics_ambient_info.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID;
}

PackedVector3Array OpenXRAndroidLightEstimationExtensionWrapper::get_spherical_harmonics_ambient_coefficients() const {
	PackedVector3Array ret;
	ret.resize(9);
	Vector3 *arr_ptr = ret.ptrw();
	for (int i = 0; i < 9; i++) {
		arr_ptr[i] = Vector3(
				spherical_harmonics_ambient_info.coefficients[i][0],
				spherical_harmonics_ambient_info.coefficients[i][1],
				spherical_harmonics_ambient_info.coefficients[i][2]);
	}
	return ret;
}

bool OpenXRAndroidLightEstimationExtensionWrapper::is_spherical_harmonics_total_valid() const {
	return spherical_harmonics_total_info.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID;
}

PackedVector3Array OpenXRAndroidLightEstimationExtensionWrapper::get_spherical_harmonics_total_coefficients() const {
	PackedVector3Array ret;
	ret.resize(9);
	Vector3 *arr_ptr = ret.ptrw();
	for (int i = 0; i < 9; i++) {
		arr_ptr[i] = Vector3(
				spherical_harmonics_total_info.coefficients[i][0],
				spherical_harmonics_total_info.coefficients[i][1],
				spherical_harmonics_total_info.coefficients[i][2]);
	}
	return ret;
}

static void create_shader_global_uniform(const String &p_name, RenderingServer::GlobalShaderParameterType p_type, Variant p_value, RenderingServer *p_rendering_server, ProjectSettings *p_project_settings, bool p_is_editor) {
	String setting_name = "shader_globals/" + p_name;
	if (!p_project_settings->has_setting(setting_name)) {
		p_rendering_server->global_shader_parameter_add(p_name, p_type, p_value);
		if (p_is_editor) {
			String type_name;
			switch (p_type) {
				case RenderingServer::GLOBAL_VAR_TYPE_BOOL: {
					type_name = "bool";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_VEC2: {
					type_name = "vec3";
				} break;
				case RenderingServer::GLOBAL_VAR_TYPE_MAT4: {
					type_name = "mat4";
				} break;
			}

			Variant setting_value = p_value;
			if (p_type == RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY) {
				// In ProjectSettings, this uses a path as a value.
				setting_value = "";
			}

			Dictionary d;
			d["type"] = type_name;
			d["value"] = setting_value;
			p_project_settings->set(setting_name, d);
		}
	}
}

static void remove_shader_global_uniform(const String &p_name, RenderingServer *p_rendering_server, ProjectSettings *p_project_settings) {
	String setting_name = "shader_globals/" + p_name;
	if (p_project_settings->has_setting(setting_name)) {
		p_rendering_server->global_shader_parameter_remove(p_name);
		p_project_settings->clear(setting_name);
	}
}

void OpenXRAndroidLightEstimationExtensionWrapper::setup_global_uniforms() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	bool enabled = project_settings->get_setting_with_override("xr/openxr/extensions/meta/environment_depth");

	if (!enabled) {
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_TEXEL_SIZE_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_LEFT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_RIGHT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT_NAME, rs, project_settings);
		remove_shader_global_uniform(META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT_NAME, rs, project_settings);

		already_setup_global_uniforms = false;
		return;
	}

	if (already_setup_global_uniforms) {
		return;
	}

	Engine *engine = Engine::get_singleton();
	ERR_FAIL_NULL(engine);

	bool is_editor = engine->is_editor_hint();

	// Set this right away, to prevent getting in a loop of project settings changes.
	already_setup_global_uniforms = true;

	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, RenderingServer::GLOBAL_VAR_TYPE_BOOL, false, rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY, Variant(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TEXEL_SIZE_NAME, RenderingServer::GLOBAL_VAR_TYPE_VEC2, Vector2(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_PROJECTION_VIEW_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_INV_PROJECTION_VIEW_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_FROM_CAMERA_PROJECTION_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
	create_shader_global_uniform(META_ENVIRONMENT_DEPTH_TO_CAMERA_PROJECTION_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection(), rs, project_settings, is_editor);
}

