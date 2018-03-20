

package com.google.zxing.client.result;

import com.google.zxing.Result;

import java.util.regex.Pattern;


public final class EmailDoCoMoResultParser extends AbstractDoCoMoResultParser {

  private static final Pattern ATEXT_ALPHANUMERIC = Pattern.compile("[a-zA-Z0-9@.!#$%&'*+\\-/=?^_`{|}~]+");

  @Override
  public EmailAddressParsedResult parse(Result result) {
    String rawText = getMassagedText(result);
    if (!rawText.startsWith("MATMSG:")) {
      return null;
    }
    String[] tos = matchDoCoMoPrefixedField("TO:", rawText, true);
    if (tos == null) {
      return null;
    }
    for (String to : tos) {
      if (!isBasicallyValidEmailAddress(to)) {
        return null;
      }
    }
    String subject = matchSingleDoCoMoPrefixedField("SUB:", rawText, false);
    String body = matchSingleDoCoMoPrefixedField("BODY:", rawText, false);
    return new EmailAddressParsedResult(tos, null, null, subject, body);
  }

  
  static boolean isBasicallyValidEmailAddress(String email) {
    return email != null && ATEXT_ALPHANUMERIC.matcher(email).matches() && email.indexOf('@') >= 0;
  }

}