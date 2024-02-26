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

	ADD_PROPERTY(PropertyInfo(Variant::INT, "storage_location", PROPERTY_HINT_ENUM, "Local,Cloud"), "set_storage_location", "get_storage_location");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_results", PROPERTY_HINT_NONE, ""), "set_max_results", "get_max_results");

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

void OpenXRFbSpatialEntityQuery::set_max_results(int p_max_results) {
	max_results = p_max_results;
}

int OpenXRFbSpatialEntityQuery::get_max_results() const {
	return max_results;
}

void OpenXRFbSpatialEntityQuery::set_uuids(Array p_uuids) {
	uuids = p_uuids;
}

Array OpenXRFbSpatialEntityQuery::get_uuids() const {
	return uuids;
}

Error OpenXRFbSpatialEntityQuery::execute() {

}
