/* Copyright 2010, Object Management Group, Inc.
* Copyright 2010, PrismTech, Corp.
* Copyright 2010, Real-Time Innovations, Inc.
* All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef  ORG_OMG_DDS_CORE_ARRAY_HPP_
#define  ORG_OMG_DDS_CORE_ARRAY_HPP_

#include <string>

// NOTICE:
//    This array implementation is derived from the GNU C++ std library
//    and is licensed under LGPL.
//    This code is intended to show a possible implementation of the
//    array class as well as to provide the mandatory API that compliant
//    implementations of the DDS-PSM-C++ API have to conform with.

namespace dds
{
namespace core
{
template <typename T> typename T::iterator begin(T& t)
{
    return t.begin();
}

template <typename T> typename T::iterator end(T& t)
{
    return t.end();
}

template <typename T> typename T::const_iterator begin(const T& t)
{
    return t.begin();
}

template <typename T> typename T::const_iterator end(const T& t)
{
    return t.end();
}


/**
 *  @brief A standard container for storing a fixed size sequence of elements.
 *
 *  @ingroup sequences
 *
 *  Sets support for random access iterators.
 *
 *  @param  Tp  Type of element. Required to be a complete type.
 *  @param  N  Number of elements.
 */
template<typename _Tp, std::size_t _Nm>
struct array
{
    typedef _Tp                         value_type;
    typedef _Tp*                                    pointer;
    typedef const _Tp*                              const_pointer;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef value_type*                     iterator;
    typedef const value_type*               const_iterator;
    typedef std::size_t                             size_type;
    typedef std::ptrdiff_t                          difference_type;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    // Support for zero-sized arrays mandatory.
    value_type _M_instance[_Nm ? _Nm : 1];

    // No explicit construct/copy/destroy for aggregate type.

    // DR 776.
    void
    fill(const value_type& __u)
    {
        std::fill_n(begin(), size(), __u);
    }

    void
    swap(array& __other)
    {
        std::swap_ranges(begin(), end(), __other.begin());
    }

    // Iterators.
    iterator
    begin()
    {
        return iterator(&_M_instance[0]);
    }

    const_iterator
    begin() const
    {
        return const_iterator(&(_M_instance[0]));
    }

    iterator
    end()
    {
        return iterator(&(_M_instance[_Nm]));
    }

    const_iterator
    end() const
    {
        return const_iterator(&(_M_instance[_Nm]));
    }

    reverse_iterator
    rbegin()
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator
    rbegin() const
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator
    rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator
    rend() const
    {
        return const_reverse_iterator(begin());
    }

    const_iterator
    cbegin() const
    {
        return const_iterator(&(_M_instance[0]));
    }

    const_iterator
    cend() const
    {
        return const_iterator(&(_M_instance[_Nm]));
    }

    const_reverse_iterator
    crbegin() const
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator
    crend() const
    {
        return const_reverse_iterator(begin());
    }

    // Capacity.
    size_type
    size() const
    {
        return _Nm;
    }

    size_type
    max_size() const
    {
        return _Nm;
    }

    bool
    empty() const
    {
        return size() == 0;
    }

    // Element access.
    reference
    operator[](size_type __n)
    {
        return _M_instance[__n];
    }

    const_reference
    operator[](size_type __n) const
    {
        return _M_instance[__n];
    }

    reference
    at(size_type __n)
    {
        if(__n >= _Nm)
        {
            std::__throw_out_of_range(__N("array::at"));
        }
        return _M_instance[__n];
    }

    const_reference
    at(size_type __n) const
    {
        if(__n >= _Nm)
        {
            std::__throw_out_of_range(__N("array::at"));
        }
        return _M_instance[__n];
    }

    reference
    front()
    {
        return *begin();
    }

    const_reference
    front() const
    {
        return *begin();
    }

    reference
    back()
    {
        return _Nm ? *(end() - 1) : *end();
    }

    const_reference
    back() const
    {
        return _Nm ? *(end() - 1) : *end();
    }

    _Tp*
    data()
    {
        return &(_M_instance[0]);
    }

    const _Tp*
    data() const
    {
        return &(_M_instance[0]);
    }
};

// Array comparisons.
template<typename _Tp, std::size_t _Nm>
inline bool
operator==(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
{
    return std::equal(__one.begin(), __one.end(), __two.begin());
}

template<typename _Tp, std::size_t _Nm>
inline bool
operator!=(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
{
    return !(__one == __two);
}

template<typename _Tp, std::size_t _Nm>
inline bool
operator<(const array<_Tp, _Nm>& __a, const array<_Tp, _Nm>& __b)
{
    return std::lexicographical_compare(__a.begin(), __a.end(),
                                        __b.begin(), __b.end());
}

template<typename _Tp, std::size_t _Nm>
inline bool
operator>(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
{
    return __two < __one;
}

template<typename _Tp, std::size_t _Nm>
inline bool
operator<=(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
{
    return !(__one > __two);
}

template<typename _Tp, std::size_t _Nm>
inline bool
operator>=(const array<_Tp, _Nm>& __one, const array<_Tp, _Nm>& __two)
{
    return !(__one < __two);
}

/// tuple_element
template<std::size_t _Int, typename _Tp>
class tuple_element;

template <typename _Tp> class tuple_size;

template<typename _Tp, std::size_t _Nm>
struct tuple_size<array<_Tp, _Nm> >
{
    static const std::size_t value = _Nm;
};

template<typename _Tp, std::size_t _Nm>
const std::size_t
tuple_size<array<_Tp, _Nm> >::value;

template<std::size_t _Int, typename _Tp, std::size_t _Nm>
struct tuple_element<_Int, array<_Tp, _Nm> >
{
    typedef _Tp type;
};

template<std::size_t _Int, typename _Tp, std::size_t _Nm>
inline _Tp&
get(array<_Tp, _Nm>& __arr)
{
    return __arr[_Int];
}

template<std::size_t _Int, typename _Tp, std::size_t _Nm>
inline const _Tp&
get(const array<_Tp, _Nm>& __arr)
{
    return __arr[_Int];
}
}
}


#endif /* ORG_OMG_DDS_CORE_ARRAY_HPP_  */
