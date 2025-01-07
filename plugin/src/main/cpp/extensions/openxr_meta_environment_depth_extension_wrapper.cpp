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

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
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

void OpenXRMetaEnvironmentDepthExtensionWrapper::cleanup() {
	meta_environment_depth_ext = false;
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
	XrEnvironmentDepthProviderCreateInfoMETA create_info = {
		XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META, // type
		nullptr, // next
		0, // createFlags
	};

	XrResult result = xrCreateEnvironmentDepthProviderMETA(SESSION, &create_info, &depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Failed to create environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::_on_session_destroyed() {
	if (depth_provider != XR_NULL_HANDLE) {
		XrResult result = xrDestroyEnvironmentDepthProviderMETA(depth_provider);
		if (XR_FAILED(result)) {
			UtilityFunctions::print_verbose("Failed to destroy environment depth provider: ", get_openxr_api()->get_error_string(result));
		}
		depth_provider = XR_NULL_HANDLE;
		hand_removal_enabled = false;
	}
}

uint64_t OpenXRMetaEnvironmentDepthExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	system_depth_properties.next = p_next_pointer;
	return reinterpret_cast<uint64_t>(&system_depth_properties);
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::start_environment_depth() {
	ERR_FAIL_NULL(depth_provider);

	XrResult result = xrStartEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Unable to start environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	depth_provider_started = true;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::stop_environment_depth() {
	ERR_FAIL_NULL(depth_provider);

	XrResult result = xrStopEnvironmentDepthProviderMETA(depth_provider);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Unable to stop environment depth provider: ", get_openxr_api()->get_error_string(result));
		return;
	}

	depth_provider_started = false;
}

bool OpenXRMetaEnvironmentDepthExtensionWrapper::is_environment_depth_started() {
	return depth_provider_started;
}

void OpenXRMetaEnvironmentDepthExtensionWrapper::set_hand_removal_enabled(bool p_enable) {
	ERR_FAIL_NULL(depth_provider);

	XrEnvironmentDepthHandRemovalSetInfoMETA info = {
		XR_TYPE_ENVIRONMENT_DEPTH_HAND_REMOVAL_SET_INFO_META, // type
		nullptr, // next
		p_enable,
	};

	XrResult result = xrSetEnvironmentDepthHandRemovalMETA(depth_provider, &info);
	if (XR_FAILED(result)) {
		UtilityFunctions::print_verbose("Unable to set hand removal enabled: ", get_openxr_api()->get_error_string(result));
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
