/**************************************************************************/
/*  openxr_fb_spatial_anchor_manager.h                                    */
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

#ifndef OPENXR_FB_SPATIAL_ANCHOR_MANAGER_H
#define OPENXR_FB_SPATIAL_ANCHOR_MANAGER_H

#include <openxr/openxr.h>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "classes/openxr_fb_spatial_entity.h"

namespace godot {
class XROrigin3D;
class XRAnchor3D;

class OpenXRFbSpatialAnchorManager : public Node {
	GDCLASS(OpenXRFbSpatialAnchorManager, Node);

	bool visible = true;

	XROrigin3D *xr_origin = nullptr;

	struct Anchor {
		ObjectID node;
		Ref<OpenXRFbSpatialEntity> entity;

		Anchor(Node *p_node, const Ref<OpenXRFbSpatialEntity> &p_entity) {
			node = p_node->get_instance_id();
			entity = p_entity;
		}
		Anchor() { }
	};
	HashMap<StringName, Anchor> anchors;

	void _on_anchor_created(bool p_success, const Ref<OpenXRFbSpatialEntity> &p_spatial_entity);
	XRAnchor3D *_create_anchor_node(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity, bool p_emit_signal = true);
	void _on_anchor_enable_locatable_completed(bool p_succeeded, OpenXRFbSpatialEntity::ComponentType p_component, bool p_enabled, const Ref<OpenXRFbSpatialEntity> &p_entity);

protected:
	void _notification(int p_what);

	void _on_openxr_session_begun();
	void _on_openxr_session_stopping();

	static void _bind_methods();

public:
	PackedStringArray _get_configuration_warnings() const override;

	void set_visible(bool p_visible);
	bool get_visible() const;
	void show();
	void hide();

	void create_anchor(const Transform3D &p_transform, const Dictionary &p_custom_data);
	void load_anchor(const StringName &p_uuid, const Dictionary &p_custom_data, OpenXRFbSpatialEntity::StorageLocation p_location);
	void track_anchor(const Ref<OpenXRFbSpatialEntity> &p_spatial_entity);
	void untrack_anchor(const Variant &p_spatial_entity_or_uuid);
	void untrack_all_anchors();

	Array get_anchor_uuids() const;
	XRAnchor3D *get_anchor_node(const StringName &p_uuid) const;
	Ref<OpenXRFbSpatialEntity> get_spatial_entity(const StringName &p_uuids) const;
};
} // namespace godot

#endif