

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.util.Locale;


public class LocaleSerializer extends AbstractSerializer {
    private static LocaleSerializer SERIALIZER = new LocaleSerializer();

    public static LocaleSerializer create() {
        return SERIALIZER;
    }

    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        if (obj == null)
            out.writeNull();
        else {
            Locale locale = (Locale) obj;

            out.writeObject(new LocaleHandle(locale.getLanguage(), locale.getCountry(), locale.getVariant()));
        }
    }
}
