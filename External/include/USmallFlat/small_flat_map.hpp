#pragma once

#include "basic_flat_map.hpp"

#include "details/small_vector_bind.hpp"

namespace Ubpa {
    template<typename Key, typename T, std::size_t N = 16, typename Compare = std::less<Key>, template<typename>class TAllocator = std::allocator>
    class small_flat_map : public basic_flat_map<details::Tsmall_vector_bind<N, TAllocator>::template Ttype, Key, T, Compare> {
        using mybase = basic_flat_map<details::Tsmall_vector_bind<N, TAllocator>::template Ttype, Key, T, Compare>;
    public:
        using mybase::mybase;
        using typename mybase::value_type;

        small_flat_map(std::initializer_list<value_type> ilist, const Compare& comp = Compare())
            : mybase(ilist, comp) {}
    };
}
