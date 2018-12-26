

package org.elasticsearch.bootstrap;

import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

public class SecurityTests extends ESTestCase {
    
    public void testEnsureExists() throws IOException {
        Path p = createTempDir();

                Path exists = p.resolve("exists");
        Files.createDirectory(exists);
        Security.ensureDirectoryExists(exists);
        Files.createTempFile(exists, null, null);
    }
    
    public void testEnsureNotExists() throws IOException { 
        Path p = createTempDir();

                Path notExists = p.resolve("notexists");
        Security.ensureDirectoryExists(notExists);
        Files.createTempFile(notExists, null, null);
    }
    
    public void testEnsureRegularFile() throws IOException {
        Path p = createTempDir();

                Path regularFile = p.resolve("regular");
        Files.createFile(regularFile);
        try {
            Security.ensureDirectoryExists(regularFile);
            fail("didn't get expected exception");
        } catch (IOException expected) {}
    }
    
    
    public void testProcessExecution() throws Exception {
        assumeTrue("test requires security manager", System.getSecurityManager() != null);
        try {
            Runtime.getRuntime().exec("ls");
            fail("didn't get expected exception");
        } catch (SecurityException expected) {}
    }
}
