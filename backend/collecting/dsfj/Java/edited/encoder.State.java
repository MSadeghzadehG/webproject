

package com.google.zxing.aztec.encoder;

import java.util.Deque;
import java.util.LinkedList;

import com.google.zxing.common.BitArray;


final class State {

  static final State INITIAL_STATE = new State(Token.EMPTY, HighLevelEncoder.MODE_UPPER, 0, 0);

      private final int mode;
      private final Token token;
      private final int binaryShiftByteCount;
    private final int bitCount;

  private State(Token token, int mode, int binaryBytes, int bitCount) {
    this.token = token;
    this.mode = mode;
    this.binaryShiftByteCount = binaryBytes;
    this.bitCount = bitCount;
                          }

  int getMode() {
    return mode;
  }

  Token getToken() {
    return token;
  }

  int getBinaryShiftByteCount() {
    return binaryShiftByteCount;
  }

  int getBitCount() {
    return bitCount;
  }

      State latchAndAppend(int mode, int value) {
        int bitCount = this.bitCount;
    Token token = this.token;
    if (mode != this.mode) {
      int latch = HighLevelEncoder.LATCH_TABLE[this.mode][mode];
      token = token.add(latch & 0xFFFF, latch >> 16);
      bitCount += latch >> 16;
    }
    int latchModeBitCount = mode == HighLevelEncoder.MODE_DIGIT ? 4 : 5;
    token = token.add(value, latchModeBitCount);
    return new State(token, mode, 0, bitCount + latchModeBitCount);
  }

      State shiftAndAppend(int mode, int value) {
        Token token = this.token;
    int thisModeBitCount = this.mode == HighLevelEncoder.MODE_DIGIT ? 4 : 5;
        token = token.add(HighLevelEncoder.SHIFT_TABLE[this.mode][mode], thisModeBitCount);
    token = token.add(value, 5);
    return new State(token, this.mode, 0, this.bitCount + thisModeBitCount + 5);
  }

      State addBinaryShiftChar(int index) {
    Token token = this.token;
    int mode = this.mode;
    int bitCount = this.bitCount;
    if (this.mode == HighLevelEncoder.MODE_PUNCT || this.mode == HighLevelEncoder.MODE_DIGIT) {
            int latch = HighLevelEncoder.LATCH_TABLE[mode][HighLevelEncoder.MODE_UPPER];
      token = token.add(latch & 0xFFFF, latch >> 16);
      bitCount += latch >> 16;
      mode = HighLevelEncoder.MODE_UPPER;
    }
    int deltaBitCount =
      (binaryShiftByteCount == 0 || binaryShiftByteCount == 31) ? 18 :
      (binaryShiftByteCount == 62) ? 9 : 8;
    State result = new State(token, mode, binaryShiftByteCount + 1, bitCount + deltaBitCount);
    if (result.binaryShiftByteCount == 2047 + 31) {
            result = result.endBinaryShift(index + 1);
    }
    return result;
  }

      State endBinaryShift(int index) {
    if (binaryShiftByteCount == 0) {
      return this;
    }
    Token token = this.token;
    token = token.addBinaryShift(index - binaryShiftByteCount, binaryShiftByteCount);
        return new State(token, mode, 0, this.bitCount);
  }

      boolean isBetterThanOrEqualTo(State other) {
    int mySize = this.bitCount + (HighLevelEncoder.LATCH_TABLE[this.mode][other.mode] >> 16);
    if (other.binaryShiftByteCount > 0 &&
        (this.binaryShiftByteCount == 0 || this.binaryShiftByteCount > other.binaryShiftByteCount)) {
      mySize += 10;         }
    return mySize <= other.bitCount;
  }

  BitArray toBitArray(byte[] text) {
            Deque<Token> symbols = new LinkedList<>();
    for (Token token = endBinaryShift(text.length).token; token != null; token = token.getPrevious()) {
      symbols.addFirst(token);
    }
    BitArray bitArray = new BitArray();
        for (Token symbol : symbols) {
      symbol.appendTo(bitArray, text);
    }
        return bitArray;
  }

  @Override
  public String toString() {
    return String.format("%s bits=%d bytes=%d", HighLevelEncoder.MODE_NAMES[mode], bitCount, binaryShiftByteCount);
  }

}
