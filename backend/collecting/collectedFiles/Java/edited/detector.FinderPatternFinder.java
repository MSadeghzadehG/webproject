

package com.google.zxing.qrcode.detector;

import com.google.zxing.DecodeHintType;
import com.google.zxing.NotFoundException;
import com.google.zxing.ResultPoint;
import com.google.zxing.ResultPointCallback;
import com.google.zxing.common.BitMatrix;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;


public class FinderPatternFinder {

  private static final int CENTER_QUORUM = 2;
  protected static final int MIN_SKIP = 3;   protected static final int MAX_MODULES = 97; 
  private final BitMatrix image;
  private final List<FinderPattern> possibleCenters;
  private boolean hasSkipped;
  private final int[] crossCheckStateCount;
  private final ResultPointCallback resultPointCallback;

  
  public FinderPatternFinder(BitMatrix image) {
    this(image, null);
  }

  public FinderPatternFinder(BitMatrix image, ResultPointCallback resultPointCallback) {
    this.image = image;
    this.possibleCenters = new ArrayList<>();
    this.crossCheckStateCount = new int[5];
    this.resultPointCallback = resultPointCallback;
  }

  protected final BitMatrix getImage() {
    return image;
  }

  protected final List<FinderPattern> getPossibleCenters() {
    return possibleCenters;
  }

  final FinderPatternInfo find(Map<DecodeHintType,?> hints) throws NotFoundException {
    boolean tryHarder = hints != null && hints.containsKey(DecodeHintType.TRY_HARDER);
    int maxI = image.getHeight();
    int maxJ = image.getWidth();
        
                    int iSkip = (3 * maxI) / (4 * MAX_MODULES);
    if (iSkip < MIN_SKIP || tryHarder) {
      iSkip = MIN_SKIP;
    }

    boolean done = false;
    int[] stateCount = new int[5];
    for (int i = iSkip - 1; i < maxI && !done; i += iSkip) {
            clearCounts(stateCount);
      int currentState = 0;
      for (int j = 0; j < maxJ; j++) {
        if (image.get(j, i)) {
                    if ((currentState & 1) == 1) {             currentState++;
          }
          stateCount[currentState]++;
        } else {           if ((currentState & 1) == 0) {             if (currentState == 4) {               if (foundPatternCross(stateCount)) {                 boolean confirmed = handlePossibleCenter(stateCount, i, j);
                if (confirmed) {
                                                      iSkip = 2;
                  if (hasSkipped) {
                    done = haveMultiplyConfirmedCenters();
                  } else {
                    int rowSkip = findRowSkip();
                    if (rowSkip > stateCount[2]) {
                                                                                        
                                                                                        i += rowSkip - stateCount[2] - iSkip;
                      j = maxJ - 1;
                    }
                  }
                } else {
                  shiftCounts2(stateCount);
                  currentState = 3;
                  continue;
                }
                                currentState = 0;
                clearCounts(stateCount);
              } else {                 shiftCounts2(stateCount);
                currentState = 3;
              }
            } else {
              stateCount[++currentState]++;
            }
          } else {             stateCount[currentState]++;
          }
        }
      }
      if (foundPatternCross(stateCount)) {
        boolean confirmed = handlePossibleCenter(stateCount, i, maxJ);
        if (confirmed) {
          iSkip = stateCount[0];
          if (hasSkipped) {
                        done = haveMultiplyConfirmedCenters();
          }
        }
      }
    }

    FinderPattern[] patternInfo = selectBestPatterns();
    ResultPoint.orderBestPatterns(patternInfo);

    return new FinderPatternInfo(patternInfo);
  }

  
  private static float centerFromEnd(int[] stateCount, int end) {
    return (end - stateCount[4] - stateCount[3]) - stateCount[2] / 2.0f;
  }

  
  protected static boolean foundPatternCross(int[] stateCount) {
    int totalModuleSize = 0;
    for (int i = 0; i < 5; i++) {
      int count = stateCount[i];
      if (count == 0) {
        return false;
      }
      totalModuleSize += count;
    }
    if (totalModuleSize < 7) {
      return false;
    }
    float moduleSize = totalModuleSize / 7.0f;
    float maxVariance = moduleSize / 2.0f;
        return
        Math.abs(moduleSize - stateCount[0]) < maxVariance &&
        Math.abs(moduleSize - stateCount[1]) < maxVariance &&
        Math.abs(3.0f * moduleSize - stateCount[2]) < 3 * maxVariance &&
        Math.abs(moduleSize - stateCount[3]) < maxVariance &&
        Math.abs(moduleSize - stateCount[4]) < maxVariance;
  }

  
  protected static boolean foundPatternDiagonal(int[] stateCount) {
    int totalModuleSize = 0;
    for (int i = 0; i < 5; i++) {
      int count = stateCount[i];
      if (count == 0) {
        return false;
      }
      totalModuleSize += count;
    }
    if (totalModuleSize < 7) {
      return false;
    }
    float moduleSize = totalModuleSize / 7.0f;
    float maxVariance = moduleSize / 1.333f;
        return
            Math.abs(moduleSize - stateCount[0]) < maxVariance &&
                    Math.abs(moduleSize - stateCount[1]) < maxVariance &&
                    Math.abs(3.0f * moduleSize - stateCount[2]) < 3 * maxVariance &&
                    Math.abs(moduleSize - stateCount[3]) < maxVariance &&
                    Math.abs(moduleSize - stateCount[4]) < maxVariance;
  }

