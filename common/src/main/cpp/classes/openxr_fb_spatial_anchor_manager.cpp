/**************************************************************************/
/*  openxr_fb_spatial_anchor_manager.cpp                                  */
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

#include "classes/openxr_fb_spatial_anchor_manager.h"

#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/xr_origin3d.hpp>
#include <godot_cpp/classes/xr_anchor3d.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialAnchorManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &OpenXRFbSpatialAnchorManager::set_visible);
	ClassDB::bind_method(D_METHOD("get_visible"), &OpenXRFbSpatialAnchorManager::get_visible);
	ClassDB::bind_method(D_METHOD("show"), &OpenXRFbSpatialAnchorManager::show);
	ClassDB::bind_method(D_METHOD("hide"), &OpenXRFbSpatialAnchorManager::hide);

	ClassDB::bind_method(D_METHOD("create_anchor", "transform", "custom_data"), &OpenXRFbSpatialAnchorManager::create_anchor, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("load_anchor", "uuid", "custom_data", "location"), &OpenXRFbSpatialAnchorManager::load_anchor, DEFVAL(Dictionary()), DEFVAL(OpenXRFbSpatialEntity::STORAGE_LOCAL));
	ClassDB::bind_method(D_METHOD("track_anchor", "spatial_entity"), &OpenXRFbSpatialAnchorManager::track_anchor);
	ClassDB::bind_method(D_METHOD("untrack_anchor", "spatial_entity_or_uuid"), &OpenXRFbSpatialAnchorManager::untrack_anchor);
	ClassDB::bind_method(D_METHOD("untrack_all_anchors"), &OpenXRFbSpatialAnchorManager::untrack_all_anchors);

	ClassDB::bind_method(D_METHOD("get_anchor_uuids"), &OpenXRFbSpatialAnchorManager::get_anchor_uuids);
	ClassDB::bind_method(D_METHOD("get_anchor_node", "uuid"), &OpenXRFbSpatialAnchorManager::get_anchor_node);
	ClassDB::bind_method(D_METHOD("get_spatial_entity", "uuid"), &OpenXRFbSpatialAnchorManager::get_spatial_entity);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible", PROPERTY_HINT_NONE, ""), "set_visible", "get_visible");

	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_anchor_tracked", PropertyInfo(Variant::Type::OBJECT, "anchor_node"), PropertyInfo(Variant::Type::OBJECT, "spatial_entity")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_anchor_untracked", PropertyInfo(Variant::Type::OBJECT, "anchor_node"), PropertyInfo(Variant::Type::OBJECT, "spatial_entity")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_anchor_create_failed", PropertyInfo(Variant::Type::TRANSFORM3D, "transform"), PropertyInfo(Variant::Type::DICTIONARY, "custom_data")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_anchor_load_failed", PropertyInfo(Variant::Type::STRING_NAME, "uuid"), PropertyInfo(Variant::Type::DICTIONARY, "custom_data"), PropertyInfo(Variant::Type::INT, "location")));
	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_anchor_track_failed", PropertyInfo(Variant::Type::OBJECT, "spatial_entity")));
}

void OpenXRFbSpatialAnchorManager::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
			if (openxr_interface.is_valid()) {
				openxr_interface->connect("session_stopping", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_openxr_session_stopping));
			}

			xr_origin = Object::cast_to<XROrigin3D>(get_parent());
		} break;
		case NOTIFICATION_EXIT_TREE: {
			Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
			if (openxr_interface.is_valid()) {
				openxr_interface->disconnect("session_stopping", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_openxr_session_stopping));
			}

			untrack_all_anchors();
			xr_origin = nullptr;
		} break;
	}
}

void OpenXRFbSpatialAnchorManager::_on_openxr_session_stopping() {
	untrack_all_anchors();
}

PackedStringArray OpenXRFbSpatialAnchorManager::_get_configuration_warnings() const {
	PackedStringArray warnings = Node::_get_configuration_warnings();

	if (is_inside_tree()) {
		XROrigin3D *origin = Object::cast_to<XROrigin3D>(get_parent());
		if (origin == nullptr) {
			warnings.push_back("Must be a child of XROrigin3D");
		}
	}

	return warnings;
}

void OpenXRFbSpatialAnchorManager::set_visible(bool p_visible) {
	visible = p_visible;

	for (KeyValue<StringName, Anchor> &E : anchors) {
		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(E.value.node));
		ERR_CONTINUE_MSG(!node, vformat("Cannot find node for anchor %s.", E.key));
		if (node) {
			node->set_visible(p_visible);
		}
	}
}

