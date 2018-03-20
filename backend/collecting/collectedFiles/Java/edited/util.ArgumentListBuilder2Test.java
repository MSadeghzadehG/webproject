
package hudson.util;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.*;
import hudson.Functions;
import hudson.Launcher.LocalLauncher;
import hudson.Launcher.RemoteLauncher;
import hudson.Proc;
import hudson.model.Slave;

import org.apache.tools.ant.util.JavaEnvUtils;
import org.junit.Rule;
import org.junit.Test;
import org.jvnet.hudson.test.Email;
import org.jvnet.hudson.test.JenkinsRule;

import com.google.common.base.Joiner;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.StringWriter;
import java.net.URL;


public class ArgumentListBuilder2Test {

    @Rule
    public JenkinsRule j = new JenkinsRule();

    
    @Test
    @Email("http:    public void slaveMask() throws Exception {
        ArgumentListBuilder args = new ArgumentListBuilder();
        args.add("java");
        args.addMasked("-version");

        Slave s = j.createSlave();
        s.toComputer().connect(false).get();

        StringWriter out = new StringWriter();
        assertEquals(0,s.createLauncher(new StreamTaskListener(out)).launch().cmds(args).join());
        System.out.println(out);
        assertTrue(out.toString().contains("$ java ********"));
    }

    @Test
    public void ensureArgumentsArePassedViaCmdExeUnmodified() throws Exception {
        assumeTrue(Functions.isWindows());

        String[] specials = new String[] {
                "~", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
                "_", "+", "{", "}", "[", "]", ":", ";", "\"", "'", "\\", "|",
                "<", ">", ",", ".", "/", "?", " "
        };

        String out = echoArgs(specials);

        String expected = String.format("%n%s", Joiner.on(" ").join(specials));
        assertThat(out, containsString(expected));
    }

    public String echoArgs(String... arguments) throws Exception {
        String testHarnessJar = new File(Class.forName("hudson.util.EchoCommand")
                .getProtectionDomain()
                .getCodeSource()
                .getLocation().toURI()).getAbsolutePath();

        ArgumentListBuilder args = new ArgumentListBuilder(
                    JavaEnvUtils.getJreExecutable("java").replaceAll("^\"|\"$", ""),
                    "-cp", testHarnessJar, "hudson.util.EchoCommand")
                .add(arguments)
                .toWindowsCommand();

        ByteArrayOutputStream out = new ByteArrayOutputStream();
        final StreamTaskListener listener = new StreamTaskListener(out);
        Proc p = new LocalLauncher(listener)
                .launch()
                .stderr(System.err)
                .stdout(out)
                .cmds(args)
                .start()
        ;
        int code = p.join();
        listener.close();

        assumeThat("Failed to run " + args, code, equalTo(0));
        return out.toString();
    }
}
