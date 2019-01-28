// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Taler Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include "crypto/scrypt.h"
#include "crypto/Lyra2Z/Lyra2Z.h"
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>
#include <chainparams.h>
#include <arith_uint256.h>
#ifndef NO_UTIL_LOG
#include <util.h>
#else
#define LogPrint(...)
#endif

uint256 CBlockHeader::GetHash() const
{
    CHashWriter writer(SER_GETHASH, PROTOCOL_VERSION | SERIALIZE_BLOCK_LEGACY);
    ::Serialize(writer, *this);
    return writer.GetHash();
}

uint256 CBlockHeader::GetPoWHash(int nHeight, const Consensus::Params& params) const
{
    uint256 thash;

    if (nHeight >= params.nLyra2ZHeight)
        lyra2z_hash(BEGIN(nVersion), BEGIN(thash));
    else
        scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));

    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}

unsigned int CBlock::GetStakeEntropyBit(int32_t height) const
{
    unsigned int nEntropyBit = UintToArith256(GetHash()).GetLow64() & 1llu;// last bit of block hash
    LogPrint(BCLog::STAKEMODIFIER, "GetStakeEntropyBit(v0.3.5+): nTime=%u hashBlock=%s entropybit=%d\n", nTime, GetHash().ToString(), nEntropyBit);
    return nEntropyBit;
}
