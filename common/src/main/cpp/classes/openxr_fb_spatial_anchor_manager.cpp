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

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/xr_origin3d.hpp>
#include <godot_cpp/classes/xr_anchor3d.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "classes/openxr_fb_spatial_entity_query.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialAnchorManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_scene", "scene"), &OpenXRFbSpatialAnchorManager::set_scene);
	ClassDB::bind_method(D_METHOD("get_scene"), &OpenXRFbSpatialAnchorManager::get_scene);

	ClassDB::bind_method(D_METHOD("set_scene_setup_method", "method_name"), &OpenXRFbSpatialAnchorManager::set_scene_setup_method);
	ClassDB::bind_method(D_METHOD("get_scene_setup_method"), &OpenXRFbSpatialAnchorManager::get_scene_setup_method);

	ClassDB::bind_method(D_METHOD("set_persist_in_local_file", "enable"), &OpenXRFbSpatialAnchorManager::set_persist_in_local_file);
	ClassDB::bind_method(D_METHOD("get_persist_in_local_file"), &OpenXRFbSpatialAnchorManager::get_persist_in_local_file);

	ClassDB::bind_method(D_METHOD("set_local_file_path", "path"), &OpenXRFbSpatialAnchorManager::set_local_file_path);
	ClassDB::bind_method(D_METHOD("get_local_file_path"), &OpenXRFbSpatialAnchorManager::get_local_file_path);

	ClassDB::bind_method(D_METHOD("set_auto_load", "enable"), &OpenXRFbSpatialAnchorManager::set_auto_load);
	ClassDB::bind_method(D_METHOD("get_auto_load"), &OpenXRFbSpatialAnchorManager::get_auto_load);

	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &OpenXRFbSpatialAnchorManager::set_visible);
	ClassDB::bind_method(D_METHOD("get_visible"), &OpenXRFbSpatialAnchorManager::get_visible);
	ClassDB::bind_method(D_METHOD("show"), &OpenXRFbSpatialAnchorManager::show);
	ClassDB::bind_method(D_METHOD("hide"), &OpenXRFbSpatialAnchorManager::hide);

	ClassDB::bind_method(D_METHOD("create_anchor", "transform", "custom_data"), &OpenXRFbSpatialAnchorManager::create_anchor, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("load_anchor", "uuid", "custom_data", "location"), &OpenXRFbSpatialAnchorManager::load_anchor, DEFVAL(Dictionary()), DEFVAL(OpenXRFbSpatialEntity::STORAGE_LOCAL));
	ClassDB::bind_method(D_METHOD("track_anchor", "spatial_entity"), &OpenXRFbSpatialAnchorManager::track_anchor);
	ClassDB::bind_method(D_METHOD("untrack_anchor", "spatial_entity_or_uuid"), &OpenXRFbSpatialAnchorManager::untrack_anchor);

	ClassDB::bind_method(D_METHOD("get_anchor_uuids"), &OpenXRFbSpatialAnchorManager::get_anchor_uuids);
	ClassDB::bind_method(D_METHOD("get_anchor_node", "uuid"), &OpenXRFbSpatialAnchorManager::get_anchor_node);
	ClassDB::bind_method(D_METHOD("get_spatial_entity", "uuid"), &OpenXRFbSpatialAnchorManager::get_spatial_entity);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_scene", "get_scene");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "scene_setup_method", PROPERTY_HINT_NONE, ""), "set_scene_setup_method", "get_scene_setup_method");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "persist_in_local_file", PROPERTY_HINT_NONE, ""), "set_persist_in_local_file", "get_persist_in_local_file");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "local_file_path", PROPERTY_HINT_NONE, ""), "set_local_file_path", "get_local_file_path");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_load", PROPERTY_HINT_NONE, ""), "set_auto_load", "get_auto_load");
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
				openxr_interface->connect("session_begun", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_openxr_session_begun));
				openxr_interface->connect("session_stopping", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_openxr_session_stopping));
			}

			xr_origin = Object::cast_to<XROrigin3D>(get_parent());
		} break;
		case NOTIFICATION_EXIT_TREE: {
			Ref<OpenXRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
			if (openxr_interface.is_valid()) {
				openxr_interface->disconnect("session_begun", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_openxr_session_begun));
				openxr_interface->disconnect("session_stopping", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_openxr_session_stopping));
			}

			// @todo clean-up!
			xr_origin = nullptr;
		} break;
	}
}

