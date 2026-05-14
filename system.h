#pragma once
#include <unordered_map>
#include <optional>
#include <string>
#include <Windows.h>
#include "usermode/src/system/system_def.h"

typedef struct _FAR_JMP_16
{
	UCHAR  OpCode;  // = 0xe9
	USHORT Offset;
} FAR_JMP_16;

typedef struct _FAR_TARGET_32
{
	ULONG Offset;
	USHORT Selector;
} FAR_TARGET_32;

typedef struct _PSEUDO_DESCRIPTOR_32 {
	USHORT Limit;
	ULONG Base;
} PSEUDO_DESCRIPTOR_32;

typedef union _KGDTENTRY64
{
	struct
	{
		USHORT  LimitLow;
		USHORT  BaseLow;
		union
		{
			struct
			{
				UCHAR   BaseMiddle;
				UCHAR   Flags1;
				UCHAR   Flags2;
				UCHAR   BaseHigh;
			} Bytes;

			struct
			{
				ULONG   BaseMiddle : 8;
				ULONG   Type : 5;
				ULONG   Dpl : 2;
				ULONG   Present : 1;
				ULONG   LimitHigh : 4;
				ULONG   System : 1;
				ULONG   LongMode : 1;
				ULONG   DefaultBig : 1;
				ULONG   Granularity : 1;
				ULONG   BaseHigh : 8;
			} Bits;
		};
		ULONG BaseUpper;
		ULONG MustBeZero;
	};
	ULONG64 Alignment;
} KGDTENTRY64, * PKGDTENTRY64;

typedef union _KIDTENTRY64
{
	struct
	{
		USHORT OffsetLow;
		USHORT Selector;
		USHORT IstIndex : 3;
		USHORT Reserved0 : 5;
		USHORT Type : 5;
		USHORT Dpl : 2;
		USHORT Present : 1;
		USHORT OffsetMiddle;
		ULONG OffsetHigh;
		ULONG Reserved1;
	};
	ULONG64 Alignment;
} KIDTENTRY64, * PKIDTENTRY64;

typedef union _KGDT_BASE
{
	struct
	{
		USHORT BaseLow;
		UCHAR BaseMiddle;
		UCHAR BaseHigh;
		ULONG BaseUpper;
	};
	ULONG64 Base;
} KGDT_BASE, * PKGDT_BASE;

typedef union _KGDT_LIMIT
{
	struct
	{
		USHORT LimitLow;
		USHORT LimitHigh : 4;
		USHORT MustBeZero : 12;
	};
	ULONG Limit;
} KGDT_LIMIT, * PKGDT_LIMIT;

#define PSB_GDT32_MAX       3

typedef struct _KDESCRIPTOR
{
	USHORT Pad[3];
	USHORT Limit;
	PVOID Base;
} KDESCRIPTOR, * PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32
{
	USHORT Pad[3];
	USHORT Limit;
	ULONG Base;
} KDESCRIPTOR32, * PKDESCRIPTOR32;

typedef struct _KSPECIAL_REGISTERS
{
	ULONG64 Cr0;
	ULONG64 Cr2;
	ULONG64 Cr3;
	ULONG64 Cr4;
	ULONG64 KernelDr0;
	ULONG64 KernelDr1;
	ULONG64 KernelDr2;
	ULONG64 KernelDr3;
	ULONG64 KernelDr6;
	ULONG64 KernelDr7;
	KDESCRIPTOR Gdtr;
	KDESCRIPTOR Idtr;
	USHORT Tr;
	USHORT Ldtr;
	ULONG MxCsr;
	ULONG64 DebugControl;
	ULONG64 LastBranchToRip;
	ULONG64 LastBranchFromRip;
	ULONG64 LastExceptionToRip;
	ULONG64 LastExceptionFromRip;
	ULONG64 Cr8;
	ULONG64 MsrGsBase;
	ULONG64 MsrGsSwap;
	ULONG64 MsrStar;
	ULONG64 MsrLStar;
	ULONG64 MsrCStar;
	ULONG64 MsrSyscallMask;
} KSPECIAL_REGISTERS, * PKSPECIAL_REGISTERS;

typedef struct _KPROCESSOR_STATE
{
	KSPECIAL_REGISTERS SpecialRegisters;
	CONTEXT ContextFrame;
} KPROCESSOR_STATE, * PKPROCESSOR_STATE;

typedef struct _PROCESSOR_START_BLOCK* PPROCESSOR_START_BLOCK;

