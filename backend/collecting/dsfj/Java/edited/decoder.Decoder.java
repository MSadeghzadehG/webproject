

package com.google.zxing.maxicode.decoder;

import com.google.zxing.ChecksumException;
import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.DecoderResult;
import com.google.zxing.common.reedsolomon.GenericGF;
import com.google.zxing.common.reedsolomon.ReedSolomonDecoder;
import com.google.zxing.common.reedsolomon.ReedSolomonException;

import java.util.Map;


public final class Decoder {

  private static final int ALL = 0;
  private static final int EVEN = 1;
  private static final int ODD = 2;

  private final ReedSolomonDecoder rsDecoder;

  public Decoder() {
    rsDecoder = new ReedSolomonDecoder(GenericGF.MAXICODE_FIELD_64);
  }

  public DecoderResult decode(BitMatrix bits) throws ChecksumException, FormatException {
    return decode(bits, null);
  }

  public DecoderResult decode(BitMatrix bits,
                              Map<DecodeHintType,?> hints) throws FormatException, ChecksumException {
    BitMatrixParser parser = new BitMatrixParser(bits);
    byte[] codewords = parser.readCodewords();

    correctErrors(codewords, 0, 10, 10, ALL);
    int mode = codewords[0] & 0x0F;
    byte[] datawords;
    switch (mode) {
      case 2:
      case 3:
      case 4:
        correctErrors(codewords, 20, 84, 40, EVEN);
        correctErrors(codewords, 20, 84, 40, ODD);
        datawords = new byte[94];
        break;
      case 5:
        correctErrors(codewords, 20, 68, 56, EVEN);
        correctErrors(codewords, 20, 68, 56, ODD);
        datawords = new byte[78];
        break;
      default:
        throw FormatException.getFormatInstance();
    }

    System.arraycopy(codewords, 0, datawords, 0, 10);
    System.arraycopy(codewords, 20, datawords, 10, datawords.length - 10);

    return DecodedBitStreamParser.decode(datawords, mode);
  }

  private void correctErrors(byte[] codewordBytes,
                             int start,
                             int dataCodewords,
                             int ecCodewords,
                             int mode) throws ChecksumException {
    int codewords = dataCodewords + ecCodewords;

        int divisor = mode == ALL ? 1 : 2;

        int[] codewordsInts = new int[codewords / divisor];
    for (int i = 0; i < codewords; i++) {
      if ((mode == ALL) || (i % 2 == (mode - 1))) {
        codewordsInts[i / divisor] = codewordBytes[i + start] & 0xFF;
      }
    }
    try {
      rsDecoder.decode(codewordsInts, ecCodewords / divisor);
    } catch (ReedSolomonException ignored) {
      throw ChecksumException.getChecksumInstance();
    }
            for (int i = 0; i < dataCodewords; i++) {
      if ((mode == ALL) || (i % 2 == (mode - 1))) {
        codewordBytes[i + start] = (byte) codewordsInts[i / divisor];
      }
    }
  }

}
