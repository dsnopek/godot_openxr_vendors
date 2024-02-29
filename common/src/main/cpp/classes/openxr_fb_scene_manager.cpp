/**************************************************************************/
/*  openxr_fb_scene_manager.cpp                                           */
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

#include "classes/openxr_fb_scene_manager.h"

#include "extensions/openxr_fb_scene_extension_wrapper.h"

using namespace godot;

bool OpenXRFbSceneManager::_set(const StringName &p_name, const Variant &p_value) {
	PackedStringArray parts = p_name.split("/", true, 2);
	if (parts[0] == "scenes") {
		const PackedStringArray &semantic_labels = OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels();
		if (parts[1] == "default" || semantic_labels.has(parts[1])) {
			scenes[parts[1]] = p_value;
			return true;
		}
	}
	return false;
}

bool OpenXRFbSceneManager::_get(const StringName &p_name, Variant &r_ret) const {
	PackedStringArray parts = p_name.split("/", true, 2);
	if (parts[0] == "scenes") {
		const PackedStringArray &semantic_labels = OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels();
		if (parts[1] == "default" || semantic_labels.has(parts[1])) {
			const Ref<PackedScene> *scene = scenes.getptr(p_name.substr(7));
			r_ret = scene ? Variant(*scene) : Variant();
			return true;
		}
	}
	return false;
}

void OpenXRFbSceneManager::_get_property_list(List<PropertyInfo> *p_list) const {
	const PackedStringArray &semantic_labels = OpenXRFbSceneExtensionWrapper::get_supported_semantic_labels();
	p_list->push_back(PropertyInfo(Variant::Type::OBJECT, "scenes/default", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"));
	for (int i = 0; i < semantic_labels.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::Type::OBJECT, "scenes/" + semantic_labels[i].to_lower(), PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"));
	}
}

void OpenXRFbSceneManager::_bind_methods() {
}
