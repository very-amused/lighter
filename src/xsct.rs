use core::ffi;
use x11::xlib::_XDisplay;

#[repr(C)]
pub struct temp_status {
	temp: ffi::c_int,
	brightness: ffi::c_double
}

extern "C" {
	pub fn get_sct_for_screen(dpy: *const _XDisplay, screen: ffi::c_int, icrtc: ffi::c_int, fdebug: ffi::c_int) -> temp_status;
}
