// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include <cmath>

namespace MATH_NAMESPACE
{
namespace simd
{

//-------------------------------------------------------------------------------------------------
// float4 members
//

VSNRAY_FORCE_INLINE float4::basic_float(float x, float y, float z, float w)
    : value(_mm_set_ps(w, z, y, x))
{
}

VSNRAY_FORCE_INLINE float4::basic_float(float const v[4])
    : value(_mm_load_ps(v))
{
}

VSNRAY_FORCE_INLINE float4::basic_float(float s)
    : value(_mm_set1_ps(s))
{
}

VSNRAY_FORCE_INLINE float4::basic_float(__m128i const& i)
    : value(_mm_cvtepi32_ps(i))
{
}

VSNRAY_FORCE_INLINE float4::basic_float(__m128 const& v)
    : value(v)
{
}

VSNRAY_FORCE_INLINE float4::operator __m128() const
{
    return value;
}


//-------------------------------------------------------------------------------------------------
// Bitwise cast
//

VSNRAY_FORCE_INLINE int4 reinterpret_as_int(float4 const& a)
{
    return _mm_castps_si128(a);
}


//-------------------------------------------------------------------------------------------------
// Static cast
//

VSNRAY_FORCE_INLINE int4 convert_to_int(float4 const& a)
{
    return _mm_cvttps_epi32(a);
}


//-------------------------------------------------------------------------------------------------
// select intrinsic
//

VSNRAY_FORCE_INLINE float4 select(mask4 const& m, float4 const& a, float4 const& b)
{
#if VSNRAY_SIMD_ISA_GE(VSNRAY_SIMD_ISA_SSE4_1)
    return _mm_blendv_ps(b, a, m.f);
#else
    return _mm_or_ps(_mm_and_ps(m.f, a), _mm_andnot_ps(m.f, b));
#endif
}


//-------------------------------------------------------------------------------------------------
// Load / store / get
//

VSNRAY_FORCE_INLINE float4 load(float const src[4])
{
    return _mm_load_ps(src);
}

VSNRAY_FORCE_INLINE float4 load_unaligned(float const src[4])
{
    return _mm_loadu_ps(src);
}

VSNRAY_FORCE_INLINE void store(float dst[4], float4 const& v)
{
    _mm_store_ps(dst, v);
}

VSNRAY_FORCE_INLINE void store_unaligned(float dst[4], float4 const& v)
{
    _mm_storeu_ps(dst, v);
}

VSNRAY_FORCE_INLINE void store_non_temporal(float dst[4], float4 const& v)
{
    _mm_stream_ps(dst, v);
}

template <size_t I>
VSNRAY_FORCE_INLINE float& get(float4& v)
{
    static_assert(I < 4, "Index out of range for SIMD vector access");

    return reinterpret_cast<float*>(&v)[I];
}

template <size_t I>
VSNRAY_FORCE_INLINE float const& get(float4 const& v)
{
    static_assert(I < 4, "Index out of range for SIMD vector access");

    return reinterpret_cast<float const*>(&v)[I];
}


//-------------------------------------------------------------------------------------------------
// Transposition
//

template <int U0, int U1, int V2, int V3>
VSNRAY_FORCE_INLINE float4 shuffle(float4 const& u, float4 const& v)
{
    return _mm_shuffle_ps(u, v, _MM_SHUFFLE(V3, V2, U1, U0));
}

template <int V0, int V1, int V2, int V3>
VSNRAY_FORCE_INLINE float4 shuffle(float4 const& v)
{
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(V3, V2, V1, V0));
}

VSNRAY_FORCE_INLINE float4 move_lo(float4 const& u, float4 const& v)
{
    return _mm_movelh_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 move_hi(float4 const& u, float4 const& v)
{
    return _mm_movehl_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 interleave_lo(float4 const& u, float4 const& v)
{
    return _mm_unpacklo_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 interleave_hi(float4 const& u, float4 const& v)
{
    return _mm_unpackhi_ps(u, v);
}


//-------------------------------------------------------------------------------------------------
// Basic arithmetics
//

VSNRAY_FORCE_INLINE float4 operator+(float4 const& v)
{
    return _mm_add_ps(_mm_setzero_ps(), v);
}

VSNRAY_FORCE_INLINE float4 operator-(float4 const& v)
{
    return _mm_sub_ps(_mm_setzero_ps(), v);
}

VSNRAY_FORCE_INLINE float4 operator+(float4 const& u, float4 const& v)
{
    return _mm_add_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 operator-(float4 const& u, float4 const& v)
{
    return _mm_sub_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 operator*(float4 const& u, float4 const& v)
{
    return _mm_mul_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 operator/(float4 const& u, float4 const& v)
{
    return _mm_div_ps(u, v);
}


//-------------------------------------------------------------------------------------------------
// Bitwise operations
//

VSNRAY_FORCE_INLINE float4 operator&(float4 const& u, float4 const& v)
{
    return _mm_and_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 operator|(float4 const& u, float4 const& v)
{
    return _mm_or_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 operator^(float4 const& u, float4 const& v)
{
    return _mm_xor_ps(u, v);
}


//-------------------------------------------------------------------------------------------------
// Logical operations
//

VSNRAY_FORCE_INLINE float4 operator&&(float4 const& u, float4 const& v)
{
    return _mm_and_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 operator||(float4 const& u, float4 const& v)
{
    return _mm_or_ps(u, v);
}


//-------------------------------------------------------------------------------------------------
// Comparisons
//

VSNRAY_FORCE_INLINE mask4 operator<(float4 const& u, float4 const& v)
{
    return _mm_cmplt_ps(u, v);
}

VSNRAY_FORCE_INLINE mask4 operator>(float4 const& u, float4 const& v)
{
    return _mm_cmpgt_ps(u, v);
}

VSNRAY_FORCE_INLINE mask4 operator<=(float4 const& u, float4 const& v)
{
    return _mm_cmple_ps(u, v);
}

VSNRAY_FORCE_INLINE mask4 operator>=(float4 const& u, float4 const& v)
{
    return _mm_cmpge_ps(u, v);
}

VSNRAY_FORCE_INLINE mask4 operator==(float4 const& u, float4 const& v)
{
    return _mm_cmpeq_ps(u, v);
}

VSNRAY_FORCE_INLINE mask4 operator!=(float4 const& u, float4 const& v)
{
    return _mm_cmpneq_ps(u, v);
}


//-------------------------------------------------------------------------------------------------
// Math functions
//

VSNRAY_FORCE_INLINE float4 min(float4 const& u, float4 const& v)
{
    return _mm_min_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 max(float4 const& u, float4 const& v)
{
    return _mm_max_ps(u, v);
}

VSNRAY_FORCE_INLINE float4 saturate(float4 const& u)
{
    return _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(u, _mm_set1_ps(1.0f)));
}

VSNRAY_FORCE_INLINE float4 abs(float4 const& u)
{
    return _mm_and_ps(u, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
}

VSNRAY_FORCE_INLINE float4 round(float4 const& v)
{
#if VSNRAY_SIMD_ISA_GE(VSNRAY_SIMD_ISA_SSE4_1)
    return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT);
#else
    // Mask out the signbits of v
    __m128 s = _mm_and_ps(v, _mm_castsi128_ps(_mm_set1_epi32(0x80000000)));
    // Magic number: 2^23 with the signbits of v
    __m128 m = _mm_or_ps(s, _mm_castsi128_ps(_mm_set1_epi32(0x4B000000)));
    __m128 x = _mm_add_ps(v, m);
    __m128 y = _mm_sub_ps(x, m);

    return y;
#endif
}

VSNRAY_FORCE_INLINE float4 ceil(float4 const& v)
{
#if VSNRAY_SIMD_ISA_GE(VSNRAY_SIMD_ISA_SSE4_1)
    return _mm_ceil_ps(v);
#else
    // i = trunc(v)
    __m128 i = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
    // r = i < v ? i i + 1 : i
    __m128 t = _mm_cmplt_ps(i, v);
    __m128 d = _mm_cvtepi32_ps(_mm_castps_si128(t)); // mask to float: 0 -> 0.0f, 0xFFFFFFFF -> -1.0f
    __m128 r = _mm_sub_ps(i, d);

    return r;
#endif
}

VSNRAY_FORCE_INLINE float4 floor(float4 const& v)
{
#if VSNRAY_SIMD_ISA_GE(VSNRAY_SIMD_ISA_SSE4_1)
    return _mm_floor_ps(v);
#else
    // i = trunc(v)
    __m128 i = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
    // r = i > v ? i - 1 : i
    __m128 t = _mm_cmpgt_ps(i, v);
    __m128 d = _mm_cvtepi32_ps(_mm_castps_si128(t)); // mask to float: 0 -> 0.0f, 0xFFFFFFFF -> -1.0f
    __m128 r = _mm_add_ps(i, d);

    return r;
#endif
}

VSNRAY_FORCE_INLINE float4 sqrt(float4 const& v)
{
    return _mm_sqrt_ps(v);
}

VSNRAY_FORCE_INLINE mask4 isinf(float4 const& v)
{
    VSNRAY_ALIGN(16) float values[4] = {};
    store(values, v);

    return mask4(
            std::isinf(values[0]),
            std::isinf(values[1]),
            std::isinf(values[2]),
            std::isinf(values[3])
            );
}

VSNRAY_FORCE_INLINE mask4 isnan(float4 const& v)
{
    return v != v;
}

VSNRAY_FORCE_INLINE mask4 isfinite(float4 const& v)
{
    return !(isinf(v) | isnan(v));
}


//-------------------------------------------------------------------------------------------------
//
//

template <unsigned N>
VSNRAY_FORCE_INLINE float4 rcp(float4 const& v)
{
    float4 x0 = _mm_rcp_ps(v);
    return rcp_step<N>(x0);
}

VSNRAY_FORCE_INLINE float4 rcp(float4 const& v)
{
    float4 x0 = _mm_rcp_ps(v);
    return rcp_step<1>(x0);
}

template <unsigned N>
VSNRAY_FORCE_INLINE float4 rsqrt(float4 const& v)
{
    float4 x0 = _mm_rsqrt_ps(v);
    return rsqrt_step<N>(v, x0);
}

VSNRAY_FORCE_INLINE float4 rsqrt(float4 const& v)
{
    float4 x0 = _mm_rsqrt_ps(v);
    return rsqrt_step<1>(v, x0);
}

VSNRAY_FORCE_INLINE float4 approx_rsqrt(float4 const& v)
{
    return _mm_rsqrt_ps(v);
}

} // simd
} // MATH_NAMESPACE
