#include <Windows.h>
#include <thread>
#include <iostream>
#include "Overlay/overlay.h"
#include "entities.hpp"
#include "renderer.hpp"
#include "globals.h"
#include "system.h"
#include "hypercall/hypercall.h"

uintptr_t clientModule = 0;

auto CaptureEntity() -> void {
    while (true) {
        g_EntitySystem.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void RenderESP() {
    g_Renderer.RenderESP();
}

int main() {
    if (!sys::set_up()) {
        printf("[-] Hypervisor system init failed\n");
        system("pause");
        return 1;
    }

    SetConsoleTitleA("exampledontberetard");

    uintptr_t pid = 0;
    printf("[+] Waiting for cs2.exe...\n");
    while (!pid)
    {
        pid = sys::get_pid_from_process_name("cs2.exe");
        Sleep(1000);
    }

    uintptr_t process_cr3 = sys::get_process_cr3(pid);
    if (!process_cr3) {
        printf("[-] Failed to get process CR3\n");
        system("pause");
        return 1;
    }

    uintptr_t base_address = sys::get_process_base_address(pid);
    if (!base_address) {
        printf("[-] base addr not found\n");
        system("pause");
        return 1;
    }

    std::cout << "[+] Found Game Base ---> 0x" << std::hex << base_address << std::dec << std::endl;

    for (int retry = 0; retry < 10; retry++) {
        clientModule = GetProcessModule(pid, L"client.dll");
        if (clientModule != 0) {
            break;
        }
        std::cout << "[DEBUG] client.dll not found, retry " << (retry + 1) << "/10" << std::endl;
        Sleep(1000); // Wait 1 second before retry
    }
    std::cout << "[+] Found Client.dll base ---> 0x" << std::hex << clientModule << std::dec << std::endl;

    g_EntitySystem.Initialize(pid);

    std::thread entityCaptureThread(CaptureEntity);
    entityCaptureThread.detach();

    create_overlay();
    render_loop();

    g_EntitySystem.Shutdown();
    sys::clean_up();
    return 0;
}
