

package com.google.zxing.client.android.result.supplement;

import android.text.Html;
import android.widget.TextView;
import com.google.zxing.client.android.HttpHelper;
import com.google.zxing.client.android.history.HistoryManager;
import com.google.zxing.client.result.URIParsedResult;

import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


final class TitleRetriever extends SupplementalInfoRetriever {

  private static final Pattern TITLE_PATTERN = Pattern.compile("<title>([^<]+)");
  private static final int MAX_TITLE_LEN = 100;

  private final String httpUrl;

  TitleRetriever(TextView textView, URIParsedResult result, HistoryManager historyManager) {
    super(textView, historyManager);
    this.httpUrl = result.getURI();
  }

  @Override
  void retrieveSupplementalInfo() {
    CharSequence contents;
    try {
      contents = HttpHelper.downloadViaHttp(httpUrl, HttpHelper.ContentType.HTML, 4096);
    } catch (IOException ioe) {
            return;
    }
    if (contents != null && contents.length() > 0) {
      Matcher m = TITLE_PATTERN.matcher(contents);
      if (m.find()) {
        String title = m.group(1);
        if (title != null && !title.isEmpty()) {
          title = Html.fromHtml(title).toString();
          if (title.length() > MAX_TITLE_LEN) {
            title = title.substring(0, MAX_TITLE_LEN) + "...";
          }
          append(httpUrl, null, new String[] {title}, httpUrl);
        }
      }
    }
  }

}
