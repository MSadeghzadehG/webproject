

package com.google.zxing.oned;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.DecodeHintType;
import com.google.zxing.NotFoundException;
import com.google.zxing.Reader;
import com.google.zxing.ReaderException;
import com.google.zxing.Result;
import com.google.zxing.common.BitArray;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;


public final class MultiFormatUPCEANReader extends OneDReader {

  private final UPCEANReader[] readers;

  public MultiFormatUPCEANReader(Map<DecodeHintType,?> hints) {
    @SuppressWarnings("unchecked")
    Collection<BarcodeFormat> possibleFormats = hints == null ? null :
        (Collection<BarcodeFormat>) hints.get(DecodeHintType.POSSIBLE_FORMATS);
    Collection<UPCEANReader> readers = new ArrayList<>();
    if (possibleFormats != null) {
      if (possibleFormats.contains(BarcodeFormat.EAN_13)) {
        readers.add(new EAN13Reader());
      } else if (possibleFormats.contains(BarcodeFormat.UPC_A)) {
        readers.add(new UPCAReader());
      }
      if (possibleFormats.contains(BarcodeFormat.EAN_8)) {
        readers.add(new EAN8Reader());
      }
      if (possibleFormats.contains(BarcodeFormat.UPC_E)) {
        readers.add(new UPCEReader());
      }
    }
    if (readers.isEmpty()) {
      readers.add(new EAN13Reader());
            readers.add(new EAN8Reader());
      readers.add(new UPCEReader());
    }
    this.readers = readers.toArray(new UPCEANReader[readers.size()]);
  }

  @Override
  public Result decodeRow(int rowNumber,
                          BitArray row,
                          Map<DecodeHintType,?> hints) throws NotFoundException {
        int[] startGuardPattern = UPCEANReader.findStartGuardPattern(row);
    for (UPCEANReader reader : readers) {
      try {
        Result result = reader.decodeRow(rowNumber, row, startGuardPattern, hints);
                                                                                                        boolean ean13MayBeUPCA =
            result.getBarcodeFormat() == BarcodeFormat.EAN_13 &&
                result.getText().charAt(0) == '0';
        @SuppressWarnings("unchecked")
        Collection<BarcodeFormat> possibleFormats =
            hints == null ? null : (Collection<BarcodeFormat>) hints.get(DecodeHintType.POSSIBLE_FORMATS);
        boolean canReturnUPCA = possibleFormats == null || possibleFormats.contains(BarcodeFormat.UPC_A);
  
        if (ean13MayBeUPCA && canReturnUPCA) {
                    Result resultUPCA = new Result(result.getText().substring(1),
                                         result.getRawBytes(),
                                         result.getResultPoints(),
                                         BarcodeFormat.UPC_A);
          resultUPCA.putAllMetadata(result.getResultMetadata());
          return resultUPCA;
        }
        return result;
      } catch (ReaderException ignored) {
              }
    }

    throw NotFoundException.getNotFoundInstance();
  }

  @Override
  public void reset() {
    for (Reader reader : readers) {
      reader.reset();
    }
  }

}
