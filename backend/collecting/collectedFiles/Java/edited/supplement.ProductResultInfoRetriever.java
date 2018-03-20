

package com.google.zxing.client.android.result.supplement;

import android.content.Context;
import android.text.Html;
import android.widget.TextView;
import com.google.zxing.client.android.HttpHelper;
import com.google.zxing.client.android.R;
import com.google.zxing.client.android.history.HistoryManager;
import com.google.zxing.client.android.LocaleManager;

import java.io.IOException;
import java.net.URLEncoder;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


final class ProductResultInfoRetriever extends SupplementalInfoRetriever {

  private static final Pattern[] PRODUCT_NAME_PRICE_PATTERNS = {
    Pattern.compile(",event\\)\">([^<]+)</a></h3>.+<span class=psrp>([^<]+)</span>"),
    Pattern.compile("owb63p\">([^<]+).+zdi3pb\">([^<]+)"),
  };

  private final String productID;
  private final String source;
  private final Context context;

  ProductResultInfoRetriever(TextView textView, String productID, HistoryManager historyManager, Context context) {
    super(textView, historyManager);
    this.productID = productID;
    this.source = context.getString(R.string.msg_google_product);
    this.context = context;
  }

  @Override
  void retrieveSupplementalInfo() throws IOException {

    String encodedProductID = URLEncoder.encode(productID, "UTF-8");
    String uri = "https:            + "/m/products?ie=utf8&oe=utf8&scoring=p&source=zxing&q=" + encodedProductID;
    CharSequence content = HttpHelper.downloadViaHttp(uri, HttpHelper.ContentType.HTML);

    for (Pattern p : PRODUCT_NAME_PRICE_PATTERNS) {
      Matcher matcher = p.matcher(content);
      if (matcher.find()) {
        append(productID,
               source,
               new String[] { unescapeHTML(matcher.group(1)), unescapeHTML(matcher.group(2)) },
               uri);
        break;
      }
    }
  }

  private static String unescapeHTML(String raw) {
    return Html.fromHtml(raw).toString();
  }

}
