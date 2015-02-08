ENTRY(start)
SECTIONS
{
	/*
	 * This section contains kernel code that is
	 * not visible from userland
	 */
	.text 0x100000 :
	{
		code = .; _code = .; __code = .;
		*(.text)
		. = ALIGN(4096);
	}
	
	/*
	 * This section contains code that is visible
	 * and executable from userland
	 */
	.utext :
	{
		ucode = .; _ucode = .; __ucode = .;
		*(.utext)
		. = ALIGN(4096);
	}

	.data :
	{
		data = .; _data = .; __data = .;
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}

	.bss :
	{
		bss = .; _bss = .; __bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	end = .; _end = .; __end = .;
} 
