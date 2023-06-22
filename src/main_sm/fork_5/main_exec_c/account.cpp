#include "account.hpp"
#include "scalar.hpp"

namespace fork_5
{

void Account::GetZeroKey (Goldilocks::Element (&zeroKey)[4])
{
    // TODO: Pre-calculate this value
    Goldilocks::Element Kin0[12];
    Kin0[0] = fr.zero();
    Kin0[1] = fr.zero();
    Kin0[2] = fr.zero();
    Kin0[3] = fr.zero();
    Kin0[4] = fr.zero();
    Kin0[5] = fr.zero();
    Kin0[6] = fr.zero();
    Kin0[7] = fr.zero();
    Kin0[8] = fr.zero();
    Kin0[9] = fr.zero();
    Kin0[10] = fr.zero();
    Kin0[11] = fr.zero();

    // Call poseidon and get the hash key
    poseidon.hash(zeroKey, Kin0);
}

void Account::GetBalanceKey (Goldilocks::Element (&balanceKey)[4])
{
    Goldilocks::Element Kin1[12];

    scalar2fea(fr, publicKey, Kin1[0], Kin1[1], Kin1[2], Kin1[3], Kin1[4], Kin1[5], Kin1[6], Kin1[7]); // TODO: Reuse this result
    if (!fr.isZero(Kin1[5]) || !fr.isZero(Kin1[6]) || !fr.isZero(Kin1[7]))
    {
        zklog.error("Accoung::GetBalanceKey() found non-zero field elements 5, 6 or 7");
        exitProcess();
    }

    Kin1[8] = zeroKey[0];
    Kin1[9] = zeroKey[1];
    Kin1[10] = zeroKey[2];
    Kin1[11] = zeroKey[3];

    // Call poseidon and get the hash key
    poseidon.hash(balanceKey, Kin1);
}

void Account::GetNonceKey (Goldilocks::Element (&nonceKey)[4])
{
    Goldilocks::Element Kin1[12];

    scalar2fea(fr, publicKey, Kin1[0], Kin1[1], Kin1[2], Kin1[3], Kin1[4], Kin1[5], Kin1[6], Kin1[7]);
    if (!fr.isZero(Kin1[5]) || !fr.isZero(Kin1[6]) || !fr.isZero(Kin1[7]))
    {
        zklog.error("Accoung::GetNonceKey() found non-zero field elements 5, 6 or 7");
        exitProcess();
    }

    Kin1[6] = fr.one();

    Kin1[8] = zeroKey[0];
    Kin1[9] = zeroKey[1];
    Kin1[10] = zeroKey[2];
    Kin1[11] = zeroKey[3];

    // Call poseidon and get the hash key
    poseidon.hash(nonceKey, Kin1);
}

zkresult Account::GetBalance (Goldilocks::Element (&oldRoot)[4], mpz_class &balance)
{
    zkresult zkResult = hashDB.get(oldRoot, balanceKey, balance, /*&smtGetResult*/NULL, NULL /*proverRequest.dbReadLog*/);
    if (zkResult != ZKR_SUCCESS)
    {
        zklog.error("Account::GetBalance() failed calling hashDB.get() result=" + zkresult2string(zkResult));
        return zkResult;
    }
    return ZKR_SUCCESS;
}

zkresult Account::SetBalance (mpz_class &balance)
{
    return ZKR_SUCCESS;
}

zkresult Account::GetNonce (uint64_t &nonce)
{
    nonce = 0;
    return ZKR_SUCCESS;
}

zkresult Account::SetNonce (uint64_t &nonce)
{
    return ZKR_SUCCESS;
}

}