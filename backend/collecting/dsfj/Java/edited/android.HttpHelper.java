

package com.google.zxing.client.android;

import android.util.Log;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;


public final class HttpHelper {

  private static final String TAG = HttpHelper.class.getSimpleName();

  private static final Collection<String> REDIRECTOR_DOMAINS = new HashSet<>(Arrays.asList(
    "amzn.to", "bit.ly", "bitly.com", "fb.me", "goo.gl", "is.gd", "j.mp", "lnkd.in", "ow.ly",
    "R.BEETAGG.COM", "r.beetagg.com", "SCN.BY", "su.pr", "t.co", "tinyurl.com", "tr.im"
  ));

  private HttpHelper() {
  }

  
  public enum ContentType {
    
    HTML,
    
    JSON,
    
    XML,
    
    TEXT,
  }

  
  public static CharSequence downloadViaHttp(String uri, ContentType type) throws IOException {
    return downloadViaHttp(uri, type, Integer.MAX_VALUE);
  }

  
  public static CharSequence downloadViaHttp(String uri, ContentType type, int maxChars) throws IOException {
    String contentTypes;
    switch (type) {
      case HTML:
        contentTypes = "application/xhtml+xml,text/html,text*";
        break;
      case JSON:
        contentTypes = "application/json,text*";
        break;
      case XML:
        contentTypes = "application/xml,text*";
        break;
      default:         contentTypes = "text*";
    }
    return downloadViaHttp(uri, contentTypes, maxChars);
  }

  private static CharSequence downloadViaHttp(String uri, String contentTypes, int maxChars) throws IOException {
    int redirects = 0;
    while (redirects < 5) {
      URL url = new URL(uri);
      HttpURLConnection connection = safelyOpenConnection(url);
      connection.setInstanceFollowRedirects(true);       connection.setRequestProperty("Accept", contentTypes);
      connection.setRequestProperty("Accept-Charset", "utf-8,*");
      connection.setRequestProperty("User-Agent", "ZXing (Android)");
      try {
        int responseCode = safelyConnect(connection);
        switch (responseCode) {
          case HttpURLConnection.HTTP_OK:
            return consume(connection, maxChars);
          case HttpURLConnection.HTTP_MOVED_TEMP:
            String location = connection.getHeaderField("Location");
            if (location != null) {
              uri = location;
              redirects++;
              continue;
            }
            throw new IOException("No Location");
          default:
            throw new IOException("Bad HTTP response: " + responseCode);
        }
      } finally {
        connection.disconnect();
      }
    }
    throw new IOException("Too many redirects");
  }

  private static String getEncoding(URLConnection connection) {
    String contentTypeHeader = connection.getHeaderField("Content-Type");
    if (contentTypeHeader != null) {
      int charsetStart = contentTypeHeader.indexOf("charset=");
      if (charsetStart >= 0) {
        return contentTypeHeader.substring(charsetStart + "charset=".length());
      }
    }
    return "UTF-8";
  }

  private static CharSequence consume(URLConnection connection, int maxChars) throws IOException {
    String encoding = getEncoding(connection);
    StringBuilder out = new StringBuilder();
    try (Reader in = new InputStreamReader(connection.getInputStream(), encoding)) {
      char[] buffer = new char[1024];
      int charsRead;
      while (out.length() < maxChars && (charsRead = in.read(buffer)) > 0) {
        out.append(buffer, 0, charsRead);
      }
    }
    return out;
  }

  public static URI unredirect(URI uri) throws IOException {
    if (!REDIRECTOR_DOMAINS.contains(uri.getHost())) {
      return uri;
    }
    URL url = uri.toURL();
    HttpURLConnection connection = safelyOpenConnection(url);
    connection.setInstanceFollowRedirects(false);
    connection.setDoInput(false);
    connection.setRequestMethod("HEAD");
    connection.setRequestProperty("User-Agent", "ZXing (Android)");
    try {
      int responseCode = safelyConnect(connection);
      switch (responseCode) {
        case HttpURLConnection.HTTP_MULT_CHOICE:
        case HttpURLConnection.HTTP_MOVED_PERM:
        case HttpURLConnection.HTTP_MOVED_TEMP:
        case HttpURLConnection.HTTP_SEE_OTHER:
        case 307:           String location = connection.getHeaderField("Location");
          if (location != null) {
            try {
              return new URI(location);
            } catch (URISyntaxException e) {
                          }
          }
      }
      return uri;
    } finally {
      connection.disconnect();
    }
  }
  
  private static HttpURLConnection safelyOpenConnection(URL url) throws IOException {
    URLConnection conn;
    try {
      conn = url.openConnection();
    } catch (NullPointerException npe) {
            Log.w(TAG, "Bad URI? " + url);
      throw new IOException(npe);
    }
    if (!(conn instanceof HttpURLConnection)) {
      throw new IOException();
    }
    return (HttpURLConnection) conn;
  }

  private static int safelyConnect(HttpURLConnection connection) throws IOException {
    try {
      connection.connect();
    } catch (RuntimeException e) {
            throw new IOException(e);
    }
    try {
      return connection.getResponseCode();
    } catch (NullPointerException | StringIndexOutOfBoundsException | IllegalArgumentException e) {
            throw new IOException(e);
    }
  }

}
