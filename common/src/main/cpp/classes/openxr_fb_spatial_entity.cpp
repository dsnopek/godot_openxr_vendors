/**************************************************************************/
/*  openxr_fb_spatial_entity.cpp                                          */
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

#include "classes/openxr_fb_spatial_entity.h"

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialEntity::_bind_methods() {

	BIND_ENUM_CONSTANT(STORAGE_LOCAL);
	BIND_ENUM_CONSTANT(STORAGE_CLOUD);

	BIND_ENUM_CONSTANT(COMPONENT_TYPE_LOCATABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_STORABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SHARABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_BOUNDED_2D);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_BOUNDED_3D);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SEMANTIC_LABELS);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_ROOM_LAYOUT);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SPACE_CONTAINER);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_TRIANGLE_MESH);

	ADD_SIGNAL(MethodInfo("set_component_enabled_completed", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "component"), PropertyInfo(Variant::Type::BOOL, "enabled")));
}

Array OpenXRFbSpatialEntity::get_supported_components() const {
	Array ret;

	Vector<XrSpaceComponentTypeFB> components = OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->get_support_components(space);
	ret.resize(components.size());
	for (int i = 0; i < components.size(); i++) {
	}
}

bool OpenXRFbSpatialEntity::is_component_supported(ComponentType p_component) const {
}

bool OpenXRFbSpatialEntity::is_component_enabled(ComponentType p_component) const {
}

void OpenXRFbSpatialEntity::set_component_enabled(ComponentType p_component, bool p_enabled) {
}

XrSpaceStorageLocationFB OpenXRFbSpatialEntity::get_openxr_storage_location(StorageLocation p_location) {
	switch (p_location) {
		case OpenXRFbSpatialEntity::STORAGE_LOCAL: {
			return XR_SPACE_STORAGE_LOCATION_LOCAL_FB;
		} break;
		case OpenXRFbSpatialEntity::STORAGE_CLOUD: {
			return XR_SPACE_STORAGE_LOCATION_CLOUD_FB;
		} break;
		default: {
			return XR_SPACE_STORAGE_LOCATION_INVALID_FB;
		}
	}
}

XrSpaceComponentTypeFB OpenXRFbSpatialEntity::get_openxr_component_type(ComponentType p_component) {
	switch (p_component) {
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE: {
			return XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_STORABLE: {
			return XR_SPACE_COMPONENT_TYPE_STORABLE_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_SHARABLE: {
			return XR_SPACE_COMPONENT_TYPE_SHARABLE_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_BOUNDED_2D: {
			return XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_BOUNDED_3D: {
			return XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_SEMANTIC_LABELS: {
			return XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_ROOM_LAYOUT: {
			return XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_SPACE_CONTAINER: {
			return XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB;
		} break;
		case OpenXRFbSpatialEntity::COMPONENT_TYPE_TRIANGLE_MESH: {
			return XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META;
		} break;
		default: {
			ERR_FAIL_V_MSG(XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB, vformat("Unknown component type: %s", p_component));
		}
	}
}

OpenXRFbSpatialEntity::OpenXRFbSpatialEntity(XrSpace p_space) {
	space = p_space;
}
