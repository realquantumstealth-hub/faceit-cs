#include "commands.h"
#include <CLI/CLI.hpp>
#include <hypercall/hypercall_def.h>
#include "../hook/hook.h"
#include "../hypercall/hypercall.h"
#include "../system/system.h"
#include "../dll_loader/dll_loader.h"

#include <print>
#include <array>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <Windows.h>

#define d_invoke_command_processor(command) process_##command(##command)
#define d_initial_process_command(command) if (*##command) d_invoke_command_processor(command)
#define d_process_command(command) else if (*##command) d_invoke_command_processor(command)

template <class t>
t get_command_option(CLI::App* app, std::string option_name)
{
	auto option = app->get_option(option_name);

	return option->empty() == false ? option->as<t>() : t{};
}

CLI::Option* add_command_option(CLI::App* app, std::string option_name)
{
	return app->add_option(option_name);
}

CLI::Option* add_transformed_command_option(CLI::App* app, std::string option_name, CLI::Transformer& transformer)
{
	CLI::Option* option = add_command_option(app, option_name);

	return option->transform(transformer);
}

std::uint8_t get_command_flag(CLI::App* app, std::string flag_name)
{
	auto option = app->get_option(flag_name);

	return !option->empty();
}

CLI::Option* add_command_flag(CLI::App* app, std::string flag_name)
{
	return app->add_flag(flag_name);
}

CLI::App* init_rgpm(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* rgpm = app.add_subcommand("rgpm", "reads memory from a given guest physical address")->ignore_case();

	add_transformed_command_option(rgpm, "physical_address", aliases_transformer)->required();
	add_command_option(rgpm, "size")->check(CLI::Range(0, 8))->required();

	return rgpm;
}

void process_rgpm(CLI::App* rgpm)
{
	const std::uint64_t guest_physical_address = get_command_option<std::uint64_t>(rgpm, "physical_address");
	const std::uint64_t size = get_command_option<std::uint64_t>(rgpm, "size");

	std::uint64_t value = 0;

	const std::uint64_t bytes_read = hypercall::read_guest_physical_memory(&value, guest_physical_address, size);

	if (bytes_read == size)
	{
		std::println("value: 0x{:x}", value);
	}
	else
	{
		std::println("failed to read");
	}
}

CLI::App* init_wgpm(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* wgpm = app.add_subcommand("wgpm", "writes memory to a given guest physical address")->ignore_case();

	add_transformed_command_option(wgpm, "physical_address", aliases_transformer)->required();
	add_command_option(wgpm, "value")->required();
	add_command_option(wgpm, "size")->check(CLI::Range(0, 8))->required();

	return wgpm;
}

void process_wgpm(CLI::App* wgpm)
{
	const std::uint64_t guest_physical_address = get_command_option<std::uint64_t>(wgpm, "physical_address");
	const std::uint64_t size = get_command_option<std::uint64_t>(wgpm, "size");

	std::uint64_t value = get_command_option<std::uint64_t>(wgpm, "value");

	const std::uint64_t bytes_written = hypercall::write_guest_physical_memory(&value, guest_physical_address, size);

	if (bytes_written == size)
	{
		std::println("success in write");
	}
	else
	{
		std::println("failed to write");
	}
}

CLI::App* init_cgpm(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* cgpm = app.add_subcommand("cgpm", "copies memory from a given source to a destination (guest physical addresses)")->ignore_case();

	add_transformed_command_option(cgpm, "destination_physical_address", aliases_transformer)->required();
	add_transformed_command_option(cgpm, "source_physical_address", aliases_transformer)->required();
	add_command_option(cgpm, "size")->required();

	return cgpm;
}

void process_cgpm(CLI::App* cgpm)
{
	const std::uint64_t guest_destination_physical_address = get_command_option<std::uint64_t>(cgpm, "destination_physical_address");
	const std::uint64_t guest_source_physical_address = get_command_option<std::uint64_t>(cgpm, "source_physical_address");
	const std::uint64_t size = get_command_option<std::uint64_t>(cgpm, "size");

	std::vector<std::uint8_t> buffer(size);

	const std::uint64_t bytes_read = hypercall::read_guest_physical_memory(buffer.data(), guest_source_physical_address, size);
	const std::uint64_t bytes_written = hypercall::write_guest_physical_memory(buffer.data(), guest_destination_physical_address, size);

	if ((bytes_read == size) && (bytes_written == size))
	{
		std::println("success in copy");
	}
	else
	{
		std::println("failed to copy");
	}
}

CLI::App* init_gvat(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* gvat = app.add_subcommand("gvat", "translates a guest virtual address to its corresponding guest physical address, with the given guest cr3 value")->ignore_case();

	add_transformed_command_option(gvat, "virtual_address", aliases_transformer)->required();
	add_transformed_command_option(gvat, "cr3", aliases_transformer)->required();

	return gvat;
}

void process_gvat(CLI::App* gvat)
{
	const std::uint64_t virtual_address = get_command_option<std::uint64_t>(gvat, "virtual_address");
	const std::uint64_t cr3 = get_command_option<std::uint64_t>(gvat, "cr3");

	const std::uint64_t physical_address = hypercall::translate_guest_virtual_address(virtual_address, cr3);

	std::println("physical address: 0x{:x}", physical_address);
}

CLI::App* init_rgvm(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* rgvm = app.add_subcommand("rgvm", "reads memory from a given guest virtual address (when given the corresponding guest cr3 value)")->ignore_case();

	add_transformed_command_option(rgvm, "virtual_address", aliases_transformer)->required();
	add_transformed_command_option(rgvm, "cr3", aliases_transformer)->required();
	add_command_option(rgvm, "size")->check(CLI::Range(0, 8))->required();

	return rgvm;
}

void process_rgvm(CLI::App* rgvm)
{
	const std::uint64_t guest_virtual_address = get_command_option<std::uint64_t>(rgvm, "virtual_address");
	const std::uint64_t cr3 = get_command_option<std::uint64_t>(rgvm, "cr3");
	const std::uint64_t size = get_command_option<std::uint64_t>(rgvm, "size");

	std::uint64_t value = 0;

	const std::uint64_t bytes_read = hypercall::read_guest_virtual_memory(&value, guest_virtual_address, cr3, size);

	if (bytes_read == size)
	{
		std::println("value: 0x{:x}", value);
	}
	else
	{
		std::println("failed to read");
	}
}

CLI::App* init_wgvm(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* wgvm = app.add_subcommand("wgvm", "writes memory from a given guest virtual address (when given the corresponding guest cr3 value)")->ignore_case();

	add_transformed_command_option(wgvm, "virtual_address", aliases_transformer)->required();
	add_transformed_command_option(wgvm, "cr3", aliases_transformer)->required();
	add_command_option(wgvm, "value")->required();
	add_command_option(wgvm, "size")->check(CLI::Range(0, 8))->required();

	return wgvm;
}

void process_wgvm(CLI::App* wgvm)
{
	const std::uint64_t guest_virtual_address = get_command_option<std::uint64_t>(wgvm, "virtual_address");
	const std::uint64_t cr3 = get_command_option<std::uint64_t>(wgvm, "cr3");
	const std::uint64_t size = get_command_option<std::uint64_t>(wgvm, "size");

	std::uint64_t value = get_command_option<std::uint64_t>(wgvm, "value");

	const std::uint64_t bytes_written = hypercall::write_guest_virtual_memory(&value, guest_virtual_address, cr3, size);

	if (bytes_written == size)
	{
		std::println("success in write at given address");
	}
	else
	{
		std::println("failed to write at given address");
	}
}

