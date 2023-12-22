//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <string>
#include <format>
#include <span>
#include <unordered_set>
#include <variant>
#include <optional>
#include <filesystem>

// Dear ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imconfig.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// Dear ImGui File Dialog
#include "ImGuiFileDialog.h"

#include "GWUnpacker.h"
#include "AtexAsm.h"
#include "AtexReader.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "bass.h"
typedef BOOL(__stdcall* LPFNBASSINIT)(int, DWORD, DWORD, HWND, const void*);
typedef HSTREAM(WINAPI* LPFNBASSSTREAMCREATEFILE)(BOOL mem, const void* file, QWORD offset, QWORD length,
                                                  DWORD flags);
typedef BOOL(WINAPI* LPFNBASSCHANNELPLAY)(DWORD handle, BOOL restart);
typedef BOOL(WINAPI* LPFNBASSCHANNELPAUSE)(DWORD handle);
typedef BOOL(WINAPI* LPFNBASSCHANNELSTOP)(DWORD handle);
typedef double(WINAPI* LPFNBASSCHANNELBYTES2SECONDS)(DWORD handle, QWORD bytes);
typedef QWORD(WINAPI* LPFNBASSCHANNELGETLENGTH)(DWORD handle, DWORD mode);
typedef DWORD(WINAPI* LPFNBASSSTREAMGETFILEPOSITION)(HSTREAM handle, DWORD mode);
typedef BOOL(WINAPI* LPFNBASSCHANNELGETINFO)(DWORD handle, BASS_CHANNELINFO* info);
typedef DWORD(WINAPI* LPFNBASSCHANNELFLAGS)(DWORD handle, DWORD flags, DWORD mask);
typedef BOOL(WINAPI* LPFNBASSSTREAMFREE)(DWORD handle);
typedef BOOL(WINAPI* LPFNBASSCHANNELSETPOSITION)(DWORD handle, QWORD pos, DWORD mode);
typedef QWORD(WINAPI* LPFNBASSCHANNELGETPOSITION)(DWORD handle, DWORD mode);
typedef QWORD(WINAPI* LPFNBASSCHANNELSECONDS2BYTES)(DWORD handle, double seconds);
typedef BOOL(WINAPI* LPFNBASSCHANNELSETATTRIBUTE)(DWORD handle, DWORD attrib, float value);

#include "bass_fx.h"
typedef DWORD(WINAPI* LPFNBASSFXTMPOCREATE)(DWORD chan, DWORD flags);

namespace DX
{
// Helper class for COM exceptions
class com_exception : public std::exception
{
public:
    com_exception(HRESULT hr) noexcept
        : result(hr)
    {
    }

    const char* what() const noexcept override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

private:
    HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw com_exception(hr);
    }
}
}

static const char* type_strings[26] = {
    " ", "AMAT", "Amp", "ATEXDXT1", "ATEXDXT2", "ATEXDXT3", "ATEXDXT4",
    "ATEXDXT5", "ATEXDXTN", "ATEXDXTA", "ATEXDXTL", "ATTXDXT1", "ATTXDXT3", "ATTXDXT5",
    "ATTXDXTN", "ATTXDXTA", "ATTXDXTL", "DDS", "FFNA - Model", "FFNA - Map", "FFNA - Unknown",
    "MFTBase", "NOT_READ", "Sound", "Text", "Unknown"
};

inline int decode_filename(int id0, int id1) { return (id0 - 0xff00ff) + (id1 * 0xff00); }

inline void encode_filehash(uint32_t filehash, int& id0_out, int& id1_out) {
    id0_out = static_cast<wchar_t>(((filehash - 1) % 0xff00) + 0x100);
    id1_out = static_cast<wchar_t>(((filehash - 1) / 0xff00) + 0x100);
}

inline std::optional<std::filesystem::path> get_executable_directory() {
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL) {
        // Get the path to the executable
        if (GetModuleFileName(hModule, path, MAX_PATH) > 0) {
            // Convert to std::filesystem::path and return the directory part
            return std::filesystem::path(path).parent_path();
        }
    }
    return std::nullopt; // Return nullopt if unable to retrieve the path
}

inline std::optional<std::filesystem::path> load_last_filepath(const std::string& filename) {
    auto exe_dir_opt = get_executable_directory();
    if (exe_dir_opt) {
        if (std::filesystem::exists(*exe_dir_opt / filename)) {
            std::ifstream infile(*exe_dir_opt / filename);
            if (infile.is_open()) {
                std::string line;
                std::getline(infile, line);
                if (std::filesystem::exists(line)) {
                    return line;
                }
            }
        }
    }

    return std::nullopt;
}

inline std::optional<std::filesystem::path> save_last_filepath(const std::filesystem::path& filepath, const std::string& filename) {
    auto exe_dir_opt = get_executable_directory();
    if (exe_dir_opt) {
        std::ofstream outfile(*exe_dir_opt / filename, std::ios::trunc);
        outfile << filepath.string();

        return filepath;
    }

    return std::nullopt;
}

enum class LODQuality : uint8_t {
    High, // Best quality
    Medium, // Medium quality
    Low, // Lowset quality
};
