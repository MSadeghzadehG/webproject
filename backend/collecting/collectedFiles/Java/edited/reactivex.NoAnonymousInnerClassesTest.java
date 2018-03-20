

package io.reactivex;

import java.io.File;
import java.net.URL;
import java.util.*;

import org.junit.Test;

public class NoAnonymousInnerClassesTest {

    @Test
    public void verify() throws Exception {
        URL u = NoAnonymousInnerClassesTest.class.getResource("/");
        File f = new File(u.toURI());

        StringBuilder b = new StringBuilder("Anonymous inner classes found:");

        Queue<File> queue = new ArrayDeque<File>();

        queue.offer(f);

        String prefix = f.getAbsolutePath();
        int count = 0;
        while (!queue.isEmpty()) {

            f = queue.poll();

            if (f.isDirectory()) {
                File[] dir = f.listFiles();
                if (dir != null && dir.length != 0) {
                    for (File g : dir) {
                        queue.offer(g);
                    }
                }
            } else {
                String name = f.getName();
                if (name.endsWith(".class") && name.contains("$")
                        && !name.contains("Perf") && !name.contains("Test")
                        && !name.startsWith("Test")) {
                    String[] parts = name.split("\\$");
                    for (String s : parts) {
                        if (Character.isDigit(s.charAt(0))) {
                            String n = f.getAbsolutePath().substring(prefix.length()).replace('\\', '.').replace('/', '.');
                            if (n.startsWith(".")) {
                                n = n.substring(1);
                            }
                            b.append("\r\n").append(n);
                            count++;
                            break;
                        }
                    }
                }
            }
        }

        if (count != 0) {
            throw new AssertionError(b.toString());
        }
    }
}
