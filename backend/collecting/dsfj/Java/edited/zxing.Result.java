

package com.google.zxing;

import java.util.EnumMap;
import java.util.Map;


public final class Result {

  private final String text;
  private final byte[] rawBytes;
  private final int numBits;
  private ResultPoint[] resultPoints;
  private final BarcodeFormat format;
  private Map<ResultMetadataType,Object> resultMetadata;
  private final long timestamp;

  public Result(String text,
                byte[] rawBytes,
                ResultPoint[] resultPoints,
                BarcodeFormat format) {
    this(text, rawBytes, resultPoints, format, System.currentTimeMillis());
  }

  public Result(String text,
                byte[] rawBytes,
                ResultPoint[] resultPoints,
                BarcodeFormat format,
                long timestamp) {
    this(text, rawBytes, rawBytes == null ? 0 : 8 * rawBytes.length,
         resultPoints, format, timestamp);
  }

  public Result(String text,
                byte[] rawBytes,
                int numBits,
                ResultPoint[] resultPoints,
                BarcodeFormat format,
                long timestamp) {
    this.text = text;
    this.rawBytes = rawBytes;
    this.numBits = numBits;
    this.resultPoints = resultPoints;
    this.format = format;
    this.resultMetadata = null;
    this.timestamp = timestamp;
  }

  
  public String getText() {
    return text;
  }

  
  public byte[] getRawBytes() {
    return rawBytes;
  }

  
  public int getNumBits() {
    return numBits;
  }

  
  public ResultPoint[] getResultPoints() {
    return resultPoints;
  }

  
  public BarcodeFormat getBarcodeFormat() {
    return format;
  }

  
  public Map<ResultMetadataType,Object> getResultMetadata() {
    return resultMetadata;
  }

  public void putMetadata(ResultMetadataType type, Object value) {
    if (resultMetadata == null) {
      resultMetadata = new EnumMap<>(ResultMetadataType.class);
    }
    resultMetadata.put(type, value);
  }

  public void putAllMetadata(Map<ResultMetadataType,Object> metadata) {
    if (metadata != null) {
      if (resultMetadata == null) {
        resultMetadata = metadata;
      } else {
        resultMetadata.putAll(metadata);
      }
    }
  }

  public void addResultPoints(ResultPoint[] newPoints) {
    ResultPoint[] oldPoints = resultPoints;
    if (oldPoints == null) {
      resultPoints = newPoints;
    } else if (newPoints != null && newPoints.length > 0) {
      ResultPoint[] allPoints = new ResultPoint[oldPoints.length + newPoints.length];
      System.arraycopy(oldPoints, 0, allPoints, 0, oldPoints.length);
      System.arraycopy(newPoints, 0, allPoints, oldPoints.length, newPoints.length);
      resultPoints = allPoints;
    }
  }

  public long getTimestamp() {
    return timestamp;
  }

  @Override
  public String toString() {
    return text;
  }

}
