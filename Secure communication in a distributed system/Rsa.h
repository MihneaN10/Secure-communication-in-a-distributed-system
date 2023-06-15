#pragma once
#include "BigIntegerLibrary.hh"
#include <iostream>
using namespace std;
BigInteger eficientPow(BigInteger n, BigInteger p, BigInteger m);
BigInteger rsaEncrypt(string sM, string sE, string sN);
string rsaDecrypt(BigInteger M, string sD, string sN);