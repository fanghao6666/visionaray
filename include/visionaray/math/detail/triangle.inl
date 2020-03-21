// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include "../aabb.h"
#include "../rectangle.h"

namespace MATH_NAMESPACE
{

//-------------------------------------------------------------------------------------------------
// Triangle members
//

template <size_t Dim, typename T, typename P>
MATH_FUNC
basic_triangle<Dim, T, P>::basic_triangle(
        vector<Dim, T> const& v1,
        vector<Dim, T> const& e1,
        vector<Dim, T> const& e2
        )
    : v1(v1)
    , e1(e1)
    , e2(e2)
{
}


//-------------------------------------------------------------------------------------------------
// Geometric functions
//

template <size_t Dim, typename T, typename P>
MATH_FUNC
inline T area(basic_triangle<Dim, T, P> const& t)
{
    return T(0.5) * length(cross(t.e1, t.e2));
}

template <size_t Dim, typename T, typename P>
MATH_FUNC
inline basic_aabb<T> get_bounds(basic_triangle<Dim, T, P> const& t)
{
    basic_aabb<T> bounds;

    bounds.invalidate();
    bounds.insert(t.v1);
    bounds.insert(t.v1 + t.e1);
    bounds.insert(t.v1 + t.e2);

    return bounds;
}

template <typename T, typename P>
MATH_FUNC
inline rectangle<min_max_layout<2, T>, T> get_bounds(basic_triangle<2, T, P> const& t)
{
    rectangle<min_max_layout<2, T>, T> bounds;

    bounds.invalidate();
    bounds.insert(t.v1);
    bounds.insert(t.v1 + t.e1);
    bounds.insert(t.v1 + t.e2);

    return bounds;
}

template <size_t Dim, typename T, typename P, typename Generator, typename U = typename Generator::value_type>
MATH_FUNC
inline vector<3, U> sample_surface(basic_triangle<Dim, T, P> const& t, Generator& gen)
{
    U u1 = gen.next();
    U u2 = gen.next();

    vector<3, U> v1(t.v1);
    vector<3, U> v2(t.v1 + t.e1);
    vector<3, U> v3(t.v1 + t.e2);

    return v1 * (U(1.0) - sqrt(u1)) + v2 * sqrt(u1) * (U(1.0) - u2) + v3 * sqrt(u1) * u2;
}

} // MATH_NAMESPACE
