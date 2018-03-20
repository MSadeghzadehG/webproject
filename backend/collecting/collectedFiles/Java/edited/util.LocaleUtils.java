

package org.elasticsearch.common.util;


import java.util.Arrays;
import java.util.Locale;
import java.util.MissingResourceException;


public class LocaleUtils {

    
    public static Locale parse(String localeStr) {
        boolean useUnderscoreAsSeparator = false;
        for (int i = 0; i < localeStr.length(); ++i) {
            final char c = localeStr.charAt(i);
            if (c == '-') {
                                break;
            } else if (c == '_') {
                useUnderscoreAsSeparator = true;
                break;
            }
        }

        final String[] parts;
        if (useUnderscoreAsSeparator) {
            parts = localeStr.split("_", -1);
        } else {
            parts = localeStr.split("-", -1);
        }

        final Locale locale = parseParts(parts);

        try {
            locale.getISO3Language();
        } catch (MissingResourceException e) {
            throw new IllegalArgumentException("Unknown language: " + parts[0], e);
        }

        try {
            locale.getISO3Country();
        } catch (MissingResourceException e) {
            throw new IllegalArgumentException("Unknown country: " + parts[1], e);
        }

        return locale;
    }

    
    @Deprecated
    public static Locale parse5x(String localeStr) {
        final String[] parts = localeStr.split("_", -1);
        return parseParts(parts);
    }

    private static Locale parseParts(String[] parts) {
        switch (parts.length) {
        case 3:
                        return new Locale(parts[0], parts[1], parts[2]);
        case 2:
                        return new Locale(parts[0], parts[1]);
        case 1:
            if ("ROOT".equalsIgnoreCase(parts[0])) {
                return Locale.ROOT;
            }
                        return new Locale(parts[0]);
        default:
            throw new IllegalArgumentException("Locales can have at most 3 parts but got " + parts.length + ": " + Arrays.asList(parts));
        }
    }
}
