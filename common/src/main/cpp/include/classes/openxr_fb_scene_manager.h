/**************************************************************************/
/*  openxr_fb_scene_manager.h                                             */
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

#ifndef OPENXR_FB_SCENE_MANAGER_H
#define OPENXR_FB_SCENE_MANAGER_H

#include <openxr/openxr.h>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/templates/hash_map.hpp>

namespace godot {

class XROrigin3D;
class XRAnchor3D;

class OpenXRFbSceneManager : public Node {
	GDCLASS(OpenXRFbSceneManager, Node);

	Ref<PackedScene> default_scene;
	StringName scene_setup_method = "setup_scene";
	bool auto_create = true;
	bool visible = true;
	HashMap<StringName, Ref<PackedScene>> scenes;

	XROrigin3D *xr_origin = nullptr;

	bool anchors_created = false;
	HashMap<StringName, ObjectID> anchor_nodes;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void _notification(int p_what);

	void _on_openxr_session_begun();
	void _on_openxr_session_stopping();

	static void _bind_methods();

public:
	PackedStringArray _get_configuration_warnings() const override;

	void set_default_scene(const Ref<PackedScene> &p_default_scene);
	Ref<PackedScene> get_default_scene() const;

	void set_scene_setup_method(const StringName &p_method);
	StringName get_scene_setup_method() const;

	void set_auto_create(bool p_auto_create);
	bool get_auto_create() const;

	void set_visible(bool p_visible);
	bool get_visible() const;
	void show();
	void hide();

	void create_scene_anchors();
	void remove_scene_anchors();
	bool are_scene_anchors_created() const;

	XRAnchor3D *get_anchor_node(const StringName &p_uuid) const;
	Array get_anchor_nodes() const;
};

} // namespace godot

#endif