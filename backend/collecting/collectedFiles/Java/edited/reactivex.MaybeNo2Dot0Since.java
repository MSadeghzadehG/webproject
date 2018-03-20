

package io.reactivex;

import static org.junit.Assert.fail;

import java.io.*;
import java.net.URL;

import org.junit.Test;


public class MaybeNo2Dot0Since {

    
    public static File findSource(String baseClassName) throws Exception {
        URL u = MaybeNo2Dot0Since.class.getResource(MaybeNo2Dot0Since.class.getSimpleName() + ".class");

        String path = new File(u.toURI()).toString().replace('\\', '/');


        int i = path.indexOf("/RxJava");
        if (i < 0) {
            System.out.println("Can't find the base RxJava directory");
            return null;
        }

                int j = path.indexOf("/", i + 6);

        String p = path.substring(0, j + 1) + "src/main/java/io/reactivex/" + baseClassName + ".java";

        File f = new File(p);

        if (!f.canRead()) {
            System.out.println("Can't read " + p);
            return null;
        }

        return f;
    }

    @Test
    public void noSince20InMaybe() throws Exception {

        File f = findSource(Maybe.class.getSimpleName());

        String line;

        StringBuilder b = new StringBuilder();

        boolean classDefPassed = false;

        BufferedReader in = new BufferedReader(new FileReader(f));
        try {
            int ln = 1;
            while (true) {
                line = in.readLine();

                if (line == null) {
                    break;
                }

                if (line.startsWith("public abstract class Maybe<")) {
                    classDefPassed = true;
                }

                if (classDefPassed) {
                    if (line.contains("@since") && line.contains("2.0") && !line.contains("2.0.")) {
                        b.append("java.lang.RuntimeException: @since 2.0 found").append("\r\n")
                        .append(" at io.reactivex.Maybe (Maybe.java:").append(ln).append(")\r\n\r\n");
                        ;
                    }
                }

                ln++;
            }
        } finally {
            in.close();
        }

        if (b.length() != 0) {
            System.out.println(b);

            fail(b.toString());
        }
    }
}
