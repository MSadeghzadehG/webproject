

package com.google.zxing.pdf417.decoder;

import java.util.Formatter;


class DetectionResultColumn {

  private static final int MAX_NEARBY_DISTANCE = 5;

  private final BoundingBox boundingBox;
  private final Codeword[] codewords;

  DetectionResultColumn(BoundingBox boundingBox) {
    this.boundingBox = new BoundingBox(boundingBox);
    codewords = new Codeword[boundingBox.getMaxY() - boundingBox.getMinY() + 1];
  }

  final Codeword getCodewordNearby(int imageRow) {
    Codeword codeword = getCodeword(imageRow);
    if (codeword != null) {
      return codeword;
    }
    for (int i = 1; i < MAX_NEARBY_DISTANCE; i++) {
      int nearImageRow = imageRowToCodewordIndex(imageRow) - i;
      if (nearImageRow >= 0) {
        codeword = codewords[nearImageRow];
        if (codeword != null) {
          return codeword;
        }
      }
      nearImageRow = imageRowToCodewordIndex(imageRow) + i;
      if (nearImageRow < codewords.length) {
        codeword = codewords[nearImageRow];
        if (codeword != null) {
          return codeword;
        }
      }
    }
    return null;
  }

  final int imageRowToCodewordIndex(int imageRow) {
    return imageRow - boundingBox.getMinY();
  }

  final void setCodeword(int imageRow, Codeword codeword) {
    codewords[imageRowToCodewordIndex(imageRow)] = codeword;
  }

  final Codeword getCodeword(int imageRow) {
    return codewords[imageRowToCodewordIndex(imageRow)];
  }

  final BoundingBox getBoundingBox() {
    return boundingBox;
  }

  final Codeword[] getCodewords() {
    return codewords;
  }

  @Override
  public String toString() {
    try (Formatter formatter = new Formatter()) {
      int row = 0;
      for (Codeword codeword : codewords) {
        if (codeword == null) {
          formatter.format("%3d:    |   %n", row++);
          continue;
        }
        formatter.format("%3d: %3d|%3d%n", row++, codeword.getRowNumber(), codeword.getValue());
      }
      return formatter.toString();
    }
  }

}
