// Copyright 2019 Caiz Development Foundation and contributors. Licensed
// under the Apache License, Version 2.0. See the COPYING file at the root
// of this distribution or at http://www.apache.org/licenses/LICENSE-2.0

#include "transactions/PathPaymentOpFrameBase.h"
#include "ledger/LedgerTxn.h"
#include "ledger/LedgerTxnEntry.h"
#include "ledger/LedgerTxnHeader.h"
#include "ledger/TrustLineWrapper.h"
#include "transactions/TransactionUtils.h"
#include "util/GlobalChecks.h"
#include "util/ProtocolVersion.h"
#include "util/XDROperators.h"

namespace caiz
{

PathPaymentOpFrameBase::PathPaymentOpFrameBase(Operation const& op,
                                               OperationResult& res,
                                               TransactionFrame& parentTx)
    : OperationFrame(op, res, parentTx)
{
}

AccountID
PathPaymentOpFrameBase::getDestID() const
{
    return toAccountID(getDestMuxedAccount());
}

void
PathPaymentOpFrameBase::insertLedgerKeysToPrefetch(
    UnorderedSet<LedgerKey>& keys) const
{
    auto destID = getDestID();
    keys.emplace(accountKey(destID));

    if (getDestAsset().type() != ASSET_TYPE_NATIVE)
    {
        keys.emplace(trustlineKey(destID, getDestAsset()));
    }
    if (getSourceAsset().type() != ASSET_TYPE_NATIVE)
    {
        keys.emplace(trustlineKey(getSourceID(), getSourceAsset()));
    }
}

bool
PathPaymentOpFrameBase::isDexOperation() const
{
    auto const& src = getSourceAsset();
    auto const& dest = getDestAsset();
    return !getPath().empty() || !(src == dest);
}

bool
PathPaymentOpFrameBase::checkIssuer(AbstractLedgerTxn& ltx, Asset const& asset)
{
    if (asset.type() != ASSET_TYPE_NATIVE)
    {
        uint32_t ledgerVersion = ltx.loadHeader().current().ledgerVersion;
        if (protocolVersionIsBefore(ledgerVersion, ProtocolVersion::V_13) &&
            !caiz::loadAccountWithoutRecord(ltx, getIssuer(asset)))
        {
            setResultNoIssuer(asset);
            return false;
        }
    }
    return true;
}

bool
PathPaymentOpFrameBase::convert(AbstractLedgerTxn& ltx,
                                int64_t maxOffersToCross,
                                Asset const& sendAsset, int64_t maxSend,
                                int64_t& amountSend, Asset const& recvAsset,
                                int64_t maxRecv, int64_t& amountRecv,
                                RoundingType round,
                                std::vector<ClaimAtom>& offerTrail)
{
    releaseAssertOrThrow(offerTrail.empty());
    releaseAssertOrThrow(!(sendAsset == recvAsset));

    // sendAsset -> recvAsset
    ConvertResult r = convertWithOffersAndPools(
        ltx, sendAsset, maxSend, amountSend, recvAsset, maxRecv, amountRecv,
        round,
        [this](LedgerTxnEntry const& o) {
            auto const& offer = o.current().data.offer();
            if (offer.sellerID == getSourceID())
            {
                // we are crossing our own offer
                return OfferFilterResult::eStopCrossSelf;
            }
            return OfferFilterResult::eKeep;
        },
        offerTrail, maxOffersToCross);

    if (amountSend < 0 || amountRecv < 0)
    {
        throw std::runtime_error("amount transferred out of bounds");
    }

    switch (r)
    {
    case ConvertResult::eFilterStopCrossSelf:
        setResultOfferCrossSelf();
        return false;
    case ConvertResult::eOK:
        if (checkTransfer(maxSend, amountSend, maxRecv, amountRecv))
        {
            break;
        }
    // fall through
    case ConvertResult::ePartial:
        setResultTooFewOffers();
        return false;
    case ConvertResult::eCrossedTooMany:
        mResult.code(opEXCEEDED_WORK_LIMIT);
        return false;
    default:
        throw std::runtime_error("unexpected convert result");
    }

    return true;
}

bool
PathPaymentOpFrameBase::shouldBypassIssuerCheck(
    std::vector<Asset> const& path) const
{
    // if the payment doesn't involve intermediate accounts
    // and the destination is the issuer we don't bother
    // checking if the destination account even exist
    // so that it's always possible to send credits back to its issuer
    return (getDestAsset().type() != ASSET_TYPE_NATIVE) && (path.size() == 0) &&
           (getSourceAsset() == getDestAsset()) &&
           (getIssuer(getDestAsset()) == getDestID());
}

bool
PathPaymentOpFrameBase::updateSourceBalance(AbstractLedgerTxn& ltx,
                                            int64_t amount,
                                            bool bypassIssuerCheck,
                                            bool doesSourceAccountExist)
{
    auto const& asset = getSourceAsset();

    if (asset.type() == ASSET_TYPE_NATIVE)
    {
        auto header = ltx.loadHeader();
        LedgerTxnEntry sourceAccount;
        if (protocolVersionStartsFrom(header.current().ledgerVersion,
                                      ProtocolVersion::V_8))
        {
            sourceAccount = caiz::loadAccount(ltx, getSourceID());
            if (!sourceAccount)
            {
                setResultMalformed();
                return false;
            }
        }
        else
        {
            sourceAccount = loadSourceAccount(ltx, header);
        }

        if (amount > getAvailableBalance(header, sourceAccount))
        { // they don't have enough to send
            setResultUnderfunded();
            return false;
        }

        if (!doesSourceAccountExist)
        {
            throw std::runtime_error("modifying account that does not exist");
        }

        auto ok = addBalance(header, sourceAccount, -amount);
        releaseAssertOrThrow(ok);
    }
    else
    {
        if (!bypassIssuerCheck && !checkIssuer(ltx, asset))
        {
            return false;
        }

        auto sourceLine = loadTrustLine(ltx, getSourceID(), asset);
        if (!sourceLine)
        {
            setResultSourceNoTrust();
            return false;
        }

        if (!sourceLine.isAuthorized())
        {
            setResultSourceNotAuthorized();
            return false;
        }

        if (!sourceLine.addBalance(ltx.loadHeader(), -amount))
        {
            setResultUnderfunded();
            return false;
        }
    }

    return true;
}

bool
PathPaymentOpFrameBase::updateDestBalance(AbstractLedgerTxn& ltx,
                                          int64_t amount,
                                          bool bypassIssuerCheck)
{
    auto destID = getDestID();
    auto const& asset = getDestAsset();

    if (asset.type() == ASSET_TYPE_NATIVE)
    {
        auto destination = caiz::loadAccount(ltx, destID);
        if (!addBalance(ltx.loadHeader(), destination, amount))
        {
            if (protocolVersionStartsFrom(
                    ltx.loadHeader().current().ledgerVersion,
                    ProtocolVersion::V_11))
            {
                setResultLineFull();
            }
            else
            {
                setResultMalformed();
            }
            return false;
        }
    }
    else
    {
        if (!bypassIssuerCheck && !checkIssuer(ltx, asset))
        {
            return false;
        }

        auto destLine = caiz::loadTrustLine(ltx, destID, asset);
        if (!destLine)
        {
            setResultDestNoTrust();
            return false;
        }

        if (!destLine.isAuthorized())
        {
            setResultDestNotAuthorized();
            return false;
        }

        if (!destLine.addBalance(ltx.loadHeader(), amount))
        {
            setResultLineFull();
            return false;
        }
    }

    return true;
}
}