void OpenXRFbSpatialAnchorManager::_on_openxr_session_begun() {
	if (auto_load && persist_in_local_file) {
		load_anchors_from_local_file();
	}
}

void OpenXRFbSpatialAnchorManager::_on_openxr_session_stopping() {
	// @todo clean-up!
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

void OpenXRFbSpatialAnchorManager::set_scene(const Ref<PackedScene> &p_scene) {
	scene = p_scene;
}

Ref<PackedScene> OpenXRFbSpatialAnchorManager::get_scene() const {
	return scene;
}

void OpenXRFbSpatialAnchorManager::set_scene_setup_method(const StringName &p_method) {
	scene_setup_method = p_method;
}

StringName OpenXRFbSpatialAnchorManager::get_scene_setup_method() const {
	return scene_setup_method;
}

void OpenXRFbSpatialAnchorManager::set_persist_in_local_file(bool p_persist_in_local_file) {
	persist_in_local_file = p_persist_in_local_file;
}

bool OpenXRFbSpatialAnchorManager::get_persist_in_local_file() const {
	return persist_in_local_file;
}

void OpenXRFbSpatialAnchorManager::set_local_file_path(const String &p_local_file) {
	local_file_path = p_local_file;
}

String OpenXRFbSpatialAnchorManager::get_local_file_path() const {
	return local_file_path;
}

void OpenXRFbSpatialAnchorManager::set_auto_load(bool p_auto_load) {
	auto_load = p_auto_load;
}

bool OpenXRFbSpatialAnchorManager::get_auto_load() const {
	return auto_load;
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
	spatial_entity->connect("openxr_fb_spatial_entity_created", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_created).bind(p_transform, spatial_entity));
}

void OpenXRFbSpatialAnchorManager::_on_anchor_created(bool p_success, const Transform3D &p_transform, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity) {
	if (p_success) {
		_track_anchor(p_spatial_entity, true);
	} else {
		emit_signal("openxr_fb_spatial_anchor_create_failed", p_transform, p_spatial_entity->get_custom_data());
	}
}

void OpenXRFbSpatialAnchorManager::load_anchor(const StringName &p_uuid, const Dictionary &p_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location) {
	Array uuids;
	uuids.push_back(p_uuid);

	Dictionary data;
	data[p_uuid] = p_custom_data;

	Ref<OpenXRFbSpatialEntityQuery> query;
	query.instantiate();
	query->query_by_uuid(uuids, p_location);
	query->connect("openxr_fb_spatial_entity_query_completed", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_load_query_completed).bind(data, p_location, true));
	query->execute();
}

void OpenXRFbSpatialAnchorManager::_on_anchor_load_query_completed(const Array &p_results, const Dictionary &p_anchors_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location, bool p_save_file) {
	Dictionary anchors_custom_data = p_anchors_custom_data.duplicate();
	for (int i = 0; i < p_results.size(); i++) {
		Ref<OpenXRFbSpatialEntity> spatial_entity = p_results[i];
		if (spatial_entity.is_valid()) {
			StringName uuid = spatial_entity->get_uuid();
			spatial_entity->set_custom_data(anchors_custom_data[uuid]);
			anchors_custom_data.erase(uuid);

			_track_anchor(spatial_entity, p_save_file);
		}
	}

	Array failed_uuids = anchors_custom_data.keys();
	for (int i = 0; i < failed_uuids.size(); i++) {
		StringName uuid = failed_uuids[i];
		emit_signal("openxr_fb_spatial_anchor_load_failed", uuid, anchors_custom_data[uuid], p_location);
	}
}

void OpenXRFbSpatialAnchorManager::track_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity) {
		_track_anchor(p_spatial_entity, true);
}

void OpenXRFbSpatialAnchorManager::_track_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_save_file) {
	if (p_spatial_entity->is_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE)) {
		_on_anchor_enable_locatable_completed(true, OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE, true, p_spatial_entity, p_save_file);
	} else {
		p_spatial_entity->connect("openxr_fb_spatial_entity_set_component_enabled_completed", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_enable_locatable_completed).bind(p_spatial_entity, p_save_file), CONNECT_ONE_SHOT);
		p_spatial_entity->set_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_LOCATABLE, true);
	}
}