typedef struct _PROCESSOR_START_BLOCK
{
	FAR_JMP_16 Jmp;
	ULONG CompletionFlag;
	PSEUDO_DESCRIPTOR_32 Gdt32;
	PSEUDO_DESCRIPTOR_32 Idt32;
	KGDTENTRY64 Gdt[PSB_GDT32_MAX + 1];
	ULONG64 TiledCr3;
	FAR_TARGET_32 PmTarget;
	FAR_TARGET_32 LmIdentityTarget;
	PVOID LmTarget;
	PPROCESSOR_START_BLOCK SelfMap;
	ULONG64 MsrPat;
	ULONG64 MsrEFER;
	KPROCESSOR_STATE ProcessorState;
} PROCESSOR_START_BLOCK;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;


typedef struct _SYSTEM_BIGPOOL_ENTRY
{
	union
	{
		PVOID VirtualAddress;
		ULONG_PTR NonPaged : 1;
	};
	ULONG_PTR SizeInBytes;
	union 
	{
		UCHAR Tag[4];
		ULONG TagUlong;
	};
} SYSTEM_BIGPOOL_ENTRY, * PSYSTEM_BIGPOOL_ENTRY;

typedef struct _SYSTEM_BIGPOOL_INFORMATION
{
	ULONG Count;
	SYSTEM_BIGPOOL_ENTRY AllocatedInfo[ANYSIZE_ARRAY];
} SYSTEM_BIGPOOL_INFORMATION, * PSYSTEM_BIGPOOL_INFORMATION;

std::uint64_t GetProcessModule(std::uint32_t pid, const wchar_t* moduleName);
std::uint64_t GetProcessModuleBase(std::uint32_t pid, const wchar_t* moduleName);

namespace sys
{
	struct kernel_module_t;

	std::uint8_t set_up();
	void clean_up();

	namespace kernel
	{
		std::uint8_t parse_modules();
		std::uint8_t dump_module_to_disk(std::string_view target_module_name, const std::string_view output_directory);

		inline std::unordered_map<std::string, kernel_module_t> modules_list = { };
	}

	namespace user
	{
		std::uint32_t query_system_information(std::int32_t information_class, void* information_out, std::uint32_t information_size, std::uint32_t* returned_size);

		std::uint32_t adjust_privilege(std::uint32_t privilege, std::uint8_t enable, std::uint8_t current_thread_only, std::uint8_t* previous_enabled_state);
		std::uint8_t set_debug_privilege(std::uint8_t state, std::uint8_t* previous_state);

		void* allocate_locked_memory(std::uint64_t size, std::uint32_t protection);
		std::uint8_t free_memory(void* address);

		std::string to_string(const std::wstring& wstring);
	}

	namespace fs
	{
		std::uint8_t exists(std::string_view path);

		std::uint8_t write_to_disk(std::string_view full_path, const std::vector<std::uint8_t>& buffer);
	}

	struct kernel_module_t
	{
		std::unordered_map<std::string, std::uint64_t> exports;
		
		std::uint64_t base_address;
		std::uint32_t size;
	};

	std::uint32_t get_pid_from_process_name(const std::string& process_name);
	std::uint64_t get_process_base_address(std::uint32_t pid);
	std::uint32_t get_image_size(std::uint64_t base_address);
	std::uint32_t get_image_size(std::uint64_t base_address, std::uint64_t process_cr3);
	std::uint64_t scan_memory(std::uint64_t base_address, std::uint32_t size, const std::uint8_t* pattern, std::size_t pattern_size);
	std::uint64_t scan_memory(std::uint64_t base_address, std::uint32_t size, const std::uint8_t* pattern, std::size_t pattern_size, std::uint64_t process_cr3);
	std::uint64_t get_peb_address(std::uint32_t target_pid, std::uint64_t kernel_cr3, std::uint64_t ps_initial_system_process);
	std::uint64_t find_target_module_base(std::uint32_t target_pid, std::uint64_t target_cr3, const std::string& module_name);
	std::uint64_t find_export_address_in_target(std::uint64_t target_module_base, std::uint64_t target_cr3, const std::string& function_name);

	inline std::uint64_t current_cr3 = 0;
	inline std::uint64_t ps_active_process_head = 0;
	inline std::uint64_t ps_initial_system_process = 0;
	inline std::uint64_t mm_physical_memory_block = 0;
	inline std::uint64_t ps_loaded_module_list = 0;

	void* get_ntoskrnl_base();
	
	std::uint64_t get_process_cr3(std::uint32_t pid);
}