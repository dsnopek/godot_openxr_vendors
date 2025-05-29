/**************************************************************************/
/*  openxr_meta_environment_depth.cpp                                     */
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

#include "classes/openxr_meta_environment_depth.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/classes/xr_server.hpp>

#include "extensions/openxr_meta_environment_depth_extension_wrapper.h"

using namespace godot;

void OpenXRMetaEnvironmentDepth::_bind_methods() {
}

void OpenXRMetaEnvironmentDepth::_notification(int p_what) {
}

void OpenXRMetaEnvironmentDepth::_on_openxr_session_begun() {
	// @todo Need to be more selective about if this is shown or not!
	OpenXRMetaEnvironmentDepthExtensionWrapper *env_depth_ext = OpenXRMetaEnvironmentDepthExtensionWrapper::get_singleton();
	if (env_depth_ext) {
		set_base(env_depth_ext->get_reprojection_mesh());
	}
}

void OpenXRMetaEnvironmentDepth::_on_openxr_session_stopping() {
	// @todo Need to be more selective about if this is shown or not!
	set_base(RID());
}

PackedStringArray OpenXRMetaEnvironmentDepth::_get_configuration_warnings() const {
	PackedStringArray warnings = Node::_get_configuration_warnings();
	return warnings;
}

AABB OpenXRMetaEnvironmentDepth::_get_aabb() const {
	AABB ret;

	// Make sure it's always visible.
	ret.position = Vector3(-1000.0, -1000.0, -1000.0);
	ret.size = Vector3(2000.0, 2000.0, 2000.0);

	return ret;
}

OpenXRMetaEnvironmentDepth::OpenXRMetaEnvironmentDepth() {
	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server) {
		Ref<XRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
		if (openxr_interface.is_valid()) {
			openxr_interface->connect("session_begun", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_begun));
			openxr_interface->connect("session_stopping", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_stopping));
		}
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		rs->instance_geometry_set_cast_shadows_setting(get_instance(), RenderingServer::SHADOW_CASTING_SETTING_OFF);
	}
}

OpenXRMetaEnvironmentDepth::~OpenXRMetaEnvironmentDepth() {
	XRServer *xr_server = XRServer::get_singleton();
	if (xr_server) {
		Ref<XRInterface> openxr_interface = XRServer::get_singleton()->find_interface("OpenXR");
		if (openxr_interface.is_valid()) {
			openxr_interface->disconnect("session_begun", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_begun));
			openxr_interface->disconnect("session_stopping", callable_mp(this, &OpenXRMetaEnvironmentDepth::_on_openxr_session_stopping));
		}
	}
}
