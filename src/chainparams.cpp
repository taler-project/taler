// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Taler Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <uint256.h>
#include <arith_uint256.h>
#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock
CreateGenesisBlock(const char *pszTimestamp, const CScript &genesisOutputScript, uint32_t nTime, uint32_t nNonce,
                   uint32_t nBits, int32_t nVersion, const CAmount &genesisReward) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4)
                                       << std::vector<unsigned char>((const unsigned char *) pszTimestamp,
                                                                     (const unsigned char *) pszTimestamp +
                                                                     strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=c079fd1ae86223e1522928776899d46e329da7919ca1e11be23643c67dd05d5f, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=985fae483ebbef9cde04a259282cb7465d52bf56824caf1a8132395e90488b12, nTime=1505338813, nBits=1e0ffff0, nNonce=725170, vtx=1)
 *   CTransaction(hash=985fae483e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(0000000000, 4294967295), coinbase 04ffff001d01043e54616c6572207065727368616a612062656c617275736b616a61206b727970746176616c697574612062792044656e6973204c2069205365726765204c20)
 *     CScriptWitness()
 *     CTxOut(nValue=50.00000000, scriptPubKey=4104f360606cf909ce34d4276ce40a)
 */
static CBlock
CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount &genesisReward) {
    const char *pszTimestamp = "\x54\x61\x6c\x65\x72\x20\x70\x65\x72\x73\x68\x61\x6a\x61\x20\x62\x65\x6c\x61\x72\x75\x73\x6b\x61\x6a\x61\x20\x6b\x72\x79\x70\x74\x61\x76\x61\x6c\x69\x75\x74\x61\x20\x62\x79\x20\x44\x65\x6e\x69\x73\x20\x4c\x20\x69\x20\x53\x65\x72\x67\x65\x20\x4c\x20";
    const CScript genesisOutputScript = CScript() << ParseHex(
            "04f360606cf909ce34d4276ce40a5dd6a844a4a72473086e0fc635f3c4195d77df513b7541dc5f6f6d01ec39e4b729893c6d42dd5e248379a32b5259f38f6bfbae")
                                                  << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout) {
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000 * 5;

        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0xc079fd1ae86223e1522928776899d46e329da7919ca1e11be23643c67dd05d5f");

        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimitLegacy = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.nPowTargetTimespan = 10 * 60;
        consensus.nPowTargetSpacingBegin = 5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4

        consensus.nPosTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPosTargetSpacing = 60 * 7 / 3;
        consensus.nStakeMinAge = 60 * 60 * 24 * 2;             // minimum age for coin age
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;             // stake age of full weight
        consensus.nStakeModifierInterval = 6 * 60 * 60;         // time to elapse before new modifier is computed
        consensus.nInitialHashTargetPoS = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.nLyra2ZHeight = 10000;
        consensus.nPowAveragingWindowv1 = 24;

        consensus.nNewDiffAdjustmentAlgorithmHeight = 250000;
        consensus.nPowAveragingWindowv2 = 120;

        consensus.TLRHeight = 130000;
        consensus.TLRInitLim = 300;
	consensus.POSLimitOkHeight = 730000;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1512086401; // December 01, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S(
                "0x000000000000000000000000000000000000000000000000014e7205088ed530"); //1310000

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S(
                "0xd3e3c9bd441e70bbb74addfedc2324ed8c394d8afc48d73ccb4b0320ea28c631"); //1310000

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x64;
        pchMessageStart[1] = 0xb1;
        pchMessageStart[2] = 0x73;
        pchMessageStart[3] = 0xd8;
        nDefaultPort = 23153;
        nPruneAfterHeight = 10000;

        genesis = CreateGenesisBlock(1505338813, 725170, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0xc079fd1ae86223e1522928776899d46e329da7919ca1e11be23643c67dd05d5f"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x985fae483ebbef9cde04a259282cb7465d52bf56824caf1a8132395e90488b12"));


        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.emplace_back("dnsseed.talercrypto.com");
        vSeeds.emplace_back("dnsseed.mikalair.me");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 65);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 50);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 193);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "tlr";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
                {
                        {1024, uint256S("0x8d769df2ac2cabb10038ba2a0ffd269e5cf93701c27256a27fb580a25106a170")},
                        {2048, uint256S("0xc4838cab89b16915d813f424198a999af82b3dce2afed5d82cab1fe9df08d701")},
                        {6602, uint256S("0xf225e2f57a5e90539a4d74b3bf1ed906a8146c64addff0f5279473fb6c5e9f0e")},
                        {20000, uint256S("0x82ad64f451be0375683efbdc7d94c1b970431b02a6a3e5057dd6cd0fb2022e70")},
                        {30000, uint256S("0x0c14a678cb406be311c75938702e4ac567146d43bbd6d4f44e8d7a879a849424")},
                        {70000, uint256S("0x8c25e55d05da7fd4e61383fcdd1232e8c8ddd85b220caefc10ac6c71bdf35b3e")},
                        {100000, uint256S("0xa6c3e93e8ac7b4af077a78c6ce27a1b2b8b7793a7737403bcb9e6f420a928547")},
                        {130001, uint256S("0x485aa2ba84c7b3b3292f655eb87baad17f81689fc851ff1bbc4461abc6aee61d")},
                        {145000, uint256S("0x01b12183eef6102c765d1f37ea2129e91649f849fd2b18239e7d2f7276927930")},
                        {500000, uint256S("0xdbd781e1a5c96e38c6f37e85ddc79f808696ff38a107334b1d2aa0d1f3c54886")},
			{728634, uint256S("0x33e82f201a0b4074af53080d26c4092e6284bdead512b8c189b9c53526078d77")},
			{752000, uint256S("0xa3d0b940f1c9254e70304652d30a516cb5df4e4d5c3888e17f0043e49f1ac7e1")},
			{1300000, uint256S("0x7275e6d31dad0a6bef88fb1f23f2930f3509c0810996805d3be9b047b7424395")},

                }
        };

        chainTxData = ChainTxData{
                // Data as of block e4a46ba89678d13821d9a00f6526c270a096e92e1f8235d8d6c5a71b19bf3299 (height 1311956).
                1603092137, // * UNIX timestamp of last known number of transactions
                1788348,       // * total number of transactions between genesis and that timestamp
                //   (the tx=... number in the SetBestChain debug.log lines)
                0.03459533310856033  // * estimated number of transactions per second after that timestamp //
        };
    }
};

