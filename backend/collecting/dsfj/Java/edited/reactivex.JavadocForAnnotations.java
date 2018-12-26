

package io.reactivex;

import static org.junit.Assert.fail;

import java.io.*;

import org.junit.*;


public class JavadocForAnnotations {

    static void checkSource(String baseClassName, boolean scheduler) throws Exception {
        File f = MaybeNo2Dot0Since.findSource(baseClassName);
        if (f == null) {
            return;
        }

        StringBuilder b = readFile(f);

        StringBuilder e = new StringBuilder();

        if (scheduler) {
            scanFor(b, "@SchedulerSupport", "Scheduler:", e, baseClassName);
        } else {
            scanFor(b, "@BackpressureSupport", "Backpressure:", e, baseClassName);
        }

        if (e.length() != 0) {
            System.out.println(e);

            fail(e.toString());
        }
    }

    public static StringBuilder readFile(File f) throws Exception {
        StringBuilder b = new StringBuilder();

        BufferedReader in = new BufferedReader(new FileReader(f));
        try {
            for (;;) {
                String line = in.readLine();

                if (line == null) {
                    break;
                }

                b.append(line).append('\n');
            }
        } finally {
            in.close();
        }

        return b;
    }

    static final void scanFor(StringBuilder sourceCode, String annotation, String inDoc,
            StringBuilder e, String baseClassName) {
        int index = 0;
        for (;;) {
            int idx = sourceCode.indexOf(annotation, index);

            if (idx < 0) {
                break;
            }

            int j = sourceCode.lastIndexOf("