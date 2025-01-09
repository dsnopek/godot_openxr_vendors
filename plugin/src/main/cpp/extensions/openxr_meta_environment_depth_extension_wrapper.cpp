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

#ifdef ANDROID
#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_OPENGL_ES
#include <jni.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
#define XR_USE_GRAPHICS_API_OPENGL
#endif

#include <openxr/openxr_platform.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

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

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_process() {
	/*
	if (depth_provider != XR_NULL_HANDLE && depth_swapchain == XR_NULL_HANDLE) {
		// Don't attempt to setup the swapchain until after the first frame.
		if (first_frame) {
			first_frame = false;
			return;
		}
		if (!setup_depth_swapchain()) {
			return;
		}
	}
	*/
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_pre_render() {
	if (unlikely(graphics_api == GRAPHICS_API_UNKNOWN)) {
		String rendering_driver = RenderingServer::get_singleton()->get_current_rendering_driver_name();
		if (rendering_driver.contains("opengl")) {
			graphics_api = GRAPHICS_API_OPENGL;
		} /*else if (rendering_driver == "vulkan") {
			graphics_api = GRAPHICS_API_VULKAN;
		} */else {
			graphics_api = GRAPHICS_API_UNSUPPORTED;
		}
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_pre_draw_viewport(const RID &p_viewport) {
	if (depth_provider == XR_NULL_HANDLE || graphics_api == GRAPHICS_API_UNSUPPORTED) {
		return;
	}

	/*
	if (depth_swapchain == XR_NULL_HANDLE) {
		// Don't attempt to setup the swapchain until after the first frame.
		if (first_frame) {
			first_frame = false;
			return;
		}
		if (!setup_depth_swapchain()) {
			return;
		}
	}
	*/

	if (!depth_provider_started) {
		return;
	}


	// @todo Acquire the swapchain image.
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
		if (!setup_depth_swapchain()) {
			return;
		}
	}

	XrResult result = xrStartEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Failed to start environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	depth_provider_started = true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::stop_environment_depth() {
	ERR_FAIL_NULL(depth_provider);

	XrResult result = xrStopEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Failed to stop environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	depth_provider_started = false;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_started() {
	return depth_provider_started;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_hand_removal_enabled(bool p_enable) {
	if (depth_provider == XR_NULL_HANDLE) {
		if (!setup_depth_swapchain()) {
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
		UtilityFunctions::print_verbose("Failed to set hand removal enabled: ", get_openxr_api()->get_error_string(result));
		return;
	}

	hand_removal_enabled = p_enable;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::get_hand_removal_enabled() const {
	return hand_removal_enabled;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::initialize_meta_environment_depth_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStartEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrStopEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyEnvironmentDepthProviderMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateEnvironmentDepthSwapchainImagesMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetEnvironmentDepthSwapchainStateMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrAcquireEnvironmentDepthImageMETA);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetEnvironmentDepthHandRemovalMETA);

	return true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::cleanup() {
	meta_environment_depth_ext = false;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::setup_depth_swapchain() {
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
		UtilityFunctions::print_verbose("Failed to create environment depth provider: ", get_openxr_api()->get_error_string(result));
		return false;
	}

	XrEnvironmentDepthSwapchainCreateInfoMETA swapchain_create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	result = xrCreateEnvironmentDepthSwapchainMETA(depth_provider, &swapchain_create_info, &depth_swapchain);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Failed to create environment depth swapchain: ", get_openxr_api()->get_error_string(result));
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
		UtilityFunctions::print_verbose("Failed to get environment depth swapchain state: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	uint32_t swapchain_length = 0;

	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Failed to get environment depth swapchain image count: ", get_openxr_api()->get_error_string(result));
		destroy_depth_provider();
		return false;
	}

	if (graphics_api == GRAPHICS_API_OPENGL) {
#ifdef ANDROID
		LocalVector<XrSwapchainImageOpenGLESKHR> swapchain_images;
#else
		LocalVector<XrSwapchainImageOpenGLKHR> swapchain_images;
#endif

		swapchain_images.resize(swapchain_length);
		for (auto &image : swapchain_images) {
#ifdef ANDROID
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
#else
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
#endif
			image.next = nullptr;
			image.image = 0;
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(depth_swapchain, swapchain_length, &swapchain_length, (XrSwapchainImageBaseHeader *)swapchain_images.ptr());
		if (XR_FAILED(result)) {
			UtilityFunctions::print_verbose("Failed to get environment depth swapchain images: ", get_openxr_api()->get_error_string(result));
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
	}

	return true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::destroy_depth_provider() {
	if (depth_swapchain != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthSwapchainMETA(depth_swapchain);
		if (XR_FAILED(result)) {
			UtilityFunctions::print_verbose("Failed to destroy environment depth swapchain: ", get_openxr_api()->get_error_string(result));
		}
		depth_swapchain = XR_NULL_HANDLE;
	}

	depth_swapchain_textures.clear();

	if (depth_provider != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthProviderMETA(depth_provider);
		if (XR_FAILED(result)) {
			UtilityFunctions::print_verbose("Failed to destroy environment depth provider: ", get_openxr_api()->get_error_string(result));
		}
		depth_provider = XR_NULL_HANDLE;
		hand_removal_enabled = false;
	}
}
