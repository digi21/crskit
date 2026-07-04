#include "pch.h"
#include "Inflate.h"

// A compact, self-contained raw-DEFLATE (RFC 1951) decoder, modelled on zlib's public-domain "puff"
// reference implementation. Correctness over speed: grid files are loaded once. No dynamic allocation.

namespace CrsKit::CoordinateTransformations::Algorithms
{
	namespace
	{
		constexpr int MAXBITS = 15;    // maximum bits in a code
		constexpr int MAXLCODES = 286; // maximum number of literal/length codes
		constexpr int MAXDCODES = 30;  // maximum number of distance codes
		constexpr int MAXCODES = MAXLCODES + MAXDCODES;
		constexpr int FIXLCODES = 288; // number of fixed literal/length codes

		struct State
		{
			unsigned char const* in; std::size_t inLen; std::size_t inPos;
			int bitBuf; int bitCnt;
			unsigned char* out; std::size_t outLen; std::size_t outPos;
		};

		// A canonical Huffman code described by the count of codes per length and the sorted symbols.
		struct Huffman { short* count; short* symbol; };

		// Return `need` bits from the stream (LSB first), or -1 if the input is exhausted.
		auto getBits(State& s, int need) -> int
		{
			long val = s.bitBuf;
			while (s.bitCnt < need)
			{
				if (s.inPos >= s.inLen) return -1;
				val |= static_cast<long>(s.in[s.inPos++]) << s.bitCnt;
				s.bitCnt += 8;
			}
			s.bitBuf = static_cast<int>(val >> need);
			s.bitCnt -= need;
			return static_cast<int>(val & ((1L << need) - 1));
		}

		// Decode one symbol using the given Huffman code; -1 on error.
		auto decode(State& s, Huffman const& h) -> int
		{
			int code = 0, first = 0, index = 0;
			for (int len = 1; len <= MAXBITS; ++len)
			{
				int const bit = getBits(s, 1);
				if (bit < 0) return -1;
				code |= bit;
				int const count = h.count[len];
				if (code - count < first) return h.symbol[index + (code - first)];
				index += count;
				first += count;
				first <<= 1;
				code <<= 1;
			}
			return -1;
		}

		// Build a Huffman code from a list of code lengths. Returns 0 for a complete code, a positive
		// value for an incomplete code, or a negative value for an over-subscribed (invalid) one.
		auto construct(Huffman& h, short const* length, int n) -> int
		{
			for (int len = 0; len <= MAXBITS; ++len) h.count[len] = 0;
			for (int sym = 0; sym < n; ++sym) h.count[length[sym]]++;
			if (h.count[0] == n) return 0;

			int left = 1;
			for (int len = 1; len <= MAXBITS; ++len)
			{
				left <<= 1;
				left -= h.count[len];
				if (left < 0) return left;
			}

			short offs[MAXBITS + 1];
			offs[1] = 0;
			for (int len = 1; len < MAXBITS; ++len) offs[len + 1] = static_cast<short>(offs[len] + h.count[len]);
			for (int sym = 0; sym < n; ++sym)
				if (length[sym] != 0) h.symbol[offs[length[sym]]++] = static_cast<short>(sym);

			return left;
		}