CLI::App* init_cgvm(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* cgvm = app.add_subcommand("cgvm", "copies memory from a given source to a destination (guest virtual addresses) (when given the corresponding guest cr3 values)")->ignore_case();

	add_transformed_command_option(cgvm, "destination_virtual_address", aliases_transformer)->required();
	add_transformed_command_option(cgvm, "destination_cr3", aliases_transformer)->required();
	add_transformed_command_option(cgvm, "source_virtual_address", aliases_transformer)->required();
	add_transformed_command_option(cgvm, "source_cr3", aliases_transformer)->required();
	add_command_option(cgvm, "size")->required();

	return cgvm;
}

void process_cgvm(CLI::App* wgvm)
{
	const std::uint64_t guest_destination_virtual_address = get_command_option<std::uint64_t>(wgvm, "destination_virtual_address");
	const std::uint64_t guest_destination_cr3 = get_command_option<std::uint64_t>(wgvm, "destination_cr3");

	const std::uint64_t guest_source_virtual_address = get_command_option<std::uint64_t>(wgvm, "source_virtual_address");
	const std::uint64_t guest_source_cr3 = get_command_option<std::uint64_t>(wgvm, "source_cr3");

	const std::uint64_t size = get_command_option<std::uint64_t>(wgvm, "size");

	std::vector<std::uint8_t> buffer(size);

	const std::uint64_t bytes_read = hypercall::read_guest_virtual_memory(buffer.data(), guest_source_virtual_address, guest_source_cr3, size);
	const std::uint64_t bytes_written = hypercall::write_guest_virtual_memory(buffer.data(), guest_destination_virtual_address, guest_destination_cr3, size);

	if ((bytes_read == size) && (bytes_written == size))
	{
		std::println("success in copy");
	}
	else
	{
		std::println("failed to copy");
	}
}

CLI::App* init_akh(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* akh = app.add_subcommand("akh", "add a hook on specified kernel code (given the guest virtual address) (asmbytes in form: 0xE8 0x12 0x23 0x34 0x45)")->ignore_case();

	add_transformed_command_option(akh, "virtual_address", aliases_transformer)->required();
	add_command_option(akh, "--asmbytes")->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)->expected(-1);
	add_command_option(akh, "--post_original_asmbytes")->multi_option_policy(CLI::MultiOptionPolicy::TakeAll)->expected(-1);
	add_command_flag(akh, "--monitor");

	return akh;
}

void process_akh(CLI::App* akh)
{
	const std::uint64_t virtual_address = get_command_option<std::uint64_t>(akh, "virtual_address");

	std::vector<uint8_t> asm_bytes = get_command_option<std::vector<uint8_t>>(akh, "--asmbytes");
	const std::vector<uint8_t> post_original_asm_bytes = get_command_option<std::vector<uint8_t>>(akh, "--post_original_asmbytes");

	const std::uint8_t monitor = get_command_flag(akh, "--monitor");

	if (monitor == 1)
	{
		std::array<std::uint8_t, 9> monitor_bytes = {
			0x51, // push rcx
			0xB9, 0x00, 0x00, 0x00, 0x00, // mov ecx, 0
			0x0F, 0xA2, // cpuid
			0x59 // pop rcx
		};

		hypercall_info_t call_info = { };

		call_info.primary_key = hypercall_primary_key;
		call_info.secondary_key = hypercall_secondary_key;
		call_info.call_type = hypercall_type_t::log_current_state;

		*reinterpret_cast<std::uint32_t*>(&monitor_bytes[2]) = static_cast<std::uint32_t>(call_info.value);

		asm_bytes.insert(asm_bytes.end(), monitor_bytes.begin(), monitor_bytes.end());
	}

	const std::uint8_t hook_status = hook::add_kernel_hook(virtual_address, asm_bytes, post_original_asm_bytes);

	if (hook_status == 1)
	{
		std::println("success in hook");
	}
	else
	{
		std::println("failed to hook");
	}
}

CLI::App* init_rkh(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* rkh = app.add_subcommand("rkh", "remove a previously placed hook on specified kernel code (given the guest virtual address)")->ignore_case();

	add_transformed_command_option(rkh, "virtual_address", aliases_transformer)->required();

	return rkh;
}

void process_rkh(CLI::App* rkh)
{
	const std::uint64_t virtual_address = get_command_option<std::uint64_t>(rkh, "virtual_address");

	const std::uint8_t hook_removal_status = hook::remove_kernel_hook(virtual_address, 1);

	if (hook_removal_status == 1)
	{
		std::println("success in hook removal");
	}
	else
	{
		std::println("failed to remove hook");
	}
}

CLI::App* init_hgpp(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* hgpp = app.add_subcommand("hgpp", "hide a physical page's real contents from the guest")->ignore_case();

	add_transformed_command_option(hgpp, "physical_address", aliases_transformer)->required();

	return hgpp;
}

void process_hgpp(CLI::App* hgpp)
{
	const std::uint64_t physical_address = get_command_option<std::uint64_t>(hgpp, "physical_address");

	const std::uint64_t hide_status = hypercall::hide_guest_physical_page(physical_address);

	if (hide_status == 1)
	{
		std::println("success in hiding page");
	}
	else
	{
		std::println("failed to hide page");
	}
}

CLI::App* init_fl(CLI::App& app)
{
	CLI::App* fl = app.add_subcommand("fl", "flush trap frame logs from hooks")->ignore_case();

	return fl;
}

// Helper function to get hypercall type name
std::string get_hypercall_type_name(std::uint64_t call_type)
{
	switch (call_type) {
		case 0: return "guest_physical_memory_operation";
		case 1: return "guest_virtual_memory_operation";
		case 2: return "translate_guest_virtual_address";
		case 3: return "read_guest_cr3";
		case 4: return "add_slat_code_hook";
		case 5: return "remove_slat_code_hook";
		case 6: return "hide_guest_physical_page";
		case 7: return "log_current_state";
		case 8: return "flush_logs";
		case 9: return "get_heap_free_page_count";
		case 10: return "get_process_base_by_pid";
		case 11: return "get_process_cr3";
		case 12: return "check_hyperv_attachment_memory_mapping";
		case 13: return "allocate_hidden_memory";
		case 14: return "free_hidden_memory";
		case 15: return "get_process_eprocess_base";
		case 16: return "call_dllmain_silently";
		case 17: return "dirbase_from_base_address";
		case 18: return "hide_hypervisor_memory";
		case 19: return "restore_hypervisor_memory";
		case 20: return "get_ntoskrnl_base_address";
		case 21: return "query_hypervisor_pfn_info";
		case 22: return "get_hypervisor_memory_info";
		default: return std::format("unknown_hypercall_{}", call_type);
	}
}