bool OpenXRFbSpatialAnchorManager::get_visible() const {
	return visible;
}

void OpenXRFbSpatialAnchorManager::show() {
	set_visible(true);
}

void OpenXRFbSpatialAnchorManager::hide() {
	set_visible(false);
}

void OpenXRFbSpatialAnchorManager::create_anchor(const Transform3D &p_transform, const Dictionary &p_custom_data) {
	Ref<OpenXRFbSpatialEntity> spatial_entity = OpenXRFbSpatialEntity::create_spatial_anchor(p_transform);
	spatial_entity->set_custom_data(p_custom_data);
	spatial_entity->connect("openxr_fb_spatial_entity_created", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_created).bind(spatial_entity));
}

void OpenXRFbSpatialAnchorManager::_on_anchor_created(bool p_success, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity) {
	// We can assume that any anchors created here will already be locatable.
	//p_request->anchor_node = _create_anchor_node(p_request->spatial_entity, false);

	//emit_signal("openxr_fb_spatial_anchor_created", p_request->anchor_node, p_request->spatial_entity);
	//emit_signal("openxr_fb_spatial_anchor_tracked", p_request->anchor_node, p_request->spatial_entity);
}

XRAnchor3D *OpenXRFbSpatialAnchorManager::_create_anchor_node(const Ref<OpenXRFbSpatialEntity> &p_entity, bool p_emit_signal) {
	p_entity->track();

	XRAnchor3D *node = memnew(XRAnchor3D);
	node->set_name(p_entity->get_uuid());
	node->set_tracker(p_entity->get_uuid());
	node->set_visible(visible);
	xr_origin->add_child(node);

	anchors[p_entity->get_uuid()] = Anchor(node, p_entity);

	if (p_emit_signal) {
		emit_signal("openxr_fb_spatial_anchor_tracked", node, p_entity);
	}

	return node;
}

void OpenXRFbSpatialAnchorManager::load_anchor(const StringName &p_uuid, const Dictionary &p_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location) {
	// @todo be sure to emit "anchor_tracked"
}

void OpenXRFbSpatialAnchorManager::track_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity) {
	// @todo Store in local storage if it hasn't been stored yet

	if (p_spatial_entity->is_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE)) {
		_create_anchor_node(p_spatial_entity);
	} else {
		p_spatial_entity->connect("openxr_fb_spatial_entity_set_component_enabled_completed", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_enable_locatable_completed).bind(p_spatial_entity), CONNECT_ONE_SHOT);
		p_spatial_entity->set_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE, true);
	}
}

void OpenXRFbSpatialAnchorManager::_on_anchor_enable_locatable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_entity) {
	ERR_FAIL_COND_MSG(!p_succeeded, vformat("Unable to make scene anchor %s locatable.", p_entity->get_uuid()));
	_create_anchor_node(p_entity);
}

void OpenXRFbSpatialAnchorManager::untrack_anchor(const Variant &p_spatial_entity_or_uuid) {
	// @todo be sure to emit "anchor_untracked"
}

void OpenXRFbSpatialAnchorManager::untrack_all_anchors() {
	for (KeyValue<StringName, Anchor> &E : anchors) {
		emit_signal("openxr_fb_spatial_anchor_untracked", E.value.node, E.value.entity);

		Node3D *node = Object::cast_to<Node3D>(ObjectDB::get_instance(E.value.node));
		if (node) {
			Node *parent = node->get_parent();
			if (parent) {
				parent->remove_child(node);
			}
			node->queue_free();
		}

		E.value.entity->untrack();
	}
	anchors.clear();
}

Array OpenXRFbSpatialAnchorManager::get_anchor_uuids() const {
	Array ret;
	ret.resize(anchors.size());
	int i = 0;
	for (const KeyValue<StringName, Anchor> &E : anchors) {
		ret[i++] = E.key;
	}
	return ret;
}

XRAnchor3D *OpenXRFbSpatialAnchorManager::get_anchor_node(const StringName &p_uuid) const {
	const Anchor *anchor = anchors.getptr(p_uuid);
	if (anchor) {
		return Object::cast_to<XRAnchor3D>(ObjectDB::get_instance(anchor->node));
	}

	return nullptr;
}

Ref<OpenXRFbSpatialEntity> OpenXRFbSpatialAnchorManager::get_spatial_entity(const StringName &p_uuid) const {
	const Anchor *anchor = anchors.getptr(p_uuid);
	if (anchor) {
		return anchor->entity;
	}

	return Ref<OpenXRFbSpatialEntity>();
}
