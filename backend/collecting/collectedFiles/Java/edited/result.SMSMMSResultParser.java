

package com.google.zxing.client.result;

import com.google.zxing.Result;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;


public final class SMSMMSResultParser extends ResultParser {

  @Override
  public SMSParsedResult parse(Result result) {
    String rawText = getMassagedText(result);
    if (!(rawText.startsWith("sms:") || rawText.startsWith("SMS:") ||
          rawText.startsWith("mms:") || rawText.startsWith("MMS:"))) {
      return null;
    }

        Map<String,String> nameValuePairs = parseNameValuePairs(rawText);
    String subject = null;
    String body = null;
    boolean querySyntax = false;
    if (nameValuePairs != null && !nameValuePairs.isEmpty()) {
      subject = nameValuePairs.get("subject");
      body = nameValuePairs.get("body");
      querySyntax = true;
    }

        int queryStart = rawText.indexOf('?', 4);
    String smsURIWithoutQuery;
        if (queryStart < 0 || !querySyntax) {
      smsURIWithoutQuery = rawText.substring(4);
    } else {
      smsURIWithoutQuery = rawText.substring(4, queryStart);
    }

    int lastComma = -1;
    int comma;
    List<String> numbers = new ArrayList<>(1);
    List<String> vias = new ArrayList<>(1);
    while ((comma = smsURIWithoutQuery.indexOf(',', lastComma + 1)) > lastComma) {
      String numberPart = smsURIWithoutQuery.substring(lastComma + 1, comma);
      addNumberVia(numbers, vias, numberPart);
      lastComma = comma;
    }
    addNumberVia(numbers, vias, smsURIWithoutQuery.substring(lastComma + 1));    

    return new SMSParsedResult(numbers.toArray(new String[numbers.size()]),
                               vias.toArray(new String[vias.size()]),
                               subject,
                               body);
  }

  private static void addNumberVia(Collection<String> numbers,
                                   Collection<String> vias,
                                   String numberPart) {
    int numberEnd = numberPart.indexOf(';');
    if (numberEnd < 0) {
      numbers.add(numberPart);
      vias.add(null);
    } else {
      numbers.add(numberPart.substring(0, numberEnd));
      String maybeVia = numberPart.substring(numberEnd + 1);
      String via;
      if (maybeVia.startsWith("via=")) {
        via = maybeVia.substring(4);
      } else {
        via = null;
      }
      vias.add(via);
    }
  }

}