// Helper function to interpret common hypervisor log markers
std::string interpret_log_marker(std::uint64_t r15, std::uint64_t r14, std::uint64_t r13)
{
	switch (r15) {
		case 0xC0DE: return std::format("HYPERCALL: {} (type={})", get_hypercall_type_name(r14), r14);
		case 0xEEE1: return "get_ntoskrnl_base_address hypercall entry";
		case 0xEEE2: return "boot address success";
		case 0xEEE3: return "boot failed, trying dynamic discovery";
		case 0xEEE4: return "dynamic discovery success";
		case 0xEEE5: return "complete failure to find ntoskrnl";
		case 0xDDD1: return "IDT discovery start";
		case 0xDDD2: return "IDT failure - no IDT available";
		case 0xDDD3: return "IDT read failure (should not occur)";
		case 0xDDD4: return "Valid IDT entry found";
		case 0xDDD5: return "Successful ntoskrnl discovery via IDT";
		case 0xDDD6: return "Final failure - ntoskrnl not found";
		case 0xDDD7: return "Fallback: estimated ntoskrnl base from kernel address";
		case 0xDE: return "call_dllmain_silently validation failure";
		case 0xD2: return "call_dllmain_silently safe address failure";
		case 0xDF: return "call_dllmain_silently execution start";
		case 0x141241251: return "call_dllmain_silently entry confirmation";
		case 0xE001: return "PE validation attempt";
		case 0xE002: return "MZ signature found";
		case 0xE003: return "Memory read failure during PE validation";
		case 0xF001: return "PFN initialization start";
		case 0xF002: return "PFN initialization failure";
		case 0xF003: return "PFN initialization success";
		case 0xEEE6: return "update_ntoskrnl_base_address hypercall entry";
		case 0xEEE7: return "ntoskrnl base update success";
		case 0xEEE8: return "ntoskrnl base update failure";
		case 0xE010: return "Export resolution start";
		case 0xE011: return "Invalid ntoskrnl base for export";
		case 0xE012: return "DOS header read failed";
		case 0xE013: return "Invalid DOS signature";
		case 0xE014: return "NT headers read failed";
		case 0xE015: return "Invalid NT signature";
		case 0xE016: return "No export directory found";
		case 0xE017: return "Export directory read failed";
		case 0xE018: return "Export found successfully";
		case 0xEEE9: return "set_kernel_cr3 hypercall entry";
		case 0xEEEA: return "kernel CR3 update success";
		case 0xEEEB: return "kernel CR3 update failure";
		case 0xEEEC: return "get_ntoskrnl_base_from_kpcr hypercall entry";
		case 0xEEED: return "KPCR ntoskrnl discovery success";
		case 0xEEEE: return "KPCR ntoskrnl discovery failure";
		case 0xF100: return "KPCR traversal start (kernel CR3 + SLAT)";
		case 0xF101: return "KPCR base read (GS:0x18)";
		case 0xF102: return "KPCR CurrentThread pointer address";
		case 0xF103: return "KPCR CurrentThread pointer value";
		case 0xF104: return "KPCR KTHREAD->Process pointer address";
		case 0xF105: return "KPCR traversal success (current EPROCESS)";
		case 0xF1E0: return "KPCR traversal aborted: missing CR3 context";
		case 0xF1E1: return "KPCR traversal failed: GS read exception";
		case 0xF1E2: return "KPCR traversal failed: KPCR base is null";
		case 0xF1E3: return "KPCR traversal failed: unable to read CurrentThread pointer";
		case 0xF1E4: return "KPCR traversal failed: CurrentThread pointer is null";
		case 0xF1E5: return "KPCR traversal failed: unable to read EPROCESS pointer";
		case 0xF1E6: return "KPCR traversal failed: EPROCESS pointer is null";
		case 0xF200: return "Debug: about to read memory";
		case 0xF201: return "Debug: memory read result";
		case 1: return "allocate_hidden_memory: input validation";
		case 2: return "allocate_hidden_memory: target process CR3";
		case 3: return "allocate_hidden_memory: pre-flight check failed";
		case 0x10: return "allocate_hidden_memory: success";
		case 0x11: return "allocate_hidden_memory: rollback";
		default:
			if (r15 >= 0x20 && r15 <= 0x2F) {
				return std::format("allocate_hidden_memory: page allocation {}", r15 - 0x20);
			}
			if (r15 >= 0x30 && r15 <= 0x39) {
				return std::format("allocate_hidden_memory: allocation failure type {}", r15 - 0x30);
			}
			if (r15 != 0) {
				return std::format("Unknown marker: 0x{:X}", r15);
			}
			return "";
	}
}

void process_fl(CLI::App* fl)
{
	constexpr std::uint64_t log_count = 1000;  // Increased from 100 to 1000
	constexpr std::uint64_t failed_log_count = -1;

	std::vector<trap_frame_log_t> logs(log_count);

	const std::uint64_t logs_flushed = hypercall::flush_logs(logs);

	if (logs_flushed == failed_log_count)
	{
		std::println("failed to flush logs");
	}
	else if (logs_flushed == 0)
	{
		std::println("there are no logs to flush");
	}
	else
	{
		std::println("success in flushing logs ({}), outputting logs now:\n\n", logs_flushed);

		for (std::uint64_t i = 0; i < logs_flushed; i++)
		{
			const trap_frame_log_t& log = logs[i];

			// Interpret the log marker for better readability
			std::string interpretation = interpret_log_marker(log.r15, log.r14, log.r13);
			
			if (!interpretation.empty()) {
				std::println("{}. [{}]", i, interpretation);
			} else {
				std::println("{}. [Standard execution log]", i);
			}

			// Show key registers with better formatting - hide zero values for clarity
			std::println("  rip=0x{:X} cr3=0x{:X}", log.rip, log.cr3);
			
			// Only show non-zero register values to reduce noise
			if (log.rax != 0) std::println("  rax=0x{:X}", log.rax);
			if (log.rcx != 0) std::println("  rcx=0x{:X}", log.rcx);
			if (log.rdx != 0) std::println("  rdx=0x{:X}", log.rdx);
			if (log.rbx != 0) std::println("  rbx=0x{:X}", log.rbx);
			if (log.rsp != 0) std::println("  rsp=0x{:X}", log.rsp);
			if (log.rbp != 0) std::println("  rbp=0x{:X}", log.rbp);
			if (log.rsi != 0) std::println("  rsi=0x{:X}", log.rsi);
			if (log.rdi != 0) std::println("  rdi=0x{:X}", log.rdi);
			if (log.r8 != 0) std::println("  r8=0x{:X}", log.r8);
			if (log.r9 != 0) std::println("  r9=0x{:X}", log.r9);
			if (log.r10 != 0) std::println("  r10=0x{:X}", log.r10);
			if (log.r11 != 0) std::println("  r11=0x{:X}", log.r11);
			if (log.r12 != 0) std::println("  r12=0x{:X}", log.r12);
			if (log.r13 != 0) std::println("  r13=0x{:X}", log.r13);
			if (log.r14 != 0) std::println("  r14=0x{:X}", log.r14);
			if (log.r15 != 0) std::println("  r15=0x{:X}", log.r15);

			// Show stack data only if it contains non-zero values
			bool has_stack_data = false;
			for (const std::uint64_t stack_value : log.stack_data)
			{
				if (stack_value != 0) {
					if (!has_stack_data) {
						std::println("  stack data:");
						has_stack_data = true;
					}
					std::println("    0x{:X}", stack_value);
				}
			}

			std::println();
		}
	}
}

CLI::App* init_hfpc(CLI::App& app)
{
	CLI::App* hfpc = app.add_subcommand("hfpc", "get hyperv-attachment's heap free page count")->ignore_case();

	return hfpc;
}

void process_hfpc(CLI::App* hfpc)
{
	const std::uint64_t heap_free_page_count = hypercall::get_heap_free_page_count();

	std::println("heap free page count: {}", heap_free_page_count);
}

CLI::App* init_lkm(CLI::App& app)
{
	CLI::App* lkm = app.add_subcommand("lkm", "print list of loaded kernel modules")->ignore_case();

	return lkm;
}

void process_lkm(CLI::App* lkm)
{
	for (const auto& [module_name, module_info] : sys::kernel::modules_list)
	{
		std::println("'{}' has a base address of: 0x{:x}, and a size of: 0x{:X}", module_name, module_info.base_address, module_info.size);
	}
}

CLI::App* init_kme(CLI::App& app)
{
	CLI::App* kme = app.add_subcommand("kme", "list the exports of a loaded kernel module (when given the name)")->ignore_case();

	add_command_option(kme, "module_name")->required();

	return kme;
}

void process_kme(CLI::App* kme)
{
	const std::string module_name = get_command_option<std::string>(kme, "module_name");

	if (sys::kernel::modules_list.contains(module_name) == false)
	{
		std::println("module not found");

		return;
	}

	const sys::kernel_module_t module = sys::kernel::modules_list[module_name];

	for (auto& [export_name, export_address] : module.exports)
	{
		std::println("{} = 0x{:X}", export_name, export_address);
	}
}

