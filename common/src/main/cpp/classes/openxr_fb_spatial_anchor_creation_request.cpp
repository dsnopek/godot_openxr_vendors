/**************************************************************************/
/*  openxr_fb_spatial_anchor_creation_request.cpp                         */
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

#include "classes/openxr_fb_spatial_anchor_creation_request.h"

#include <godot_cpp/classes/xr_anchor3d.hpp>

using namespace godot;

void OpenXRFbSpatialAnchorCreationRequest::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_anchor_node"), &OpenXRFbSpatialAnchorCreationRequest::get_anchor_node);
	ClassDB::bind_method(D_METHOD("get_spatial_entity"), &OpenXRFbSpatialAnchorCreationRequest::get_spatial_entity);
	ClassDB::bind_method(D_METHOD("is_completed"), &OpenXRFbSpatialAnchorCreationRequest::is_completed);
	ClassDB::bind_method(D_METHOD("is_success"), &OpenXRFbSpatialAnchorCreationRequest::is_success);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "anchor_node", PROPERTY_HINT_NONE, ""), "", "get_anchor_node");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "spatial_entity", PROPERTY_HINT_NONE, ""), "", "get_spatial_entity");

	ADD_SIGNAL(MethodInfo("openxr_fb_spatial_anchor_creation_request_completed", PropertyInfo(Variant::Type::BOOL, "succeeded")));
}

String OpenXRFbSpatialAnchorCreationRequest::_to_string() const {
	return String("[OpenXRFbSpatialAnchorCreationRequest completed=") + completed + String(" success=") + success + String("]");
}

XRAnchor3D *OpenXRFbSpatialAnchorCreationRequest::get_anchor_node() const {
	return anchor_node;
}

Ref<OpenXRFbSpatialEntity> OpenXRFbSpatialAnchorCreationRequest::get_spatial_entity() const {
	return spatial_entity;
}

bool OpenXRFbSpatialAnchorCreationRequest::is_completed() const {
	return completed;
}

bool OpenXRFbSpatialAnchorCreationRequest::is_success() const {
	return success;
}
