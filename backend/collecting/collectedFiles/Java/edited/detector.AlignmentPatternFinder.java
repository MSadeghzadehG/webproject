

package com.google.zxing.qrcode.detector;

import com.google.zxing.NotFoundException;
import com.google.zxing.ResultPointCallback;
import com.google.zxing.common.BitMatrix;

import java.util.ArrayList;
import java.util.List;


final class AlignmentPatternFinder {

  private final BitMatrix image;
  private final List<AlignmentPattern> possibleCenters;
  private final int startX;
  private final int startY;
  private final int width;
  private final int height;
  private final float moduleSize;
  private final int[] crossCheckStateCount;
  private final ResultPointCallback resultPointCallback;

  
  AlignmentPatternFinder(BitMatrix image,
                         int startX,
                         int startY,
                         int width,
                         int height,
                         float moduleSize,
                         ResultPointCallback resultPointCallback) {
    this.image = image;
    this.possibleCenters = new ArrayList<>(5);
    this.startX = startX;
    this.startY = startY;
    this.width = width;
    this.height = height;
    this.moduleSize = moduleSize;
    this.crossCheckStateCount = new int[3];
    this.resultPointCallback = resultPointCallback;
  }

  
  AlignmentPattern find() throws NotFoundException {
    int startX = this.startX;
    int height = this.height;
    int maxJ = startX + width;
    int middleI = startY + (height / 2);
            int[] stateCount = new int[3];
    for (int iGen = 0; iGen < height; iGen++) {
            int i = middleI + ((iGen & 0x01) == 0 ? (iGen + 1) / 2 : -((iGen + 1) / 2));
      stateCount[0] = 0;
      stateCount[1] = 0;
      stateCount[2] = 0;
      int j = startX;
                        while (j < maxJ && !image.get(j, i)) {
        j++;
      }
      int currentState = 0;
      while (j < maxJ) {
        if (image.get(j, i)) {
                    if (currentState == 1) {             stateCount[1]++;
          } else {             if (currentState == 2) {               if (foundPatternCross(stateCount)) {                 AlignmentPattern confirmed = handlePossibleCenter(stateCount, i, j);
                if (confirmed != null) {
                  return confirmed;
                }
              }
              stateCount[0] = stateCount[2];
              stateCount[1] = 1;
              stateCount[2] = 0;
              currentState = 1;
            } else {
              stateCount[++currentState]++;
            }
          }
        } else {           if (currentState == 1) {             currentState++;
          }
          stateCount[currentState]++;
        }
        j++;
      }
      if (foundPatternCross(stateCount)) {
        AlignmentPattern confirmed = handlePossibleCenter(stateCount, i, maxJ);
        if (confirmed != null) {
          return confirmed;
        }
      }

    }

            if (!possibleCenters.isEmpty()) {
      return possibleCenters.get(0);
    }

    throw NotFoundException.getNotFoundInstance();
  }

  
  private static float centerFromEnd(int[] stateCount, int end) {
    return (end - stateCount[2]) - stateCount[1] / 2.0f;
  }

  
  private boolean foundPatternCross(int[] stateCount) {
    float moduleSize = this.moduleSize;
    float maxVariance = moduleSize / 2.0f;
    for (int i = 0; i < 3; i++) {
      if (Math.abs(moduleSize - stateCount[i]) >= maxVariance) {
        return false;
      }
    }
    return true;
  }

  
  private float crossCheckVertical(int startI, int centerJ, int maxCount,
      int originalStateCountTotal) {
    BitMatrix image = this.image;

    int maxI = image.getHeight();
    int[] stateCount = crossCheckStateCount;
    stateCount[0] = 0;
    stateCount[1] = 0;
    stateCount[2] = 0;

        int i = startI;
    while (i >= 0 && image.get(centerJ, i) && stateCount[1] <= maxCount) {
      stateCount[1]++;
      i--;
    }
        if (i < 0 || stateCount[1] > maxCount) {
      return Float.NaN;
    }
    while (i >= 0 && !image.get(centerJ, i) && stateCount[0] <= maxCount) {
      stateCount[0]++;
      i--;
    }
    if (stateCount[0] > maxCount) {
      return Float.NaN;
    }

        i = startI + 1;
    while (i < maxI && image.get(centerJ, i) && stateCount[1] <= maxCount) {
      stateCount[1]++;
      i++;
    }
    if (i == maxI || stateCount[1] > maxCount) {
      return Float.NaN;
    }
    while (i < maxI && !image.get(centerJ, i) && stateCount[2] <= maxCount) {
      stateCount[2]++;
      i++;
    }
    if (stateCount[2] > maxCount) {
      return Float.NaN;
    }

    int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
    if (5 * Math.abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
      return Float.NaN;
    }

    return foundPatternCross(stateCount) ? centerFromEnd(stateCount, i) : Float.NaN;
  }

  
  private AlignmentPattern handlePossibleCenter(int[] stateCount, int i, int j) {
    int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
    float centerJ = centerFromEnd(stateCount, j);
    float centerI = crossCheckVertical(i, (int) centerJ, 2 * stateCount[1], stateCountTotal);
    if (!Float.isNaN(centerI)) {
      float estimatedModuleSize = (stateCount[0] + stateCount[1] + stateCount[2]) / 3.0f;
      for (AlignmentPattern center : possibleCenters) {
                if (center.aboutEquals(estimatedModuleSize, centerI, centerJ)) {
          return center.combineEstimate(centerI, centerJ, estimatedModuleSize);
        }
      }
            AlignmentPattern point = new AlignmentPattern(centerJ, centerI, estimatedModuleSize);
      possibleCenters.add(point);
      if (resultPointCallback != null) {
        resultPointCallback.foundPossibleResultPoint(point);
      }
    }
    return null;
  }

}
