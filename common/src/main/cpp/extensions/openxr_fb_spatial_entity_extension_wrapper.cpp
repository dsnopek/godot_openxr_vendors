/**************************************************************************/
/*  openxr_fb_spatial_entity_extension_wrapper.cpp                        */
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

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRFbSpatialEntityExtensionWrapper *OpenXRFbSpatialEntityExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityExtensionWrapper *OpenXRFbSpatialEntityExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityExtensionWrapper::OpenXRFbSpatialEntityExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbSpatialEntityExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_EXTENSION_NAME] = &fb_spatial_entity_ext;
	singleton = this;
}

OpenXRFbSpatialEntityExtensionWrapper::~OpenXRFbSpatialEntityExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_supported"), &OpenXRFbSpatialEntityExtensionWrapper::is_spatial_entity_supported);
}

void OpenXRFbSpatialEntityExtensionWrapper::cleanup() {
	fb_spatial_entity_ext = false;
}

Dictionary OpenXRFbSpatialEntityExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_ext) {
		bool result = initialize_fb_spatial_entity_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity extension");
			fb_spatial_entity_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityExtensionWrapper::initialize_fb_spatial_entity_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateSpatialAnchorFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceUuidFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrEnumerateSpaceSupportedComponentsFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetSpaceComponentStatusFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrGetSpaceComponentStatusFB);

	return true;
}

/*
uint64_t OpenXRFbSpatialEntityExtensionWrapper::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {

}
*/

bool OpenXRFbSpatialEntityExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB) {
		on_set_component_enabled_complete((const XrEventDataSpaceSetStatusCompleteFB *)event);
		return true;
	}

	return false;
}

Vector<XrSpaceComponentTypeFB> OpenXRFbSpatialEntityExtensionWrapper::get_support_components(const XrSpace &space) {
	Vector<XrSpaceComponentTypeFB> components;

	uint32_t numComponents = 0;
	xrEnumerateSpaceSupportedComponentsFB(space, 0, &numComponents, nullptr);
	components.resize(numComponents);
	xrEnumerateSpaceSupportedComponentsFB(space, numComponents, &numComponents, components.ptrw());

	return components;
}

bool OpenXRFbSpatialEntityExtensionWrapper::is_component_enabled(const XrSpace &space, XrSpaceComponentTypeFB type) {
	XrSpaceComponentStatusFB status = { XR_TYPE_SPACE_COMPONENT_STATUS_FB, nullptr };
	xrGetSpaceComponentStatusFB(space, type, &status);
	return (status.enabled && !status.changePending);
}

void OpenXRFbSpatialEntityExtensionWrapper::set_component_enabled(const XrSpace &p_space, XrSpaceComponentTypeFB p_component, bool p_enabled, SetComponentEnabledCallback p_callback, void *p_userdata) {
	XrSpaceComponentStatusSetInfoFB request = {
		XR_TYPE_SPACE_COMPONENT_STATUS_SET_INFO_FB,
		nullptr,
		p_component,
		p_enabled,
		0,
	};
	XrAsyncRequestIdFB request_id;
	XrResult result = xrSetSpaceComponentStatusFB(p_space, &request, &request_id);
	if (!XR_SUCCEEDED(result)) {
		p_callback(result, p_component, p_enabled, p_userdata);
		return;
	}

	set_component_enabled_info[request_id] = SetComponentEnabledInfo(p_callback, p_userdata);
}

void OpenXRFbSpatialEntityExtensionWrapper::on_set_component_enabled_complete(const XrEventDataSpaceSetStatusCompleteFB *event) {
	if (!set_component_enabled_info.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_SET_STATUS_COMPLETE_FB");
		return;
	}

	SetComponentEnabledInfo *info = set_component_enabled_info.getptr(event->requestId);
	info->callback(event->result, event->componentType, event->enabled, info->userdata);
	set_component_enabled_info.erase(event->requestId);
}
