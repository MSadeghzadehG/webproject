

package com.google.zxing.client.result;

import java.util.regex.Pattern;


public final class URIParsedResult extends ParsedResult {

  private static final Pattern USER_IN_HOST = Pattern.compile(":
  public boolean isPossiblyMaliciousURI() {
    return USER_IN_HOST.matcher(uri).find();
  }

  @Override
  public String getDisplayResult() {
    StringBuilder result = new StringBuilder(30);
    maybeAppend(title, result);
    maybeAppend(uri, result);
    return result.toString();
  }

  
  private static String massageURI(String uri) {
    uri = uri.trim();
    int protocolEnd = uri.indexOf(':');
    if (protocolEnd < 0 || isColonFollowedByPortNumber(uri, protocolEnd)) {
                  uri = "http:    }
    return uri;
  }

  private static boolean isColonFollowedByPortNumber(String uri, int protocolEnd) {
    int start = protocolEnd + 1;
    int nextSlash = uri.indexOf('/', start);
    if (nextSlash < 0) {
      nextSlash = uri.length();
    }
    return ResultParser.isSubstringOfDigits(uri, start, nextSlash - start);
  }


}