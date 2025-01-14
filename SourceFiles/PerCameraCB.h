#pragma once
#include <DirectXMath.h>

// Per-camera constant buffer struct
struct PerCameraCB
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT3 position;
    float pad[1];
    // Add any other per-camera data here
};
