

package com.google.zxing.multi;

import com.google.zxing.BinaryBitmap;
import com.google.zxing.ChecksumException;
import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.NotFoundException;
import com.google.zxing.Reader;
import com.google.zxing.Result;
import com.google.zxing.ResultPoint;

import java.util.Map;


public final class ByQuadrantReader implements Reader {

  private final Reader delegate;

  public ByQuadrantReader(Reader delegate) {
    this.delegate = delegate;
  }

  @Override
  public Result decode(BinaryBitmap image)
      throws NotFoundException, ChecksumException, FormatException {
    return decode(image, null);
  }

  @Override
  public Result decode(BinaryBitmap image, Map<DecodeHintType,?> hints)
      throws NotFoundException, ChecksumException, FormatException {

    int width = image.getWidth();
    int height = image.getHeight();
    int halfWidth = width / 2;
    int halfHeight = height / 2;

    try {
            return delegate.decode(image.crop(0, 0, halfWidth, halfHeight), hints);
    } catch (NotFoundException re) {
          }

    try {
      Result result = delegate.decode(image.crop(halfWidth, 0, halfWidth, halfHeight), hints);
      makeAbsolute(result.getResultPoints(), halfWidth, 0);
      return result;
    } catch (NotFoundException re) {
          }

    try {
      Result result = delegate.decode(image.crop(0, halfHeight, halfWidth, halfHeight), hints);
      makeAbsolute(result.getResultPoints(), 0, halfHeight);
      return result;
    } catch (NotFoundException re) {
          }

    try {
      Result result = delegate.decode(image.crop(halfWidth, halfHeight, halfWidth, halfHeight), hints);
      makeAbsolute(result.getResultPoints(), halfWidth, halfHeight);
      return result;
    } catch (NotFoundException re) {
          }

    int quarterWidth = halfWidth / 2;
    int quarterHeight = halfHeight / 2;
    BinaryBitmap center = image.crop(quarterWidth, quarterHeight, halfWidth, halfHeight);
    Result result = delegate.decode(center, hints);
    makeAbsolute(result.getResultPoints(), quarterWidth, quarterHeight);
    return result;
  }

  @Override
  public void reset() {
    delegate.reset();
  }

  private static void makeAbsolute(ResultPoint[] points, int leftOffset, int topOffset) {
    if (points != null) {
      for (int i = 0; i < points.length; i++) {
        ResultPoint relative = points[i];
        points[i] = new ResultPoint(relative.getX() + leftOffset, relative.getY() + topOffset);
      }
    }
  }

}
