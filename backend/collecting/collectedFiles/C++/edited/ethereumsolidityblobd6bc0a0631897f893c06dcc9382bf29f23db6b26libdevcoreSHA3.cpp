


#include "SHA3.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace dev;

namespace dev
{

namespace keccak
{



#define decshake(bits) \
  int shake##bits(uint8_t*, size_t, const uint8_t*, size_t);

#define decsha3(bits) \
  int sha3_##bits(uint8_t*, size_t, const uint8_t*, size_t);

#define deckeccak(bits) \
  int keccak##bits(uint8_t*, size_t, const uint8_t*, size_t);

decshake(128)
decshake(256)
decsha3(224)
decsha3(256)
decsha3(384)
decsha3(512)
deckeccak(224)
deckeccak(256)
deckeccak(384)
deckeccak(512)




static const uint8_t rho[24] = \
  { 1,  3,   6, 10, 15, 21,
	28, 36, 45, 55,  2, 14,
	27, 41, 56,  8, 25, 43,
	62, 18, 39, 61, 20, 44};
static const uint8_t pi[24] = \
  {10,  7, 11, 17, 18, 3,
	5, 16,  8, 21, 24, 4,
   15, 23, 19, 13, 12, 2,
   20, 14, 22,  9, 6,  1};
static const uint64_t RC[24] = \
  {1ULL, 0x8082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
   0x808bULL, 0x80000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
   0x8aULL, 0x88ULL, 0x80008009ULL, 0x8000000aULL,
   0x8000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
   0x8000000000008002ULL, 0x8000000000000080ULL, 0x800aULL, 0x800000008000000aULL,
   0x8000000080008081ULL, 0x8000000000008080ULL, 0x80000001ULL, 0x8000000080008008ULL};


#define rol(x, s) (((x) << s) | ((x) >> (64 - s)))
#define REPEAT6(e) e e e e e e
#define REPEAT24(e) REPEAT6(e e e e)
#define REPEAT5(e) e e e e e
#define FOR5(v, s, e) \
  v = 0;            \
  REPEAT5(e; v += s;)


static inline void keccakf(void* state) {
  uint64_t* a = (uint64_t*)state;
  uint64_t b[5] = {0};

  for (int i = 0; i < 24; i++) {
	uint8_t x, y;
		FOR5(x, 1,
		 b[x] = 0;
		 FOR5(y, 5,
			  b[x] ^= a[x + y]; ))
	FOR5(x, 1,
		 FOR5(y, 5,
			  a[y + x] ^= b[(x + 4) % 5] ^ rol(b[(x + 1) % 5], 1); ))
		uint64_t t = a[1];
	x = 0;
	REPEAT24(b[0] = a[pi[x]];
			 a[pi[x]] = rol(t, rho[x]);
			 t = b[0];
			 x++; )
		FOR5(y,
	   5,
	   FOR5(x, 1,
			b[x] = a[y + x];)
	   FOR5(x, 1,
			a[y + x] = b[x] ^ ((~b[(x + 1) % 5]) & b[(x + 2) % 5]); ))
		a[0] ^= RC[i];
  }
}





#define _(S) do { S } while (0)
#define FOR(i, ST, L, S) \
  _(for (size_t i = 0; i < L; i += ST) { S; })
#define mkapply_ds(NAME, S)                                          \
  static inline void NAME(uint8_t* dst,                              \
						  const uint8_t* src,                        \
						  size_t len) {                              \
	FOR(i, 1, len, S);                                               \
  }
#define mkapply_sd(NAME, S)                                          \
  static inline void NAME(const uint8_t* src,                        \
						  uint8_t* dst,                              \
						  size_t len) {                              \
	FOR(i, 1, len, S);                                               \
  }

mkapply_ds(xorin, dst[i] ^= src[i])  mkapply_sd(setout, dst[i] = src[i])  
#define P keccakf
#define Plen 200

#define foldP(I, L, F) \
  while (L >= rate) {  \
	F(a, I, rate);     \
	P(a);              \
	I += rate;         \
	L -= rate;         \
  }


static inline int hash(uint8_t* out, size_t outlen,
					   const uint8_t* in, size_t inlen,
					   size_t rate, uint8_t delim) {
  if ((out == NULL) || ((in == NULL) && inlen != 0) || (rate >= Plen)) {
	return -1;
  }
  uint8_t a[Plen] = {0};
    foldP(in, inlen, xorin);
    a[inlen] ^= delim;
  a[rate - 1] ^= 0x80;
    xorin(a, in, inlen);
    P(a);
    foldP(out, outlen, setout);
  setout(a, out, outlen);
  memset(a, 0, 200);
  return 0;
}


#define defshake(bits)                                            \
  int shake##bits(uint8_t* out, size_t outlen,                    \
				  const uint8_t* in, size_t inlen) {              \
	return hash(out, outlen, in, inlen, 200 - (bits / 4), 0x1f);  \
  }
#define defsha3(bits)                                             \
  int sha3_##bits(uint8_t* out, size_t outlen,                    \
				  const uint8_t* in, size_t inlen) {              \
	if (outlen > (bits/8)) {                                      \
	  return -1;                                                  \
	}                                                             \
	return hash(out, outlen, in, inlen, 200 - (bits / 4), 0x06);  \
  }
#define defkeccak(bits)                                             \
  int keccak##bits(uint8_t* out, size_t outlen,                    \
				  const uint8_t* in, size_t inlen) {              \
	if (outlen > (bits/8)) {                                      \
	  return -1;                                                  \
	}                                                             \
	return hash(out, outlen, in, inlen, 200 - (bits / 4), 0x01);  \
  }


defshake(128)
defshake(256)


defsha3(224)
defsha3(256)
defsha3(384)
defsha3(512)


defkeccak(224)
defkeccak(256)
defkeccak(384)
defkeccak(512)

}

bool keccak256(bytesConstRef _input, bytesRef o_output)
{
		if (o_output.size() != 32)
		return false;
	keccak::keccak256(o_output.data(), 32, _input.data(), _input.size());
	return true;
}

}
