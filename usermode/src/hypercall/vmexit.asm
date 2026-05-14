.code
	launch_raw_hypercall proc
		push rbx
		push rcx
		push rdx
		
		xor eax, eax
		cpuid
		
		cmp ebx, 68747541h
		jne intel_path
		cmp edx, 69746e65h
		jne intel_path
		cmp ecx, 444d4163h
		jne intel_path
		
		mov eax, 1337h
		jmp do_hypercall
		
	intel_path:
		mov eax, 1338h
		
	do_hypercall:
		pop rdx
		pop rcx
		pop rbx
		cpuid
		ret
	launch_raw_hypercall endp
END