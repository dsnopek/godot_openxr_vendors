/**************************************************************************/
/*  openxr_fb_spatial_entity_query.cpp                                    */
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

#include "classes/openxr_fb_spatial_entity_query.h"

#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"

using namespace godot;

void OpenXRFbSpatialEntityQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_storage_location", "location"), &OpenXRFbSpatialEntityQuery::set_storage_location);
	ClassDB::bind_method(D_METHOD("get_storage_location"), &OpenXRFbSpatialEntityQuery::get_storage_location);
	ClassDB::bind_method(D_METHOD("set_max_results", "count"), &OpenXRFbSpatialEntityQuery::set_max_results);
	ClassDB::bind_method(D_METHOD("get_max_results"), &OpenXRFbSpatialEntityQuery::get_max_results);
	ClassDB::bind_method(D_METHOD("set_timeout", "seconds"), &OpenXRFbSpatialEntityQuery::set_timeout);
	ClassDB::bind_method(D_METHOD("get_timeout"), &OpenXRFbSpatialEntityQuery::get_timeout);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "storage_location", PROPERTY_HINT_ENUM, "Local,Cloud"), "set_storage_location", "get_storage_location");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_results", PROPERTY_HINT_NONE, ""), "set_max_results", "get_max_results");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "timeout", PROPERTY_HINT_RANGE, "0.001,4096,0.001,or_greater,exp,suffix:s"), "set_timeout", "get_timeout");

	BIND_ENUM_CONSTANT(STORAGE_LOCAL);
	BIND_ENUM_CONSTANT(STORAGE_CLOUD);

	ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::Type::ARRAY, "results")));
}

void OpenXRFbSpatialEntityQuery::set_storage_location(StorageLocation p_location) {
	location = p_location;
}

OpenXRFbSpatialEntityQuery::StorageLocation OpenXRFbSpatialEntityQuery::get_storage_location() const {
	return location;
}

void OpenXRFbSpatialEntityQuery::set_max_results(uint32_t p_max_results) {
	max_results = p_max_results;
}

uint32_t OpenXRFbSpatialEntityQuery::get_max_results() const {
	return max_results;
}

void OpenXRFbSpatialEntityQuery::set_timeout(float p_timeout) {
	timeout = p_timeout;
}

float OpenXRFbSpatialEntityQuery::get_timeout() const {
	return timeout;
}

void OpenXRFbSpatialEntityQuery::set_uuids(Array p_uuids) {
	uuids = p_uuids;
}

Array OpenXRFbSpatialEntityQuery::get_uuids() const {
	return uuids;
}

Error OpenXRFbSpatialEntityQuery::execute() {
	void *next_filter = nullptr;

	if (uuids.size() > 0) {
		// @todo Add the uuids filter
	}

	XrSpaceStorageLocationFilterInfoFB filter = {
		XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB, // type
		next_filter, // next
		XR_SPACE_STORAGE_LOCATION_INVALID_FB, // location
	};
	switch (location) {
		case STORAGE_LOCAL: {
			filter.location = XR_SPACE_STORAGE_LOCATION_LOCAL_FB;
		} break;

		case STORAGE_CLOUD: {
			filter.location = XR_SPACE_STORAGE_LOCATION_CLOUD_FB;
		} break;

		default:
			break;
	}
	ERR_FAIL_COND_V(filter.location == XR_SPACE_STORAGE_LOCATION_INVALID_FB, ERR_INVALID_DATA);

	XrSpaceQueryInfoFB query = {
		XR_TYPE_SPACE_QUERY_INFO_FB, // type
		nullptr, // next
		XR_SPACE_QUERY_ACTION_LOAD_FB, // queryAction
		max_results, // maxResultsCount
		filter.location == XR_SPACE_STORAGE_LOCATION_CLOUD_FB ? (XrDuration)(timeout * 1000000) : 0, // timeout
		(XrSpaceFilterInfoBaseHeaderFB *)&filter, // filter
		nullptr, // excludeFilter
	};

	request_id = OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB *)&query, &OpenXRFbSpatialEntityQuery::_results_callback, this);

	return request_id == 0 ? FAILED : OK;
}

void OpenXRFbSpatialEntityQuery::_results_callback(const Vector<XrSpaceQueryResultFB> &p_results, void *p_userdata) {
	OpenXRFbSpatialEntityQuery *self = (OpenXRFbSpatialEntityQuery *)p_userdata;

	Array results;
	for (int i = 0; i < p_results.size(); i++) {
		const uint8_t *data = p_results[i].uuid.data;
		char uuid_str[37];

		sprintf(uuid_str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			data[0], data[1], data[2], data[3],
			data[4], data[5],
			data[6], data[7],
			data[8], data[9],
			data[10], data[11], data[12], data[13], data[14], data[15]);

		results.push_back(String(uuid_str));
	}

	self->emit_signal("completed", results);
}
