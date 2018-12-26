
package jenkins.slaves;

import org.junit.Rule;
import org.junit.Test;
import org.jvnet.hudson.test.For;
import org.jvnet.hudson.test.Issue;
import org.jvnet.hudson.test.JenkinsRule;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThat;

@For(RemotingVersionInfo.class)
public class RemotingVersionInfoTest {

    @Rule
    public JenkinsRule j = new JenkinsRule();

    @Test
    @Issue("JENKINS-48766")
    public void warShouldIncludeRemotingManifestEntries() throws Exception {
        ZipFile jenkinsWar = new ZipFile(new File(j.getWebAppRoot(), "../jenkins.war"));
        ZipEntry entry = new JarEntry("META-INF/MANIFEST.MF");
        try (InputStream inputStream = jenkinsWar.getInputStream(entry)) {
            assertNotNull("Cannot open input stream for /META-INF/MANIFEST.MF", inputStream);
            Manifest manifest = new Manifest(inputStream);

            assertAttributeValue(manifest, "Remoting-Embedded-Version", RemotingVersionInfo.getEmbeddedVersion());
            assertAttributeValue(manifest, "Remoting-Minimum-Supported-Version", RemotingVersionInfo.getMinimumSupportedVersion());
        }
    }

    private void assertAttributeValue(Manifest manifest, String attributeName, Object expectedValue) throws AssertionError {
        assertThat("Wrong value of manifest attribute " + attributeName,
                manifest.getMainAttributes().getValue(attributeName),
                equalTo(expectedValue.toString()));
    }
}
