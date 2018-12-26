

package jenkins;

import com.google.common.collect.Iterators;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.jar.JarFile;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ErrorCollector;
import org.jvnet.hudson.test.Issue;
import org.jvnet.hudson.test.WarExploder;

public class ClassPathTest {

    @Rule
    public ErrorCollector errors = new ErrorCollector();

    @Ignore("TODO too many failures to solve them all now")
    @Issue("JENKINS-46754")
    @Test
    public void uniqueness() throws Exception {
        Map<String, List<String>> entries = new TreeMap<>();
        for (File jar : new File(WarExploder.getExplodedDir(), "WEB-INF/lib").listFiles((dir, name) -> name.endsWith(".jar"))) {
            String jarname = jar.getName();
            try (JarFile jf = new JarFile(jar)) {
                Iterators.forEnumeration(jf.entries()).forEachRemaining(e -> {
                    String name = e.getName();
                    if (name.startsWith("META-INF/") || name.endsWith("/") || !name.contains("/")) {
                        return;
                    }
                    entries.computeIfAbsent(name, k -> new ArrayList<>()).add(jarname);
                });
            }
        }
        entries.forEach((name, jarnames) -> {
            if (jarnames.size() > 1) {                 errors.addError(new AssertionError(name + " duplicated in " + jarnames));
            }
        });
    }

}
