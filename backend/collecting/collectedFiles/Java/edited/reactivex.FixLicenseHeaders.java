

package io.reactivex;

import java.io.*;
import java.util.*;

import org.junit.Test;


public class FixLicenseHeaders {

    String[] header = {
    "",
    ""
    };

    @Test
    public void checkAndUpdateLicenses() throws Exception {
        if (System.getenv("CI") != null) {
                        return;
        }
        File f = MaybeNo2Dot0Since.findSource("Flowable");
        if (f == null) {
            return;
        }

        Queue<File> dirs = new ArrayDeque<File>();

        File parent = f.getParentFile();
        dirs.offer(parent);
        dirs.offer(new File(parent.getAbsolutePath().replace('\\', '/').replace("src/main/java", "src/perf/java")));
        dirs.offer(new File(parent.getAbsolutePath().replace('\\', '/').replace("src/main/java", "src/test/java")));

        StringBuilder fail = new StringBuilder();

        while (!dirs.isEmpty()) {
            f = dirs.poll();

            File[] list = f.listFiles();
            if (list != null && list.length != 0) {

                for (File u : list) {
                    if (u.isDirectory()) {
                        dirs.offer(u);
                    } else {
                        if (u.getName().endsWith(".java")) {

                            List<String> lines = new ArrayList<String>();
                            BufferedReader in = new BufferedReader(new FileReader(u));
                            try {
                                for (;;) {
                                    String line = in.readLine();
                                    if (line == null) {
                                        break;
                                    }

                                    lines.add(line);
                                }
                            } finally {
                                in.close();
                            }

                            if (!lines.get(0).equals(header[0]) && !lines.get(1).equals(header[1])) {
                                fail.append("java.lang.RuntimeException: missing header added, refresh and re-run tests!\r\n")
                                .append(" at ")
                                ;

                                String fn = u.toString().replace('\\', '/');

                                int idx = fn.indexOf("io/reactivex/");

                                fn = fn.substring(idx).replace('/', '.').replace(".java", "");

                                fail.append(fn).append(" (")
                                ;

                                int jdx = fn.lastIndexOf('.');

                                fail.append(fn.substring(jdx + 1));

                                fail.append(".java:1)\r\n\r\n");

                                lines.addAll(0, Arrays.asList(header));

                                PrintWriter w = new PrintWriter(new FileWriter(u));

                                try {
                                    for (String s : lines) {
                                        w.println(s);
                                    }
                                } finally {
                                    w.close();
                                }
                            }
                        }
                    }
                }
            }
        }

        if (fail.length() != 0) {
            System.out.println(fail);
            throw new AssertionError(fail.toString());
        }
    }
}
