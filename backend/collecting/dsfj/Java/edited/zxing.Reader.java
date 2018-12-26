

package com.google.zxing;

import java.util.Map;


public interface Reader {

  
  Result decode(BinaryBitmap image) throws NotFoundException, ChecksumException, FormatException;

  
  Result decode(BinaryBitmap image, Map<DecodeHintType,?> hints)
      throws NotFoundException, ChecksumException, FormatException;

  
  void reset();

}
