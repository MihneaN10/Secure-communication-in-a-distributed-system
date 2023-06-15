#pragma once
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

//RFC 8017 section 4
using namespace std;

static const size_t BLOCK_INTS = 16;
static const size_t BLOCK_BYTES = BLOCK_INTS * 4;

inline static uint32_t rotl(const uint32_t value, const size_t bits);
inline static void transform(uint32_t H[], uint32_t block[BLOCK_INTS], uint64_t& transforms);
inline static void buffer_to_block(const string& buffer, uint32_t block[BLOCK_INTS]);

class SHA1
{
	uint32_t H[5];
	string buffer;
	uint64_t transforms;
public:
	SHA1() {
		H[0] = 0x67452301;
		H[1] = 0xefcdab89;
		H[2] = 0x98badcfe;
		H[3] = 0x10325476;
		H[4] = 0xc3d2e1f0;
		buffer = "";
		transforms = 0;
	}

	void update(const string& s) {
		istringstream is(s);
		update(is);
	}

	void update(std::istream& is)
	{
		while (true)
		{
			char sbuf[BLOCK_BYTES];
			is.read(sbuf, BLOCK_BYTES - buffer.size());
			buffer.append(sbuf, (std::size_t)is.gcount());
			if (buffer.size() != BLOCK_BYTES)
			{
				return;
			}
			uint32_t block[BLOCK_INTS];
			buffer_to_block(buffer, block);
			transform(H, block, transforms);
			buffer.clear();
		}
	}

	string final() {
		uint64_t total_bits = (transforms * BLOCK_BYTES + buffer.size()) * 8;

		buffer += (char)0x80;
		size_t orig_size = buffer.size();
		while (buffer.size() < BLOCK_BYTES)
		{
			buffer += (char)0x00;
		}

		uint32_t block[BLOCK_INTS];
		buffer_to_block(buffer, block);

		if (orig_size > BLOCK_BYTES - 8)
		{
			transform(H, block, transforms);
			for (size_t i = 0; i < BLOCK_INTS - 2; i++)
			{
				block[i] = 0;
			}
		}

		block[BLOCK_INTS - 1] = (uint32_t)total_bits;
		block[BLOCK_INTS - 2] = (uint32_t)(total_bits >> 32);
		transform(H, block, transforms);

		std::ostringstream result;
		for (size_t i = 0; i < sizeof(H) / sizeof(H[0]); i++)
		{
			result << std::hex << setfill('0') << setw(8);
			result << H[i];
		}

		H[0] = 0x67452301;
		H[1] = 0xefcdab89;
		H[2] = 0x98badcfe;
		H[3] = 0x10325476;
		H[4] = 0xc3d2e1f0;
		buffer = "";
		transforms = 0;

		return result.str();
	}

};

inline static uint32_t rotl(const uint32_t value, const size_t bits)
{
	return (value << bits) | (value >> (32 - bits));
}

inline static void transform(uint32_t H[], uint32_t block[BLOCK_INTS], uint64_t& transforms)
{
	uint32_t W[80];
	memset(W, 0, 80);
	for (int t = 0; t <= 79; t++)
	{
		if (t <= 15)
		{
			W[t] = block[t];
		}
		else
		{
			W[t] = rotl(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
		}
	}

	uint32_t a = H[0];
	uint32_t b = H[1];
	uint32_t c = H[2];
	uint32_t d = H[3];
	uint32_t e = H[4];

	for (int t = 0; t <= 79; t++)
	{
		uint32_t T = rotl(a, 5);
		if (t <= 19)
		{
			T = T + ((b & c) ^ ((~b) & d)) + e + 0x5a827999 + W[t];
		}
		if (t <= 39 && t >= 20)
		{
			T = T + (b ^ c ^ d) + e + 0x6ed9eba1 + W[t];
		}
		if (t <= 59 && t >= 40)
		{
			T = T + ((b & c) ^ (b & d) ^ (c & d)) + e + 0x8f1bbcdc + W[t];
		}
		if (t >= 60)
		{
			T = T + (b ^ c ^ d) + e + 0xca62c1d6 + W[t];
		}
		e = d;
		d = c;
		c = rotl(b, 30);
		b = a;
		a = T;
	}

	H[0] += a;
	H[1] += b;
	H[2] += c;
	H[3] += d;
	H[4] += e;

	transforms++;
}

inline static void buffer_to_block(const string& buffer, uint32_t block[BLOCK_INTS])
{
	for (size_t i = 0; i < BLOCK_INTS; i++)
	{
		block[i] = (buffer[4 * i + 3] & 0xff)
			| (buffer[4 * i + 2] & 0xff) << 8
			| (buffer[4 * i + 1] & 0xff) << 16
			| (buffer[4 * i + 0] & 0xff) << 24;
	}
}

/*int main()
{
	SHA1 sha1;
	cout << "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\n";
	sha1.update("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
	cout << hex << sha1.final();
	return 0;
}*/