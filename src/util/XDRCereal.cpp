// Copyright 2020 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "util/XDRCereal.h"

void
cereal_override(cereal::JSONOutputArchive& ar, const caiz::PublicKey& s,
                const char* field)
{
    xdr::archive(ar, caiz::KeyUtils::toStrKey<caiz::PublicKey>(s), field);
}

void
cereal_override(cereal::JSONOutputArchive& ar,
                const caiz::MuxedAccount& muxedAccount, const char* field)
{
    switch (muxedAccount.type())
    {
    case caiz::KEY_TYPE_ED25519:
        xdr::archive(ar, caiz::KeyUtils::toStrKey(toAccountID(muxedAccount)),
                     field);
        return;
    case caiz::KEY_TYPE_MUXED_ED25519:
        xdr::archive(
            ar,
            std::make_tuple(
                cereal::make_nvp("id", muxedAccount.med25519().id),
                cereal::make_nvp("accountID", caiz::KeyUtils::toStrKey(
                                                  toAccountID(muxedAccount)))),
            field);
        return;
    default:
        // this would be a bug
        abort();
    }
}

void
cerealPoolAsset(cereal::JSONOutputArchive& ar, const caiz::Asset& asset,
                const char* field)
{
    xdr::archive(ar, std::string("INVALID"), field);
}

void
cerealPoolAsset(cereal::JSONOutputArchive& ar,
                const caiz::TrustLineAsset& asset, const char* field)
{
    cereal_override(ar, asset.liquidityPoolID(), field);
}

void
cerealPoolAsset(cereal::JSONOutputArchive& ar,
                const caiz::ChangeTrustAsset& asset, const char* field)
{
    auto const& cp = asset.liquidityPool().constantProduct();

    ar.setNextName(field);
    ar.startNode();

    xdr::archive(ar, cp.assetA, "assetA");
    xdr::archive(ar, cp.assetB, "assetB");

    xdr::archive(ar, cp.fee, "fee");
    ar.finishNode();
}
