/**************************************************************************/
/*  openxr_fb_spatial_entity_query.h                                      */
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

#ifndef OPENXR_FB_SPATIAL_ENTITY_QUERY_H
#define OPENXR_FB_SPATIAL_ENTITY_QUERY_H

#include <openxr/openxr.h>
#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class OpenXRFbSpatialEntityQuery : public RefCounted {
	GDCLASS(OpenXRFbSpatialEntityQuery, RefCounted);

public:
	enum StorageLocation {
		STORAGE_LOCAL,
		STORAGE_CLOUD,
	};

private:
	StorageLocation location = STORAGE_LOCAL;
	int max_results = 25;
	Array uuids;

	XrAsyncRequestIdFB request_id = 0;

protected:
	static void _bind_methods();

public:
	void set_storage_location(StorageLocation p_location);
	StorageLocation get_storage_location() const;

	void set_max_results(int p_max_results);
	int get_max_results() const;

	void set_uuids(Array p_uuids);
	Array get_uuids() const;

	Error execute();
};

} // namespace godot

VARIANT_ENUM_CAST(OpenXRFbSpatialEntityQuery::StorageLocation);

#endif
