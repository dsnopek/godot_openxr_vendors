/**************************************************************************/
/*  openxr_fb_spatial_anchor_creation_request.h                           */
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

#ifndef OPENXR_FB_SPATIAL_ANCHOR_CREATION_REQUEST_H
#define OPENXR_FB_SPATIAL_ANCHOR_CREATION_REQUEST_H

#include <godot_cpp/classes/ref_counted.hpp>

#include "classes/openxr_fb_spatial_entity.h"

namespace godot {
class XRAnchor3D;

class OpenXRFbSpatialAnchorCreationRequest : public RefCounted {
	GDCLASS(OpenXRFbSpatialAnchorCreationRequest, RefCounted);

	friend class OpenXRFbSpatialAnchorManager;

	XRAnchor3D *anchor_node = nullptr;
	Ref<OpenXRFbSpatialEntity> spatial_entity;
	bool completed = false;
	bool success = false;

protected:
	static void _bind_methods();

	String _to_string() const;

public:
	XRAnchor3D *get_anchor_node() const;
	Ref<OpenXRFbSpatialEntity> get_spatial_entity() const;

	bool is_completed() const;
	bool is_success() const;

	OpenXRFbSpatialAnchorCreationRequest() = default;
};
} // namespace godot

#endif
