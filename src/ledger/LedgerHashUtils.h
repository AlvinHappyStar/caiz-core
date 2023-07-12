#pragma once

// Copyright 2018 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "crypto/ShortHash.h"
#include "ledger/InternalLedgerEntry.h"
#include "ledger/LedgerTypeUtils.h"
#include "util/HashOfHash.h"
#include "xdr/Caiz-ledger-entries.h"
#include "xdr/Caiz-ledger.h"
#include <functional>

namespace caiz
{

static PoolID const&
getLiquidityPoolID(Asset const& asset)
{
    throw std::runtime_error("cannot get PoolID from Asset");
}

static PoolID const&
getLiquidityPoolID(TrustLineAsset const& tlAsset)
{
    return tlAsset.liquidityPoolID();
}

static inline void
hashMix(size_t& h, size_t v)
{
    // from https://github.com/ztanml/fast-hash (MIT license)
    v ^= v >> 23;
    v *= 0x2127599bf4325c37ULL;
    v ^= v >> 47;
    h ^= v;
    h *= 0x880355f21e6d1965ULL;
}

template <typename T>
static size_t
getAssetHash(T const& asset)
{
    size_t res = asset.type();

    switch (asset.type())
    {
    case caiz::ASSET_TYPE_NATIVE:
        break;
    case caiz::ASSET_TYPE_CREDIT_ALPHANUM4:
    {
        auto& a4 = asset.alphaNum4();
        hashMix(res, std::hash<caiz::uint256>()(a4.issuer.ed25519()));
        hashMix(res, caiz::shortHash::computeHash(caiz::ByteSlice(
                         a4.assetCode.data(), a4.assetCode.size())));
        break;
    }
    case caiz::ASSET_TYPE_CREDIT_ALPHANUM12:
    {
        auto& a12 = asset.alphaNum12();
        hashMix(res, std::hash<caiz::uint256>()(a12.issuer.ed25519()));
        hashMix(res, caiz::shortHash::computeHash(caiz::ByteSlice(
                         a12.assetCode.data(), a12.assetCode.size())));
        break;
    }
    case caiz::ASSET_TYPE_POOL_SHARE:
    {
        hashMix(res, std::hash<caiz::uint256>()(getLiquidityPoolID(asset)));
        break;
    }
    default:
        throw std::runtime_error("unknown Asset type");
    }
    return res;
}

}

// implements a default hasher for "LedgerKey"
namespace std
{
template <> class hash<caiz::Asset>
{
  public:
    size_t
    operator()(caiz::Asset const& asset) const
    {
        return caiz::getAssetHash<caiz::Asset>(asset);
    }
};

template <> class hash<caiz::TrustLineAsset>
{
  public:
    size_t
    operator()(caiz::TrustLineAsset const& asset) const
    {
        return caiz::getAssetHash<caiz::TrustLineAsset>(asset);
    }
};

template <> class hash<caiz::LedgerKey>
{
  public:
    size_t
    operator()(caiz::LedgerKey const& lk) const
    {
        size_t res = lk.type();
        switch (lk.type())
        {
        case caiz::ACCOUNT:
            caiz::hashMix(res, std::hash<caiz::uint256>()(
                                      lk.account().accountID.ed25519()));
            break;
        case caiz::TRUSTLINE:
        {
            auto& tl = lk.trustLine();
            caiz::hashMix(
                res, std::hash<caiz::uint256>()(tl.accountID.ed25519()));
            caiz::hashMix(res, hash<caiz::TrustLineAsset>()(tl.asset));
            break;
        }
        case caiz::DATA:
            caiz::hashMix(res, std::hash<caiz::uint256>()(
                                      lk.data().accountID.ed25519()));
            caiz::hashMix(
                res,
                caiz::shortHash::computeHash(caiz::ByteSlice(
                    lk.data().dataName.data(), lk.data().dataName.size())));
            break;
        case caiz::OFFER:
            caiz::hashMix(
                res, caiz::shortHash::computeHash(caiz::ByteSlice(
                         &lk.offer().offerID, sizeof(lk.offer().offerID))));
            break;
        case caiz::CLAIMABLE_BALANCE:
            caiz::hashMix(res, std::hash<caiz::uint256>()(
                                      lk.claimableBalance().balanceID.v0()));
            break;
        case caiz::LIQUIDITY_POOL:
            caiz::hashMix(res, std::hash<caiz::uint256>()(
                                      lk.liquidityPool().liquidityPoolID));
            break;
#ifdef ENABLE_NEXT_PROTOCOL_VERSION_UNSAFE_FOR_PRODUCTION
        case caiz::CONTRACT_DATA:
            switch (lk.contractData().contract.type())
            {
            case caiz::SC_ADDRESS_TYPE_ACCOUNT:
                caiz::hashMix(
                    res, std::hash<caiz::uint256>()(
                             lk.contractData().contract.accountId().ed25519()));
                break;
            case caiz::SC_ADDRESS_TYPE_CONTRACT:
                caiz::hashMix(res,
                                 std::hash<caiz::uint256>()(
                                     lk.contractData().contract.contractId()));
                break;
            }
            caiz::hashMix(
                res, caiz::shortHash::xdrComputeHash(lk.contractData().key));
            caiz::hashMix(
                res, std::hash<int32_t>()(lk.contractData().durability));
            caiz::hashMix(res, std::hash<int32_t>()(getLeType(lk)));
            break;
        case caiz::CONTRACT_CODE:
            caiz::hashMix(
                res, std::hash<caiz::uint256>()(lk.contractCode().hash));
            caiz::hashMix(res, std::hash<int32_t>()(getLeType(lk)));
            break;
        case caiz::CONFIG_SETTING:
            caiz::hashMix(
                res, std::hash<int32_t>()(lk.configSetting().configSettingID));
            break;
#endif
        default:
            abort();
        }
        return res;
    }
};

template <> class hash<caiz::InternalLedgerKey>
{
  public:
    size_t
    operator()(caiz::InternalLedgerKey const& glk) const
    {
        return glk.hash();
    }
};
}