CLI::App* init_dkm(CLI::App& app)
{
	CLI::App* dkm = app.add_subcommand("dkm", "dump kernel module to a file on disk")->ignore_case();

	add_command_option(dkm, "module_name")->required();
	add_command_option(dkm, "output_directory")->required();

	return dkm;
}

void process_dkm(CLI::App* dkm)
{
	const std::string module_name = get_command_option<std::string>(dkm, "module_name");

	if (sys::kernel::modules_list.contains(module_name) == false)
	{
		std::println("module not found");

		return;
	}

	const std::string output_directory = get_command_option<std::string>(dkm, "output_directory");

	const std::uint8_t status = sys::kernel::dump_module_to_disk(module_name, output_directory);

	if (status == 1)
	{
		std::println("success in dumping module");
	}
	else
	{
		std::println("failed to dump module");
	}
}

CLI::App* init_gva(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* gva = app.add_subcommand("gva", "get the numerical value of an alias")->ignore_case();

	add_transformed_command_option(gva, "alias_name", aliases_transformer)->required();

	return gva;
}

CLI::App* init_gpb(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* gpb = app.add_subcommand("gpb", "get process base address by PID")->ignore_case();

	add_command_option(gpb, "pid")->required();
	// No longer require kernel_cr3 as an argument, we will use the one found at startup
	// add_transformed_command_option(gpb, "kernel_cr3", aliases_transformer);

	return gpb;
}

void process_gva(CLI::App* gva)
{
	const std::uint64_t alias_value = get_command_option<std::uint64_t>(gva, "alias_name");

	std::println("alias value: 0x{:X}", alias_value);
}



void process_gpb(CLI::App* gpb)
{
	uint64_t target_pid = get_command_option<uint64_t>(gpb, "pid");

	// Check if we found PsInitialSystemProcess during initialization.
	if (sys::ps_initial_system_process == 0) {
		std::println("Error: PsInitialSystemProcess not found. Cannot proceed.");
		return;
	}

	// Use the new, reliable hypercall with PsInitialSystemProcess.
	uint64_t process_base = hypercall::get_process_base(target_pid, sys::ps_initial_system_process, sys::current_cr3);

	if (process_base == 0) {
		std::println("process with PID {} not found or failed to get process base", target_pid);
	}
	else {
		std::println("process base address for PID {}: 0x{:X}", target_pid, process_base);
	}
}

	std::unordered_map<std::string, std::uint64_t> form_aliases()
	{
		std::unordered_map<std::string, std::uint64_t> aliases = { 
			{ "current_cr3", sys::current_cr3 },
			{ "mm_physical_memory_block", sys::mm_physical_memory_block }
		};

		for (auto& [module_name, module_info] : sys::kernel::modules_list)
		{
			aliases.insert({ module_name, module_info.base_address });
			aliases.insert(module_info.exports.begin(), module_info.exports.end());
		}

		return aliases;

	
	}
	


CLI::App* init_chkmap(CLI::App& app, CLI::Transformer& aliases_transformer)
{
	CLI::App* chkmap = app.add_subcommand("chkmap", "check if hyperv attachment memory is mapped by OS")->ignore_case();

	// Make parameters optional - will use discovered values as defaults
	add_transformed_command_option(chkmap, "kernel_cr3", aliases_transformer);
	add_transformed_command_option(chkmap, "memory_map_address", aliases_transformer);

	return chkmap;
}

CLI::App* init_vuworld(CLI::App& app)
{
	CLI::App* vuworld = app.add_subcommand("vuworld", "find VALORANT process and scan for UWorld pattern")->ignore_case();

	return vuworld;
}

CLI::App* init_loaddll(CLI::App& app)
{
	CLI::App* loaddll = app.add_subcommand("loaddll", "load a DLL into a target process using regular hypervisor memory allocation")->ignore_case();

	add_command_option(loaddll, "pid")->required();
	add_command_option(loaddll, "dll_path")->required();

	return loaddll;
}

CLI::App* init_stealthdll(CLI::App& app)
{
	CLI::App* stealthdll = app.add_subcommand("stealthdll", "load a DLL into a target process using hidden memory (stealth injection)")->ignore_case();

	add_command_option(stealthdll, "pid")->required();
	add_command_option(stealthdll, "dll_path")->required();

	return stealthdll;
}

CLI::App* init_rs(CLI::App& app)
{
	CLI::App* rs = app.add_subcommand("rs", "test read speed of uint64_t values from usermode process memory")->ignore_case();

	add_command_option(rs, "--iterations")->default_val("10000000");
	add_command_option(rs, "--duration")->default_val("10");
	add_command_option(rs, "--batch_size")->default_val("1024");
	add_command_option(rs, "--threads")->default_val("4");
	add_command_flag(rs, "--optimized");

	return rs;
}

CLI::App* init_notepad_cr3(CLI::App& app)
{
	CLI::App* notepad_cr3 = app.add_subcommand("notepad_cr3", "get CR3 (directory base) of notepad.exe process")->ignore_case();

	return notepad_cr3;
}

CLI::App* init_hide_hv_memory(CLI::App& app)
{
	CLI::App* hide_hv_memory = app.add_subcommand("hide_hv_memory", "hide hypervisor memory from MmPfnDatabase")->ignore_case();
	
	add_command_option(hide_hv_memory, "physical_base")->required();
	add_command_option(hide_hv_memory, "size")->required();

	return hide_hv_memory;
}

CLI::App* init_test_ntoskrnl(CLI::App& app)
{
	CLI::App* test_ntoskrnl = app.add_subcommand("test_ntoskrnl", "test ntoskrnl base address discovery only")->ignore_case();

	return test_ntoskrnl;
}

