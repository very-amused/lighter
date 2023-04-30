mod xsct;

fn main() {
	let display_temp = xsct::get_display_temp(0);
	println!("Display temp is: {}", display_temp);
}
