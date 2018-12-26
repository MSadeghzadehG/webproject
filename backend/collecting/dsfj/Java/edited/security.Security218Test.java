package jenkins.security;

import hudson.model.Node.Mode;
import hudson.model.Slave;
import hudson.remoting.Channel;
import hudson.slaves.DumbSlave;
import hudson.slaves.JNLPLauncher;
import hudson.slaves.RetentionStrategy;
import java.io.File;
import org.apache.tools.ant.util.JavaEnvUtils;
import org.junit.After;
import org.junit.Rule;
import org.junit.Test;
import org.jvnet.hudson.test.Issue;
import org.jvnet.hudson.test.JenkinsRule;

import java.io.Serializable;
import java.util.Collections;
import java.util.logging.Level;
import org.apache.commons.io.FileUtils;
import org.codehaus.groovy.runtime.MethodClosure;
import static org.hamcrest.Matchers.containsString;

import static org.junit.Assert.*;
import org.junit.rules.TemporaryFolder;
import org.jvnet.hudson.test.LoggerRule;


@Issue("SECURITY-218")
public class Security218Test implements Serializable {
    @Rule
    public transient JenkinsRule j = new JenkinsRule();

    @Rule
    public TemporaryFolder tmp = new TemporaryFolder();

    @Rule
    public LoggerRule logging = new LoggerRule().record(ClassFilterImpl.class, Level.FINE);

    
    private transient Process jnlp;

    
    @Test
    public void dumbSlave() throws Exception {
        check(j.createOnlineSlave());
    }

    
    @Test
    public void jnlpSlave() throws Exception {
        DumbSlave s = createJnlpSlave("test");
        launchJnlpSlave(s);
        check(s);
    }

    
    @SuppressWarnings("ConstantConditions")
    private void check(DumbSlave s) throws Exception {
        try {
            Object o = s.getComputer().getChannel().call(new EvilReturnValue());
            fail("Expected the connection to die: " + o);
        } catch (Exception e) {
            assertThat(e.getMessage(), containsString(MethodClosure.class.getName()));
        }
    }
    private static class EvilReturnValue extends MasterToSlaveCallable<Object, RuntimeException> {
        @Override
        public Object call() {
            return new MethodClosure("oops", "trim");
        }
    }

    
    public DumbSlave createJnlpSlave(String name) throws Exception {
        DumbSlave s = new DumbSlave(name, "", System.getProperty("java.io.tmpdir") + '/' + name, "2", Mode.NORMAL, "", new JNLPLauncher(true), RetentionStrategy.INSTANCE, Collections.EMPTY_LIST);
        j.jenkins.addNode(s);
        return s;
    }

    
    public Channel launchJnlpSlave(Slave slave) throws Exception {
        j.createWebClient().goTo("computer/"+slave.getNodeName()+"/slave-agent.jnlp?encrypt=true", "application/octet-stream");
        String secret = slave.getComputer().getJnlpMac();
        File slaveJar = tmp.newFile();
        FileUtils.copyURLToFile(new Slave.JnlpJar("slave.jar").getURL(), slaveJar);
                ProcessBuilder pb = new ProcessBuilder(JavaEnvUtils.getJreExecutable("java"),
                "-jar", slaveJar.getAbsolutePath(),
                "-jnlpUrl", j.getURL() + "computer/"+slave.getNodeName()+"/slave-agent.jnlp", "-secret", secret);

        pb.inheritIO();
        System.err.println("Running: " + pb.command());

        jnlp = pb.start();

        for (int i = 0; i < 600; i++) {
            if (slave.getComputer().isOnline()) {
                return slave.getComputer().getChannel();
            }
            Thread.sleep(100);
        }

        throw new AssertionError("JNLP slave agent failed to connect");
    }

    @After
    public void tearDown() {
        if (jnlp !=null)
            jnlp.destroy();
    }
}