  private int[] getCrossCheckStateCount() {
    clearCounts(crossCheckStateCount);
    return crossCheckStateCount;
  }

  protected final void clearCounts(int[] counts) {
    for (int x = 0; x < counts.length; x++) {
      counts[x] = 0;
    }
  }

  protected final void shiftCounts2(int[] stateCount) {
    stateCount[0] = stateCount[2];
    stateCount[1] = stateCount[3];
    stateCount[2] = stateCount[4];
    stateCount[3] = 1;
    stateCount[4] = 0;
  }

  
  private boolean crossCheckDiagonal(int centerI, int centerJ) {
    int[] stateCount = getCrossCheckStateCount();

        int i = 0;
    while (centerI >= i && centerJ >= i && image.get(centerJ - i, centerI - i)) {
      stateCount[2]++;
      i++;
    }
    if (stateCount[2] == 0) {
      return false;
    }

        while (centerI >= i && centerJ >= i && !image.get(centerJ - i, centerI - i)) {
      stateCount[1]++;
      i++;
    }
    if (stateCount[1] == 0) {
      return false;
    }

        while (centerI >= i && centerJ >= i && image.get(centerJ - i, centerI - i)) {
      stateCount[0]++;
      i++;
    }
    if (stateCount[0] == 0) {
      return false;
    }

    int maxI = image.getHeight();
    int maxJ = image.getWidth();

        i = 1;
    while (centerI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, centerI + i)) {
      stateCount[2]++;
      i++;
    }

    while (centerI + i < maxI && centerJ + i < maxJ && !image.get(centerJ + i, centerI + i)) {
      stateCount[3]++;
      i++;
    }
    if (stateCount[3] == 0) {
      return false;
    }

