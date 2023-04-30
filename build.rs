fn main() {
	cc::Build::new()
		.file("xsct/xsct.c")
		.include("xsct")
		.compile("xsct");
}