

package com.google.zxing.aztec.detector;

import com.google.zxing.NotFoundException;
import com.google.zxing.aztec.AztecDetectorResult;
import com.google.zxing.aztec.decoder.Decoder;
import com.google.zxing.aztec.detector.Detector.Point;
import com.google.zxing.aztec.encoder.AztecCode;
import com.google.zxing.aztec.encoder.Encoder;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.DecoderResult;
import org.junit.Assert;
import org.junit.Test;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Random;
import java.util.TreeSet;


public final class DetectorTest extends Assert {

  @Test
  public void testErrorInParameterLocatorZeroZero() throws Exception {
            testErrorInParameterLocator("X");
  }

  @Test
  public void testErrorInParameterLocatorCompact() throws Exception {
    testErrorInParameterLocator("This is an example Aztec symbol for Wikipedia.");
  }

  @Test
  public void testErrorInParameterLocatorNotCompact() throws Exception {
    String alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxyz";
    testErrorInParameterLocator(alphabet + alphabet + alphabet);
  }

    private static void testErrorInParameterLocator(String data) throws Exception {
    AztecCode aztec = Encoder.encode(data.getBytes(StandardCharsets.ISO_8859_1), 25, Encoder.DEFAULT_AZTEC_LAYERS);
    Random random = new Random(aztec.getMatrix().hashCode());       int layers = aztec.getLayers();
    boolean compact = aztec.isCompact();
    List<Point> orientationPoints = getOrientationPoints(aztec);
    for (boolean isMirror : new boolean[] { false, true }) {
      for (BitMatrix matrix : getRotations(aztec.getMatrix())) {
                for (int error1 = 0; error1 < orientationPoints.size(); error1++) {
          for (int error2 = error1; error2 < orientationPoints.size(); error2++) {
            BitMatrix copy = isMirror ? transpose(matrix) : clone(matrix);
            copy.flip(orientationPoints.get(error1).getX(), orientationPoints.get(error1).getY());
            if (error2 > error1) {
                            copy.flip(orientationPoints.get(error2).getX(), orientationPoints.get(error2).getY());
            }
                        AztecDetectorResult r = new Detector(makeLarger(copy, 3)).detect(isMirror);
            assertNotNull(r);
            assertEquals(r.getNbLayers(), layers);
            assertEquals(r.isCompact(), compact);
            DecoderResult res = new Decoder().decode(r);
            assertEquals(data, res.getText());
          }
        }
                for (int i = 0; i < 5; i++) {
          BitMatrix copy = clone(matrix);
          Collection<Integer> errors = new TreeSet<>();
          while (errors.size() < 3) {
                        errors.add(random.nextInt(orientationPoints.size()));
          }
          for (int error : errors) {
            copy.flip(orientationPoints.get(error).getX(), orientationPoints.get(error).getY());
          }
          try {
            new Detector(makeLarger(copy, 3)).detect(false);
            fail("Should not reach here");
          } catch (NotFoundException expected) {
                      }
        }
      }
    }
  }

    private static BitMatrix makeLarger(BitMatrix input, int factor) {
    int width = input.getWidth();
    BitMatrix output = new BitMatrix(width * factor);
    for (int inputY = 0; inputY < width; inputY++) {
      for (int inputX = 0; inputX < width; inputX++) {
        if (input.get(inputX, inputY)) {
          output.setRegion(inputX * factor, inputY * factor, factor, factor);
        }
      }
    }
    return output;
  }

    private static Iterable<BitMatrix> getRotations(BitMatrix matrix0) {
    BitMatrix matrix90 = rotateRight(matrix0);
    BitMatrix matrix180 = rotateRight(matrix90);
    BitMatrix matrix270 = rotateRight(matrix180);
    return Arrays.asList(matrix0, matrix90, matrix180, matrix270);
  }

    private static BitMatrix rotateRight(BitMatrix input) {
    int width = input.getWidth();
    BitMatrix result = new BitMatrix(width);
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < width; y++) {
        if (input.get(x,y)) {
          result.set(y, width - x - 1);
        }
      }
    }
    return result;
  }

      private static BitMatrix transpose(BitMatrix input) {
    int width = input.getWidth();
    BitMatrix result = new BitMatrix(width);
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < width; y++) {
        if (input.get(x, y)) {
          result.set(y, x);
        }
      }
    }
    return result;
  }

  private static BitMatrix clone(BitMatrix input)  {
    int width = input.getWidth();
    BitMatrix result = new BitMatrix(width);
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < width; y++) {
        if (input.get(x,y)) {
          result.set(x,y);
        }
      }
    }
    return result;
  }

  private static List<Point> getOrientationPoints(AztecCode code) {
    int center = code.getMatrix().getWidth() / 2;
    int offset = code.isCompact() ? 5 : 7;
    List<Point> result = new ArrayList<>();
    for (int xSign = -1; xSign <= 1; xSign += 2) {
      for (int ySign = -1; ySign <= 1; ySign += 2) {
        result.add(new Point(center + xSign * offset, center + ySign * offset));
        result.add(new Point(center + xSign * (offset - 1), center + ySign * offset));
        result.add(new Point(center + xSign * offset, center + ySign * (offset - 1)));
      }
    }
    return result;
  }

}
