/**************************************************************************/
/*  GodotOpenXRLocationFinderInternal.kt                                  */
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

package org.godotengine.openxr.vendors

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.location.LocationManager
import android.os.Build
import android.util.Log
import androidx.core.content.ContextCompat
import androidx.core.location.LocationManagerCompat
import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.plugin.UsedByGodot
import org.godotengine.godot.variant.Callable

/**
 * A plugin used internally to get the device's current location.
 */
class GodotOpenXRLocationFinderInternal(godot: Godot?) : GodotPlugin(godot) {
	internal companion object {
		private val TAG = GodotOpenXRLocationFinderInternal::class.java.simpleName
	}

	override fun getPluginName() = "GodotOpenXRLocationFinderInternal"

	@UsedByGodot
	private fun findLocation(callback: Callable) {
		val context = activity
		if (context == null) {
			Log.e(TAG, "Unable to find location: no activity")
			invokeCallback(callback, false, 0f, 0f)
			return
		}

		if (!hasLocationPermission(context)) {
			Log.e(TAG, "Unable to find location: missing location permission")
			invokeCallback(callback, false, 0f, 0f)
			return
		}

		val locationManager = context.getSystemService(Context.LOCATION_SERVICE) as? LocationManager
		if (locationManager == null) {
			Log.e(TAG, "Unable to find location: no LocationManager service")
			invokeCallback(callback, false, 0f, 0f)
			return
		}

		val provider = buildList {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) add(LocationManager.FUSED_PROVIDER)
			add(LocationManager.NETWORK_PROVIDER)
			add(LocationManager.GPS_PROVIDER)
		}.firstOrNull { locationManager.isProviderEnabled(it) }

		if (provider == null) {
			Log.e(TAG, "Unable to find location: no enabled location provider")
			invokeCallback(callback, false, 0f, 0f)
			return
		}

		try {
			LocationManagerCompat.getCurrentLocation(
				locationManager,
				provider,
				null,
				ContextCompat.getMainExecutor(context)
			) { location ->
				if (location != null) {
					invokeCallback(callback, true, location.latitude.toFloat(), location.longitude.toFloat())
				} else {
					Log.e(TAG, "Unable to find location: provider $provider returned no location")
					invokeCallback(callback, false, 0f, 0f)
				}
			}
		} catch (e: SecurityException) {
			Log.e(TAG, "Unable to find location", e)
			invokeCallback(callback, false, 0f, 0f)
		}
	}

	private fun hasLocationPermission(context: Context): Boolean {
		return ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED ||
			ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION) == PackageManager.PERMISSION_GRANTED
	}

	private fun invokeCallback(callback: Callable, success: Boolean, latitude: Float, longitude: Float) {
		runOnRenderThread {
			callback.call(success, latitude, longitude)
		}
	}
}
