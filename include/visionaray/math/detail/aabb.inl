// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include <utility>

#include "../axis.h"
#include "../limits.h"


namespace MATH_NAMESPACE
{

//--------------------------------------------------------------------------------------------------
// aabb members
//

template <typename T>
MATH_FUNC
inline basic_aabb<T>::basic_aabb(vector<3, T> const& min, vector<3, T> const& max)
    : min(min)
    , max(max)
{
}

template <typename T>
template <typename U>
MATH_FUNC
inline basic_aabb<T>::basic_aabb(basic_aabb<U> const& rhs)
    : min(rhs.min)
    , max(rhs.max)
{
}

template <typename T>
template <typename U>
MATH_FUNC
inline basic_aabb<T>::basic_aabb(vector<3, U> const& min, vector<3, U> const& max)
    : min(min)
    , max(max)
{
}

template <typename T>
template <typename U>
MATH_FUNC
inline basic_aabb<T>& basic_aabb<T>::operator=(basic_aabb<U> const& rhs)
{
    min = vector<3, T>(rhs.min);
    max = vector<3, T>(rhs.max);

    return *this;
}

template <typename T>
MATH_FUNC
inline vector<3, T> basic_aabb<T>::center() const
{
    return (max + min) * value_type(0.5);
}

template <typename T>
MATH_FUNC
inline vector<3, T> basic_aabb<T>::size() const
{
    return max - min;
}

template <typename T>
MATH_FUNC
inline vector<3, T> basic_aabb<T>::safe_size() const
{
    auto s = max - min;

    s.x = MATH_NAMESPACE::max(T(0.0), s.x);
    s.y = MATH_NAMESPACE::max(T(0.0), s.y);
    s.z = MATH_NAMESPACE::max(T(0.0), s.z);

    return s;
}

template <typename T>
MATH_FUNC
inline void basic_aabb<T>::invalidate()
{
    min = vec_type(numeric_limits<T>::max());
    max = vec_type(numeric_limits<T>::lowest());
}

template <typename T>
MATH_FUNC
inline bool basic_aabb<T>::invalid() const
{
    return min.x > max.x || min.y > max.y || min.z > max.z;
}

template <typename T>
MATH_FUNC
inline bool basic_aabb<T>::valid() const
{
    return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}

template <typename T>
MATH_FUNC
inline bool basic_aabb<T>::empty() const
{
    return min.x >= max.x || min.y >= max.y || min.z >= max.z;
}

template <typename T>
MATH_FUNC
inline bool basic_aabb<T>::contains(vector<3, T> const& v) const
{
    return v.x >= min.x && v.x <= max.x
        && v.y >= min.y && v.y <= max.y
        && v.z >= min.z && v.z <= max.z;
}

template <typename T>
MATH_FUNC
inline bool basic_aabb<T>::contains(basic_aabb<T> const& b) const
{
    return contains(b.min) && contains(b.max);
}

template <typename T>
MATH_FUNC
inline void basic_aabb<T>::insert(vec_type const& v)
{
    min = MATH_NAMESPACE::min(min, v);
    max = MATH_NAMESPACE::max(max, v);
}

template <typename T>
MATH_FUNC
inline void basic_aabb<T>::insert(basic_aabb const& v)
{
    min = MATH_NAMESPACE::min(min, v.min);
    max = MATH_NAMESPACE::max(max, v.max);
}


//--------------------------------------------------------------------------------------------------
// Comparisons
//

template <typename T>
MATH_FUNC
inline auto operator==(basic_aabb<T> const& lhs, basic_aabb<T> const& rhs)
    -> decltype(lhs.min == rhs.min)
{
    return lhs.min == rhs.min && lhs.max == rhs.max;
}

template <typename T>
MATH_FUNC
inline auto operator!=(basic_aabb<T> const& lhs, basic_aabb<T> const& rhs)
    -> decltype(lhs.min != rhs.min)
{
    return lhs.min != rhs.min || lhs.max != rhs.max;
}


//--------------------------------------------------------------------------------------------------
// Geometric functions
//

template <typename T>
MATH_FUNC
inline basic_aabb<T> combine(basic_aabb<T> const& a, basic_aabb<T> const& b)
{
    return basic_aabb<T>( min(a.min, b.min), max(a.max, b.max) );
}

template <typename T>
MATH_FUNC
inline basic_aabb<T> combine(basic_aabb<T> const& a, vector<3, T> const& b)
{
    return basic_aabb<T>( min(a.min, b), max(a.max, b) );
}

template <typename T>
MATH_FUNC
inline basic_aabb<T> intersect(basic_aabb<T> const& a, basic_aabb<T> const& b)
{
    return basic_aabb<T>( max(a.min, b.min), min(a.max, b.max) );
}

template <typename T>
MATH_FUNC
inline T half_surface_area(basic_aabb<T> const& box)
{
    auto s = box.size();
    return s.x * s.y + s.y * s.z + s.z * s.x;
}

template <typename T>
MATH_FUNC
inline T safe_half_surface_area(basic_aabb<T> const& box)
{
    auto s = box.safe_size();
    return s.x * s.y + s.y * s.z + s.z * s.x;
}

template <typename T>
MATH_FUNC
inline T surface_area(basic_aabb<T> const& box)
{
    return T(2.0) * half_surface_area(box);
}

template <typename T>
MATH_FUNC
inline T safe_surface_area(basic_aabb<T> const& box)
{
    return T(2.0) * safe_half_surface_area(box);
}

template <typename T>
MATH_FUNC
inline T volume(basic_aabb<T> const& box)
{
    auto s = box.size();
    return s.x * s.y * s.z;
}

template <typename T>
MATH_FUNC
inline T overlap_ratio_union(basic_aabb<T> const& lhs, basic_aabb<T> const& rhs)
{
    auto I = intersect(lhs, rhs);

    if (I.empty())
    {
        // bounding boxes do not overlap.
        return T(0.0);
    }

    return volume(I) / volume(combine(lhs, rhs));
}

template <typename T>
MATH_FUNC
inline T overlap_ratio_min(basic_aabb<T> const& lhs, basic_aabb<T> const& rhs)
{
    auto I = intersect(lhs, rhs);

    if (lhs.empty() || rhs.empty())
    {
        // an empty bounding box never overlaps another bounding box
        return T(0.0);
    }

    if (I.empty())
    {
        // bounding boxes do not overlap.
        return T(0.0);
    }

    return volume(I) / min(volume(lhs), volume(rhs));
}

template <typename T>
MATH_FUNC
inline T overlap_ratio(basic_aabb<T> const& lhs, basic_aabb<T> const& rhs)
{
//  return overlap_ratio_union(lhs, rhs);
    return overlap_ratio_min(lhs, rhs);
}

template <typename T>
inline std::pair<basic_aabb<T>, basic_aabb<T>> split(basic_aabb<T> const& box, cartesian_axis<3> axis, T splitpos)
{
    vector<3, T> min1 = box.min;
    vector<3, T> min2 = box.min;
    vector<3, T> max1 = box.max;
    vector<3, T> max2 = box.max;

    max1[axis] = splitpos;
    min2[axis] = splitpos;

    basic_aabb<T> box1(min1, max1);
    basic_aabb<T> box2(min2, max2);
    return std::make_pair(box1, box2);
}

template <typename T>
MATH_FUNC
inline typename basic_aabb<T>::vertex_list compute_vertices(basic_aabb<T> const& box)
{
    vector<3, T> min = box.min;
    vector<3, T> max = box.max;

    return {{
        { max.x, max.y, max.z },
        { min.x, max.y, max.z },
        { min.x, min.y, max.z },
        { max.x, min.y, max.z },
        { min.x, max.y, min.z },
        { max.x, max.y, min.z },
        { max.x, min.y, min.z },
        { min.x, min.y, min.z }
    }};
}

} // MATH_NAMESPACE
