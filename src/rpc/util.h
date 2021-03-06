// Copyright (c) 2017 The Taler Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_UTIL_H
#define BITCOIN_RPC_UTIL_H

#include <string>
#include <vector>

class CKeyStore;
class CPubKey;
class CKeyID;
class CScript;

CPubKey HexToPubKey(const std::string& hex_in);
CPubKey AddrToPubKey(CKeyStore* const keystore, const std::string& addr_in);
CScript CreateMultisigRedeemscript(const int required, const std::vector<CPubKey>& pubkeys);
CScript CreateAtomicSwapRedeemscript(CKeyID initiator, CKeyID redeemer, int64_t locktime, int64_t secretSize, std::vector<unsigned char> secretHash);

#endif // BITCOIN_RPC_UTIL_H