void process_test_ntoskrnl(CLI::App* test_ntoskrnl)
{
	std::println("=== Testing & Fixing ntoskrnl Base Address ===\n");

	// Add error handling for hypervisor calls
	std::uint64_t hypervisor_base = 0;
	try {
		hypervisor_base = hypercall::get_ntoskrnl_base_address();
	} catch (...) {
		std::println("[ERROR] Exception occurred while calling get_ntoskrnl_base_address()");
		hypervisor_base = 0;
	}

	void* usermode_base_ptr = nullptr;
	std::uint64_t usermode_base = 0;
	try {
		usermode_base_ptr = sys::get_ntoskrnl_base();
		usermode_base = reinterpret_cast<std::uint64_t>(usermode_base_ptr);
	} catch (...) {
		std::println("[ERROR] Exception occurred while calling sys::get_ntoskrnl_base()");
		usermode_base = 0;
	}

	std::println("[INFO] Current hypervisor ntoskrnl base: 0x{:X}", hypervisor_base);
	std::println("[INFO] Usermode discovered ntoskrnl base: 0x{:X}", usermode_base);

	// Validate addresses are reasonable
	if (hypervisor_base != 0 && hypervisor_base < 0xFFFF000000000000ULL) {
		std::println("[WARNING] Hypervisor base address seems invalid (not in kernel space)");
		hypervisor_base = 0;
	}
	if (usermode_base != 0 && usermode_base < 0xFFFF000000000000ULL) {
		std::println("[WARNING] Usermode base address seems invalid (not in kernel space)");
		usermode_base = 0;
	}

	if (hypervisor_base != usermode_base && usermode_base != 0) {
		std::println("[FIXING] Updating hypervisor with correct ntoskrnl base...");
		
		std::uint64_t update_result = 0;
		try {
			update_result = hypercall::update_ntoskrnl_base_address(usermode_base);
		} catch (...) {
			std::println("[ERROR] Exception occurred while calling update_ntoskrnl_base_address()");
			update_result = 0xFFFFFFFF; // Invalid status
		}
		
		if (update_result == 0) { // STATUS_SUCCESS
			std::println("[SUCCESS] Hypervisor ntoskrnl base updated successfully!");
			
			// Verify the update
			std::uint64_t new_hypervisor_base = 0;
			try {
				new_hypervisor_base = hypercall::get_ntoskrnl_base_address();
			} catch (...) {
				std::println("[ERROR] Exception occurred while verifying update");
				new_hypervisor_base = 0;
			}
			
			if (new_hypervisor_base == usermode_base) {
				std::println("[VERIFIED] Hypervisor now reports correct base: 0x{:X}", new_hypervisor_base);
			} else {
				std::println("[WARNING] Update verification failed - hypervisor still reports: 0x{:X}", new_hypervisor_base);
			}
		} else {
			std::println("[FAILED] Failed to update hypervisor ntoskrnl base, error: 0x{:X}", update_result);
		}
	} else if (hypervisor_base == usermode_base && hypervisor_base != 0) {
		std::println("[PERFECT] Hypervisor and usermode bases already match!");
	} else if (usermode_base == 0) {
		std::println("[ERROR] Could not discover ntoskrnl base from usermode");
	} else if (hypervisor_base == 0) {
		std::println("[ERROR] Could not get ntoskrnl base from hypervisor");
	} else {
		std::println("[ERROR] Unexpected condition");
	}
	
	std::println("\n=== Testing KPCR Method ===");
	std::println("Testing hypervisor's KPCR-based ntoskrnl discovery...");
	
	std::uint64_t kpcr_base = 0;
	try {
		kpcr_base = hypercall::get_ntoskrnl_base_from_kpcr();
	} catch (...) {
		std::println("[ERROR] Exception occurred while calling get_ntoskrnl_base_from_kpcr()");
		kpcr_base = 0;
	}
	
	if (kpcr_base != 0) {
		// Validate KPCR result
		if (kpcr_base < 0xFFFF000000000000ULL) {
			std::println("[WARNING] KPCR method returned invalid address (not in kernel space): 0x{:X}", kpcr_base);
			kpcr_base = 0;
		} else {
			std::println("[SUCCESS] KPCR method found ntoskrnl base: 0x{:X}", kpcr_base);
			
			if (kpcr_base == usermode_base && usermode_base != 0) {
				std::println("[PERFECT] KPCR method matches usermode discovery!");
			} else if (usermode_base != 0) {
				std::println("[INFO] KPCR base differs from usermode: 0x{:X} vs 0x{:X}", kpcr_base, usermode_base);
			}
		}
	} else {
		std::println("[FAILED] KPCR method could not find ntoskrnl base");
	}
	
	std::println("\n=== Setting Kernel CR3 ===");
	if (sys::current_cr3 != 0) {
		std::println("Setting hypervisor kernel CR3 to System process CR3: 0x{:X}", sys::current_cr3);
		
		std::uint64_t result = hypercall::set_kernel_cr3(sys::current_cr3);
		if (result == 0) {
			std::println("[SUCCESS] Hypervisor kernel CR3 updated successfully!");
		} else {
			std::println("[FAILED] Failed to update hypervisor kernel CR3, error: 0x{:X}", result);
		}
	} else {
		std::println("[ERROR] No System process CR3 available");
	}
	
	/*std::println("\n=== Step 1-2: Testing KPCR → EPROCESS Navigation ===");
	std::println("Testing KPCR access and navigation to current EPROCESS...");
	
	std::uint64_t eprocess_result = hypercall::get_system_process_cr3_from_kpcr();
	if (eprocess_result != 0) {
		std::println("[SUCCESS] Steps 1-2: KPCR → EPROCESS navigation successful");
		std::println("Current EPROCESS: 0x{:X}", eprocess_result);
	} else {
		std::println("[FAILED] Steps 1-2: KPCR → EPROCESS navigation failed");
	}
	
	std::println("\n[NOTE] Steps 1-2 complete. Next: Find System process and extract CR3");
	std::println("Use 'pfn_query_demo' to test if MmPfn queries work after fixes.");*/
}

CLI::App* init_pfn_query_demo(CLI::App& app)
{
	CLI::App* pfn_query_demo = app.add_subcommand("pfn_query_demo", "demonstrate querying hypervisor memory pages in MmPfnDatabase")->ignore_case();

	return pfn_query_demo;
}

void process_chkmap(CLI::App* chkmap)
{
	// Use discovered values as defaults if parameters not provided
	std::uint64_t kernel_cr3 = get_command_option<std::uint64_t>(chkmap, "kernel_cr3");
	std::uint64_t memory_map_address = get_command_option<std::uint64_t>(chkmap, "memory_map_address");

	// If kernel_cr3 not provided, use the discovered current_cr3
	if (kernel_cr3 == 0) {
		kernel_cr3 = sys::current_cr3;
		std::println("Using discovered kernel CR3: 0x{:X}", kernel_cr3);
	}

	// If memory_map_address not provided, use the discovered mm_physical_memory_block
	if (memory_map_address == 0) {
		memory_map_address = sys::mm_physical_memory_block;
		std::println("Using discovered MmPhysicalMemoryBlock: 0x{:X}", memory_map_address);
	}

	// Validate that we have the required values
	if (kernel_cr3 == 0) {
		std::println("Error: No kernel CR3 available. Please provide kernel_cr3 parameter or ensure system initialization found current_cr3.");
		return;
	}

	if (memory_map_address == 0) {
		std::println("Error: No memory map address available. Please provide memory_map_address parameter or ensure system initialization found mm_physical_memory_block.");
		return;
	}

	memory_mapping_check_result_t result = {};
	std::uint64_t status = hypercall::check_hyperv_attachment_memory_mapping(kernel_cr3, memory_map_address, result);

	if (status == 1) {
		std::println("Check Completed: Mapped by OS = {}", result.is_mapped_by_os);
		std::println("Physical Address Checked: 0x{:X}", result.physical_address_checked);
		std::println("Page Count Checked: {}", result.page_count_checked);
		std::println("Verification Status: 0x{:X}", result.verification_status);
	} else {
		std::println("Check Failed");
	}
}

void process_vuworld(CLI::App* vuworld) {
    std::string process_name = "VALORANT-Win64-Shipping.exe";
    uint32_t pid = sys::get_pid_from_process_name(process_name);

    if (pid == 0) {
        std::println("Process not found: {}", process_name);
        return;
    }

    std::println("Found {} with PID: {}", process_name, pid);

    // Get the target process CR3 using the new hypercall
    std::uint64_t process_cr3 = hypercall::get_process_cr3(pid, sys::ps_initial_system_process, sys::current_cr3);
    if (process_cr3 == 0) {
        std::println("Failed to get process CR3 for PID: {}", pid);
        return;
    }
    
    std::println("Process CR3: 0x{:X}", process_cr3);

    uint64_t base_address = sys::get_process_base_address(pid);
    if (base_address == 0) {
        std::println("Failed to get process base address for PID: {}", pid);
        return;
    }

    // Get ntoskrnl.exe base address
    void* ntoskrnl_base = sys::get_ntoskrnl_base();
    if (!ntoskrnl_base) {
        std::println("Failed to get ntoskrnl.exe base address");
        return;
    }

    // Test the new dirbase_from_base_address function
    std::uint64_t dirbase = hypercall::dirbase_from_base_address(reinterpret_cast<void*>(base_address), ntoskrnl_base);
    if (dirbase != 0) {
        std::println("Directory base from base address: 0x{:X}", dirbase);
        std::println("Comparison - Process CR3: 0x{:X}, Dirbase: 0x{:X}", process_cr3, dirbase);
    } else {
        std::println("Failed to get directory base from base address");
    }

    // Use the new get_image_size function with process CR3
    uint32_t image_size = sys::get_image_size(base_address, process_cr3);
    if (image_size == 0) {
        std::println("Failed to get image size for base address: 0x{:X}", base_address);
        return;
    }

    std::println("Base address: 0x{:X}, Image size: 0x{:X}", base_address, image_size);

    // Pattern to be scanned
    std::uint8_t uworld_pattern[] = {
        0x4C, 0x8D, 0x3D, 0xCC, 0xCC, 0xCC, 0xCC, 0x0F,
        0x10, 0x45, 0xCC, 0x83, 0xE1, 0xCC, 0x4C, 0x89,
        0x7D, 0xCC, 0x0F, 0x11, 0x55, 0xCC, 0x41
    };

    std::println("Scanning {} bytes for UWorld pattern...", image_size);
    
    // Use the new scan_memory function with process CR3
    std::uint64_t pattern_address = sys::scan_memory(base_address, image_size, uworld_pattern, sizeof(uworld_pattern), process_cr3);

    if (pattern_address != 0) {
        std::println("UWorld pattern found at: 0x{:X}", pattern_address);
        
        // Calculate the relative offset from base address
        std::uint64_t offset = pattern_address - base_address;
        std::println("Pattern offset from base: +0x{:X}", offset);
        
        // Read the value at the pattern address
        std::uint64_t pattern_value = 0;
        std::uint64_t bytes_read = hypercall::read_guest_virtual_memory(
            &pattern_value, pattern_address + 0x0038, process_cr3, sizeof(pattern_value));
            
        if (bytes_read == sizeof(pattern_value)) {
            std::println("Value at pattern address: 0x{:X}", pattern_value);
        } else {
            std::println("Failed to read value from pattern address");
        }
    } else {
        std::println("UWorld pattern not found in VALORANT process memory.");
    }
}



