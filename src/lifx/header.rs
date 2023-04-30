const PROTOCOL: u16 = 1024;
// Protocol bitfields:
const ADDRESSABLE: u8 = 0x10; // set bit 4 to 1
const TAGGED: u8 = 0x20; // set bit 5 to 1

#[repr(packed)]
struct Header {
	// Frame header
	size: u16, // Total message size
	protocol: u16, // Protocol and bitfields
	source: u32, // Unique client ID
	// Frame address
	target: [u8; 8], // 6 byte device serial number, 0 means all devices, last two bytes are always 0
	_reserved0: [u8; 6],
	res_ack_flags: u8, // bit 0 is res_required, 1 is ack_required, all other bits are reserved
	sequence: u8, // Wrap around message sequence number
	// Protocol header
	_reserved1: [u8; 8],
	message_type: u16, // Message type determines the payload being used
	_reserved2: [u8; 2]
}