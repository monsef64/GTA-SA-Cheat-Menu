// D3D9Math.h - The small amount of Direct3D maths the overlay needs.
//
// The world-to-screen projection used to come from d3dx9.h, but that header
// only ships with the old "DirectX SDK (June 2010)" and is not part of the
// Windows SDK. Implementing the projection here means the project builds
// with nothing but Visual Studio and the Windows SDK.
//
// The maths is a faithful reimplementation of D3DXVec3Project():
//
//   D3DXMatrixMultiply(&m1, pWorld, pView);
//   D3DXMatrixMultiply(&m2, &m1, pProjection);
//   D3DXVec3TransformCoord(&vec, pV, &m2);
//   pScreen->x = pViewport->X + (1.0f + vec.x) * pViewport->Width  / 2.0f;
//   pScreen->y = pViewport->Y + (1.0f - vec.y) * pViewport->Height / 2.0f;
//   pScreen->z = pViewport->MinZ + vec.z * (pViewport->MaxZ - pViewport->MinZ);
//
// Note that the three matrices are multiplied together *first* and the point
// is transformed only once. Doing the view and projection transforms
// separately gives different results, because each transform would perform
// its own perspective divide.

#pragma once

#include <cmath>

#include <d3d9.h>

namespace sa {

// A row-major 4x4 matrix, laid out exactly like D3DMATRIX (and therefore
// like D3DXMATRIX too).
//
// IDirect3DDevice9::GetTransform() hands back a D3DMATRIX, which is
// declared in d3d9.h and so needs no DirectX SDK.
struct Matrix4x4 {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

// A point with three components.
struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Multiplies two matrices: result = a * b.
//
// This is D3DXMatrixMultiply() for row-major matrices.
inline Matrix4x4 MultiplyMatrices(const Matrix4x4& a, const Matrix4x4& b) {
    Matrix4x4 result;
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            result.m[row][column] =
                a.m[row][0] * b.m[0][column] +
                a.m[row][1] * b.m[1][column] +
                a.m[row][2] * b.m[2][column] +
                a.m[row][3] * b.m[3][column];
        }
    }
    return result;
}

// Transforms a point by a matrix and performs the perspective divide, the
// same way D3DXVec3TransformCoord() does.
//
// D3D uses row vectors, so the fourth row of the matrix (_41.._44) is the
// one that produces the w component.
inline Vector3 TransformCoord(const Vector3& v, const Matrix4x4& matrix) {
    const float w = v.x * matrix._41 + v.y * matrix._42 +
                    v.z * matrix._43 + matrix._44;
    if (w == 0.0f) return {0.0f, 0.0f, 0.0f};

    const float reciprocalW = 1.0f / w;
    Vector3 result;
    result.x = (v.x * matrix._11 + v.y * matrix._21 + v.z * matrix._31 +
                matrix._41) * reciprocalW;
    result.y = (v.x * matrix._12 + v.y * matrix._22 + v.z * matrix._32 +
                matrix._42) * reciprocalW;
    result.z = (v.x * matrix._13 + v.y * matrix._23 + v.z * matrix._33 +
                matrix._43) * reciprocalW;
    return result;
}

// The identity matrix.
inline Matrix4x4 IdentityMatrix() {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) result.m[i][j] = 0.0f;
    }
    result._11 = result._22 = result._33 = result._44 = 1.0f;
    return result;
}

// Reads the current transform for a given D3D transform state.
//
// GetTransform() takes a D3DMATRIX*, which is declared in d3d9.h and so is
// always available. D3DMATRIX and Matrix4x4 have the same layout, so the
// result is read straight into our own type.
inline HRESULT ReadTransformMatrix(IDirect3DDevice9* device,
                                  D3DTRANSFORMSTATETYPE state,
                                  Matrix4x4* out) {
    if (device == nullptr || out == nullptr) return E_FAIL;

    D3DMATRIX matrix;
    const HRESULT result = device->GetTransform(state, &matrix);
    if (FAILED(result)) return result;

    // D3DMATRIX and Matrix4x4 have the same layout, so the copy is safe.
    *out = *reinterpret_cast<const Matrix4x4*>(&matrix);
    return S_OK;
}

// Projects a world-space point into screen space, the same way
// D3DXVec3Project() does.
inline Vector3 ProjectPoint(const Vector3& world, const D3DVIEWPORT9& viewport,
                            const Matrix4x4& projection, const Matrix4x4& view) {
    // The world matrix is the identity here, so it is left out of the
    // multiply. The view and projection matrices are combined first and the
    // point is transformed only once, exactly like D3DXVec3Project().
    const Matrix4x4 worldProjection = MultiplyMatrices(view, projection);
    const Vector3 transformed = TransformCoord(world, worldProjection);

    // Map from normalised device coordinates to screen pixels.
    Vector3 screen;
    screen.x = (viewport.X + (1.0f + transformed.x) * viewport.Width * 0.5f);
    screen.y = (viewport.Y + (1.0f - transformed.y) * viewport.Height * 0.5f);
    screen.z = viewport.MinZ + transformed.z * (viewport.MaxZ - viewport.MinZ);
    return screen;
}

}  // namespace sa
