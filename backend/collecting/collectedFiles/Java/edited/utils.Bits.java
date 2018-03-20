

package com.badlogic.gdx.utils;

import java.util.Arrays;


public class Bits {

	long[] bits = {0};

	public Bits () {
	}

	
	public Bits (int nbits) {
		checkCapacity(nbits >>> 6);
	}

	
	public boolean get (int index) {
		final int word = index >>> 6;
		if (word >= bits.length) return false;
		return (bits[word] & (1L << (index & 0x3F))) != 0L;
	}

	
	public boolean getAndClear (int index) {
		final int word = index >>> 6;
		if (word >= bits.length) return false;
		long oldBits = bits[word];
		bits[word] &= ~(1L << (index & 0x3F));
		return bits[word] != oldBits;
	}

	
	public boolean getAndSet (int index) {
		final int word = index >>> 6;
		checkCapacity(word);
		long oldBits = bits[word];
		bits[word] |= 1L << (index & 0x3F);
		return bits[word] == oldBits;
	}

	
	public void set (int index) {
		final int word = index >>> 6;
		checkCapacity(word);
		bits[word] |= 1L << (index & 0x3F);
	}

	
	public void flip (int index) {
		final int word = index >>> 6;
		checkCapacity(word);
		bits[word] ^= 1L << (index & 0x3F);
	}

	private void checkCapacity (int len) {
		if (len >= bits.length) {
			long[] newBits = new long[len + 1];
			System.arraycopy(bits, 0, newBits, 0, bits.length);
			bits = newBits;
		}
	}

	
	public void clear (int index) {
		final int word = index >>> 6;
		if (word >= bits.length) return;
		bits[word] &= ~(1L << (index & 0x3F));
	}

	
	public void clear () {
		long[] bits = this.bits;
		int length = bits.length;
		for (int i = 0; i < length; i++) {
			bits[i] = 0L;
		}
	}

	
	public int numBits () {
		return bits.length << 6;
	}

	
	public int length () {
		long[] bits = this.bits;
		for (int word = bits.length - 1; word >= 0; --word) {
			long bitsAtWord = bits[word];
			if (bitsAtWord != 0) {
				for (int bit = 63; bit >= 0; --bit) {
					if ((bitsAtWord & (1L << (bit & 0x3F))) != 0L) {
						return (word << 6) + bit + 1;
					}
				}
			}
		}
		return 0;
	}

	
	public boolean isEmpty () {
		long[] bits = this.bits;
		int length = bits.length;
		for (int i = 0; i < length; i++) {
			if (bits[i] != 0L) {
				return false;
			}
		}
		return true;
	}

	
	public int nextSetBit (int fromIndex) {
		long[] bits = this.bits;
		int word = fromIndex >>> 6;
		int bitsLength = bits.length;
		if (word >= bitsLength) return -1;
		long bitsAtWord = bits[word];
		if (bitsAtWord != 0) {
			for (int i = fromIndex & 0x3f; i < 64; i++) {
				if ((bitsAtWord & (1L << (i & 0x3F))) != 0L) {
					return (word << 6) + i;
				}
			}
		}
		for (word++; word < bitsLength; word++) {
			if (word != 0) {
				bitsAtWord = bits[word];
				if (bitsAtWord != 0) {
					for (int i = 0; i < 64; i++) {
						if ((bitsAtWord & (1L << (i & 0x3F))) != 0L) {
							return (word << 6) + i;
						}
					}
				}
			}
		}
		return -1;
	}

	
	public int nextClearBit (int fromIndex) {
		long[] bits = this.bits;
		int word = fromIndex >>> 6;
		int bitsLength = bits.length;
		if (word >= bitsLength) return bits.length << 6;
		long bitsAtWord = bits[word];
		for (int i = fromIndex & 0x3f; i < 64; i++) {
			if ((bitsAtWord & (1L << (i & 0x3F))) == 0L) {
				return (word << 6) + i;
			}
		}
		for (word++; word < bitsLength; word++) {
			if (word == 0) {
				return word << 6;
			}
			bitsAtWord = bits[word];
			for (int i = 0; i < 64; i++) {
				if ((bitsAtWord & (1L << (i & 0x3F))) == 0L) {
					return (word << 6) + i;
				}
			}
		}
		return bits.length << 6;
	}

	
	public void and (Bits other) {
		int commonWords = Math.min(bits.length, other.bits.length);
		for (int i = 0; commonWords > i; i++) {
			bits[i] &= other.bits[i];
		}
		
		if (bits.length > commonWords) {
			for (int i = commonWords, s = bits.length; s > i; i++) {
				bits[i] = 0L;
			}
		}
	}

	
	public void andNot (Bits other) {
		for (int i = 0, j = bits.length, k = other.bits.length; i < j && i < k; i++) {
			bits[i] &= ~other.bits[i];
		}
	}

	
	public void or (Bits other) {
		int commonWords = Math.min(bits.length, other.bits.length);
		for (int i = 0; commonWords > i; i++) {
			bits[i] |= other.bits[i];
		}
		
		if (commonWords < other.bits.length) {
			checkCapacity(other.bits.length);
			for (int i = commonWords, s = other.bits.length; s > i; i++) {
				bits[i] = other.bits[i];
			}
		}
	}

	
	public void xor (Bits other) {
		int commonWords = Math.min(bits.length, other.bits.length);
		
		for (int i = 0; commonWords > i; i++) {
			bits[i] ^= other.bits[i];
		}
		
		if (commonWords < other.bits.length) {
			checkCapacity(other.bits.length);
			for (int i = commonWords, s = other.bits.length; s > i; i++) {
				bits[i] = other.bits[i];
			}
		}
	}

	
	public boolean intersects (Bits other) {
		long[] bits = this.bits;
		long[] otherBits = other.bits;
		for (int i = Math.min(bits.length, otherBits.length) - 1; i >= 0; i--) {
			if ((bits[i] & otherBits[i]) != 0) {
				return true;
			}
		}
		return false;
	}

	
	public boolean containsAll (Bits other) {
		long[] bits = this.bits;
		long[] otherBits = other.bits;
		int otherBitsLength = otherBits.length;
		int bitsLength = bits.length;

		for (int i = bitsLength; i < otherBitsLength; i++) {
			if (otherBits[i] != 0) {
				return false;
			}
		}
		for (int i = Math.min(bitsLength, otherBitsLength) - 1; i >= 0; i--) {
			if ((bits[i] & otherBits[i]) != otherBits[i]) {
				return false;
			}
		}
		return true;
	}
	
	@Override
	public int hashCode() {
		final int word = length() >>> 6;
		int hash = 0;
		for (int i = 0; word >= i; i++) {
			hash = 127 * hash + (int)(bits[i] ^ (bits[i] >>> 32));
		}
		return hash;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		
		Bits other = (Bits) obj;
		long[] otherBits = other.bits;
		
		int commonWords = Math.min(bits.length, otherBits.length);
		for (int i = 0; commonWords > i; i++) {
			if (bits[i] != otherBits[i])
				return false;
		}
		
		if (bits.length == otherBits.length)
			return true;
		
		return length() == other.length();
	}
}
