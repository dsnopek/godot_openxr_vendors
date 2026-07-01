/**************************************************************************/
/*  openxr_android_global_passthrough_dimming_extension.cpp               */
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

#include "extensions/openxr_android_global_passthrough_dimming_extension.h"

#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidGlobalPassthroughDimmingExtension *OpenXRAndroidGlobalPassthroughDimmingExtension::singleton = nullptr;

OpenXRAndroidGlobalPassthroughDimmingExtension *OpenXRAndroidGlobalPassthroughDimmingExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidGlobalPassthroughDimmingExtension());
	}
	return singleton;
}

OpenXRAndroidGlobalPassthroughDimmingExtension::OpenXRAndroidGlobalPassthroughDimmingExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidGlobalPassthroughDimmingExtension singleton already exists.");

	request_extensions[XR_ANDROID_GLOBAL_PASSTHROUGH_DIMMING_EXTENSION_NAME] = &android_global_passthrough_dimming_ext;
	singleton = this;
}

OpenXRAndroidGlobalPassthroughDimmingExtension::~OpenXRAndroidGlobalPassthroughDimmingExtension() {
	cleanup();
	singleton = nullptr;
}

Dictionary OpenXRAndroidGlobalPassthroughDimmingExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRAndroidGlobalPassthroughDimmingExtension::is_enabled);
	ClassDB::bind_method(D_METHOD("get_supported_dimming_levels"), &OpenXRAndroidGlobalPassthroughDimmingExtension::get_supported_dimming_levels);
	ClassDB::bind_method(D_METHOD("get_dimming_level"), &OpenXRAndroidGlobalPassthroughDimmingExtension::get_dimming_level);
	ClassDB::bind_method(D_METHOD("set_dimming_level", "dimming_level"), &OpenXRAndroidGlobalPassthroughDimmingExtension::set_dimming_level);

	ADD_SIGNAL(MethodInfo("openxr_android_global_passthrough_dimming_level_changed"));
}

bool OpenXRAndroidGlobalPassthroughDimmingExtension::is_enabled() const {
	return android_global_passthrough_dimming_ext && global_dimming_properties.supportsGlobalDimming;
}

PackedFloat32Array OpenXRAndroidGlobalPassthroughDimmingExtension::get_supported_dimming_levels() {
	ERR_FAIL_COND_V(!is_enabled(), PackedFloat32Array());

	Ref<OpenXRAPIExtension> openxr_api = get_openxr_api();
	ERR_FAIL_NULL_V(openxr_api, PackedFloat32Array());

	XrInstance xr_instance = (XrInstance)openxr_api->get_instance();
	XrSystemId xr_system_id = (XrSystemId)openxr_api->get_system_id();
	ERR_FAIL_COND_V(xr_system_id == XR_NULL_SYSTEM_ID, PackedFloat32Array());

	uint32_t count = 0;
	XrResult result = xrEnumerateSupportedGlobalDimmingLevelsANDROID(xr_instance, xr_system_id, 0, &count, nullptr);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrEnumerateSupportedGlobalDimmingLevelsANDROID failed to get dimming level count: %s", openxr_api->get_error_string(result)));
		return PackedFloat32Array();
	}

	PackedFloat32Array ret;
	ret.resize(count);

	result = xrEnumerateSupportedGlobalDimmingLevelsANDROID(xr_instance, xr_system_id, count, &count, ret.ptrw());
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrEnumerateSupportedGlobalDimmingLevelsANDROID failed: %s", openxr_api->get_error_string(result)));
		return PackedFloat32Array();
	}

	return ret;
}

float OpenXRAndroidGlobalPassthroughDimmingExtension::get_dimming_level() const {
	return global_dimming_frame_end_info.globalDimmingLevel;
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::set_dimming_level(float p_dimming_level) {
	ERR_FAIL_COND(!is_enabled());
	global_dimming_frame_end_info.globalDimmingLevel = p_dimming_level;
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::_on_instance_created(uint64_t p_instance) {
	if (android_global_passthrough_dimming_ext) {
		bool result = initialize_android_global_passthrough_dimming_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize XR_ANDROID_global_passthrough_dimming extension");
			android_global_passthrough_dimming_ext = false;
		}
	}
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::_on_session_created(uint64_t p_session) {
	if (is_enabled()) {
		get_openxr_api()->register_frame_info_extension(this);
		_update_current_dimming_level();
	}
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::_on_session_destroyed() {
	if (is_enabled()) {
		get_openxr_api()->unregister_frame_info_extension(this);
	}
}

uint64_t OpenXRAndroidGlobalPassthroughDimmingExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (android_global_passthrough_dimming_ext) {
		global_dimming_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&global_dimming_properties);
	}

	return reinterpret_cast<uint64_t>(p_next_pointer);
}

uint64_t OpenXRAndroidGlobalPassthroughDimmingExtension::_set_frame_end_info_and_get_next_pointer(void *p_next_pointer) {
	if (!is_enabled()) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	global_dimming_frame_end_info.next = p_next_pointer;
	return reinterpret_cast<uint64_t>(&global_dimming_frame_end_info);
}

bool OpenXRAndroidGlobalPassthroughDimmingExtension::_on_event_polled(const void *p_event) {
	if (!android_global_passthrough_dimming_ext) {
		return false;
	}

	if (static_cast<const XrEventDataBuffer *>(p_event)->type == XR_TYPE_EVENT_DATA_GLOBAL_DIMMING_LEVEL_CHANGED_ANDROID) {
		_update_current_dimming_level();
		return true;
	}

	return false;
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::_update_current_dimming_level() {
	float current_dimming_level = -1.0f;

	XrResult result = xrGetGlobalDimmingLevelANDROID(SESSION, &current_dimming_level);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr(vformat("xrGetGlobalDimmingLevelANDROID failed: %s", get_openxr_api()->get_error_string(result)));
		return;
	}

	if (current_dimming_level != global_dimming_frame_end_info.globalDimmingLevel) {
		global_dimming_frame_end_info.globalDimmingLevel = current_dimming_level;
		emit_signal("openxr_android_global_passthrough_dimming_level_changed");
	}
}

bool OpenXRAndroidGlobalPassthroughDimmingExtension::initialize_android_global_passthrough_dimming_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrGetGlobalDimmingLevelANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSupportedGlobalDimmingLevelsANDROID);

	return true;
}

void OpenXRAndroidGlobalPassthroughDimmingExtension::cleanup() {
	android_global_passthrough_dimming_ext = false;
}
