#pragma once

// Copyright 2015 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "overlay/CaizXDR.h"

namespace caiz
{
void dumpXdrStream(std::string const& filename, bool json);
void printXdr(std::string const& filename, std::string const& filetype,
              bool base64, bool compact, bool rawMode);
void signtxn(std::string const& filename, std::string netId, bool base64);
void priv2pub();
}
