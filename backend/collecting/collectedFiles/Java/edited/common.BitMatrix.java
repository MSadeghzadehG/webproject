

package com.google.zxing.common;

import java.util.Arrays;


public final class BitMatrix implements Cloneable {

  private final int width;
  private final int height;
  private final int rowSize;
  private final int[] bits;

  
  public BitMatrix(int dimension) {
    this(dimension, dimension);
  }

  
  public BitMatrix(int width, int height) {
    if (width < 1 || height < 1) {
      throw new IllegalArgumentException("Both dimensions must be greater than 0");
    }
    this.width = width;
    this.height = height;
    this.rowSize = (width + 31) / 32;
    bits = new int[rowSize * height];
  }

  private BitMatrix(int width, int height, int rowSize, int[] bits) {
    this.width = width;
    this.height = height;
    this.rowSize = rowSize;
    this.bits = bits;
  }

  
  public static BitMatrix parse(boolean[][] image) {
    int height = image.length;
    int width = image[0].length;
    BitMatrix bits = new BitMatrix(width, height);
    for (int i = 0; i < height; i++) {
      boolean[] imageI = image[i];
      for (int j = 0; j < width; j++) {
        if (imageI[j]) {
          bits.set(j, i);
        }
      }
    }
    return bits;
  }

  public static BitMatrix parse(String stringRepresentation, String setString, String unsetString) {
    if (stringRepresentation == null) {
      throw new IllegalArgumentException();
    }

    boolean[] bits = new boolean[stringRepresentation.length()];
    int bitsPos = 0;
    int rowStartPos = 0;
    int rowLength = -1;
    int nRows = 0;
    int pos = 0;
    while (pos < stringRepresentation.length()) {
      if (stringRepresentation.charAt(pos) == '\n' ||
          stringRepresentation.charAt(pos) == '\r') {
        if (bitsPos > rowStartPos) {
          if (rowLength == -1) {
            rowLength = bitsPos - rowStartPos;
          } else if (bitsPos - rowStartPos != rowLength) {
            throw new IllegalArgumentException("row lengths do not match");
          }
          rowStartPos = bitsPos;
          nRows++;
        }
        pos++;
      }  else if (stringRepresentation.substring(pos, pos + setString.length()).equals(setString)) {
        pos += setString.length();
        bits[bitsPos] = true;
        bitsPos++;
      } else if (stringRepresentation.substring(pos, pos + unsetString.length()).equals(unsetString)) {
        pos += unsetString.length();
        bits[bitsPos] = false;
        bitsPos++;
      } else {
        throw new IllegalArgumentException(
            "illegal character encountered: " + stringRepresentation.substring(pos));
      }
    }

        if (bitsPos > rowStartPos) {
      if (rowLength == -1) {
        rowLength = bitsPos - rowStartPos;
      } else if (bitsPos - rowStartPos != rowLength) {
        throw new IllegalArgumentException("row lengths do not match");
      }
      nRows++;
    }

    BitMatrix matrix = new BitMatrix(rowLength, nRows);
    for (int i = 0; i < bitsPos; i++) {
      if (bits[i]) {
        matrix.set(i % rowLength, i / rowLength);
      }
    }
    return matrix;
  }

  
  public boolean get(int x, int y) {
    int offset = y * rowSize + (x / 32);
    return ((bits[offset] >>> (x & 0x1f)) & 1) != 0;
  }

  
  public void set(int x, int y) {
    int offset = y * rowSize + (x / 32);
    bits[offset] |= 1 << (x & 0x1f);
  }

