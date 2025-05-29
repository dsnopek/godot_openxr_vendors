/**************************************************************************/
/*  openxr_meta_environment_depth_extension_wrapper.cpp                   */
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

#include "extensions/openxr_meta_environment_depth_extension_wrapper.h"

#ifdef ANDROID_ENABLED
#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_OPENGL_ES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <jni.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>

#include <openxr/openxr_platform.h>
#endif

#include <openxr/internal/xr_linear.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static const char *META_ENVIRONMENT_DEPTH_AVAILABLE_NAME = "META_ENVIRONMENT_DEPTH_AVAILABLE";
static const char *META_ENVIRONMENT_DEPTH_TEXTURE_NAME = "META_ENVIRONMENT_DEPTH_TEXTURE";
static const char *META_ENVIRONMENT_DEPTH_PROJECTION_LEFT_NAME = "META_ENVIRONMENT_DEPTH_PROJECTION_LEFT";
static const char *META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT_NAME = "META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT";
static const char *META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT_NAME = "META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT";
static const char *META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT_NAME = "META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT";

static const char *META_ENVIRONMENT_DEPTH_REPROJECTION_SHADER_CODE =
		"shader_type spatial;\n"
		//"render_mode unshaded, shadows_disabled, cull_disabled, depth_test_disabled, depth_draw_always;\n"
		//"render_mode shadow_to_opacity, shadows_disabled, cull_disabled, depth_test_disabled, depth_draw_always;\n"
		"render_mode unshaded, shadows_disabled, cull_disabled, depth_test_disabled, depth_draw_always;\n"
		"global uniform bool META_ENVIRONMENT_DEPTH_AVAILABLE;\n"
		"global uniform highp sampler2DArray META_ENVIRONMENT_DEPTH_TEXTURE : filter_nearest;\n"
		"global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_LEFT;\n"
		"global uniform highp mat4 META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT;\n"
		"global uniform highp mat4 META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT;\n"
		"global uniform highp mat4 META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT;\n"
		"varying vec2 uv_interp;\n"
		"void vertex() {\n"
		"\tuv_interp = VERTEX.xy * 0.5 + 0.5;\n"
		"\tPOSITION = vec4(VERTEX.xyz, 1.0);\n"
		"}\n"
		"void fragment() {\n"
		"\thighp mat4 depth_proj = uint(VIEW_INDEX) == uint(0) ? META_ENVIRONMENT_DEPTH_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT;\n"
		"\thighp mat4 depth_inv_proj = uint(VIEW_INDEX) == uint(0) ? META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT : META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT;\n"
		"\thighp vec4 clip = vec4(uv_interp * 2.0 - 1.0, 1.0, 1.0);\n"
		//"\thighp vec4 world_pos = INV_PROJECTION_MATRIX * INV_VIEW_MATRIX * clip;\n"
		//"\thighp vec4 world_pos = INV_VIEW_MATRIX * INV_PROJECTION_MATRIX * clip;\n"
		"\thighp vec4 world_pos = INV_VIEW_MATRIX * INV_PROJECTION_MATRIX * clip;\n"
		"\tworld_pos /= world_pos.w;\n"
		"\thighp vec4 reprojected = depth_proj * world_pos;\n"
		"\treprojected /= reprojected.w;\n"
		"\thighp vec2 reprojected_uv = reprojected.xy * 0.5 + 0.5;\n"
		"\thighp float depth = texture(META_ENVIRONMENT_DEPTH_TEXTURE, vec3(reprojected_uv, float(VIEW_INDEX))).r;\n"
		"\tif (depth == 0.0) {\n"
		"\tdiscard;\n"
		"\t}\n"
		"\thighp vec4 clip_back = vec4(reprojected.xy, depth * 2.0 - 1.0, 1.0);\n"
		"\thighp vec4 world_back = depth_inv_proj * clip_back;\n"
		"\tworld_back /= world_back.w;\n"
		"\thighp vec4 cur_clip = PROJECTION_MATRIX * VIEW_MATRIX * world_back;\n"
		"\thighp float cur_ndc_z = cur_clip.z / cur_clip.w;\n"
		// Test:
		//"\tALBEDO = vec3(depth);\n"
		"\tALBEDO = vec3(cur_ndc_z * 0.5 + 0.5);\n"
		//"\tALBEDO = vec3(0.0);\n"
		//"\tDEPTH = 1.0 - (cur_ndc_z * 0.5 + 0.5);\n"
		"\tDEPTH = (cur_ndc_z * 0.5 + 0.5);\n"
		"}\n";

OpenXRMetaEnvironmentDepthExtensionWrapper *OpenXRMetaEnvironmentDepthExtensionWrapper::singleton = nullptr;

OpenXRMetaEnvironmentDepthExtensionWrapper *OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRMetaEnvironmentDepthExtensionWrapper());
	}
	return singleton;
}

