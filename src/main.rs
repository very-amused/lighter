mod xsct;

fn main() {
	xsct::set_display_temp(3500);
	let display_temp = xsct::get_display_temp();
	println!("Display temp is: {}", display_temp);
}
