

package com.google.zxing.common;

import java.io.ByteArrayOutputStream;


public final class BitSourceBuilder {

  private final ByteArrayOutputStream output;
  private int nextByte;
  private int bitsLeftInNextByte;

  public BitSourceBuilder() {
    output = new ByteArrayOutputStream();
    nextByte = 0;
    bitsLeftInNextByte = 8;
  }

  public void write(int value, int numBits) {
    if (numBits <= bitsLeftInNextByte) {
      nextByte <<= numBits;
      nextByte |= value;
      bitsLeftInNextByte -= numBits;
      if (bitsLeftInNextByte == 0) {
        output.write(nextByte);
        nextByte = 0;
        bitsLeftInNextByte = 8;
      }
    } else {
      int bitsToWriteNow = bitsLeftInNextByte;
      int numRestOfBits = numBits - bitsToWriteNow;
      int mask = 0xFF >> (8 - bitsToWriteNow);
      int valueToWriteNow = (value >>> numRestOfBits) & mask;
      write(valueToWriteNow, bitsToWriteNow);
      write(value, numRestOfBits);
    }
  }

  public byte[] toByteArray() {
    if (bitsLeftInNextByte < 8) {
      write(0, bitsLeftInNextByte);
    }
    return output.toByteArray();
  }

}