/**************************************************************************/
/*  openxr_fb_hand_tracking_aim_extension_wrapper.cpp                     */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Original contributed implementation:                                   */
/*   Copyright (c) 2022-2023 MattaKis Consulting Kft. (Migeran)           */
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

#include "include/openxr_fb_hand_tracking_aim_extension_wrapper.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/object.hpp>

using namespace godot;

OpenXRFbHandTrackingAimExtensionWrapper *OpenXRFbHandTrackingAimExtensionWrapper::singleton = nullptr;

OpenXRFbHandTrackingAimExtensionWrapper *OpenXRFbHandTrackingAimExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbHandTrackingAimExtensionWrapper());
	}
	return singleton;
}

OpenXRFbHandTrackingAimExtensionWrapper::OpenXRFbHandTrackingAimExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {

	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRFbHandTrackingAimExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_HAND_TRACKING_AIM_EXTENSION_NAME] = &fb_hand_tracking_aim_ext;
	singleton = this;
}

OpenXRFbHandTrackingAimExtensionWrapper::~OpenXRFbHandTrackingAimExtensionWrapper() {
	cleanup();
}

void OpenXRFbHandTrackingAimExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_available"), &OpenXRFbHandTrackingAimExtensionWrapper::is_available);
	ClassDB::bind_method(D_METHOD("is_tracking_data_valid"), &OpenXRFbHandTrackingAimExtensionWrapper::is_tracking_data_valid);
	ClassDB::bind_method(D_METHOD("is_tracking_data_computed"), &OpenXRFbHandTrackingAimExtensionWrapper::is_tracking_data_computed);
	ClassDB::bind_method(D_METHOD("is_index_finger_pinching"), &OpenXRFbHandTrackingAimExtensionWrapper::is_index_finger_pinching);
	ClassDB::bind_method(D_METHOD("is_middle_finger_pinching"), &OpenXRFbHandTrackingAimExtensionWrapper::is_middle_finger_pinching);
	ClassDB::bind_method(D_METHOD("is_ring_finger_pinching"), &OpenXRFbHandTrackingAimExtensionWrapper::is_ring_finger_pinching);
	ClassDB::bind_method(D_METHOD("is_little_finger_pinching"), &OpenXRFbHandTrackingAimExtensionWrapper::is_little_finger_pinching);
	ClassDB::bind_method(D_METHOD("is_system_gesture_active"), &OpenXRFbHandTrackingAimExtensionWrapper::is_system_gesture_active);
	ClassDB::bind_method(D_METHOD("is_dominant_hand"), &OpenXRFbHandTrackingAimExtensionWrapper::is_dominant_hand);
	ClassDB::bind_method(D_METHOD("is_menu_gesture_active"), &OpenXRFbHandTrackingAimExtensionWrapper::is_menu_gesture_active);
}

void OpenXRFbHandTrackingAimExtensionWrapper::cleanup() {
	fb_hand_tracking_aim_ext = false;
}

godot::Dictionary OpenXRFbHandTrackingAimExtensionWrapper::_get_requested_extensions() {
	godot::Dictionary result;
	for (auto ext: request_extensions) {
		godot::String key = ext.first;
		uint64_t value = reinterpret_cast<uint64_t>(ext.second);
		result[key] = (godot::Variant)value;
	}
	return result;
}

void OpenXRFbHandTrackingAimExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

uint64_t OpenXRFbHandTrackingAimExtensionWrapper::_set_hand_joint_locations_and_get_next_pointer(int32_t p_hand_index, void *p_next_pointer) {
	if (!fb_hand_tracking_aim_ext) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	aim_state[p_hand_index].type = XR_TYPE_HAND_TRACKING_AIM_STATE_FB;
	aim_state[p_hand_index].next = p_next_pointer;
	aim_state[p_hand_index].status = 0;

	return reinterpret_cast<uint64_t>(&aim_state[p_hand_index]);
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_tracking_data_valid(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_VALID_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_tracking_data_computed(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_COMPUTED_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_index_finger_pinching(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_INDEX_PINCHING_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_middle_finger_pinching(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_MIDDLE_PINCHING_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_ring_finger_pinching(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_RING_PINCHING_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_little_finger_pinching(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_LITTLE_PINCHING_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_system_gesture_active(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_SYSTEM_GESTURE_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_dominant_hand(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_DOMINANT_HAND_BIT_FB;
}

bool OpenXRFbHandTrackingAimExtensionWrapper::is_menu_gesture_active(int p_hand_index) {
	ERR_FAIL_COND_V_MSG(!is_available(), false, "OpenXR extension XR_FB_hand_tracking_aim is not available");
	ERR_FAIL_INDEX_V_MSG(p_hand_index, MAX_TRACKED_HANDS, false, vformat("Invalid hand index %d", p_hand_index));
	return aim_state[p_hand_index].status & XR_HAND_TRACKING_AIM_MENU_PRESSED_BIT_FB;
}