void process_stealthdll(CLI::App* stealthdll)
{
    const std::uint32_t target_pid = get_command_option<std::uint32_t>(stealthdll, "pid");
    const std::string dll_path = get_command_option<std::string>(stealthdll, "dll_path");
    
    // Check if we have the required system information
    if (sys::ps_initial_system_process == 0) {
        std::println("Error: PsInitialSystemProcess not found. Cannot proceed.");
        return;
    }
    
    if (sys::current_cr3 == 0) {
        std::println("Error: Kernel CR3 not found. Cannot proceed.");
        return;
    }
    
    std::println("[STEALTH] Loading DLL: {} into PID: {} using hidden memory", dll_path, target_pid);
    
    // Load the DLL into the target process using stealth injection
    std::uint64_t base_address = dll_loader::load_dll_stealthily(
        target_pid,
        dll_path,
        sys::ps_initial_system_process,
        sys::current_cr3
    );
    
    if (base_address != 0) {
        std::println("[STEALTH] DLL loaded successfully in hidden memory!");
        std::println("[STEALTH] Base address: 0x{:X}", base_address);
        std::println("[STEALTH] DLL is now invisible to normal process enumeration.");
    } else {
        std::println("[STEALTH] Failed to load DLL into target process.");
    }
}

void process_rs(CLI::App* rs)
{
    const std::uint64_t max_iterations = get_command_option<std::uint64_t>(rs, "--iterations");
    const std::uint64_t max_duration_seconds = get_command_option<std::uint64_t>(rs, "--duration");
    
    // Get current process PID
    const std::uint32_t current_pid = GetCurrentProcessId();
    
    // Check if we have the required system information
    if (sys::ps_initial_system_process == 0) {
        std::println("Error: PsInitialSystemProcess not found. Cannot proceed.");
        return;
    }
    
    if (sys::current_cr3 == 0) {
        std::println("Error: Kernel CR3 not found. Cannot proceed.");
        return;
    }
    
    // Get current process CR3
    std::uint64_t process_cr3 = hypercall::get_process_cr3(current_pid, sys::ps_initial_system_process, sys::current_cr3);
    if (process_cr3 == 0) {
        std::println("Failed to get process CR3 for current PID: {}", current_pid);
        return;
    }
    
    // Create a test variable in our process memory
    static std::uint64_t test_variable = 0xDEADBEEFCAFEBABE;
    std::uint64_t test_address = reinterpret_cast<std::uint64_t>(&test_variable);
    
    std::println("=== Read Speed Test ===");
    std::println("Testing address: 0x{:X}", test_address);
    std::println("Process PID: {}", current_pid);
    std::println("Process CR3: 0x{:X}", process_cr3);
    std::println("Max iterations: {}", max_iterations);
    std::println("Max duration: {} seconds", max_duration_seconds);
    std::println();
    
    // Variables for statistics
    std::vector<double> read_times;
    read_times.reserve(max_iterations);
    
    std::uint64_t successful_reads = 0;
    std::uint64_t failed_reads = 0;
    
    const auto start_time = std::chrono::high_resolution_clock::now();
    const auto max_duration = std::chrono::seconds(max_duration_seconds);
    
    // Perform read speed test
    for (std::uint64_t i = 0; i < max_iterations; i++) {
        const auto current_time = std::chrono::high_resolution_clock::now();
        if (current_time - start_time >= max_duration) {
            std::println("Stopping test due to time limit ({} seconds)", max_duration_seconds);
            break;
        }
        
        std::uint64_t read_value = 0;
        
        const auto read_start = std::chrono::high_resolution_clock::now();
        const std::uint64_t bytes_read = hypercall::read_guest_virtual_memory(
            &read_value, test_address, process_cr3, sizeof(std::uint64_t));
        const auto read_end = std::chrono::high_resolution_clock::now();
        
        const auto read_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(read_end - read_start);
        const double read_time_ns = static_cast<double>(read_duration.count());
        
        if (bytes_read == sizeof(std::uint64_t)) {
            successful_reads++;
            read_times.push_back(read_time_ns);
            
            // Verify the read value
            if (read_value != test_variable) {
                std::println("Warning: Read value mismatch at iteration {}. Expected: 0x{:X}, Got: 0x{:X}", 
                           i + 1, test_variable, read_value);
            }
        } else {
            failed_reads++;
        }
        
        // Print progress every 1000 iterations
        if ((i + 1) % 1000 == 0) {
            std::println("Progress: {} / {} iterations ({:.1f}%)", 
                       i + 1, max_iterations, 
                       static_cast<double>(i + 1) / max_iterations * 100.0);
        }
    }
    
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Calculate statistics
    if (successful_reads > 0) {
        // Sort read times for percentile calculations
        std::sort(read_times.begin(), read_times.end());
        
        const double total_reads = static_cast<double>(successful_reads);
        const double total_time_seconds = static_cast<double>(total_duration.count()) / 1000.0;
        
        // Calculate average, min, max
        const double avg_time_ns = std::accumulate(read_times.begin(), read_times.end(), 0.0) / total_reads;
        const double min_time_ns = read_times.front();
        const double max_time_ns = read_times.back();
        
        // Calculate percentiles
        const double p50_time_ns = read_times[read_times.size() / 2];
        const double p95_time_ns = read_times[static_cast<size_t>(read_times.size() * 0.95)];
        const double p99_time_ns = read_times[static_cast<size_t>(read_times.size() * 0.99)];
        
        // Calculate reads per second
        const double reads_per_second = total_reads / total_time_seconds;
        
        std::println();
        std::println("=== Results ===");
        std::println("Total duration: {:.2f} seconds", total_time_seconds);
        std::println("Successful reads: {}", successful_reads);
        std::println("Failed reads: {}", failed_reads);
        std::println("Success rate: {:.2f}%", (total_reads / (total_reads + failed_reads)) * 100.0);
        std::println();
        std::println("=== Performance Metrics ===");
        std::println("Reads per second: {:.2f}", reads_per_second);
        std::println("Average read time: {:.2f} ns ({:.2f} μs)", avg_time_ns, avg_time_ns / 1000.0);
        std::println("Minimum read time: {:.2f} ns ({:.2f} μs)", min_time_ns, min_time_ns / 1000.0);
        std::println("Maximum read time: {:.2f} ns ({:.2f} μs)", max_time_ns, max_time_ns / 1000.0);
        std::println();
        std::println("=== Percentiles ===");
        std::println("50th percentile (median): {:.2f} ns ({:.2f} μs)", p50_time_ns, p50_time_ns / 1000.0);
        std::println("95th percentile: {:.2f} ns ({:.2f} μs)", p95_time_ns, p95_time_ns / 1000.0);
        std::println("99th percentile: {:.2f} ns ({:.2f} μs)", p99_time_ns, p99_time_ns / 1000.0);
    } else {
        std::println();
        std::println("=== Results ===");
        std::println("No successful reads completed!");
        std::println("Failed reads: {}", failed_reads);
    }
}

