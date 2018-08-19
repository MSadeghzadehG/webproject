

package org.elasticsearch.bootstrap;

import org.apache.lucene.util.Constants;
import org.elasticsearch.test.ESTestCase;


public class SystemCallFilterTests extends ESTestCase {

    
    static final String EXECUTABLE = Constants.WINDOWS ? "calc" : "ls";

    @Override
    public void setUp() throws Exception {
        super.setUp();
        assumeTrue("requires system call filter installation", Natives.isSystemCallFilterInstalled());
                assumeTrue("cannot test with security manager enabled", System.getSecurityManager() == null);
                                if (!JNANatives.LOCAL_SYSTEM_CALL_FILTER_ALL) {
            try {
                SystemCallFilter.init(createTempDir());
            } catch (Exception e) {
                throw new RuntimeException("unable to forcefully apply system call filter to test thread", e);
            }
        }
    }

    public void testNoExecution() throws Exception {
        try {
            Runtime.getRuntime().exec(EXECUTABLE);
            fail("should not have been able to execute!");
        } catch (Exception expected) {
                        
        }
    }

        public void testNoExecutionFromThread() throws Exception {
        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    Runtime.getRuntime().exec(EXECUTABLE);
                    fail("should not have been able to execute!");
                } catch (Exception expected) {
                                    }
            }
        };
        t.start();
        t.join();
    }
}
