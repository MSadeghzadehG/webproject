package jenkins;

import static org.junit.Assert.fail;

import hudson.remoting.Channel;
import hudson.remoting.Which;
import org.apache.commons.io.output.NullOutputStream;
import org.junit.Test;

import java.io.File;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import org.apache.commons.io.IOUtils;


public class RemotingJarSignatureTest {
    
    @Test
    public void testSignature() throws Exception {
        File jar = Which.jarFile(Channel.class);
        System.out.println("Verifying "+jar);

        JarFile myJar = new JarFile(jar,true);

        Enumeration<JarEntry> entries = myJar.entries();
        while (entries.hasMoreElements()) {
            JarEntry entry = entries.nextElement();
            if (entry.isDirectory())    continue;

                        String name = entry.getName();
            if (name.equals("META-INF/MANIFEST.MF")) continue;
            if (name.startsWith("META-INF/") && name.endsWith(".SF")) continue;
            if (name.startsWith("META-INF/") && name.endsWith(".RSA")) continue;
            if (name.startsWith("META-INF/") && name.endsWith(".DSA")) continue;

                        IOUtils.copy(myJar.getInputStream(entry), new NullOutputStream());
            if (entry.getCodeSigners()==null) {
                fail("No signature for " + name);
            }
        }
    }
}
