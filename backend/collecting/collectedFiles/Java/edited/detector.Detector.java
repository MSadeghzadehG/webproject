

package com.google.zxing.datamatrix.detector;

import com.google.zxing.NotFoundException;
import com.google.zxing.ResultPoint;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.DetectorResult;
import com.google.zxing.common.GridSampler;
import com.google.zxing.common.detector.MathUtils;
import com.google.zxing.common.detector.WhiteRectangleDetector;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public final class Detector {

  private final BitMatrix image;
  private final WhiteRectangleDetector rectangleDetector;

  public Detector(BitMatrix image) throws NotFoundException {
    this.image = image;
    rectangleDetector = new WhiteRectangleDetector(image);
  }

  
  public DetectorResult detect() throws NotFoundException {

    ResultPoint[] cornerPoints = rectangleDetector.detect();
    ResultPoint pointA = cornerPoints[0];
    ResultPoint pointB = cornerPoints[1];
    ResultPoint pointC = cornerPoints[2];
    ResultPoint pointD = cornerPoints[3];

                List<ResultPointsAndTransitions> transitions = new ArrayList<>(4);
    transitions.add(transitionsBetween(pointA, pointB));
    transitions.add(transitionsBetween(pointA, pointC));
    transitions.add(transitionsBetween(pointB, pointD));
    transitions.add(transitionsBetween(pointC, pointD));
    Collections.sort(transitions, new ResultPointsAndTransitionsComparator());

            ResultPointsAndTransitions lSideOne = transitions.get(0);
    ResultPointsAndTransitions lSideTwo = transitions.get(1);

            Map<ResultPoint,Integer> pointCount = new HashMap<>();
    increment(pointCount, lSideOne.getFrom());
    increment(pointCount, lSideOne.getTo());
    increment(pointCount, lSideTwo.getFrom());
    increment(pointCount, lSideTwo.getTo());

    ResultPoint maybeTopLeft = null;
    ResultPoint bottomLeft = null;
    ResultPoint maybeBottomRight = null;
    for (Map.Entry<ResultPoint,Integer> entry : pointCount.entrySet()) {
      ResultPoint point = entry.getKey();
      Integer value = entry.getValue();
      if (value == 2) {
        bottomLeft = point;       } else {
                if (maybeTopLeft == null) {
          maybeTopLeft = point;
        } else {
          maybeBottomRight = point;
        }
      }
    }

    if (maybeTopLeft == null || bottomLeft == null || maybeBottomRight == null) {
      throw NotFoundException.getNotFoundInstance();
    }

        ResultPoint[] corners = { maybeTopLeft, bottomLeft, maybeBottomRight };
        ResultPoint.orderBestPatterns(corners);

        ResultPoint bottomRight = corners[0];
    bottomLeft = corners[1];
    ResultPoint topLeft = corners[2];

        ResultPoint topRight;
    if (!pointCount.containsKey(pointA)) {
      topRight = pointA;
    } else if (!pointCount.containsKey(pointB)) {
      topRight = pointB;
    } else if (!pointCount.containsKey(pointC)) {
      topRight = pointC;
    } else {
      topRight = pointD;
    }

                
            
    int dimensionTop = transitionsBetween(topLeft, topRight).getTransitions();
    int dimensionRight = transitionsBetween(bottomRight, topRight).getTransitions();

    if ((dimensionTop & 0x01) == 1) {
            dimensionTop++;
    }
    dimensionTop += 2;

    if ((dimensionRight & 0x01) == 1) {
            dimensionRight++;
    }
    dimensionRight += 2;

    BitMatrix bits;
    ResultPoint correctedTopRight;

                if (4 * dimensionTop >= 7 * dimensionRight || 4 * dimensionRight >= 7 * dimensionTop) {
      
      correctedTopRight =
          correctTopRightRectangular(bottomLeft, bottomRight, topLeft, topRight, dimensionTop, dimensionRight);
      if (correctedTopRight == null) {
        correctedTopRight = topRight;
      }

      dimensionTop = transitionsBetween(topLeft, correctedTopRight).getTransitions();
      dimensionRight = transitionsBetween(bottomRight, correctedTopRight).getTransitions();

      if ((dimensionTop & 0x01) == 1) {
                dimensionTop++;
      }

      if ((dimensionRight & 0x01) == 1) {
                dimensionRight++;
      }

      bits = sampleGrid(image, topLeft, bottomLeft, bottomRight, correctedTopRight, dimensionTop, dimensionRight);

    } else {
      
      int dimension = Math.min(dimensionRight, dimensionTop);
            correctedTopRight = correctTopRight(bottomLeft, bottomRight, topLeft, topRight, dimension);
      if (correctedTopRight == null) {
        correctedTopRight = topRight;
      }

            int dimensionCorrected = Math.max(transitionsBetween(topLeft, correctedTopRight).getTransitions(),
                                transitionsBetween(bottomRight, correctedTopRight).getTransitions());
      dimensionCorrected++;
      if ((dimensionCorrected & 0x01) == 1) {
        dimensionCorrected++;
      }

      bits = sampleGrid(image,
                        topLeft,
                        bottomLeft,
                        bottomRight,
                        correctedTopRight,
                        dimensionCorrected,
                        dimensionCorrected);
    }

    return new DetectorResult(bits, new ResultPoint[]{topLeft, bottomLeft, bottomRight, correctedTopRight});
  }

  
  private ResultPoint correctTopRightRectangular(ResultPoint bottomLeft,
                                                 ResultPoint bottomRight,
                                                 ResultPoint topLeft,
                                                 ResultPoint topRight,
                                                 int dimensionTop,
                                                 int dimensionRight) {

    float corr = distance(bottomLeft, bottomRight) / (float) dimensionTop;
    int norm = distance(topLeft, topRight);
    float cos = (topRight.getX() - topLeft.getX()) / norm;
    float sin = (topRight.getY() - topLeft.getY()) / norm;

    ResultPoint c1 = new ResultPoint(topRight.getX() + corr * cos, topRight.getY() + corr * sin);

    corr = distance(bottomLeft, topLeft) / (float) dimensionRight;
    norm = distance(bottomRight, topRight);
    cos = (topRight.getX() - bottomRight.getX()) / norm;
    sin = (topRight.getY() - bottomRight.getY()) / norm;

    ResultPoint c2 = new ResultPoint(topRight.getX() + corr * cos, topRight.getY() + corr * sin);

    if (!isValid(c1)) {
      if (isValid(c2)) {
        return c2;
      }
      return null;
    }
    if (!isValid(c2)) {
      return c1;
    }

    int l1 = Math.abs(dimensionTop - transitionsBetween(topLeft, c1).getTransitions()) +
          Math.abs(dimensionRight - transitionsBetween(bottomRight, c1).getTransitions());
    int l2 = Math.abs(dimensionTop - transitionsBetween(topLeft, c2).getTransitions()) +
    Math.abs(dimensionRight - transitionsBetween(bottomRight, c2).getTransitions());

    if (l1 <= l2) {
      return c1;
    }

    return c2;
  }

  
  private ResultPoint correctTopRight(ResultPoint bottomLeft,
                                      ResultPoint bottomRight,
                                      ResultPoint topLeft,
                                      ResultPoint topRight,
                                      int dimension) {

    float corr = distance(bottomLeft, bottomRight) / (float) dimension;
    int norm = distance(topLeft, topRight);
    float cos = (topRight.getX() - topLeft.getX()) / norm;
    float sin = (topRight.getY() - topLeft.getY()) / norm;

    ResultPoint c1 = new ResultPoint(topRight.getX() + corr * cos, topRight.getY() + corr * sin);

    corr = distance(bottomLeft, topLeft) / (float) dimension;
    norm = distance(bottomRight, topRight);
    cos = (topRight.getX() - bottomRight.getX()) / norm;
    sin = (topRight.getY() - bottomRight.getY()) / norm;

    ResultPoint c2 = new ResultPoint(topRight.getX() + corr * cos, topRight.getY() + corr * sin);

    if (!isValid(c1)) {
      if (isValid(c2)) {
        return c2;
      }
      return null;
    }
    if (!isValid(c2)) {
      return c1;
    }

    int l1 = Math.abs(transitionsBetween(topLeft, c1).getTransitions() -
                      transitionsBetween(bottomRight, c1).getTransitions());
    int l2 = Math.abs(transitionsBetween(topLeft, c2).getTransitions() -
                      transitionsBetween(bottomRight, c2).getTransitions());

    return l1 <= l2 ? c1 : c2;
  }

  private boolean isValid(ResultPoint p) {
    return p.getX() >= 0 && p.getX() < image.getWidth() && p.getY() > 0 && p.getY() < image.getHeight();
  }

  private static int distance(ResultPoint a, ResultPoint b) {
    return MathUtils.round(ResultPoint.distance(a, b));
  }

  
  private static void increment(Map<ResultPoint,Integer> table, ResultPoint key) {
    Integer value = table.get(key);
    table.put(key, value == null ? 1 : value + 1);
  }

  private static BitMatrix sampleGrid(BitMatrix image,
                                      ResultPoint topLeft,
                                      ResultPoint bottomLeft,
                                      ResultPoint bottomRight,
                                      ResultPoint topRight,
                                      int dimensionX,
                                      int dimensionY) throws NotFoundException {

    GridSampler sampler = GridSampler.getInstance();

    return sampler.sampleGrid(image,
                              dimensionX,
                              dimensionY,
                              0.5f,
                              0.5f,
                              dimensionX - 0.5f,
                              0.5f,
                              dimensionX - 0.5f,
                              dimensionY - 0.5f,
                              0.5f,
                              dimensionY - 0.5f,
                              topLeft.getX(),
                              topLeft.getY(),
                              topRight.getX(),
                              topRight.getY(),
                              bottomRight.getX(),
                              bottomRight.getY(),
                              bottomLeft.getX(),
                              bottomLeft.getY());
  }

  
  private ResultPointsAndTransitions transitionsBetween(ResultPoint from, ResultPoint to) {
        int fromX = (int) from.getX();
    int fromY = (int) from.getY();
    int toX = (int) to.getX();
    int toY = (int) to.getY();
    boolean steep = Math.abs(toY - fromY) > Math.abs(toX - fromX);
    if (steep) {
      int temp = fromX;
      fromX = fromY;
      fromY = temp;
      temp = toX;
      toX = toY;
      toY = temp;
    }

    int dx = Math.abs(toX - fromX);
    int dy = Math.abs(toY - fromY);
    int error = -dx / 2;
    int ystep = fromY < toY ? 1 : -1;
    int xstep = fromX < toX ? 1 : -1;
    int transitions = 0;
    boolean inBlack = image.get(steep ? fromY : fromX, steep ? fromX : fromY);
    for (int x = fromX, y = fromY; x != toX; x += xstep) {
      boolean isBlack = image.get(steep ? y : x, steep ? x : y);
      if (isBlack != inBlack) {
        transitions++;
        inBlack = isBlack;
      }
      error += dy;
      if (error > 0) {
        if (y == toY) {
          break;
        }
        y += ystep;
        error -= dx;
      }
    }
    return new ResultPointsAndTransitions(from, to, transitions);
  }

  
  private static final class ResultPointsAndTransitions {

    private final ResultPoint from;
    private final ResultPoint to;
    private final int transitions;

    private ResultPointsAndTransitions(ResultPoint from, ResultPoint to, int transitions) {
      this.from = from;
      this.to = to;
      this.transitions = transitions;
    }

    ResultPoint getFrom() {
      return from;
    }

    ResultPoint getTo() {
      return to;
    }

    int getTransitions() {
      return transitions;
    }

    @Override
    public String toString() {
      return from + "/" + to + '/' + transitions;
    }
  }

  
  private static final class ResultPointsAndTransitionsComparator
      implements Comparator<ResultPointsAndTransitions>, Serializable {
    @Override
    public int compare(ResultPointsAndTransitions o1, ResultPointsAndTransitions o2) {
      return o1.getTransitions() - o2.getTransitions();
    }
  }

}
