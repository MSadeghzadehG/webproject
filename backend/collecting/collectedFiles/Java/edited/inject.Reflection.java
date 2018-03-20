


package org.elasticsearch.common.inject;

import java.lang.reflect.Constructor;


class Reflection {

    
    static class InvalidConstructor {
        InvalidConstructor() {
            throw new AssertionError();
        }
    }

    @SuppressWarnings("unchecked")
    static <T> Constructor<T> invalidConstructor() {
        try {
            return (Constructor<T>) InvalidConstructor.class.getConstructor();
        } catch (NoSuchMethodException e) {
            throw new AssertionError(e);
        }
    }
}
