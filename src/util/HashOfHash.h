// Copyright 2020 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#pragma once
#include <xdr/Caiz-types.h>

namespace std
{
template <> struct hash<caiz::uint256>
{
    size_t operator()(caiz::uint256 const& x) const noexcept;
};
}