    while (centerI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, centerI + i)) {
      stateCount[4]++;
      i++;
    }
    if (stateCount[4] == 0) {
      return false;
    }

    return foundPatternDiagonal(stateCount);
  }

  
  private float crossCheckVertical(int startI, int centerJ, int maxCount,
      int originalStateCountTotal) {
    BitMatrix image = this.image;

    int maxI = image.getHeight();
    int[] stateCount = getCrossCheckStateCount();

        int i = startI;
    while (i >= 0 && image.get(centerJ, i)) {
      stateCount[2]++;
      i--;
    }
    if (i < 0) {
      return Float.NaN;
    }
    while (i >= 0 && !image.get(centerJ, i) && stateCount[1] <= maxCount) {
      stateCount[1]++;
      i--;
    }
        if (i < 0 || stateCount[1] > maxCount) {
      return Float.NaN;
    }
    while (i >= 0 && image.get(centerJ, i) && stateCount[0] <= maxCount) {
      stateCount[0]++;
      i--;
    }
    if (stateCount[0] > maxCount) {
      return Float.NaN;
    }

        i = startI + 1;
    while (i < maxI && image.get(centerJ, i)) {
      stateCount[2]++;
      i++;
    }
    if (i == maxI) {
      return Float.NaN;
    }
    while (i < maxI && !image.get(centerJ, i) && stateCount[3] < maxCount) {
      stateCount[3]++;
      i++;
    }
    if (i == maxI || stateCount[3] >= maxCount) {
      return Float.NaN;
    }
    while (i < maxI && image.get(centerJ, i) && stateCount[4] < maxCount) {
      stateCount[4]++;
      i++;
    }
    if (stateCount[4] >= maxCount) {
      return Float.NaN;
    }

            int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] +
        stateCount[4];
    if (5 * Math.abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
      return Float.NaN;
    }

    return foundPatternCross(stateCount) ? centerFromEnd(stateCount, i) : Float.NaN;
  }

  
  private float crossCheckHorizontal(int startJ, int centerI, int maxCount,
      int originalStateCountTotal) {
    BitMatrix image = this.image;

    int maxJ = image.getWidth();
    int[] stateCount = getCrossCheckStateCount();

    int j = startJ;
    while (j >= 0 && image.get(j, centerI)) {
      stateCount[2]++;
      j--;
    }
    if (j < 0) {
      return Float.NaN;
    }
    while (j >= 0 && !image.get(j, centerI) && stateCount[1] <= maxCount) {
      stateCount[1]++;
      j--;
    }
    if (j < 0 || stateCount[1] > maxCount) {
      return Float.NaN;
    }
    while (j >= 0 && image.get(j, centerI) && stateCount[0] <= maxCount) {
      stateCount[0]++;
      j--;
    }
    if (stateCount[0] > maxCount) {
      return Float.NaN;
    }

    j = startJ + 1;
    while (j < maxJ && image.get(j, centerI)) {
      stateCount[2]++;
      j++;
    }
    if (j == maxJ) {
      return Float.NaN;
    }
    while (j < maxJ && !image.get(j, centerI) && stateCount[3] < maxCount) {
      stateCount[3]++;
      j++;
    }
    if (j == maxJ || stateCount[3] >= maxCount) {
      return Float.NaN;
    }
    while (j < maxJ && image.get(j, centerI) && stateCount[4] < maxCount) {
      stateCount[4]++;
      j++;
    }
    if (stateCount[4] >= maxCount) {
      return Float.NaN;
    }

            int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] +
        stateCount[4];
    if (5 * Math.abs(stateCountTotal - originalStateCountTotal) >= originalStateCountTotal) {
      return Float.NaN;
    }

    return foundPatternCross(stateCount) ? centerFromEnd(stateCount, j) : Float.NaN;
  }

  
  @Deprecated
  protected final boolean handlePossibleCenter(int[] stateCount, int i, int j, boolean pureBarcode) {
    return handlePossibleCenter(stateCount, i, j);
  }

  
  protected final boolean handlePossibleCenter(int[] stateCount, int i, int j) {
    int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] +
        stateCount[4];
    float centerJ = centerFromEnd(stateCount, j);
    float centerI = crossCheckVertical(i, (int) centerJ, stateCount[2], stateCountTotal);
    if (!Float.isNaN(centerI)) {
            centerJ = crossCheckHorizontal((int) centerJ, (int) centerI, stateCount[2], stateCountTotal);
      if (!Float.isNaN(centerJ) && crossCheckDiagonal((int) centerI, (int) centerJ)) {
        float estimatedModuleSize = stateCountTotal / 7.0f;
        boolean found = false;
        for (int index = 0; index < possibleCenters.size(); index++) {
          FinderPattern center = possibleCenters.get(index);
                    if (center.aboutEquals(estimatedModuleSize, centerI, centerJ)) {
            possibleCenters.set(index, center.combineEstimate(centerI, centerJ, estimatedModuleSize));
            found = true;
            break;
          }
        }
        if (!found) {
          FinderPattern point = new FinderPattern(centerJ, centerI, estimatedModuleSize);
          possibleCenters.add(point);
          if (resultPointCallback != null) {
            resultPointCallback.foundPossibleResultPoint(point);
          }
        }
        return true;
      }
    }
    return false;
  }

  
  private int findRowSkip() {
    int max = possibleCenters.size();
    if (max <= 1) {
      return 0;
    }
    ResultPoint firstConfirmedCenter = null;
    for (FinderPattern center : possibleCenters) {
      if (center.getCount() >= CENTER_QUORUM) {
        if (firstConfirmedCenter == null) {
          firstConfirmedCenter = center;
        } else {
                                                            hasSkipped = true;
          return (int) (Math.abs(firstConfirmedCenter.getX() - center.getX()) -
              Math.abs(firstConfirmedCenter.getY() - center.getY())) / 2;
        }
      }
    }
    return 0;
  }

  
  private boolean haveMultiplyConfirmedCenters() {
    int confirmedCount = 0;
    float totalModuleSize = 0.0f;
    int max = possibleCenters.size();
    for (FinderPattern pattern : possibleCenters) {
      if (pattern.getCount() >= CENTER_QUORUM) {
        confirmedCount++;
        totalModuleSize += pattern.getEstimatedModuleSize();
      }
    }
    if (confirmedCount < 3) {
      return false;
    }
                    float average = totalModuleSize / max;
    float totalDeviation = 0.0f;
    for (FinderPattern pattern : possibleCenters) {
      totalDeviation += Math.abs(pattern.getEstimatedModuleSize() - average);
    }
    return totalDeviation <= 0.05f * totalModuleSize;
  }

  
  private FinderPattern[] selectBestPatterns() throws NotFoundException {

    int startSize = possibleCenters.size();
    if (startSize < 3) {
            throw NotFoundException.getNotFoundInstance();
    }

        if (startSize > 3) {
            float totalModuleSize = 0.0f;
      float square = 0.0f;
      for (FinderPattern center : possibleCenters) {
        float size = center.getEstimatedModuleSize();
        totalModuleSize += size;
        square += size * size;
      }
      float average = totalModuleSize / startSize;
      float stdDev = (float) Math.sqrt(square / startSize - average * average);

      Collections.sort(possibleCenters, new FurthestFromAverageComparator(average));

      float limit = Math.max(0.2f * average, stdDev);

      for (int i = 0; i < possibleCenters.size() && possibleCenters.size() > 3; i++) {
        FinderPattern pattern = possibleCenters.get(i);
        if (Math.abs(pattern.getEstimatedModuleSize() - average) > limit) {
          possibleCenters.remove(i);
          i--;
        }
      }
    }

    if (possibleCenters.size() > 3) {
      
      float totalModuleSize = 0.0f;
      for (FinderPattern possibleCenter : possibleCenters) {
        totalModuleSize += possibleCenter.getEstimatedModuleSize();
      }

      float average = totalModuleSize / possibleCenters.size();

      Collections.sort(possibleCenters, new CenterComparator(average));

      possibleCenters.subList(3, possibleCenters.size()).clear();
    }

    return new FinderPattern[]{
        possibleCenters.get(0),
        possibleCenters.get(1),
        possibleCenters.get(2)
    };
  }

  
  private static final class FurthestFromAverageComparator implements Comparator<FinderPattern>, Serializable {
    private final float average;
    private FurthestFromAverageComparator(float f) {
      average = f;
    }
    @Override
    public int compare(FinderPattern center1, FinderPattern center2) {
      return Float.compare(Math.abs(center2.getEstimatedModuleSize() - average),
                           Math.abs(center1.getEstimatedModuleSize() - average));
    }
  }

  
  private static final class CenterComparator implements Comparator<FinderPattern>, Serializable {
    private final float average;
    private CenterComparator(float f) {
      average = f;
    }
    @Override
    public int compare(FinderPattern center1, FinderPattern center2) {
      int countCompare = Integer.compare(center2.getCount(), center1.getCount());
      if (countCompare == 0) {
        return Float.compare(Math.abs(center1.getEstimatedModuleSize() - average),
                             Math.abs(center2.getEstimatedModuleSize() - average));
      }
      return countCompare;
    }
  }

}
