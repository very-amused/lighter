use std::ptr::null;
use x11::xlib::{_XDisplay, self};

// Min/max input temps xsct can handle
const XSCT_MIN_TEMP: i32 = 0;
const XSCT_MAX_TEMP: i32 = 10_000;
/// lighter is currently only designed with desktop use in mind
const BRIGHTNESS_NORM: f64 = 1.0;
// Link fdebug option to rust build mode
#[cfg(not(debug_assertions))]
const FDEBUG: i32 = 0;
#[cfg(debug_assertions)]
const FDEBUG: i32 = 1;
/// Do not select specific CRTC
const ICRTC: i32 = -1;

#[repr(C)]
struct temp_status {
	temp: i32,
	brightness: f64
}

impl temp_status {
	pub fn new(mut temp: i32) -> Self {
		// Clamp to XSCT min/max temp
		if temp < XSCT_MIN_TEMP {
			temp = XSCT_MIN_TEMP;
		} else if temp > XSCT_MAX_TEMP {
			temp = XSCT_MAX_TEMP;
		}
		Self { temp, brightness: BRIGHTNESS_NORM }
	}
}

extern "C" {
	fn get_sct_for_screen(dpy: *const _XDisplay, screen: i32, icrtc: i32, fdebug: i32) -> temp_status;
	fn sct_for_screen(dpy: *const _XDisplay, screen: i32, icrtc: i32, temp: temp_status, fdebug: i32);
}

// Get the currently set display temperature
pub fn get_display_temp() -> i32 {
	unsafe {
		let dpy = xlib::XOpenDisplay(null());
		if dpy.is_null() {
			// Replicate xsct error handling
			eprintln!("XOpenDisplay(NULL) failed! Ensure DISPLAY is set correctly.");
			return -1;
		}
		const SCREEN: i32 = 0; // Use screen 0 for temp reference
		let status = get_sct_for_screen(dpy, SCREEN, ICRTC, FDEBUG);
		xlib::XCloseDisplay(dpy);
		status.temp
	}
}

// Pass None as screen to apply to all screens
pub fn set_display_temp(temp: i32) {
	unsafe {
		let dpy = xlib::XOpenDisplay(null());
		if dpy.is_null() {
			// Replicate xsct error handling
			eprintln!("XOpenDisplay(NULL) failed! Ensure DISPLAY is set correctly.");
			return;
		}
		let screen_count = xlib::XScreenCount(dpy);
		// Set temperature for all xrandr 'screens', note that these do not correspond to real displays
		for s in 0..screen_count {
			sct_for_screen(dpy, s, ICRTC, temp_status::new(temp), FDEBUG);
		}
		xlib::XCloseDisplay(dpy);
	}
}