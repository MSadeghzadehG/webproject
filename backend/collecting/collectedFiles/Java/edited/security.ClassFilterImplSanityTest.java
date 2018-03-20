
package jenkins.security;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.TreeSet;
import java.util.stream.Collectors;
import org.apache.commons.io.IOUtils;
import static org.hamcrest.Matchers.*;
import static org.junit.Assert.assertThat;
import org.junit.Test;
import org.jvnet.hudson.test.For;


@For(ClassFilterImpl.class)
public class ClassFilterImplSanityTest {

    @Test
    public void whitelistSanity() throws Exception {
        try (InputStream is = ClassFilterImpl.class.getResourceAsStream("whitelisted-classes.txt")) {
            List<String> lines = IOUtils.readLines(is, StandardCharsets.UTF_8).stream().filter(line -> !line.matches("#.*|\\s*")).collect(Collectors.toList());
            assertThat("whitelist is NOT ordered", new TreeSet<>(lines), contains(lines.toArray(new String[0])));
            for (String line : lines) {
                try {
                    Class.forName(line);
                } catch (ClassNotFoundException x) {
                    System.err.println("skipping checks of unknown class " + line);
                }
            }
        }
    }

}