/**
 * Testnet (v3)
 */

class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 100; //210000 * 5;


        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimitLegacy = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");


        consensus.nPowTargetTimespan = 10 * 60;
        consensus.nPowTargetSpacingBegin = 5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 2 * 4 / 3;//6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 2 * 4;// 8064; // nPowTargetTimespan / nPowTargetSpacing * 4

        consensus.nPosTargetTimespan = 14 * 24 * 60 * 60 / 14; // two weeks
        consensus.nPosTargetSpacing = 60 * 7 / 3;
        consensus.nStakeMinAge = 60 * 60 * 24 * 2 / 60 / 10;         // minimum age for coin age
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90 / 60;             // stake age of full weight
        consensus.nStakeModifierInterval = 6 * 60 * 60 / 60 / 20;     // time to elapse before new modifier is computed
        consensus.nInitialHashTargetPoS = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.nLyra2ZHeight = 10;
        consensus.nPowAveragingWindowv1 = 24;

        consensus.nNewDiffAdjustmentAlgorithmHeight = 21000;
        consensus.nPowAveragingWindowv2 = 120;

        consensus.TLRHeight = 120;
        consensus.TLRInitLim = 50;
        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0; // December 01, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 99999999999; // January 31st, 2018

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00"); //0

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S(
                "0x0c14a678cb406be311c75938702e4ac567146d43bbd6d4f44e8d7a879a849424"); //30000

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfd;
        pchMessageStart[1] = 0xd2;
        pchMessageStart[2] = 0xc8;
        pchMessageStart[3] = 0x07;
        nDefaultPort = 18333;
        nPruneAfterHeight = 10000;

        genesis = CreateGenesisBlock(1317798646, 393879, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(genesis.hashMerkleRoot ==
               uint256S("0x985fae483ebbef9cde04a259282cb7465d52bf56824caf1a8132395e90488b12"));


        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.emplace_back("testseed.mikalair.me"); // only supports x9
        vSeeds.emplace_back("testseed.talercrypto.com"); // only supports x9

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tlt";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = true;

        checkpointData = {
                {
                }
        };

        chainTxData = ChainTxData{
                // Data as of block c206d8e1cd4b54bffff0e4a56b72a9c536da3580d82af209d6a4a4e6c6919ee4 (height 78423).
                0, // * UNIX timestamp of last known number of transactions
                0,       // * total number of transactions between genesis and that timestamp
                //   (the tx=... number in the SetBestChain debug.log lines)
                0.0  // * estimated number of transactions per second after that timestamp // 206710/(1533041886-1505338813=27703073)
        };

    }
};

/**
 * Regression test
 */

class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 210000;


        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimitLegacy = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.nPowTargetTimespan = 10 * 60;
        consensus.nPowTargetSpacingBegin = 5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4

        consensus.nPosTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPosTargetSpacing = 10 * 60;
        consensus.nStakeMinAge = 60 * 60 * 24 * 2;             // minimum age for coin age
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;             // stake age of full weight
        consensus.nStakeModifierInterval = 6 * 60 * 60;         // time to elapse before new modifier is computed
        consensus.nInitialHashTargetPoS = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.nLyra2ZHeight = 10;
        consensus.nPowAveragingWindowv1 = 24;

        consensus.TLRHeight = 120;
        consensus.TLRInitLim = 50;
        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0; // December 01, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 99999999999; // January 31st, 2018

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00"); //0

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00"); //30000

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 2, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0x530827f38f93b43ed12af0b3ad25a288dc02ed74d6d7857862df51fc56c416f9"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x97ddfbbae6be97fd6cdf3e7ca13232a3afff2353e29badfab7f73011edd4ced9"));


        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rgt";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = true;

        checkpointData = {
                {
                }
        };

        chainTxData = ChainTxData{
                // Data as of block c206d8e1cd4b54bffff0e4a56b72a9c536da3580d82af209d6a4a4e6c6919ee4 (height 78423).
                0, // * UNIX timestamp of last known number of transactions
                0,       // * total number of transactions between genesis and that timestamp
                //   (the tx=... number in the SetBestChain debug.log lines)
                0.0  // * estimated number of transactions per second after that timestamp // 206710/(1533041886-1505338813=27703073)
        };
    }
};


static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string &chain) {
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string &network) {
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout) {
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