void OpenXRFbSpatialAnchorManager::_on_anchor_enable_locatable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_save_file) {
	ERR_FAIL_COND_MSG(!p_succeeded, vformat("Unable to make spatial anchor %s locatable.", p_spatial_entity->get_uuid()));

	if (p_spatial_entity->is_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_STORABLE)) {
		_on_anchor_enable_locatable_completed(true, OpenXRFbSpatialEntity::COMPONENT_TYPE_STORABLE, true, p_spatial_entity, p_save_file);
	} else {
		p_spatial_entity->connect("openxr_fb_spatial_entity_set_component_enabled_completed", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_enable_storable_completed).bind(p_spatial_entity, p_save_file), CONNECT_ONE_SHOT);
		p_spatial_entity->set_component_enabled(OpenXRFbSpatialEntity::COMPONENT_TYPE_STORABLE, true);
	}
}

void OpenXRFbSpatialAnchorManager::_on_anchor_enable_storable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_save_file) {
	ERR_FAIL_COND_MSG(!p_succeeded, vformat("Unable to make spatial anchor %s storable.", p_spatial_entity->get_uuid()));

	p_spatial_entity->connect("openxr_fb_spatial_entity_saved", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_saved).bind(p_spatial_entity, p_save_file), CONNECT_ONE_SHOT);
	p_spatial_entity->save_to_storage(OpenXRFbSpatialEntity::STORAGE_LOCAL);
}

void OpenXRFbSpatialAnchorManager::_on_anchor_saved(bool p_succeeded, OpenXRFbSpatialEntity::StorageLocation p_location, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_save_file) {
	ERR_FAIL_COND_MSG(!p_succeeded, vformat("Unable to save spatial anchor %s to local storage.", p_spatial_entity->get_uuid()));
	_complete_anchor_setup(p_spatial_entity, p_save_file);
}

void OpenXRFbSpatialAnchorManager::_complete_anchor_setup(const Ref<OpenXRFbSpatialEntity> &p_entity, bool p_save_file) {
	p_entity->track();

	XRAnchor3D *node = memnew(XRAnchor3D);
	node->set_name(p_entity->get_uuid());
	node->set_tracker(p_entity->get_uuid());
	node->set_visible(visible);
	xr_origin->add_child(node);

	anchors[p_entity->get_uuid()] = Anchor(node, p_entity);

	if (persist_in_local_file && p_save_file) {
		save_anchors_to_local_file();
	}

	emit_signal("openxr_fb_spatial_anchor_tracked", node, p_entity);
}

void OpenXRFbSpatialAnchorManager::untrack_anchor(const Variant &p_spatial_entity_or_uuid) {
	// @todo be sure to emit "anchor_untracked"
}

/*
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
*/

Error OpenXRFbSpatialAnchorManager::save_anchors_to_local_file() {
	Ref<FileAccess> file = FileAccess::open(local_file_path, FileAccess::WRITE);
	if (file.is_null()) {
		return FileAccess::get_open_error();
	}

	Dictionary anchor_data;
	for (const KeyValue<StringName, Anchor> &E : anchors) {
		anchor_data[E.key] = E.value.entity->get_custom_data();
	}

	file->store_string(JSON::stringify(anchor_data));

	return OK;
}

Error OpenXRFbSpatialAnchorManager::load_anchors_from_local_file() {
	Ref<FileAccess> file = FileAccess::open(local_file_path, FileAccess::READ);
	if (file.is_null()) {
		return FileAccess::get_open_error();
	}

	Ref<JSON> json;
	json.instantiate();
	Error parse_error = json->parse(file->get_as_text());
	if (parse_error != OK) {
		return parse_error;
	}

	Dictionary anchor_data = json->get_data();

	Ref<OpenXRFbSpatialEntityQuery> query;
	query.instantiate();
	query->query_by_uuid(anchor_data.keys(), OpenXRFbSpatialEntity::STORAGE_LOCAL);
	query->connect("openxr_fb_spatial_entity_query_completed", callable_mp(this, &OpenXRFbSpatialAnchorManager::_on_anchor_load_query_completed).bind(anchor_data, OpenXRFbSpatialEntity::STORAGE_LOCAL, false));
	query->execute();

	return OK;
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
