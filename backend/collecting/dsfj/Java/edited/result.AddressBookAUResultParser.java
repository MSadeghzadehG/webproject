

package com.google.zxing.client.result;

import com.google.zxing.Result;

import java.util.ArrayList;
import java.util.List;


public final class AddressBookAUResultParser extends ResultParser {

  @Override
  public AddressBookParsedResult parse(Result result) {
    String rawText = getMassagedText(result);
        if (!rawText.contains("MEMORY") || !rawText.contains("\r\n")) {
      return null;
    }

            String name = matchSinglePrefixedField("NAME1:", rawText, '\r', true);
    String pronunciation = matchSinglePrefixedField("NAME2:", rawText, '\r', true);

    String[] phoneNumbers = matchMultipleValuePrefix("TEL", 3, rawText, true);
    String[] emails = matchMultipleValuePrefix("MAIL", 3, rawText, true);
    String note = matchSinglePrefixedField("MEMORY:", rawText, '\r', false);
    String address = matchSinglePrefixedField("ADD:", rawText, '\r', true);
    String[] addresses = address == null ? null : new String[] {address};
    return new AddressBookParsedResult(maybeWrap(name),
                                       null,
                                       pronunciation,
                                       phoneNumbers,
                                       null,
                                       emails,
                                       null,
                                       null,
                                       note,
                                       addresses,
                                       null,
                                       null,
                                       null,
                                       null,
                                       null,
                                       null);
  }

  private static String[] matchMultipleValuePrefix(String prefix,
                                                   int max,
                                                   String rawText,
                                                   boolean trim) {
    List<String> values = null;
    for (int i = 1; i <= max; i++) {
      String value = matchSinglePrefixedField(prefix + i + ':', rawText, '\r', trim);
      if (value == null) {
        break;
      }
      if (values == null) {
        values = new ArrayList<>(max);       }
      values.add(value);
    }
    if (values == null) {
      return null;
    }
    return values.toArray(new String[values.size()]);
  }

}