		// Length and distance base values / extra bits for codes 257..285 and 0..29.
		constexpr short lens[29] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258 };
		constexpr short lext[29] = { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0 };
		constexpr short dists[30] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };
		constexpr short dext[30] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };

		auto inflateCodes(State& s, Huffman const& lencode, Huffman const& distcode) -> int
		{
			int symbol{};
			do
			{
				symbol = decode(s, lencode);
				if (symbol < 0) return symbol;
				if (symbol < 256)
				{
					if (s.outPos >= s.outLen) return -1;
					s.out[s.outPos++] = static_cast<unsigned char>(symbol);
				}
				else if (symbol > 256)
				{
					symbol -= 257;
					if (symbol >= 29) return -1;
					int const len = lens[symbol] + getBits(s, lext[symbol]);
					symbol = decode(s, distcode);
					if (symbol < 0) return symbol;
					std::size_t const dist = static_cast<std::size_t>(dists[symbol]) + getBits(s, dext[symbol]);
					if (dist > s.outPos || s.outPos + len > s.outLen) return -1;
					for (int i = 0; i < len; ++i, ++s.outPos) s.out[s.outPos] = s.out[s.outPos - dist];
				}
			} while (symbol != 256);
			return 0;
		}

		auto inflateStored(State& s) -> int
		{
			s.bitBuf = 0; s.bitCnt = 0;               // stored blocks are byte-aligned
			if (s.inPos + 4 > s.inLen) return -1;
			int const len = s.in[s.inPos] | (s.in[s.inPos + 1] << 8);
			int const nlen = s.in[s.inPos + 2] | (s.in[s.inPos + 3] << 8);
			s.inPos += 4;
			if (len != (0xFFFF & ~nlen)) return -1;
			if (s.inPos + len > s.inLen || s.outPos + len > s.outLen) return -1;
			for (int i = 0; i < len; ++i) s.out[s.outPos++] = s.in[s.inPos++];
			return 0;
		}

		auto inflateFixed(State& s) -> int
		{
			short lengths[FIXLCODES];
			int i = 0;
			for (; i < 144; ++i) lengths[i] = 8;
			for (; i < 256; ++i) lengths[i] = 9;
			for (; i < 280; ++i) lengths[i] = 7;
			for (; i < FIXLCODES; ++i) lengths[i] = 8;

			short lc[MAXBITS + 1], ls[FIXLCODES];
			Huffman lencode{ lc, ls };
			construct(lencode, lengths, FIXLCODES);

			for (i = 0; i < MAXDCODES; ++i) lengths[i] = 5;
			short dc[MAXBITS + 1], ds[MAXDCODES];
			Huffman distcode{ dc, ds };
			construct(distcode, lengths, MAXDCODES);

			return inflateCodes(s, lencode, distcode);
		}

		auto inflateDynamic(State& s) -> int
		{
			static constexpr short order[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };

			int const nlen = getBits(s, 5) + 257;
			int const ndist = getBits(s, 5) + 1;
			int const ncode = getBits(s, 4) + 4;
			if (nlen > MAXLCODES || ndist > MAXDCODES) return -1;

			short lengths[MAXCODES] = {};
			int index = 0;
			for (; index < ncode; ++index) lengths[order[index]] = static_cast<short>(getBits(s, 3));
			for (; index < 19; ++index) lengths[order[index]] = 0;

			short clc[MAXBITS + 1], cls[19];
			Huffman clcode{ clc, cls };
			if (construct(clcode, lengths, 19) != 0) return -1;

			index = 0;
			while (index < nlen + ndist)
			{
				int symbol = decode(s, clcode);
				if (symbol < 0) return symbol;
				if (symbol < 16)
				{
					lengths[index++] = static_cast<short>(symbol);
				}
				else
				{
					short len = 0;
					if (symbol == 16)
					{
						if (index == 0) return -1;
						len = lengths[index - 1];
						symbol = 3 + getBits(s, 2);
					}
					else if (symbol == 17) symbol = 3 + getBits(s, 3);
					else symbol = 11 + getBits(s, 7);
					if (index + symbol > nlen + ndist) return -1;
					while (symbol--) lengths[index++] = len;
				}
			}
			if (lengths[256] == 0) return -1;

			short lc[MAXBITS + 1], ls[MAXLCODES];
			Huffman lencode{ lc, ls };
			int err = construct(lencode, lengths, nlen);
			if (err < 0 || (err > 0 && nlen - lencode.count[0] != 1)) return -1;

			short dc[MAXBITS + 1], ds[MAXDCODES];
			Huffman distcode{ dc, ds };
			err = construct(distcode, lengths + nlen, ndist);
			if (err < 0 || (err > 0 && ndist - distcode.count[0] != 1)) return -1;

			return inflateCodes(s, lencode, distcode);
		}
	}

	bool InflateRaw(unsigned char const* src, std::size_t srcLen, unsigned char* dst, std::size_t dstLen)
	{
		State s{ src, srcLen, 0, 0, 0, dst, dstLen, 0 };
		int last{};
		do
		{
			last = getBits(s, 1);
			int const type = getBits(s, 2);
			if (last < 0 || type < 0) return false;

			int err = 0;
			if (type == 0) err = inflateStored(s);
			else if (type == 1) err = inflateFixed(s);
			else if (type == 2) err = inflateDynamic(s);
			else return false;
			if (err != 0) return false;
		} while (last == 0);

		return s.outPos == dstLen;
	}
}