void process_notepad_cr3(CLI::App* notepad_cr3)
{
    std::string process_name = "notepad.exe";
    
    // Get notepad.exe PID
    uint32_t pid = sys::get_pid_from_process_name(process_name);
    if (pid == 0) {
        std::println("Process not found: {}", process_name);
        std::println("Make sure notepad.exe is running before using this command.");
        return;
    }
    
    std::println("Found {} with PID: {}", process_name, pid);
    
    // Get the process base address
    uint64_t base_address = sys::get_process_base_address(pid);
    if (base_address == 0) {
        std::println("Failed to get process base address for PID: {}", pid);
        return;
    }
    
    std::println("Process base address: 0x{:X}", base_address);
    
    // Get ntoskrnl.exe base address
    void* ntoskrnl_base = sys::get_ntoskrnl_base();
    if (!ntoskrnl_base) {
        std::println("Failed to get ntoskrnl.exe base address");
        return;
    }
    
    std::println("ntoskrnl.exe base address: 0x{:X}", reinterpret_cast<uintptr_t>(ntoskrnl_base));
    
    // Test the dirbase_from_base_address function
    std::uint64_t dirbase = hypercall::dirbase_from_base_address(reinterpret_cast<void*>(base_address), ntoskrnl_base);
    
    if (dirbase != 0) {
        std::println("Successfully found CR3 (directory base) for {}: 0x{:X}", process_name, dirbase);
        
        // Verify the CR3 by trying to read from the process base address using this CR3
        std::uint64_t test_value = 0;
        std::uint64_t bytes_read = hypercall::read_guest_virtual_memory(
            &test_value, base_address, dirbase, sizeof(test_value));
            
        if (bytes_read == sizeof(test_value)) {
            std::println("CR3 verification successful - read {} bytes from process memory", bytes_read);
            std::println("Value at base address: 0x{:X}", test_value);
        } else {
            std::println("CR3 verification failed - could not read from process memory using found CR3");
        }
    } else {
        std::println("Failed to find CR3 for {}", process_name);
        std::println("This could indicate:");
        std::println("  - The process is not currently mapped in physical memory");
        std::println("  - The dirbase_from_base_address function needs adjustment");
        std::println("  - The ntoskrnl.exe base address is incorrect");
    }
}

void process_hide_hv_memory(CLI::App* hide_hv_memory)
{
    std::uint64_t physical_base = get_command_option<std::uint64_t>(hide_hv_memory, "physical_base");
    std::uint64_t size = get_command_option<std::uint64_t>(hide_hv_memory, "size");
    
    std::println("Attempting to hide hypervisor memory:");
    std::println("  Physical base: 0x{:X}", physical_base);
    std::println("  Size: 0x{:X} bytes ({} pages)", size, (size + 0xFFF) / 0x1000);
    
    // Call the hypercall to hide the memory
    std::uint64_t result = hypercall::hide_hypervisor_memory(physical_base, size);
    
    if (result == 0) { // STATUS_SUCCESS
        std::println("Successfully hidden hypervisor memory from MmPfnDatabase");
        std::println("The specified physical memory range should now be invisible to:");
        std::println("  - Memory scanning tools");
        std::println("  - Kernel memory managers");
        std::println("  - Anti-cheat systems that scan physical memory");
    } else {
        std::println("Failed to hide hypervisor memory. Error code: 0x{:X}", result);
        std::println("Possible reasons:");
        std::println("  - MmPfnDatabase not initialized");
        std::println("  - Invalid physical address range");
        std::println("  - Memory access denied");
        std::println("  - ntoskrnl.exe exports not found");
    }
}

// Helper function to print page location as string
const char* get_page_location_string(std::uint8_t page_location) {
    switch (page_location) {
        case 0: return "ZeroedPageList";
        case 1: return "FreePageList";
        case 2: return "StandbyPageList";
        case 3: return "ModifiedPageList";
        case 4: return "ModifiedNoWritePageList";
        case 5: return "BadPageList";
        case 6: return "ActiveAndValid";
        case 7: return "TransitionPage";
        default: return "Unknown";
    }
}

// Helper function to print cache attribute as string
const char* get_cache_attribute_string(std::uint8_t cache_attr) {
    switch (cache_attr) {
        case 0: return "MiNonCached";
        case 1: return "MiCached";
        case 2: return "MiWriteCombined";
        case 3: return "MiHardwareCoherentCached";
        default: return "Unknown";
    }
}

