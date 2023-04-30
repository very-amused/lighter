use std::ptr::null;
use x11::xlib::{_XDisplay, self};

#[repr(C)]
struct temp_status {
	temp: i32,
	brightness: f64
}

// Link fdebug option to rust build mode
#[cfg(not(debug_assertions))]
const FDEBUG: i32 = 0;
#[cfg(debug_assertions)]
const FDEBUG: i32 = 1;

// TODO: document
const ICRTC: i32 = 0;

extern "C" {
	fn get_sct_for_screen(dpy: *const _XDisplay, screen: i32, icrtc: i32, fdebug: i32) -> temp_status;
}

// Get the currently set display temperature
pub fn get_display_temp(mut screen: i32) -> i32 {
	unsafe {
		let dpy = xlib::XOpenDisplay(null());
		if dpy.is_null() {
			// Replicate xsct error handling
			eprintln!("XOpenDisplay(NULL) failed! Ensure DISPLAY is set correctly.");
			return -1;
		}
		// Validate screen argument
		let screen_count = xlib::XScreenCount(dpy);
		if screen >= screen_count || screen < 0 {
			screen = 0;
		}
		let status = get_sct_for_screen(dpy, screen, ICRTC, FDEBUG);
		status.temp
	}
}
