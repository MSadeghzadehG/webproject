

package com.google.zxing.common;

import java.util.List;


public final class DecoderResult {

  private final byte[] rawBytes;
  private int numBits;
  private final String text;
  private final List<byte[]> byteSegments;
  private final String ecLevel;
  private Integer errorsCorrected;
  private Integer erasures;
  private Object other;
  private final int structuredAppendParity;
  private final int structuredAppendSequenceNumber;

  public DecoderResult(byte[] rawBytes,
                       String text,
                       List<byte[]> byteSegments,
                       String ecLevel) {
    this(rawBytes, text, byteSegments, ecLevel, -1, -1);
  }

  public DecoderResult(byte[] rawBytes,
                       String text,
                       List<byte[]> byteSegments,
                       String ecLevel,
                       int saSequence,
                       int saParity) {
    this.rawBytes = rawBytes;
    this.numBits = rawBytes == null ? 0 : 8 * rawBytes.length;
    this.text = text;
    this.byteSegments = byteSegments;
    this.ecLevel = ecLevel;
    this.structuredAppendParity = saParity;
    this.structuredAppendSequenceNumber = saSequence;
  }

  
  public byte[] getRawBytes() {
    return rawBytes;
  }

  
  public int getNumBits() {
    return numBits;
  }

  
  public void setNumBits(int numBits) {
    this.numBits = numBits;
  }

  
  public String getText() {
    return text;
  }

  
  public List<byte[]> getByteSegments() {
    return byteSegments;
  }

  
  public String getECLevel() {
    return ecLevel;
  }

  
  public Integer getErrorsCorrected() {
    return errorsCorrected;
  }

  public void setErrorsCorrected(Integer errorsCorrected) {
    this.errorsCorrected = errorsCorrected;
  }

  
  public Integer getErasures() {
    return erasures;
  }

  public void setErasures(Integer erasures) {
    this.erasures = erasures;
  }

  
  public Object getOther() {
    return other;
  }

  public void setOther(Object other) {
    this.other = other;
  }

  public boolean hasStructuredAppend() {
    return structuredAppendParity >= 0 && structuredAppendSequenceNumber >= 0;
  }

  public int getStructuredAppendParity() {
    return structuredAppendParity;
  }

  public int getStructuredAppendSequenceNumber() {
    return structuredAppendSequenceNumber;
  }

}