void process_pfn_query_demo(CLI::App* pfn_query_demo)
{
    std::println("\n=== Hypervisor PFN Query Demo ===\n");
    
    // First, get the ntoskrnl base address to show it's working
    std::uint64_t ntoskrnl_base = hypercall::get_ntoskrnl_base_address();
    std::println("ntoskrnl.exe base address: 0x{:X}", ntoskrnl_base);
    
    if (!ntoskrnl_base) {
        std::println("Failed to get ntoskrnl base address!");
        return;
    }
    
    // Get the actual hypervisor memory layout
    hypervisor_memory_info_t memory_info = {};
    std::uint64_t result = hypercall::get_hypervisor_memory_info(&memory_info);
    
    if (result != 0) {
        std::println("Failed to get hypervisor memory info! Status: 0x{:X}", result);
        return;
    }
    
    if (!memory_info.is_valid) {
        std::println("Hypervisor memory info is not valid!");
        return;
    }
    
    std::println("\nHypervisor Memory Layout:");
    std::println("  Main Attachment: 0x{:X} - 0x{:X} ({} pages, {} KB)", 
                memory_info.physical_base, 
                memory_info.physical_base + memory_info.size_bytes,
                memory_info.page_count, 
                memory_info.size_bytes / 1024);
    
    if (memory_info.heap_physical_base != 0) {
        std::println("  Heap Memory: 0x{:X} - 0x{:X} ({} pages, {} KB)", 
                    memory_info.heap_physical_base, 
                    memory_info.heap_physical_base + memory_info.heap_size_bytes,
                    memory_info.heap_page_count, 
                    memory_info.heap_size_bytes / 1024);
    }
    
    if (memory_info.uefi_boot_base != 0) {
        std::println("  UEFI Boot: 0x{:X} - 0x{:X} ({} pages, {} KB)", 
                    memory_info.uefi_boot_base, 
                    memory_info.uefi_boot_base + memory_info.uefi_boot_size,
                    memory_info.uefi_boot_size / 4096, 
                    memory_info.uefi_boot_size / 1024);
    }
    
    // Create a list of actual hypervisor memory addresses to query
    std::vector<std::uint64_t> hypervisor_addresses;
    
    // Add some pages from the main hypervisor attachment
    if (memory_info.physical_base != 0 && memory_info.page_count > 0) {
        hypervisor_addresses.push_back(memory_info.physical_base); // First page
        
        if (memory_info.page_count > 1) {
            hypervisor_addresses.push_back(memory_info.physical_base + 0x1000); // Second page
        }
        
        if (memory_info.page_count > 10) {
            hypervisor_addresses.push_back(memory_info.physical_base + (10 * 0x1000)); // 10th page
        }
        
        if (memory_info.page_count > 1) {
            // Last page
            hypervisor_addresses.push_back(memory_info.physical_base + ((memory_info.page_count - 1) * 0x1000));
        }
    }
    
    // Add some pages from the heap if available
    if (memory_info.heap_physical_base != 0 && memory_info.heap_page_count > 0) {
        hypervisor_addresses.push_back(memory_info.heap_physical_base); // First heap page
        
        if (memory_info.heap_page_count > 1) {
            hypervisor_addresses.push_back(memory_info.heap_physical_base + 0x1000); // Second heap page
        }
    }
    
    std::println("\nQuerying {} actual hypervisor memory pages...\n", hypervisor_addresses.size());
    
    for (auto physical_addr : hypervisor_addresses) {
        hypervisor_pfn_info_t pfn_info = {};
        
        // Determine which memory region this address belongs to
        std::string region_type = "Unknown";
        if (physical_addr >= memory_info.physical_base && 
            physical_addr < (memory_info.physical_base + memory_info.size_bytes)) {
            region_type = "Hypervisor Attachment";
        } else if (memory_info.heap_physical_base != 0 && 
                   physical_addr >= memory_info.heap_physical_base && 
                   physical_addr < (memory_info.heap_physical_base + memory_info.heap_size_bytes)) {
            region_type = "Hypervisor Heap";
        } else if (memory_info.uefi_boot_base != 0 && 
                   physical_addr >= memory_info.uefi_boot_base && 
                   physical_addr < (memory_info.uefi_boot_base + memory_info.uefi_boot_size)) {
            region_type = "UEFI Boot";
        }
        
        std::println("\n--- Querying Physical Address: 0x{:X} ({}) ---", physical_addr, region_type);
        
        std::uint64_t result = hypercall::query_hypervisor_pfn_info(physical_addr, &pfn_info);
        
        if (result == 0) { // STATUS_SUCCESS
            std::println("Query Status: SUCCESS");
            std::println("PFN Index: 0x{:X}", pfn_info.pfn_index);
            std::println("Is Hypervisor Page: {}", pfn_info.is_hypervisor_page ? "YES" : "NO");
            std::println("Is Valid: {}", pfn_info.is_valid ? "YES" : "NO");
            
            std::println("\nPFN Details:");
            std::println("  Reference Count: {}", pfn_info.reference_count);
            std::println("  Share Count: {}", pfn_info.share_count);
            std::println("  Page Location: {} ({})", 
                        get_page_location_string(pfn_info.e1_flags.page_location),
                        static_cast<int>(pfn_info.e1_flags.page_location));
            std::println("  Cache Attribute: {} ({})", 
                        get_cache_attribute_string(pfn_info.e1_flags.cache_attribute),
                        static_cast<int>(pfn_info.e1_flags.cache_attribute));
            
            std::println("\nFlags:");
            std::println("  Modified: {}", pfn_info.e1_flags.modified ? "YES" : "NO");
            std::println("  Write In Progress: {}", pfn_info.e1_flags.write_in_progress ? "YES" : "NO");
            std::println("  Read In Progress: {}", pfn_info.e1_flags.read_in_progress ? "YES" : "NO");
            std::println("  Priority: {}", static_cast<int>(pfn_info.e3_flags.priority));
            std::println("  On Prototype PTE: {}", pfn_info.e3_flags.on_prototype_pte ? "YES" : "NO");
            std::println("  Removal Requested: {}", pfn_info.e3_flags.removal_requested ? "YES" : "NO");
            std::println("  Parity Error: {}", pfn_info.e3_flags.parity_error ? "YES" : "NO");
            
            std::println("\nPTE Information:");
            std::println("  PTE Address: 0x{:X}", pfn_info.pte_address);
            std::println("  PTE Frame: 0x{:X}", pfn_info.pte_frame);
            
            std::println("\nList Entry:");
            std::println("  Flink: 0x{:X}", pfn_info.list_entry.flink);
            std::println("  Blink: 0x{:X}", pfn_info.list_entry.blink);
            
            std::println("\nOther:");
            std::println("  Page Color: {}", pfn_info.page_color);
            std::println("  Node Blink: {}", pfn_info.node_blink);
            
            // Print first few bytes of raw PFN data for debugging
            std::print("\nRaw PFN Data (first 16 bytes): ");
            for (int i = 0; i < 16; i++) {
                std::print("{:02X} ", static_cast<int>(pfn_info.raw_pfn_data[i]));
            }
            std::println("");
            
        } else {
            std::println("Query failed with status: 0x{:X}", result);
        }
    }
    
    std::println("\n=== Analysis Summary ===");
    std::println("This shows you exactly how Windows sees your hypervisor memory pages!");
    std::println("Key things to look for:");
    std::println("  - Reference Count: Should be > 0 for active pages");
    std::println("  - Page Location: Shows which memory list the page is on");
    std::println("  - Modified Bit: Indicates if the page has been written to");
    std::println("  - Cache Attributes: How the page is cached by the CPU");
    std::println("  - Is Hypervisor Page: Whether our detection logic identifies it");
    std::println("\n=== Demo Complete ===");
}


void commands::process(const std::string command)
{
	if (command.empty() == true)
	{
}
	

	CLI::App app;
	app.require_subcommand();

	sys::kernel::parse_modules();

	const std::unordered_map<std::string, std::uint64_t> aliases = form_aliases();

	CLI::Transformer aliases_transformer = CLI::Transformer(aliases, CLI::ignore_case);

	aliases_transformer.description(" can_use_aliases");

	CLI::App* rgpm = init_rgpm(app, aliases_transformer);
	CLI::App* wgpm = init_wgpm(app, aliases_transformer);
	CLI::App* cgpm = init_cgpm(app, aliases_transformer);
	CLI::App* gvat = init_gvat(app, aliases_transformer);
	CLI::App* rgvm = init_rgvm(app, aliases_transformer);
	CLI::App* wgvm = init_wgvm(app, aliases_transformer);
	CLI::App* cgvm = init_cgvm(app, aliases_transformer);
	CLI::App* akh = init_akh(app, aliases_transformer);
	CLI::App* rkh = init_rkh(app, aliases_transformer);
	CLI::App* gva = init_gva(app, aliases_transformer);
	CLI::App* gpb = init_gpb(app, aliases_transformer);
	CLI::App* hgpp = init_hgpp(app, aliases_transformer);
	CLI::App* fl = init_fl(app);
	CLI::App* hfpc = init_hfpc(app);
	CLI::App* lkm = init_lkm(app);
	CLI::App* kme = init_kme(app);
	CLI::App* dkm = init_dkm(app);
	CLI::App* chkmap = init_chkmap(app, aliases_transformer);
	CLI::App* vuworld = init_vuworld(app);
	CLI::App* loaddll = init_loaddll(app);
	CLI::App* stealthdll = init_stealthdll(app);
	CLI::App* rs = init_rs(app);
	CLI::App* notepad_cr3 = init_notepad_cr3(app);
	CLI::App* hide_hv_memory = init_hide_hv_memory(app);
	CLI::App* test_ntoskrnl = init_test_ntoskrnl(app);
	CLI::App* pfn_query_demo = init_pfn_query_demo(app);

	try
	{
		app.parse(command);

		d_initial_process_command(rgpm);
		d_process_command(wgpm);
		d_process_command(cgpm);
		d_process_command(gvat);
		d_process_command(rgvm);
		d_process_command(wgvm);
		d_process_command(cgvm);
		d_process_command(akh);
		d_process_command(rkh);
		d_process_command(gva);
		d_process_command(gpb);
		d_process_command(hgpp);
		d_process_command(fl);
		d_process_command(hfpc);
		d_process_command(lkm);
		d_process_command(kme);
		d_process_command(dkm);
		d_process_command(chkmap);
		d_process_command(vuworld);
		d_process_command(stealthdll);
		d_process_command(rs);
		d_process_command(notepad_cr3);
		d_process_command(hide_hv_memory);
		d_process_command(test_ntoskrnl);
		d_process_command(pfn_query_demo);
	}
	catch (const CLI::ParseError& error)
	{
		app.exit(error);
	}
}

