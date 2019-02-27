#include <init.h>
#include <rpc/server.h>
#include <pow.h>
#include <chainparams.h>
#include <validation.h>
#include <rpc/blockchain.h>
#include <wallet/rpcwallet.h>
#include <base58.h>
#include <miner.h>
#include <timedata.h>
#include <util.h>
#include <wallet/wallet.h>
#include <core_io.h>

const int DAY = 24 * 60 * 60;

double CalcMintingProbability(uint32_t nBits, int timeOffset, CAmount nValue, int64_t nTime) {
    int64_t nTimeWeight = std::min((GetAdjustedTime() - nTime) + timeOffset, Params().GetConsensus().nStakeMaxAge) - Params().GetConsensus().nStakeMinAge;
    arith_uint256 bnCoinDayWeight = arith_uint256(static_cast<uint64_t >(nValue)) * nTimeWeight / COIN / DAY;

    arith_uint256 bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);

    const double targetLimit = (~arith_uint256(0)).getdouble();
    return (bnCoinDayWeight * bnTargetPerCoinDay).getdouble() / targetLimit;
}

double CalculateMintingProbabilityWithinPeriod(uint32_t nBits, int minutes, CAmount nValue, int64_t nTime)
{
    double prob = 1;
    double p;
    int d = minutes / (60 * 24); // Number of full days
    int m = minutes % (60 * 24); // Number of minutes in the last day
    int timeOffset = DAY;

    // Probabilities for the first d days
    for(int i = 0; i < d; i++, timeOffset += DAY) {
        p = pow(1 - CalcMintingProbability(nBits, timeOffset, nValue, nTime), DAY);
        prob *= p;
    }

    // Probability for the m minutes of the last day
    p = pow(1 - CalcMintingProbability(nBits, timeOffset, nValue, nTime), 60 * m);
    prob *= p;

    prob = 1 - prob;
    return prob;
}


UniValue listminting(const JSONRPCRequest& request)
{
    CWallet * const pwallet = GetWalletForJSONRPCRequest(request);
    if (!EnsureWalletIsAvailable(pwallet, request.fHelp)) {
        return NullUniValue;
    }

    if (request.fHelp || request.params.size() > 4)
        throw std::runtime_error(
                "listminting count skip minweight maxweight\n"
                "1. count          (numeric, optional, default=0) The number of outputs to return (0 - all)\n"
                "2. skip           (numeric, optional, default=0) The number of outputs to skip\n"
                "3. minweight      (numeric, optional, default=0) Min output weight\n"
                "4. maxweight      (numeric, optional, default=0) Max output weight (0 - unlimited)\n"
                "Return all mintable outputs and provide details for each of them.");

    int64_t nCount = 0;
    if (!request.params[0].isNull()) {
        nCount = request.params[0].get_int64();
        if (nCount < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    }

    int64_t nSkip = 0;
    if (!request.params[1].isNull()) {
        nSkip = request.params[1].get_int64();
        if (nSkip < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative skip");
    }

    uint64_t nMinWeight = 0;
    if (!request.params[2].isNull()) {
        int64_t minWeight = request.params[2].get_int64();
        if (minWeight < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative minweight");
        nMinWeight = minWeight;
    }

    uint64_t nMaxWeight = 0;
    if (!request.params[3].isNull()) {
        int64_t maxWeight = request.params[3].get_int64();
        if (maxWeight < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative maxweight");
        nMaxWeight = maxWeight;
    }

    LOCK2(cs_main, pwallet->cs_wallet);
    const CBlockIndex *p = GetLastBlockIndex(chainActive.Tip(), Params().GetConsensus(), true);
    uint32_t nBits = (p == nullptr) ? UintToArith256(Params().GetConsensus().nInitialHashTargetPoS).GetCompact() : p->nBits;

    UniValue ret(UniValue::VARR);

    int64_t minAge = Params().GetConsensus().nStakeMinAge / DAY;

    std::vector<COutput> vCoins;
    pwallet->AvailableCoins(vCoins, true, nullptr, 0, 0, MAX_MONEY, MAX_MONEY, 0, 1);

    for (auto &out: vCoins) {
        if (nSkip != 0) {
            --nSkip;
            continue;
        }

        if (nCount != 0 && ret.size() >= (size_t)nCount) {
            break;
        }

        if (nCount != 0 && ret.size() >= (size_t)nCount) {
            break;
        }

        const CBlockIndex *pindex = nullptr;
        out.tx->GetDepthInMainChain(pindex);

        if (!pindex)
            continue;

        uint64_t nTime = pindex->nTime;
        CAmount nValue = out.tx->tx->vout[out.i].nValue;

        int64_t nDayWeight = (std::min((GetAdjustedTime() - (uint32_t)nTime), Params().GetConsensus().nStakeMaxAge) - Params().GetConsensus().nStakeMinAge) / DAY;
        int64_t coinAge = std::max(nValue * nDayWeight / COIN, (int64_t)0);

        if (coinAge < nMinWeight) {
            continue;
        }

        if (nMaxWeight != 0 && coinAge > nMaxWeight) {
            continue;
        }

        CTxDestination address;
        const CScript& scriptPubKey = out.tx->tx->vout[out.i].scriptPubKey;
        ExtractDestination(scriptPubKey, address);

        std::string status = "immature";
        int64_t attempts = 0;
        if (((GetAdjustedTime() - nTime) / DAY) >= minAge) {
            status = "mature";
            attempts = GetAdjustedTime() - nTime - Params().GetConsensus().nStakeMinAge;
        }

        UniValue obj(UniValue::VOBJ);
        obj.push_back(Pair("address",                   EncodeDestination(address)));
        obj.push_back(Pair("txid",                      out.tx->GetHash().GetHex()));
        obj.push_back(Pair("vout",                      out.i));
        obj.push_back(Pair("time",                      nTime));
        obj.push_back(Pair("amount",                    ValueFromAmount(nValue)));
        obj.push_back(Pair("status",                    status));
        obj.push_back(Pair("age-in-day",                ((GetAdjustedTime() - nTime) / DAY)));
        obj.push_back(Pair("coin-day-weight",           coinAge));
        obj.push_back(Pair("minting-probability-10min", CalculateMintingProbabilityWithinPeriod(nBits, 10, nValue, nTime)));
        obj.push_back(Pair("minting-probability-24h",   CalculateMintingProbabilityWithinPeriod(nBits, 60*24, nValue, nTime)));
        obj.push_back(Pair("minting-probability-30d",   CalculateMintingProbabilityWithinPeriod(nBits, 60*24*30, nValue, nTime)));
        obj.push_back(Pair("minting-probability-90d",   CalculateMintingProbabilityWithinPeriod(nBits, 60*24*90, nValue, nTime)));
        obj.push_back(Pair("attempts",                  attempts));
        ret.push_back(obj);

    }

    return ret;
}

static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         argNames
  //  --------------------- ------------------------  -----------------------  ----------
    { "minting",            "listminting",            &listminting,            {"count", "skip", "minweight", "maxweight"} },
};

void RegisterMintingRPCCommands(CRPCTable &t)
{
    for (auto &command: commands)
        t.appendCommand(command.name, &command);
}
