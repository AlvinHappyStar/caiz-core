#pragma once

// Copyright 2017 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "xdr/Caiz-types.h"
#include <vector>

namespace caiz
{

struct SCPEnvelope;
struct SCPStatement;
struct CaizValue;

std::vector<Hash> getTxSetHashes(SCPEnvelope const& envelope);
std::vector<CaizValue> getCaizValues(SCPStatement const& envelope);
}
