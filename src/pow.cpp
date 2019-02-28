// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Taler Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util.h>

//pos: find last block index up to pindex
const CBlockIndex *GetLastBlockIndex(const CBlockIndex *pindex, const Consensus::Params &params, bool fProofOfStake) {
    const int32_t TLRHeight = params.TLRHeight;

    if (fProofOfStake) {
        while (pindex && pindex->pprev && !pindex->IsProofOfStake()) {
            if (pindex->nHeight <= TLRHeight)
                return nullptr;
            pindex = pindex->pprev;
        }
    } else {
        while (pindex && pindex->pprev && pindex->IsProofOfStake())
            pindex = pindex->pprev;
    }

    return pindex;
}

uint32_t GetNextWorkRequired(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params,
                             bool fProofOfStake) {
    assert(pindexLast != nullptr);
    return fProofOfStake ?
           GetNextWorkRequiredForPos(pindexLast, pblock, params) :
           GetNextWorkRequiredForPow(pindexLast, pblock, params);
}

unsigned int
GetNextWorkRequiredForPow(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    int nNextHeight = pindexLast->nHeight + 1;

    if (nNextHeight < params.nLyra2ZHeight) {
        return GetNextWorkRequiredBTC(pindexLast, pblock, params);
    } else if (nNextHeight < params.nLyra2ZHeight + params.nPowAveragingWindowv1) {
        return UintToArith256(params.powLimit).GetCompact();
    } else if (nNextHeight < params.nNewDiffAdjustmentAlgorithmHeight) {
        return DarkGravityWaveOld(pindexLast, params);
    } else if (nNextHeight < params.nNewDiffAdjustmentAlgorithmHeight + params.nPowAveragingWindowv2) {
        return 0x1c08b5b1;  // 29.39
    }

    return DarkGravityWave(pindexLast, params);
}

unsigned int DarkGravityWave(const CBlockIndex *pindexLast, const Consensus::Params &params) {
    /* current difficulty formula, dash - DarkGravity v3, written by Evan Duffield - evan@dash.org */
    const CBlockIndex *pBlockLastSolved = GetLastBlockIndex(pindexLast, params, false);
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = params.nPowAveragingWindowv2;
    int64_t nPastBlocksMax = params.nPowAveragingWindowv2;
    int64_t CountBlocks = 0;
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;

    if (pBlockLastSolved == nullptr || pBlockLastSolved->nHeight == 0 || pBlockLastSolved->nHeight < PastBlocksMin) {
        return UintToArith256(params.powLimit).GetCompact();
    }

    const CBlockIndex *pBlockReading = pBlockLastSolved;

    for (unsigned int i = 1; pBlockReading && pBlockReading->nHeight > 0; ++i) {
        if (nPastBlocksMax > 0 && i > nPastBlocksMax) {
            break;
        }

        CountBlocks++;

        if (CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) {
                PastDifficultyAverage.SetCompact(pBlockReading->nBits);
            } else {
                PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) +
                                         (arith_uint256().SetCompact(pBlockReading->nBits))) / (CountBlocks + 1);
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if (LastBlockTime > 0) {
            int64_t Diff = (LastBlockTime - pBlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = pBlockReading->GetBlockTime();

        pBlockReading = GetLastBlockIndex(pBlockReading->pprev, params, false);
    }

    arith_uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * params.getPowTargetSpacing(pindexLast->nHeight);

    if (nActualTimespan < _nTargetTimespan / 3)
        nActualTimespan = _nTargetTimespan / 3;
    if (nActualTimespan > _nTargetTimespan * 3)
        nActualTimespan = _nTargetTimespan * 3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    if (bnNew > UintToArith256(params.powLimit)) {
        bnNew = UintToArith256(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int DarkGravityWaveOld(const CBlockIndex *pindexLast, const Consensus::Params &params) {
    /* current difficulty formula, dash - DarkGravity v3, written by Evan Duffield - evan@dash.org */
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 24;
    int64_t PastBlocksMax = 24;
    int64_t CountBlocks = 0;
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;

    if (BlockLastSolved == nullptr || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) {
        return UintToArith256(params.powLimit).GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if (CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else {
                PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) +
                                         (arith_uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1);
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if (LastBlockTime > 0) {
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == nullptr) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * params.getPowTargetSpacing(pindexLast->nHeight);

    if (nActualTimespan < _nTargetTimespan / 3)
        nActualTimespan = _nTargetTimespan / 3;
    if (nActualTimespan > _nTargetTimespan * 3)
        nActualTimespan = _nTargetTimespan * 3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    if (bnNew > UintToArith256(params.powLimit)) {
        bnNew = UintToArith256(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int
GetNextWorkRequiredBTC(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimitLegacy).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight + 1) % params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1) != 0) {
        if (params.fPowAllowMinDifficultyBlocks) {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() +
                                         params.getPowTargetSpacing(pindexLast->nHeight) * 2)
                return nProofOfWorkLimit;
            else {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex *pindex = pindexLast;
                while (pindex->pprev &&
                       pindex->nHeight % params.DifficultyAdjustmentIntervalPow(pindex->nHeight) != 0 &&
                       pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    // Taler: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int64_t blockstogoback = params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1) - 1;
    if ((pindexLast->nHeight + 1) != params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1))
        blockstogoback = params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1);

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex *pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int
CalculateNextWorkRequired(const CBlockIndex *pindexLast, int64_t nFirstBlockTime, const Consensus::Params &params) {
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan / 4)
        nActualTimespan = params.nPowTargetTimespan / 4;
    if (nActualTimespan > params.nPowTargetTimespan * 4)
        nActualTimespan = params.nPowTargetTimespan * 4;

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    // Taler: intermediate uint256 can overflow by 1 bit
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimitLegacy);
    bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}


uint32_t
GetNextWorkRequiredForPos(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    assert((pindexLast->nHeight + 1) > params.TLRHeight + params.TLRInitLim);

    const CBlockIndex *pindexPrev = GetLastBlockIndex(pindexLast, params, true);
    if (pindexPrev == nullptr)
        return UintToArith256(params.nInitialHashTargetPoS).GetCompact();

    const CBlockIndex *pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, params, true);
    if (pindexPrevPrev == nullptr || pindexPrevPrev->pprev == nullptr)
        return UintToArith256(params.nInitialHashTargetPoS).GetCompact();

    const int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    const int64_t nInterval = params.DifficultyAdjustmentIntervalPos();
    arith_uint256 nProofOfWorkLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    bnNew *= ((nInterval - 1) * params.nPosTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * params.nPosTargetSpacing);

    if (bnNew > nProofOfWorkLimit)
        bnNew = nProofOfWorkLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, int nHeight, unsigned int nBits, const Consensus::Params &params) {
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;
    const uint256 &powLimit = nHeight < params.nLyra2ZHeight ? params.powLimitLegacy : params.powLimit;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(powLimit))
        return false;

    // Check proof of work matches claimed amount
    return !(UintToArith256(hash) > bnTarget);
}
