

package com.google.zxing.client.j2se;

import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.common.BitMatrix;

import com.beust.jcommander.JCommander;

import java.nio.file.Paths;
import java.util.EnumMap;
import java.util.Locale;
import java.util.Map;


public final class CommandLineEncoder {

  private CommandLineEncoder() {
  }

  public static void main(String[] args) throws Exception {
    EncoderConfig config = new EncoderConfig();
    JCommander jCommander = new JCommander(config);
    jCommander.parse(args);
    jCommander.setProgramName(CommandLineEncoder.class.getSimpleName());
    if (config.help) {
      jCommander.usage();
      return;
    }

    String outFileString = config.outputFileBase;
    if (EncoderConfig.DEFAULT_OUTPUT_FILE_BASE.equals(outFileString)) {
      outFileString += '.' + config.imageFormat.toLowerCase(Locale.ENGLISH);
    }
    Map<EncodeHintType, Object> hints = new EnumMap<>(EncodeHintType.class);
    if (config.errorCorrectionLevel != null) {
      hints.put(EncodeHintType.ERROR_CORRECTION, config.errorCorrectionLevel);
    }
    BitMatrix matrix = new MultiFormatWriter().encode(
        config.contents.get(0), config.barcodeFormat, config.width,
        config.height, hints);
    MatrixToImageWriter.writeToPath(matrix, config.imageFormat,
        Paths.get(outFileString));
  }

}