  public void unset(int x, int y) {
    int offset = y * rowSize + (x / 32);
    bits[offset] &= ~(1 << (x & 0x1f));
  }

  
  public void flip(int x, int y) {
    int offset = y * rowSize + (x / 32);
    bits[offset] ^= 1 << (x & 0x1f);
  }

  
  public void xor(BitMatrix mask) {
    if (width != mask.getWidth() || height != mask.getHeight()
        || rowSize != mask.getRowSize()) {
      throw new IllegalArgumentException("input matrix dimensions do not match");
    }
    BitArray rowArray = new BitArray(width / 32 + 1);
    for (int y = 0; y < height; y++) {
      int offset = y * rowSize;
      int[] row = mask.getRow(y, rowArray).getBitArray();
      for (int x = 0; x < rowSize; x++) {
        bits[offset + x] ^= row[x];
      }
    }
  }

  
  public void clear() {
    int max = bits.length;
    for (int i = 0; i < max; i++) {
      bits[i] = 0;
    }
  }

  
  public void setRegion(int left, int top, int width, int height) {
    if (top < 0 || left < 0) {
      throw new IllegalArgumentException("Left and top must be nonnegative");
    }
    if (height < 1 || width < 1) {
      throw new IllegalArgumentException("Height and width must be at least 1");
    }
    int right = left + width;
    int bottom = top + height;
    if (bottom > this.height || right > this.width) {
      throw new IllegalArgumentException("The region must fit inside the matrix");
    }
    for (int y = top; y < bottom; y++) {
      int offset = y * rowSize;
      for (int x = left; x < right; x++) {
        bits[offset + (x / 32)] |= 1 << (x & 0x1f);
      }
    }
  }

  
  public BitArray getRow(int y, BitArray row) {
    if (row == null || row.getSize() < width) {
      row = new BitArray(width);
    } else {
      row.clear();
    }
    int offset = y * rowSize;
    for (int x = 0; x < rowSize; x++) {
      row.setBulk(x * 32, bits[offset + x]);
    }
    return row;
  }

  
  public void setRow(int y, BitArray row) {
    System.arraycopy(row.getBitArray(), 0, bits, y * rowSize, rowSize);
  }

  
  public void rotate180() {
    int width = getWidth();
    int height = getHeight();
    BitArray topRow = new BitArray(width);
    BitArray bottomRow = new BitArray(width);
    for (int i = 0; i < (height + 1) / 2; i++) {
      topRow = getRow(i, topRow);
      bottomRow = getRow(height - 1 - i, bottomRow);
      topRow.reverse();
      bottomRow.reverse();
      setRow(i, bottomRow);
      setRow(height - 1 - i, topRow);
    }
  }

  
  public int[] getEnclosingRectangle() {
    int left = width;
    int top = height;
    int right = -1;
    int bottom = -1;

    for (int y = 0; y < height; y++) {
      for (int x32 = 0; x32 < rowSize; x32++) {
        int theBits = bits[y * rowSize + x32];
        if (theBits != 0) {
          if (y < top) {
            top = y;
          }
          if (y > bottom) {
            bottom = y;
          }
          if (x32 * 32 < left) {
            int bit = 0;
            while ((theBits << (31 - bit)) == 0) {
              bit++;
            }
            if ((x32 * 32 + bit) < left) {
              left = x32 * 32 + bit;
            }
          }
          if (x32 * 32 + 31 > right) {
            int bit = 31;
            while ((theBits >>> bit) == 0) {
              bit--;
            }
            if ((x32 * 32 + bit) > right) {
              right = x32 * 32 + bit;
            }
          }
        }
      }
    }

    if (right < left || bottom < top) {
      return null;
    }

    return new int[] {left, top, right - left + 1, bottom - top + 1};
  }

  
  public int[] getTopLeftOnBit() {
    int bitsOffset = 0;
    while (bitsOffset < bits.length && bits[bitsOffset] == 0) {
      bitsOffset++;
    }
    if (bitsOffset == bits.length) {
      return null;
    }
    int y = bitsOffset / rowSize;
    int x = (bitsOffset % rowSize) * 32;

    int theBits = bits[bitsOffset];
    int bit = 0;
    while ((theBits << (31 - bit)) == 0) {
      bit++;
    }
    x += bit;
    return new int[] {x, y};
  }

  public int[] getBottomRightOnBit() {
    int bitsOffset = bits.length - 1;
    while (bitsOffset >= 0 && bits[bitsOffset] == 0) {
      bitsOffset--;
    }
    if (bitsOffset < 0) {
      return null;
    }

    int y = bitsOffset / rowSize;
    int x = (bitsOffset % rowSize) * 32;

    int theBits = bits[bitsOffset];
    int bit = 31;
    while ((theBits >>> bit) == 0) {
      bit--;
    }
    x += bit;

    return new int[] {x, y};
  }

  
  public int getWidth() {
    return width;
  }

  
  public int getHeight() {
    return height;
  }

  
  public int getRowSize() {
    return rowSize;
  }

  @Override
  public boolean equals(Object o) {
    if (!(o instanceof BitMatrix)) {
      return false;
    }
    BitMatrix other = (BitMatrix) o;
    return width == other.width && height == other.height && rowSize == other.rowSize &&
    Arrays.equals(bits, other.bits);
  }

  @Override
  public int hashCode() {
    int hash = width;
    hash = 31 * hash + width;
    hash = 31 * hash + height;
    hash = 31 * hash + rowSize;
     hash = 31 * hash + Arrays.hashCode(bits);
    return hash;
  }

  
  @Override
  public String toString() {
    return toString("X ", "  ");
  }

  
  public String toString(String setString, String unsetString) {
    return buildToString(setString, unsetString, "\n");
  }

  
  @Deprecated
  public String toString(String setString, String unsetString, String lineSeparator) {
    return buildToString(setString, unsetString, lineSeparator);
  }

  private String buildToString(String setString, String unsetString, String lineSeparator) {
    StringBuilder result = new StringBuilder(height * (width + 1));
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        result.append(get(x, y) ? setString : unsetString);
      }
      result.append(lineSeparator);
    }
    return result.toString();
  }

  @Override
  public BitMatrix clone() {
    return new BitMatrix(width, height, rowSize, bits.clone());
  }

}