OpenXRMetaEnvironmentDepthExtensionWrapper::OpenXRMetaEnvironmentDepthExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRMetaEnvironmentDepthExtensionWrapper singleton already exists.");

	request_extensions[XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME] = &meta_environment_depth_ext;
	singleton = this;

#ifndef ANDROID_ENABLED
	graphics_api = GRAPHICS_API_UNSUPPORTED;
#endif
}

OpenXRMetaEnvironmentDepthExtensionWrapper::~OpenXRMetaEnvironmentDepthExtensionWrapper() {
	cleanup();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_environment_depth_supported"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_supported);
	ClassDB::bind_method(D_METHOD("is_hand_removal_supported"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_hand_removal_supported);

	ClassDB::bind_method(D_METHOD("start_environment_depth"), &OpenXRMetaEnvironmentDepthExtensionWrapper::start_environment_depth);
	ClassDB::bind_method(D_METHOD("stop_environment_depth"), &OpenXRMetaEnvironmentDepthExtensionWrapper::stop_environment_depth);
	ClassDB::bind_method(D_METHOD("is_environment_depth_started"), &OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_started);

	ClassDB::bind_method(D_METHOD("set_hand_removal_enabled", "enable"), &OpenXRMetaEnvironmentDepthExtensionWrapper::set_hand_removal_enabled);
	ClassDB::bind_method(D_METHOD("get_hand_removal_enabled"), &OpenXRMetaEnvironmentDepthExtensionWrapper::get_hand_removal_enabled);
}

Dictionary OpenXRMetaEnvironmentDepthExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (meta_environment_depth_ext) {
		bool result = initialize_meta_environment_depth_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize XR_META_environment_depth extension");
			meta_environment_depth_ext = false;
		}
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_session_created(uint64_t p_session) {
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_session_destroyed() {
	destroy_depth_provider();
}

static void xrMatrix4x4f_to_godot_projection(XrMatrix4x4f *m, Projection &p) {
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			p.columns[j][i] = m->m[j * 4 + i];
		}
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_pre_render() {
#ifdef ANDROID_ENABLED
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (unlikely(graphics_api == GRAPHICS_API_UNKNOWN)) {
		String rendering_driver = RenderingServer::get_singleton()->get_current_rendering_driver_name();
		if (rendering_driver.contains("opengl")) {
			graphics_api = GRAPHICS_API_OPENGL;
		} else if (rendering_driver == "vulkan") {
			graphics_api = GRAPHICS_API_VULKAN;
		} else {
			graphics_api = GRAPHICS_API_UNSUPPORTED;
		}

		setup_global_uniforms();
	}

	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, false);
	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, RID());

	if (depth_provider == XR_NULL_HANDLE || !depth_provider_started) {
		return;
	}

	Ref<OpenXRAPIExtension> openxr_api = get_openxr_api();
	ERR_FAIL_COND(openxr_api.is_null());

	XrEnvironmentDepthImageAcquireInfoMETA acquire_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META, // type
		nullptr, // next
		(XrSpace)openxr_api->get_play_space(),
		openxr_api->get_predicted_display_time(),
	};

	XrEnvironmentDepthImageMETA depth_image = {
		XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META, // type
		nullptr, // next
		0, // swapchainIndex
		0.0, // nearZ
		0.0, // farZ
		{
				// views
				{
						XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META, // type
						nullptr, // next
				},
				{
						XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META, // type
						nullptr, // next
				},
		}
	};

	XrResult result = xrAcquireEnvironmentDepthImageMETA(depth_provider, &acquire_info, &depth_image);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to acquire environment depth image: ", get_openxr_api()->get_error_string(result));
		return;
	}

	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, true);
	rs->global_shader_parameter_set(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, depth_swapchain_textures[depth_image.swapchainIndex]);

	for (int i = 0; i < 2; i++) {
		XrPosef local_from_depth_eye = depth_image.views[i].pose;
		XrPosef depth_eye_from_local;
		XrPosef_Invert(&depth_eye_from_local, &local_from_depth_eye);

		XrMatrix4x4f view_mat;
		XrMatrix4x4f_CreateFromRigidTransform(&view_mat, &depth_eye_from_local);

		XrMatrix4x4f projection_mat;
		XrMatrix4x4f_CreateProjectionFov(
				&projection_mat,
				GRAPHICS_OPENGL,
				depth_image.views[i].fov,
				depth_image.nearZ,
				std::isfinite(depth_image.farZ) ? depth_image.farZ : 0);

		// Copy into Godot projections.
		Projection godot_view_mat;
		xrMatrix4x4f_to_godot_projection(&view_mat, godot_view_mat);
		Projection godot_projection_mat;
		xrMatrix4x4f_to_godot_projection(&projection_mat, godot_projection_mat);

		Projection godot_combined_mat = godot_projection_mat * godot_view_mat;
		rs->global_shader_parameter_set(i == 0 ? META_ENVIRONMENT_DEPTH_PROJECTION_LEFT_NAME : META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT_NAME, godot_combined_mat);
		rs->global_shader_parameter_set(i == 0 ? META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT_NAME : META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT_NAME, godot_combined_mat.inverse());
	}
