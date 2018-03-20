

package com.google.zxing.client.android.result;

import com.google.zxing.client.android.LocaleManager;
import com.google.zxing.client.android.R;
import com.google.zxing.client.result.ParsedResult;
import com.google.zxing.client.result.URIParsedResult;

import android.app.Activity;

import java.util.Locale;


public final class URIResultHandler extends ResultHandler {
      private static final String[] SECURE_PROTOCOLS = {
    "otpauth:"
  };

  private static final int[] buttons = {
      R.string.button_open_browser,
      R.string.button_share_by_email,
      R.string.button_share_by_sms,
      R.string.button_search_book_contents,
  };

  public URIResultHandler(Activity activity, ParsedResult result) {
    super(activity, result);
  }

  @Override
  public int getButtonCount() {
    if (LocaleManager.isBookSearchUrl(((URIParsedResult) getResult()).getURI())) {
      return buttons.length;
    }
    return buttons.length - 1;
  }

  @Override
  public int getButtonText(int index) {
    return buttons[index];
  }

  @Override
  public Integer getDefaultButtonID() {
    return 0;
  }

  @Override
  public void handleButtonPress(int index) {
    URIParsedResult uriResult = (URIParsedResult) getResult();
    String uri = uriResult.getURI();
    switch (index) {
      case 0:
        openURL(uri);
        break;
      case 1:
        shareByEmail(uri);
        break;
      case 2:
        shareBySMS(uri);
        break;
      case 3:
        searchBookContents(uri);
        break;
    }
  }

  @Override
  public int getDisplayTitle() {
    return R.string.result_uri;
  }

  @Override
  public boolean areContentsSecure() {
    URIParsedResult uriResult = (URIParsedResult) getResult();
    String uri = uriResult.getURI().toLowerCase(Locale.ENGLISH);
    for (String secure : SECURE_PROTOCOLS) {
      if (uri.startsWith(secure)) {
        return true;
      }
    }
    return false;
  }
}
