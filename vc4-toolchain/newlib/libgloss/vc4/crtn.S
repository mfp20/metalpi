	.file		"crtn.S"

	.section	".init"
	ldm pc, (sp++)
	
	.section	".fini"
	ldm pc, (sp++)
