

package com.alibaba.com.caucho.hessian.io;

import java.util.Locale;


public class LocaleHandle implements java.io.Serializable, HessianHandle {
    private String language;
    private String country;
    private String variant;

    public LocaleHandle(String language, String country, String variant) {
        this.language = language;
        this.country = country;
        this.variant = variant;
    }

    private Object readResolve() {
        return new Locale(language, country, variant);
    }
}
