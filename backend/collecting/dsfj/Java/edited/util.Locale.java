

package java.util;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.ObjectStreamField;
import java.io.Serializable;



public final class Locale implements Cloneable, Serializable {

    private static final long serialVersionUID = 9149081749638150636L;

    
    public static final Locale CANADA = new Locale(true, "en", "CA");

    
    public static final Locale CANADA_FRENCH = new Locale(true, "fr", "CA");

    
    public static final Locale CHINA = new Locale(true, "zh", "CN");

    
    public static final Locale CHINESE = new Locale(true, "zh", "");

    
    public static final Locale ENGLISH = new Locale(true, "en", "");

    
    public static final Locale FRANCE = new Locale(true, "fr", "FR");

    
    public static final Locale FRENCH = new Locale(true, "fr", "");

    
    public static final Locale GERMAN = new Locale(true, "de", "");

    
    public static final Locale GERMANY = new Locale(true, "de", "DE");

    
    public static final Locale ITALIAN = new Locale(true, "it", "");

    
    public static final Locale ITALY = new Locale(true, "it", "IT");

    
    public static final Locale JAPAN = new Locale(true, "ja", "JP");

    
    public static final Locale JAPANESE = new Locale(true, "ja", "");

    
    public static final Locale KOREA = new Locale(true, "ko", "KR");

    
    public static final Locale KOREAN = new Locale(true, "ko", "");

    
    public static final Locale PRC = new Locale(true, "zh", "CN");

    
    public static final Locale ROOT = new Locale(true, "", "");

    
    public static final Locale SIMPLIFIED_CHINESE = new Locale(true, "zh", "CN");

    
    public static final Locale TAIWAN = new Locale(true, "zh", "TW");

    
    public static final Locale TRADITIONAL_CHINESE = new Locale(true, "zh", "TW");

    
    public static final Locale UK = new Locale(true, "en", "GB");

    
    public static final Locale US = new Locale(true, "en", "US");

    private static Locale defaultLocale = US;


    private transient String countryCode;
    private transient String languageCode;
    private transient String variantCode;
    private transient String cachedToStringResult;

    
    private Locale(boolean unused, String lowerCaseLanguageCode, String upperCaseCountryCode) {
        this.languageCode = lowerCaseLanguageCode;
        this.countryCode = upperCaseCountryCode;
        this.variantCode = "";
    }

    
    public Locale(String language) {
        this(language, "", "");
    }

    
    public Locale(String language, String country) {
        this(language, country, "");
    }

    
    public Locale(String language, String country, String variant) {
        if (language == null || country == null || variant == null) {
            throw new NullPointerException("language=" + language +
                                           ",country=" + country +
                                           ",variant=" + variant);
        }
        if (language.isEmpty() && country.isEmpty()) {
            languageCode = "";
            countryCode = "";
            variantCode = variant;
            return;
        }

        languageCode = language.toLowerCase();
                        if (languageCode.equals("he")) {
            languageCode = "iw";
        } else if (languageCode.equals("id")) {
            languageCode = "in";
        } else if (languageCode.equals("yi")) {
            languageCode = "ji";
        }

        countryCode = country.toUpperCase();

                variantCode = variant;
    }


    
    @Override public boolean equals(Object object) {
        if (object == this) {
            return true;
        }
        if (object instanceof Locale) {
            Locale o = (Locale) object;
            return languageCode.equals(o.languageCode)
                    && countryCode.equals(o.countryCode)
                    && variantCode.equals(o.variantCode);
        }
        return false;
    }


    
    public String getCountry() {
        return countryCode;
    }

    
    public static Locale getDefault() {
        return defaultLocale;
    }


    
    public String getLanguage() {
        return languageCode;
    }

    
    public String getVariant() {
        return variantCode;
    }

    @Override
    public synchronized int hashCode() {
        return countryCode.hashCode() + languageCode.hashCode()
                + variantCode.hashCode();
    }

    
    public synchronized static void setDefault(Locale locale) {
        if (locale == null) {
            throw new NullPointerException("locale == null");
        }
        defaultLocale = locale;
    }

    
    @Override
    public final String toString() {
        String result = cachedToStringResult;
        if (result == null) {
            result = cachedToStringResult = toNewString(languageCode, countryCode, variantCode);
        }
        return result;
    }

    private static String toNewString(String languageCode, String countryCode, String variantCode) {
                if (languageCode.length() == 0 && countryCode.length() == 0) {
            return "";
        }
                                        StringBuilder result = new StringBuilder(11);
        result.append(languageCode);
        if (countryCode.length() > 0 || variantCode.length() > 0) {
            result.append('_');
        }
        result.append(countryCode);
        if (variantCode.length() > 0) {
            result.append('_');
        }
        result.append(variantCode);
        return result.toString();
    }

}
