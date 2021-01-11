#include <cstdio>
#include <cstdint>
#include <span>
#include <thread>
#include <windows.h>

using namespace std::literals::chrono_literals;

extern "C" __cdecl std::uint32_t SPCI_get_spot(short handle, std::uint32_t *x, std::uint32_t *y);
extern "C" __cdecl std::uint32_t SPCI_get_area(short handle, std::uint32_t *w, std::uint32_t *h);
extern "C" __cdecl std::uint32_t SPCI_get_cpar(short handle, double *amplitudes, double *lifetimes);

std::array hdl_chain = {
    0x9fcu,
};

std::array pixels_chain = {
    0x038u,
    0x038u,
    0x00cu,
    0x054u,
    0x0ccu,
    0x1f0u,
};

std::array photons_chain = {
    0x098u,
    0x004u,
    0x00cu,
    0x078u,
    0x0b8u,
    0xb48u,
};

template <typename T>
T follow_pointer_chain(uintptr_t addr, const std::span<std::uint32_t> offsets) {
    for (auto off: offsets) {
        if (IsBadReadPtr(reinterpret_cast<void *>(addr), sizeof(addr)))
            return {};
        addr = *reinterpret_cast<uintptr_t *>(addr) + off;
    }

    if (IsBadReadPtr(reinterpret_cast<void *>(addr), sizeof(addr)))
        return {};
    return *reinterpret_cast<T *>(addr);
}

DWORD WINAPI dll_thread(HMODULE hModule) {
    AllocConsole();
    FILE *console = new FILE();
    freopen_s(&console, "CONOUT$", "w", stdout);

    uintptr_t spci = reinterpret_cast<uintptr_t>(GetModuleHandle("SPCImage.exe"));
    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle("base.dll"));

    while (true) {
        auto hdl = follow_pointer_chain<std::uint16_t>(spci + 0xa9a8, hdl_chain);
        if (hdl < 0 || hdl > 2000) {
            printf("Invalid handle: %u\n", hdl);
            continue;
        }

        double lifetimes[3];
        double amplitudes[3];
        if (auto rc = SPCI_get_cpar(hdl, amplitudes, lifetimes); !rc) {
            std::printf("Lifetimes:\n  tau_1: %g, tau_2: %g, tau_3: %g\n",
                lifetimes[0], lifetimes[1], lifetimes[2]);
            std::printf("Amplitudes:\n  a_1: %g, a_2: %g, a_3: %g\n",
                amplitudes[0], amplitudes[1], amplitudes[2]);
        }

        if (auto num_phots = follow_pointer_chain<double>(base + 0x36138, photons_chain); num_phots)
            std::printf("Photons within gate: %g\n", num_phots);

        if (auto pixels = follow_pointer_chain<double>(spci + 0x104ef4, pixels_chain); pixels > 1)
            std::printf("Pixels in ROI: %u\n", static_cast<std::uint32_t>(pixels));

        std::this_thread::sleep_for(500ms);
    }

    fclose(console);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

__declspec(dllexport) BOOL APIENTRY DllMain(HMODULE hModule, DWORD callReason, LPVOID lpReserved) {
    switch (callReason) {
        case DLL_PROCESS_ATTACH: {
            CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)dll_thread, hModule, 0, 0));
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return true;
}
