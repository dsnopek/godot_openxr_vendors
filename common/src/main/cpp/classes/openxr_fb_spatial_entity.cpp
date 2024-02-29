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
#include "extensions/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include "extensions/openxr_fb_scene_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialEntity::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_uuid"), &OpenXRFbSpatialEntity::get_uuid);

	ClassDB::bind_method(D_METHOD("get_supported_components"), &OpenXRFbSpatialEntity::get_supported_components);
	ClassDB::bind_method(D_METHOD("is_component_supported", "component"), &OpenXRFbSpatialEntity::is_component_supported);
	ClassDB::bind_method(D_METHOD("is_component_enabled", "component"), &OpenXRFbSpatialEntity::is_component_enabled);
	ClassDB::bind_method(D_METHOD("set_component_enabled", "component"), &OpenXRFbSpatialEntity::set_component_enabled);

	ClassDB::bind_method(D_METHOD("get_semantic_labels"), &OpenXRFbSpatialEntity::get_semantic_labels);
	ClassDB::bind_method(D_METHOD("get_room_layout"), &OpenXRFbSpatialEntity::get_room_layout);
	ClassDB::bind_method(D_METHOD("get_contained_uuids"), &OpenXRFbSpatialEntity::get_contained_uuids);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "uuid", PROPERTY_HINT_NONE, ""), "", "get_uuid");

	BIND_ENUM_CONSTANT(STORAGE_LOCAL);
	BIND_ENUM_CONSTANT(STORAGE_CLOUD);

	BIND_ENUM_CONSTANT(COMPONENT_TYPE_LOCATABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_STORABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SHARABLE);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_BOUNDED_2D);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_BOUNDED_3D);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_SEMANTIC_LABELS);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_ROOM_LAYOUT);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_CONTAINER);
	BIND_ENUM_CONSTANT(COMPONENT_TYPE_TRIANGLE_MESH);

	ADD_SIGNAL(MethodInfo("set_component_enabled_completed", PropertyInfo(Variant::Type::BOOL, "succeeded"), PropertyInfo(Variant::Type::INT, "component"), PropertyInfo(Variant::Type::BOOL, "enabled")));
}

String OpenXRFbSpatialEntity::_to_string() const {
	return String("[OpenXRFbSpatialEntity ") + uuid + String("]");
}

StringName OpenXRFbSpatialEntity::get_uuid() const {
	return uuid;
}

Array OpenXRFbSpatialEntity::get_supported_components() const {
	Array ret;

	Vector<XrSpaceComponentTypeFB> components = OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->get_support_components(space);
	ret.resize(components.size());
	for (int i = 0; i < components.size(); i++) {
		ret[i] = from_openxr_component_type(components[i]);
	}

	return ret;
}

bool OpenXRFbSpatialEntity::is_component_supported(ComponentType p_component) const {
	return get_supported_components().has(p_component);
}

bool OpenXRFbSpatialEntity::is_component_enabled(ComponentType p_component) const {
	return OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(space, to_openxr_component_type(p_component));
}

void OpenXRFbSpatialEntity::set_component_enabled(ComponentType p_component, bool p_enabled) {
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(space, to_openxr_component_type(p_component), p_enabled, OpenXRFbSpatialEntity::_on_set_component_enabled_completed, this);
}

void OpenXRFbSpatialEntity::_on_set_component_enabled_completed(XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata) {
	OpenXRFbSpatialEntity *self = (OpenXRFbSpatialEntity *)p_userdata;
	self->emit_signal("set_component_enabled_completed", XR_SUCCEEDED(p_result), from_openxr_component_type(p_component), p_enabled);
}

PackedStringArray OpenXRFbSpatialEntity::get_semantic_labels() const {
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_semantic_labels(space);
}

Dictionary OpenXRFbSpatialEntity::get_room_layout() const {
	return OpenXRFbSceneExtensionWrapper::get_singleton()->get_room_layout(space);
}

Array OpenXRFbSpatialEntity::get_contained_uuids() const {
	Vector<XrUuidEXT> uuids = OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton()->get_contained_uuids(space);

	Array ret;
	ret.resize(uuids.size());
	for (int i = 0; i < uuids.size(); i++) {
		ret[i] = OpenXRUtilities::uuid_to_string_name(uuids[i]);
	}
	return ret;
}

XrSpaceStorageLocationFB OpenXRFbSpatialEntity::to_openxr_storage_location(StorageLocation p_location) {
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

XrSpaceComponentTypeFB OpenXRFbSpatialEntity::to_openxr_component_type(ComponentType p_component) {
	switch (p_component) {
		case COMPONENT_TYPE_LOCATABLE: {
			return XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB;
		} break;
		case COMPONENT_TYPE_STORABLE: {
			return XR_SPACE_COMPONENT_TYPE_STORABLE_FB;
		} break;
		case COMPONENT_TYPE_SHARABLE: {
			return XR_SPACE_COMPONENT_TYPE_SHARABLE_FB;
		} break;
		case COMPONENT_TYPE_BOUNDED_2D: {
			return XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB;
		} break;
		case COMPONENT_TYPE_BOUNDED_3D: {
			return XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB;
		} break;
		case COMPONENT_TYPE_SEMANTIC_LABELS: {
			return XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB;
		} break;
		case COMPONENT_TYPE_ROOM_LAYOUT: {
			return XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB;
		} break;
		case COMPONENT_TYPE_CONTAINER: {
			return XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB;
		} break;
		case COMPONENT_TYPE_TRIANGLE_MESH: {
			return XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META;
		} break;
		default: {
			ERR_FAIL_V_MSG(XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB, vformat("Unknown component type: %s", p_component));
		}
	}
}

OpenXRFbSpatialEntity::ComponentType OpenXRFbSpatialEntity::from_openxr_component_type(XrSpaceComponentTypeFB p_component) {
	switch (p_component) {
		case XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB: {
			return COMPONENT_TYPE_LOCATABLE;
		} break;
		case XR_SPACE_COMPONENT_TYPE_STORABLE_FB: {
			return COMPONENT_TYPE_STORABLE;
		} break;
		case XR_SPACE_COMPONENT_TYPE_SHARABLE_FB: {
			return COMPONENT_TYPE_SHARABLE;
		} break;
		case XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB: {
			return COMPONENT_TYPE_BOUNDED_2D;
		} break;
		case XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB: {
			return COMPONENT_TYPE_BOUNDED_3D;
		} break;
		case XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB: {
			return COMPONENT_TYPE_SEMANTIC_LABELS;
		} break;
		case XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB: {
			return COMPONENT_TYPE_ROOM_LAYOUT;
		} break;
		case XR_SPACE_COMPONENT_TYPE_SPACE_CONTAINER_FB: {
			return COMPONENT_TYPE_CONTAINER;
		} break;
		case XR_SPACE_COMPONENT_TYPE_TRIANGLE_MESH_META: {
			return COMPONENT_TYPE_TRIANGLE_MESH;
		} break;
		case XR_SPACE_COMPONENT_TYPE_MAX_ENUM_FB:
		default: {
			ERR_FAIL_V_MSG(COMPONENT_TYPE_LOCATABLE, vformat("Unknown OpenXR component type: %s", p_component));
		}
	}
}

XrSpace OpenXRFbSpatialEntity::get_space() {
	return space;
}

OpenXRFbSpatialEntity::OpenXRFbSpatialEntity(XrSpace p_space, const XrUuidEXT &p_uuid) {
	space = p_space;
	uuid = OpenXRUtilities::uuid_to_string_name(p_uuid);
}
