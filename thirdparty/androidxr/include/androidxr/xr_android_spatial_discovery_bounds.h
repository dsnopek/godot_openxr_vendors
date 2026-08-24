#ifndef XR_ANDROID_SPATIAL_DISCOVERY_BOUNDS_H_
#define XR_ANDROID_SPATIAL_DISCOVERY_BOUNDS_H_ 1

/*
** Copyright 2017-2026 The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0 OR MIT
*/

/*
** This header is generated from the Khronos OpenXR XML API Registry.
**
*/


#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_ANDROID_spatial_discovery_bounds

// XR_ANDROID_spatial_discovery_bounds is a preprocessor guard. Do not pass it to API calls.
#define XR_ANDROID_spatial_discovery_bounds 1
// The runtime supports slink:XrSpatialBoundsSpherefANDROID filter for slink:XrSpatialDiscoverySnapshotCreateInfoEXT; This does not require any configuration structure to be included during spatial context creation.
#define XR_SPATIAL_CAPABILITY_FEATURE_SPHERE_BOUNDS_FILTER_ANDROID ((XrSpatialCapabilityFeatureEXT) 1000761000U)
// The runtime supports slink:XrSpatialBoundsBoxfANDROID filter for slink:XrSpatialDiscoverySnapshotCreateInfoEXT; This does not require any configuration structure to be included during spatial context creation.
#define XR_SPATIAL_CAPABILITY_FEATURE_BOX_BOUNDS_FILTER_ANDROID ((XrSpatialCapabilityFeatureEXT) 1000761001U)
// The runtime supports slink:XrSpatialBoundsFrustumfANDROID filter for slink:XrSpatialDiscoverySnapshotCreateInfoEXT; This does not require any configuration structure to be included during spatial context creation.
#define XR_SPATIAL_CAPABILITY_FEATURE_FRUSTUM_BOUNDS_FILTER_ANDROID ((XrSpatialCapabilityFeatureEXT) 1000761002U)
#define XR_TYPE_SPATIAL_BOUNDS_SPHEREF_ANDROID ((XrStructureType) 1000761000U)
#define XR_TYPE_SPATIAL_BOUNDS_BOXF_ANDROID ((XrStructureType) 1000761001U)
#define XR_TYPE_SPATIAL_BOUNDS_FRUSTUMF_ANDROID ((XrStructureType) 1000761002U)

#define XR_ANDROID_spatial_discovery_bounds_SPEC_VERSION 1
#define XR_ANDROID_SPATIAL_DISCOVERY_BOUNDS_EXTENSION_NAME "XR_ANDROID_spatial_discovery_bounds"
// XrSpatialBoundsSpherefANDROID extends XrSpatialDiscoverySnapshotCreateInfoEXT
typedef struct XrSpatialBoundsSpherefANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
    XrSpheref                   sphere;
} XrSpatialBoundsSpherefANDROID;

// XrSpatialBoundsBoxfANDROID extends XrSpatialDiscoverySnapshotCreateInfoEXT
typedef struct XrSpatialBoundsBoxfANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
    XrBoxf                      box;
} XrSpatialBoundsBoxfANDROID;

// XrSpatialBoundsFrustumfANDROID extends XrSpatialDiscoverySnapshotCreateInfoEXT
typedef struct XrSpatialBoundsFrustumfANDROID {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrSpace                     space;
    XrTime                      time;
    XrFrustumf                  frustum;
} XrSpatialBoundsFrustumfANDROID;


// Reflection macros
#define XR_LIST_STRUCT_XrSpatialBoundsSpherefANDROID(_) \
    _(type) \
    _(next) \
    _(space) \
    _(time) \
    _(sphere)

#define XR_LIST_STRUCT_XrSpatialBoundsBoxfANDROID(_) \
    _(type) \
    _(next) \
    _(space) \
    _(time) \
    _(box)

#define XR_LIST_STRUCT_XrSpatialBoundsFrustumfANDROID(_) \
    _(type) \
    _(next) \
    _(space) \
    _(time) \
    _(frustum)

#endif /* XR_ANDROID_spatial_discovery_bounds */

#ifdef __cplusplus
}
#endif

#endif
