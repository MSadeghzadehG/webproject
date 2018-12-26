

package com.google.zxing.client.android;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import java.util.Arrays;
import java.util.Collection;
import java.util.Locale;
import java.util.Map;
import java.util.HashMap;


public final class LocaleManager {

  private static final String DEFAULT_TLD = "com";
  private static final String DEFAULT_COUNTRY = "US";
  private static final String DEFAULT_LANGUAGE = "en";

  
  private static final Map<String,String> GOOGLE_COUNTRY_TLD;
  static {
    GOOGLE_COUNTRY_TLD = new HashMap<>();
    GOOGLE_COUNTRY_TLD.put("AR", "com.ar");     GOOGLE_COUNTRY_TLD.put("AU", "com.au");     GOOGLE_COUNTRY_TLD.put("BR", "com.br");     GOOGLE_COUNTRY_TLD.put("BG", "bg");     GOOGLE_COUNTRY_TLD.put(Locale.CANADA.getCountry(), "ca");
    GOOGLE_COUNTRY_TLD.put(Locale.CHINA.getCountry(), "cn");
    GOOGLE_COUNTRY_TLD.put("CZ", "cz");     GOOGLE_COUNTRY_TLD.put("DK", "dk");     GOOGLE_COUNTRY_TLD.put("FI", "fi");     GOOGLE_COUNTRY_TLD.put(Locale.FRANCE.getCountry(), "fr");
    GOOGLE_COUNTRY_TLD.put(Locale.GERMANY.getCountry(), "de");
    GOOGLE_COUNTRY_TLD.put("GR", "gr");     GOOGLE_COUNTRY_TLD.put("HU", "hu");     GOOGLE_COUNTRY_TLD.put("ID", "co.id");     GOOGLE_COUNTRY_TLD.put("IL", "co.il");     GOOGLE_COUNTRY_TLD.put(Locale.ITALY.getCountry(), "it");
    GOOGLE_COUNTRY_TLD.put(Locale.JAPAN.getCountry(), "co.jp");
    GOOGLE_COUNTRY_TLD.put(Locale.KOREA.getCountry(), "co.kr");
    GOOGLE_COUNTRY_TLD.put("NL", "nl");     GOOGLE_COUNTRY_TLD.put("PL", "pl");     GOOGLE_COUNTRY_TLD.put("PT", "pt");     GOOGLE_COUNTRY_TLD.put("RO", "ro");     GOOGLE_COUNTRY_TLD.put("RU", "ru");     GOOGLE_COUNTRY_TLD.put("SK", "sk");     GOOGLE_COUNTRY_TLD.put("SI", "si");     GOOGLE_COUNTRY_TLD.put("ES", "es");     GOOGLE_COUNTRY_TLD.put("SE", "se");     GOOGLE_COUNTRY_TLD.put("CH", "ch");     GOOGLE_COUNTRY_TLD.put(Locale.TAIWAN.getCountry(), "tw");
    GOOGLE_COUNTRY_TLD.put("TR", "com.tr");     GOOGLE_COUNTRY_TLD.put("UA", "com.ua");     GOOGLE_COUNTRY_TLD.put(Locale.UK.getCountry(), "co.uk");
    GOOGLE_COUNTRY_TLD.put(Locale.US.getCountry(), "com");
  }

  
  private static final Map<String,String> GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD;
  static {
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD = new HashMap<>();
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put("AU", "com.au");         GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put(Locale.FRANCE.getCountry(), "fr");
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put(Locale.GERMANY.getCountry(), "de");
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put(Locale.ITALY.getCountry(), "it");
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put(Locale.JAPAN.getCountry(), "co.jp");
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put("NL", "nl");     GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put("ES", "es");     GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put("CH", "ch");     GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put(Locale.UK.getCountry(), "co.uk");
    GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD.put(Locale.US.getCountry(), "com");
  }

  
  private static final Map<String,String> GOOGLE_BOOK_SEARCH_COUNTRY_TLD = GOOGLE_COUNTRY_TLD;

  private static final Collection<String> TRANSLATED_HELP_ASSET_LANGUAGES =
      Arrays.asList("de", "en", "es", "fa", "fr", "it", "ja", "ko", "nl", "pt", "ru", "uk", "zh-rCN", "zh");

  private LocaleManager() {}

  
  public static String getCountryTLD(Context context) {
    return doGetTLD(GOOGLE_COUNTRY_TLD, context);
  }

  
  public static String getProductSearchCountryTLD(Context context) {
    return doGetTLD(GOOGLE_PRODUCT_SEARCH_COUNTRY_TLD, context);
  }

  
  public static String getBookSearchCountryTLD(Context context) {
    return doGetTLD(GOOGLE_BOOK_SEARCH_COUNTRY_TLD, context);
  }

  
  public static boolean isBookSearchUrl(String url) {
    return url.startsWith("http:  }

  private static String getSystemCountry() {
    Locale locale = Locale.getDefault();
    return locale == null ? DEFAULT_COUNTRY : locale.getCountry();
  }

  private static String getSystemLanguage() {
    Locale locale = Locale.getDefault();
    if (locale == null) {
      return DEFAULT_LANGUAGE;
    }
        if (Locale.SIMPLIFIED_CHINESE.equals(locale)) {
      return "zh-rCN";
    }
    return locale.getLanguage();
  }

  static String getTranslatedAssetLanguage() {
    String language = getSystemLanguage();
    return TRANSLATED_HELP_ASSET_LANGUAGES.contains(language) ? language : DEFAULT_LANGUAGE;
  }

  private static String doGetTLD(Map<String,String> map, Context context) {
    String tld = map.get(getCountry(context));
    return tld == null ? DEFAULT_TLD : tld;
  }

  private static String getCountry(Context context) {
    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
    String countryOverride = prefs.getString(PreferencesActivity.KEY_SEARCH_COUNTRY, "-");
    if (countryOverride != null && !countryOverride.isEmpty() && !"-".equals(countryOverride)) {
      return countryOverride;
    }
    return getSystemCountry();
  }

}
