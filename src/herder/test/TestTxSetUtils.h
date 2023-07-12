// Copyright 2022 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "herder/TxSetUtils.h"
#include "xdr/Caiz-ledger.h"

namespace caiz
{
namespace testtxset
{

using ComponentPhases = std::vector<
    std::pair<std::optional<int64_t>, std::vector<TransactionFrameBasePtr>>>;
TxSetFrameConstPtr makeNonValidatedGeneralizedTxSet(
    std::vector<ComponentPhases> const& txsPerBaseFee, Application& app,
    Hash const& previousLedgerHash);

TxSetFrameConstPtr makeNonValidatedTxSetBasedOnLedgerVersion(
    uint32_t ledgerVersion, std::vector<TransactionFrameBasePtr> const& txs,
    Application& app, Hash const& previousLedgerHash);
} // namespace testtxset
} // namespace caiz