

package io.reactivex;

import java.io.File;
import java.util.*;


public final class BaseTypeParser {

    private BaseTypeParser() {
        throw new IllegalStateException("No instances!");
    }

    public static class RxMethod {
        public String signature;

        public String backpressureKind;

        public String schedulerKind;

        public String javadoc;

        public String backpressureDocumentation;

        public String schedulerDocumentation;

        public int javadocLine;

        public int methodLine;

        public int backpressureDocLine;

        public int schedulerDocLine;
    }

    public static List<RxMethod> parse(File f, String baseClassName) throws Exception {
        List<RxMethod> list = new ArrayList<RxMethod>();

        StringBuilder b = JavadocForAnnotations.readFile(f);

        int baseIndex = b.indexOf("public abstract class " + baseClassName);

        if (baseIndex < 0) {
            throw new AssertionError("Wrong base class file: " + baseClassName);
        }

        for (;;) {
            RxMethod m = new RxMethod();

            int javadocStart = b.indexOf("", javadocStart + 2);

            m.javadoc = b.substring(javadocStart, javadocEnd + 2);
            m.javadocLine = JavadocForAnnotations.lineNumber(b, javadocStart);

            int backpressureDoc = b.indexOf("<dt><b>Backpressure:</b></dt>", javadocStart);
            if (backpressureDoc > 0 && backpressureDoc < javadocEnd) {
                m.backpressureDocLine = JavadocForAnnotations.lineNumber(b, backpressureDoc);
                int nextDD = b.indexOf("</dd>", backpressureDoc);
                if (nextDD > 0 && nextDD < javadocEnd) {
                    m.backpressureDocumentation = b.substring(backpressureDoc, nextDD + 5);
                }
            }

            int schedulerDoc = b.indexOf("<dt><b>Scheduler:</b></dt>", javadocStart);
            if (schedulerDoc > 0 && schedulerDoc < javadocEnd) {
                m.schedulerDocLine = JavadocForAnnotations.lineNumber(b, schedulerDoc);
                int nextDD = b.indexOf("</dd>", schedulerDoc);
                if (nextDD > 0 && nextDD < javadocEnd) {
                    m.schedulerDocumentation = b.substring(schedulerDoc, nextDD + 5);
                }
            }

            int staticMethodDef = b.indexOf("public static ", javadocEnd + 2);
            if (staticMethodDef < 0) {
                staticMethodDef = Integer.MAX_VALUE;
            }
            int instanceMethodDef = b.indexOf("public final ", javadocEnd + 2);
            if (instanceMethodDef < 0) {
                instanceMethodDef = Integer.MAX_VALUE;
            }

            int javadocStartNext = b.indexOf("