#endif
}

uint64_t OpenXRMetaEnvironmentDepthExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (meta_environment_depth_ext) {
		system_depth_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_depth_properties);
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::start_environment_depth() {
	if (depth_provider == XR_NULL_HANDLE) {
		if (!create_depth_provider()) {
			return;
		}
	}

	XrResult result = xrStartEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to start environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	setup_global_uniforms();

	depth_provider_started = true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::stop_environment_depth() {
	ERR_FAIL_NULL(depth_provider);

	XrResult result = xrStopEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to stop environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	depth_provider_started = false;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_started() {
	return depth_provider_started;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_hand_removal_enabled(bool p_enable) {
	if (depth_provider == XR_NULL_HANDLE) {
		if (!create_depth_provider()) {
			return;
		}
	}

	XrEnvironmentDepthHandRemovalSetInfoMETA info = {
		XR_TYPE_ENVIRONMENT_DEPTH_HAND_REMOVAL_SET_INFO_META, // type
		nullptr, // next
		p_enable,
	};

	XrResult result = xrSetEnvironmentDepthHandRemovalMETA(depth_provider, &info);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to set hand removal enabled: ", get_openxr_api()->get_error_string(result));
		return;
	}

	hand_removal_enabled = p_enable;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::get_hand_removal_enabled() const {
	return hand_removal_enabled;
}

RID OpenXRMetaEnvironmentDepthExtensionWrapper::get_reprojection_mesh() {
	if (!reprojection_mesh.is_valid()) {
		RenderingServer *rs = RenderingServer::get_singleton();
		ERR_FAIL_NULL_V(rs, RID());

		reprojection_shader = rs->shader_create();
		rs->shader_set_code(reprojection_shader, META_ENVIRONMENT_DEPTH_REPROJECTION_SHADER_CODE);

		reprojection_material = rs->material_create();
		rs->material_set_shader(reprojection_material, reprojection_shader);
		rs->material_set_render_priority(reprojection_material, -1);

		PackedVector3Array vertices;
		vertices.resize(3);
		vertices[0] = Vector3(-1.0f, -1.0f, 1.0f);
		vertices[1] = Vector3(3.0f, -1.0f, 1.0f);
		vertices[2] = Vector3(-1.0f, 3.0f, 1.0f);

		Array arr;
		arr.resize(RenderingServer::ARRAY_MAX);
		arr[RenderingServer::ARRAY_VERTEX] = vertices;

		reprojection_mesh = rs->mesh_create();
		rs->mesh_add_surface_from_arrays(reprojection_mesh, RenderingServer::PRIMITIVE_TRIANGLES, arr);
		rs->mesh_surface_set_material(reprojection_mesh, 0, reprojection_material);
	}

	return reprojection_mesh;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::setup_global_uniforms() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	TypedArray<StringName> existing_uniforms = rs->global_shader_parameter_get_list();

	if (!existing_uniforms.has(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME)) {
		rs->global_shader_parameter_add(META_ENVIRONMENT_DEPTH_AVAILABLE_NAME, RenderingServer::GLOBAL_VAR_TYPE_BOOL, false);
	}
	if (!existing_uniforms.has(META_ENVIRONMENT_DEPTH_TEXTURE_NAME)) {
		rs->global_shader_parameter_add(META_ENVIRONMENT_DEPTH_TEXTURE_NAME, RenderingServer::GLOBAL_VAR_TYPE_SAMPLER2DARRAY, Variant());
	}
	if (!existing_uniforms.has(META_ENVIRONMENT_DEPTH_PROJECTION_LEFT_NAME)) {
		rs->global_shader_parameter_add(META_ENVIRONMENT_DEPTH_PROJECTION_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection());
	}
	if (!existing_uniforms.has(META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT_NAME)) {
		rs->global_shader_parameter_add(META_ENVIRONMENT_DEPTH_PROJECTION_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection());
	}
	if (!existing_uniforms.has(META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT_NAME)) {
		rs->global_shader_parameter_add(META_ENVIRONMENT_DEPTH_INV_PROJECTION_LEFT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection());
	}
	if (!existing_uniforms.has(META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT_NAME)) {
		rs->global_shader_parameter_add(META_ENVIRONMENT_DEPTH_INV_PROJECTION_RIGHT_NAME, RenderingServer::GLOBAL_VAR_TYPE_MAT4, Projection());
	}
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::initialize_meta_environment_depth_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStartEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthSwapchainMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateEnvironmentDepthSwapchainImagesMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetEnvironmentDepthSwapchainStateMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrAcquireEnvironmentDepthImageMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetEnvironmentDepthHandRemovalMETA);

	return true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::cleanup() {
	meta_environment_depth_ext = false;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::create_depth_provider() {
#ifdef ANDROID_ENABLED
	if (!meta_environment_depth_ext) {
		UtilityFunctions::printerr("Environment depth not supported");
		return false;
	}
	if (graphics_api == GRAPHICS_API_UNKNOWN) {
		_on_pre_render();
	}
	if (graphics_api == GRAPHICS_API_UNSUPPORTED) {
		UtilityFunctions::printerr("Environment depth is not supported with the current graphics API");
		return false;
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rs, false);

	XrResult result;

	XrEnvironmentDepthProviderCreateInfoMETA provider_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	result = xrCreateEnvironmentDepthProviderMETA(SESSION, &provider_create_info, &depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create environment depth provider: ", get_openxr_api()->get_error_string(result));
		return false;
	}

	XrEnvironmentDepthSwapchainCreateInfoMETA swapchain_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	result = xrCreateEnvironmentDepthSwapchainMETA(depth_provider, &swapchain_create_info, &depth_swapchain);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create environment depth swapchain: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	XrEnvironmentDepthSwapchainStateMETA swapchain_state = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_STATE_META, // type
		nullptr, // next
		0, // width
		0, // height
	};

	result = xrGetEnvironmentDepthSwapchainStateMETA(depth_swapchain, &swapchain_state);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to get environment depth swapchain state: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	uint32_t swapchain_length = 0;

	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to get environment depth swapchain image count: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	if (graphics_api == GRAPHICS_API_OPENGL) {
		LocalVector<XrSwapchainImageOpenGLESKHR> swapchain_images;

		swapchain_images.resize(swapchain_length);
		for (auto &image : swapchain_images) {
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
			image.next = nullptr;
			image.image = 0;
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader *)swapchain_images.ptr());
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to get environment depth swapchain images: ", get_openxr_api()->get_error_string(result));
			destroy_depth_provider();
			return false;
		}

		depth_swapchain_textures.reserve(swapchain_length);

		for (const auto &image : swapchain_images) {
			RID texture = rs->texture_create_from_native_handle(
					RenderingServer::TextureType::TEXTURE_TYPE_LAYERED,
					Image::Format::FORMAT_RH, // GL_DEPTH_COMPONENT16
					image.image,
					swapchain_state.width,
					swapchain_state.height,
					1,
					2,
					RenderingServer::TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY);

			depth_swapchain_textures.push_back(texture);
		}
	} else if (graphics_api == GRAPHICS_API_VULKAN) {
		LocalVector<XrSwapchainImageVulkanKHR> swapchain_images;

		swapchain_images.resize(swapchain_length);
		for (auto &image : swapchain_images) {
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
			image.next = nullptr;
			image.image = 0;
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader *)swapchain_images.ptr());
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to get environment depth swapchain images: ", get_openxr_api()->get_error_string(result));
			destroy_depth_provider();
			return false;
		}

		depth_swapchain_textures.reserve(swapchain_length);

		RenderingDevice *rendering_device = RenderingServer::get_singleton()->get_rendering_device();

		for (const auto &image : swapchain_images) {
			RID rd_texture = rendering_device->texture_create_from_extension(
					RenderingDevice::TEXTURE_TYPE_2D_ARRAY,
					RenderingDevice::DATA_FORMAT_D16_UNORM,
					RenderingDevice::TEXTURE_SAMPLES_1,
					RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT,
					reinterpret_cast<uint64_t>(image.image),
					swapchain_state.width,
					swapchain_state.height,
					1,
					2);

			RID texture = rs->texture_rd_create(rd_texture, RenderingServer::TextureLayeredType::TEXTURE_LAYERED_2D_ARRAY);

			depth_swapchain_textures.push_back(texture);
		}
	}

	return true;
#else
	return false;
#endif
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::destroy_depth_provider() {
	if (depth_swapchain != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthSwapchainMETA(depth_swapchain);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to destroy environment depth swapchain: ", get_openxr_api()->get_error_string(result));
		}
		depth_swapchain = XR_NULL_HANDLE;
	}

	depth_swapchain_textures.clear();

	if (depth_provider != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthProviderMETA(depth_provider);
		if (XR_FAILED(result)) {
			UtilityFunctions::printerr("Failed to destroy environment depth provider: ", get_openxr_api()->get_error_string(result));
		}
		depth_provider = XR_NULL_HANDLE;
		hand_removal_enabled = false;
	}
}
