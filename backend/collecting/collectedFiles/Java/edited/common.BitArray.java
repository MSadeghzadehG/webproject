

package com.google.zxing.common;

import java.util.Arrays;


public final class BitArray implements Cloneable {

  private int[] bits;
  private int size;

  public BitArray() {
    this.size = 0;
    this.bits = new int[1];
  }

  public BitArray(int size) {
    this.size = size;
    this.bits = makeArray(size);
  }

    BitArray(int[] bits, int size) {
    this.bits = bits;
    this.size = size;
  }

  public int getSize() {
    return size;
  }

  public int getSizeInBytes() {
    return (size + 7) / 8;
  }

  private void ensureCapacity(int size) {
    if (size > bits.length * 32) {
      int[] newBits = makeArray(size);
      System.arraycopy(bits, 0, newBits, 0, bits.length);
      this.bits = newBits;
    }
  }

  
  public boolean get(int i) {
    return (bits[i / 32] & (1 << (i & 0x1F))) != 0;
  }

  
  public void set(int i) {
    bits[i / 32] |= 1 << (i & 0x1F);
  }

  
  public void flip(int i) {
    bits[i / 32] ^= 1 << (i & 0x1F);
  }

  
  public int getNextSet(int from) {
    if (from >= size) {
      return size;
    }
    int bitsOffset = from / 32;
    int currentBits = bits[bitsOffset];
        currentBits &= ~((1 << (from & 0x1F)) - 1);
    while (currentBits == 0) {
      if (++bitsOffset == bits.length) {
        return size;
      }
      currentBits = bits[bitsOffset];
    }
    int result = (bitsOffset * 32) + Integer.numberOfTrailingZeros(currentBits);
    return result > size ? size : result;
  }

  
  public int getNextUnset(int from) {
    if (from >= size) {
      return size;
    }
    int bitsOffset = from / 32;
    int currentBits = ~bits[bitsOffset];
        currentBits &= ~((1 << (from & 0x1F)) - 1);
    while (currentBits == 0) {
      if (++bitsOffset == bits.length) {
        return size;
      }
      currentBits = ~bits[bitsOffset];
    }
    int result = (bitsOffset * 32) + Integer.numberOfTrailingZeros(currentBits);
    return result > size ? size : result;
  }

  
  public void setBulk(int i, int newBits) {
    bits[i / 32] = newBits;
  }

  
  public void setRange(int start, int end) {
    if (end < start || start < 0 || end > size) {
      throw new IllegalArgumentException();
    }
    if (end == start) {
      return;
    }
    end--;     int firstInt = start / 32;
    int lastInt = end / 32;
    for (int i = firstInt; i <= lastInt; i++) {
      int firstBit = i > firstInt ? 0 : start & 0x1F;
      int lastBit = i < lastInt ? 31 : end & 0x1F;
            int mask = (2 << lastBit) - (1 << firstBit);
      bits[i] |= mask;
    }
  }

  
  public void clear() {
    int max = bits.length;
    for (int i = 0; i < max; i++) {
      bits[i] = 0;
    }
  }

  
  public boolean isRange(int start, int end, boolean value) {
    if (end < start || start < 0 || end > size) {
      throw new IllegalArgumentException();
    }
    if (end == start) {
      return true;     }
    end--;     int firstInt = start / 32;
    int lastInt = end / 32;
    for (int i = firstInt; i <= lastInt; i++) {
      int firstBit = i > firstInt ? 0 : start & 0x1F;
      int lastBit = i < lastInt ? 31 : end & 0x1F;
            int mask = (2 << lastBit) - (1 << firstBit);

                  if ((bits[i] & mask) != (value ? mask : 0)) {
        return false;
      }
    }
    return true;
  }

  public void appendBit(boolean bit) {
    ensureCapacity(size + 1);
    if (bit) {
      bits[size / 32] |= 1 << (size & 0x1F);
    }
    size++;
  }

  
  public void appendBits(int value, int numBits) {
    if (numBits < 0 || numBits > 32) {
      throw new IllegalArgumentException("Num bits must be between 0 and 32");
    }
    ensureCapacity(size + numBits);
    for (int numBitsLeft = numBits; numBitsLeft > 0; numBitsLeft--) {
      appendBit(((value >> (numBitsLeft - 1)) & 0x01) == 1);
    }
  }

  public void appendBitArray(BitArray other) {
    int otherSize = other.size;
    ensureCapacity(size + otherSize);
    for (int i = 0; i < otherSize; i++) {
      appendBit(other.get(i));
    }
  }

  public void xor(BitArray other) {
    if (size != other.size) {
      throw new IllegalArgumentException("Sizes don't match");
    }
    for (int i = 0; i < bits.length; i++) {
                  bits[i] ^= other.bits[i];
    }
  }

  
  public void toBytes(int bitOffset, byte[] array, int offset, int numBytes) {
    for (int i = 0; i < numBytes; i++) {
      int theByte = 0;
      for (int j = 0; j < 8; j++) {
        if (get(bitOffset)) {
          theByte |= 1 << (7 - j);
        }
        bitOffset++;
      }
      array[offset + i] = (byte) theByte;
    }
  }

  
  public int[] getBitArray() {
    return bits;
  }

  
  public void reverse() {
    int[] newBits = new int[bits.length];
        int len = (size - 1) / 32;
    int oldBitsLen = len + 1;
    for (int i = 0; i < oldBitsLen; i++) {
      long x = bits[i];
      x = ((x >>  1) & 0x55555555L) | ((x & 0x55555555L) <<  1);
      x = ((x >>  2) & 0x33333333L) | ((x & 0x33333333L) <<  2);
      x = ((x >>  4) & 0x0f0f0f0fL) | ((x & 0x0f0f0f0fL) <<  4);
      x = ((x >>  8) & 0x00ff00ffL) | ((x & 0x00ff00ffL) <<  8);
      x = ((x >> 16) & 0x0000ffffL) | ((x & 0x0000ffffL) << 16);
      newBits[len - i] = (int) x;
    }
        if (size != oldBitsLen * 32) {
      int leftOffset = oldBitsLen * 32 - size;
      int currentInt = newBits[0] >>> leftOffset;
      for (int i = 1; i < oldBitsLen; i++) {
        int nextInt = newBits[i];
        currentInt |= nextInt << (32 - leftOffset);
        newBits[i - 1] = currentInt;
        currentInt = nextInt >>> leftOffset;
      }
      newBits[oldBitsLen - 1] = currentInt;
    }
    bits = newBits;
  }

  private static int[] makeArray(int size) {
    return new int[(size + 31) / 32];
  }

  @Override
  public boolean equals(Object o) {
    if (!(o instanceof BitArray)) {
      return false;
    }
    BitArray other = (BitArray) o;
    return size == other.size && Arrays.equals(bits, other.bits);
  }

  @Override
  public int hashCode() {
    return 31 * size + Arrays.hashCode(bits);
  }

  @Override
  public String toString() {
    StringBuilder result = new StringBuilder(size);
    for (int i = 0; i < size; i++) {
      if ((i & 0x07) == 0) {
        result.append(' ');
      }
      result.append(get(i) ? 'X' : '.');
    }
    return result.toString();
  }

  @Override
  public BitArray clone() {
    return new BitArray(bits.clone(), size);
  }

}
