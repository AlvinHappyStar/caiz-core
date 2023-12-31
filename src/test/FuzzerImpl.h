#pragma once

// Copyright 2019 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "ledger/LedgerTxn.h"
#include "test/Fuzzer.h"
#include "util/Timer.h"
#include "xdr/Caiz-ledger-entries.h"
#include "xdr/Caiz-types.h"

namespace caiz
{

class XDRInputFileStream;
class Simulation;
class Application;
struct CaizMessage;
struct Operation;

namespace FuzzUtils
{
size_t static constexpr NUM_STORED_LEDGER_KEYS = 0x100U;
using StoredLedgerKeys = std::array<LedgerKey, NUM_STORED_LEDGER_KEYS>;
StoredLedgerKeys::difference_type static constexpr NUM_UNVALIDATED_LEDGER_KEYS =
    0x40;
size_t static constexpr NUM_VALIDATED_LEDGER_KEYS =
    NUM_STORED_LEDGER_KEYS - NUM_UNVALIDATED_LEDGER_KEYS;

size_t static constexpr NUM_STORED_POOL_IDS = 0x7U;
using StoredPoolIDs = std::array<PoolID, NUM_STORED_POOL_IDS>;
}

class TransactionFuzzer : public Fuzzer
{
  public:
    TransactionFuzzer()
    {
    }
    void inject(std::string const& filename) override;
    void initialize() override;
    void shutdown() override;
    void genFuzz(std::string const& filename) override;
    int xdrSizeLimit() override;

  private:
    void storeSetupLedgerKeysAndPoolIDs(AbstractLedgerTxn& ltx);
    void storeSetupPoolIDs(AbstractLedgerTxn& ltx,
                           std::vector<LedgerEntry> const& entries);
    void initializeAccounts(AbstractLedgerTxn& ltxOuter);
    void initializeTrustLines(AbstractLedgerTxn& ltxOuter);
    void initializeClaimableBalances(AbstractLedgerTxn& ltxOuter);
    void initializeOffers(AbstractLedgerTxn& ltxOuter);
    void initializeLiquidityPools(AbstractLedgerTxn& ltxOuter);
    void reduceNativeBalancesAfterSetup(AbstractLedgerTxn& ltxOuter);
    void adjustTrustLineBalancesAfterSetup(AbstractLedgerTxn& ltxOuter);
    void reduceTrustLineLimitsAfterSetup(AbstractLedgerTxn& ltxOuter);
    VirtualClock mClock;
    std::shared_ptr<Application> mApp;
    PublicKey mSourceAccountID;
    FuzzUtils::StoredLedgerKeys mStoredLedgerKeys;
    FuzzUtils::StoredPoolIDs mStoredPoolIDs;
};

class OverlayFuzzer : public Fuzzer
{
    int const ACCEPTOR_INDEX = 0;
    int const INITIATOR_INDEX = 1;

  public:
    OverlayFuzzer()
    {
    }
    void inject(std::string const& filename) override;
    void initialize() override;
    void shutdown() override;
    void genFuzz(std::string const& filename) override;
    int xdrSizeLimit() override;

  private:
    std::shared_ptr<Simulation> mSimulation;
    FuzzUtils::StoredLedgerKeys mStoredLedgerKeys;
    FuzzUtils::StoredPoolIDs mStoredPoolIDs;
};
}