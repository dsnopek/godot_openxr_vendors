/**************************************************************************/
/*  openxr_android_global_passthrough_dimming_extension.h                 */
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

#pragma once

#include <androidxr/androidxr.h>
#include <godot_cpp/classes/open_xr_extension_wrapper.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include "util.h"

using namespace godot;

class OpenXRAndroidGlobalPassthroughDimmingExtension : public OpenXRExtensionWrapper {
	GDCLASS(OpenXRAndroidGlobalPassthroughDimmingExtension, OpenXRExtensionWrapper);

public:
	godot::Dictionary _get_requested_extensions(uint64_t p_xr_version) override;

	void _on_instance_created(uint64_t p_instance) override;
	void _on_instance_destroyed() override;
	void _on_session_created(uint64_t p_session) override;
	void _on_session_destroyed() override;
	uint64_t _set_system_properties_and_get_next_pointer(void *p_next_pointer) override;
	uint64_t _set_frame_end_info_and_get_next_pointer(void *p_next_pointer) override;
	bool _on_event_polled(const void *p_event) override;

	static OpenXRAndroidGlobalPassthroughDimmingExtension *get_singleton();

	bool is_enabled() const;

	PackedFloat32Array get_supported_dimming_levels();
	float get_dimming_level() const;
	void set_dimming_level(float p_dimming_level);

	OpenXRAndroidGlobalPassthroughDimmingExtension();
	~OpenXRAndroidGlobalPassthroughDimmingExtension();

protected:
	static void _bind_methods();

private:
	bool initialize_android_global_passthrough_dimming_extension(const XrInstance &p_instance);
	void cleanup();

	static OpenXRAndroidGlobalPassthroughDimmingExtension *singleton;

	HashMap<String, bool *> request_extensions;

	bool android_global_passthrough_dimming_ext = false;

	XrSystemGlobalDimmingPropertiesANDROID global_dimming_properties = {
		XR_TYPE_SYSTEM_GLOBAL_DIMMING_PROPERTIES_ANDROID, // type
		nullptr, // next
		XR_FALSE, // supportsGlobalDimming
	};

	XrGlobalDimmingFrameEndInfoANDROID global_dimming_frame_end_info = {
		XR_TYPE_GLOBAL_DIMMING_FRAME_END_INFO_ANDROID,
		nullptr, // next
		-1.0, // globalDimmingLevel
	};

	void _update_current_dimming_level();

	EXT_PROTO_XRRESULT_FUNC2(xrGetGlobalDimmingLevelANDROID,
			(XrSession), session,
			(float *), dimmingLevel)

	EXT_PROTO_XRRESULT_FUNC5(xrEnumerateSupportedGlobalDimmingLevelsANDROID,
			(XrInstance), instance,
			(XrSystemId), systemId,
			(uint32_t), supportedGlobalDimmingLevelCapacityInput,
			(uint32_t *), supportedGlobalDimmingLevelCountOutput,
			(float *), supportedGlobalDimmingLevels)
};
