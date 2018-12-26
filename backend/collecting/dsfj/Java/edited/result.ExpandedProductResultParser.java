



package com.google.zxing.client.result;

import java.util.HashMap;
import java.util.Map;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.Result;


public final class ExpandedProductResultParser extends ResultParser {

  @Override
  public ExpandedProductParsedResult parse(Result result) {
    BarcodeFormat format = result.getBarcodeFormat();
    if (format != BarcodeFormat.RSS_EXPANDED) {
            return null;
    }
    String rawText = getMassagedText(result);

    String productID = null;
    String sscc = null;
    String lotNumber = null;
    String productionDate = null;
    String packagingDate = null;
    String bestBeforeDate = null;
    String expirationDate = null;
    String weight = null;
    String weightType = null;
    String weightIncrement = null;
    String price = null;
    String priceIncrement = null;
    String priceCurrency = null;
    Map<String,String> uncommonAIs = new HashMap<>();

    int i = 0;

    while (i < rawText.length()) {
      String ai = findAIvalue(i, rawText);
      if (ai == null) {
                        return null;
      }
      i += ai.length() + 2;
      String value = findValue(i, rawText);
      i += value.length();

      switch (ai) {
        case "00":
          sscc = value;
          break;
        case "01":
          productID = value;
          break;
        case "10":
          lotNumber = value;
          break;
        case "11":
          productionDate = value;
          break;
        case "13":
          packagingDate = value;
          break;
        case "15":
          bestBeforeDate = value;
          break;
        case "17":
          expirationDate = value;
          break;
        case "3100":
        case "3101":
        case "3102":
        case "3103":
        case "3104":
        case "3105":
        case "3106":
        case "3107":
        case "3108":
        case "3109":
          weight = value;
          weightType = ExpandedProductParsedResult.KILOGRAM;
          weightIncrement = ai.substring(3);
          break;
        case "3200":
        case "3201":
        case "3202":
        case "3203":
        case "3204":
        case "3205":
        case "3206":
        case "3207":
        case "3208":
        case "3209":
          weight = value;
          weightType = ExpandedProductParsedResult.POUND;
          weightIncrement = ai.substring(3);
          break;
        case "3920":
        case "3921":
        case "3922":
        case "3923":
          price = value;
          priceIncrement = ai.substring(3);
          break;
        case "3930":
        case "3931":
        case "3932":
        case "3933":
          if (value.length() < 4) {
                                                return null;
          }
          price = value.substring(3);
          priceCurrency = value.substring(0, 3);
          priceIncrement = ai.substring(3);
          break;
        default:
                    uncommonAIs.put(ai, value);
          break;
      }
    }

    return new ExpandedProductParsedResult(rawText,
                                           productID,
                                           sscc,
                                           lotNumber,
                                           productionDate,
                                           packagingDate,
                                           bestBeforeDate,
                                           expirationDate,
                                           weight,
                                           weightType,
                                           weightIncrement,
                                           price,
                                           priceIncrement,
                                           priceCurrency,
                                           uncommonAIs);
  }

  private static String findAIvalue(int i, String rawText) {
    char c = rawText.charAt(i);
        if (c != '(') {
      return null;
    }

    CharSequence rawTextAux = rawText.substring(i + 1);

    StringBuilder buf = new StringBuilder();
    for (int index = 0; index < rawTextAux.length(); index++) {
      char currentChar = rawTextAux.charAt(index);
      if (currentChar == ')') {
        return buf.toString();
      }
      if (currentChar < '0' || currentChar > '9') {
        return null;
      }
      buf.append(currentChar);
    }
    return buf.toString();
  }

  private static String findValue(int i, String rawText) {
    StringBuilder buf = new StringBuilder();
    String rawTextAux = rawText.substring(i);

    for (int index = 0; index < rawTextAux.length(); index++) {
      char c = rawTextAux.charAt(index);
      if (c == '(') {
                        if (findAIvalue(index, rawTextAux) != null) {
          break;
        }
        buf.append('(');
      } else {
        buf.append(c);
      }
    }
    return buf.toString();
  }
}
