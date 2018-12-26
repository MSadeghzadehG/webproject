

package io.reactivex;

import static org.junit.Assert.fail;

import java.lang.reflect.*;

import org.junit.Test;


public class PublicFinalMethods {

    static void scan(Class<?> clazz) {
        for (Method m : clazz.getMethods()) {
            if (m.getDeclaringClass() == clazz) {
                if ((m.getModifiers() & Modifier.STATIC) == 0) {
                    if ((m.getModifiers() & (Modifier.PUBLIC | Modifier.FINAL)) == Modifier.PUBLIC) {
                        fail("Not final: " + m);
                    }
                }
            }
        }
    }

    @Test
    public void flowable() {
        scan(Flowable.class);
    }

    @Test
    public void observable() {
        scan(Observable.class);
    }

    @Test
    public void single() {
        scan(Single.class);
    }

    @Test
    public void completable() {
        scan(Completable.class);
    }

    @Test
    public void maybe() {
        scan(Maybe.class);
    }
}
