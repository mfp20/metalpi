	.text
	v32add -, H(0,0), H(0,0) SETF
	v16bitplanes -, 0x00ff SETF
	v32memread  H32(48, 0), -, 1
