#include "Rsa.h"


BigInteger eficientPow(BigInteger n, BigInteger p, BigInteger m)
{
	BigInteger Main = n, Extra = 1;
	if (m > 0) {
		while (Extra != 0 || p != 1)
		{
			if (p % 2 == 1 && p > 1)
				Extra = (Extra * Main) % m;
			if (p > 1)
			{
				Main = (Main * Main) % m;
				p = p / 2;
			}
			else
			{
				Main = (Main * Extra) % m;
				Extra = 0;
			}
		}
	}
	else
	{
		while (Extra != 0 || p != 1)
		{
			if (p % 2 == 1 && p > 1)
				Extra = (Extra * Main);
			if (p > 1)
			{
				Main = (Main * Main);
				p = p / 2;
			}
			else
			{
				Main = (Main * Extra);
				Extra = 0;
			}
		}
	}
	return Main;
}

BigInteger rsaEncrypt(string sM, string sE, string sN)
{
	BigInteger e = stringToBigInteger(sE), N = stringToBigInteger(sN);
	BigInteger M = 0;
	for (string::iterator it = sM.begin(); it != sM.end(); ++it)
	{
		M = M * 256 + *it;
	}
	BigInteger C = eficientPow(M, e, N);
	return C;
}

string rsaDecrypt(BigInteger M, string sD, string sN)
{
	BigInteger d = stringToBigInteger(sD), N = stringToBigInteger(sN);
	M = eficientPow(M, d, N);
	string s, ss = "";
	while (M != 0)
	{
		s = bigIntegerToString(M % 256);
		M = M / 256;
		ss = ((char)stoi(s)) + ss;
	}
	return ss;
}

/*int main5() {
	string M = "";
	cout << "\nIntroduce mesajul: ";
	char c;
	cin.get(c);
	while (c != '\n')
	{
		M = M + c;
		cin.get(c);
	}
	BigInteger MM = rsaEncrypt(M, "65537", "7243867490989233467384967162493995507079997637730128226620729167848417921603363941441104994233585312212872796728534164766298500253607968097500254417259297");
	cout << rsaDecrypt(MM, "4542601372425889452409896401180680033415837510344275903041926352140535571387887052951323950781974357961693345053465308015087386170660785695019453523218433", "7243867490989233467384967162493995507079997637730128226620729167848417921603363941441104994233585312212872796728534164766298500253607968097500254417259297");
	return 0;
}*/