
package hudson.model;

import hudson.util.StreamTaskListener;
import hudson.util.NullStream;
import hudson.util.FormValidation;
import hudson.Launcher;
import hudson.Extension;
import hudson.EnvVars;
import hudson.slaves.NodeSpecific;
import hudson.tools.ToolInstallation;
import hudson.tools.ToolDescriptor;
import hudson.tools.ToolInstaller;
import hudson.tools.ToolProperty;
import hudson.util.XStream2;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.List;
import java.util.Arrays;
import java.util.Collections;
import java.util.logging.Level;
import java.util.logging.Logger;

import jenkins.model.Jenkins;
import org.jenkinsci.Symbol;
import org.kohsuke.stapler.DataBoundConstructor;
import org.kohsuke.accmod.Restricted;
import org.kohsuke.accmod.restrictions.NoExternalUse;


public final class JDK extends ToolInstallation implements NodeSpecific<JDK>, EnvironmentSpecific<JDK> {

    
    public static final String DEFAULT_NAME = "(System)";

    @Restricted(NoExternalUse.class)
    public static boolean isDefaultName(String name) {
        if ("(Default)".equals(name)) {
                        return true;
        }
        return DEFAULT_NAME.equals(name) || name == null;
    }

    
    @Deprecated     private transient String javaHome;

    public JDK(String name, String javaHome) {
        super(name, javaHome, Collections.<ToolProperty<?>>emptyList());
    }

    @DataBoundConstructor
    public JDK(String name, String home, List<? extends ToolProperty<?>> properties) {
        super(name, home, properties);
    }

    
    @Deprecated
    public String getJavaHome() {
        return getHome();
    }

    
    public File getBinDir() {
        return new File(getHome(),"bin");
    }
    
    private File getExecutable() {
        String execName = (File.separatorChar == '\\') ? "java.exe" : "java";
        return new File(getHome(),"bin/"+execName);
    }

    
    public boolean getExists() {
        return getExecutable().exists();
    }

    
    @Deprecated
    public void buildEnvVars(Map<String,String> env) {
        String home = getHome();
        if (home == null) {
            return;
        }
                env.put("PATH+JDK",home+"/bin");
        env.put("JAVA_HOME", home);
    }

    
    @Override
    public void buildEnvVars(EnvVars env) {
        buildEnvVars((Map)env);
    }

    public JDK forNode(Node node, TaskListener log) throws IOException, InterruptedException {
        return new JDK(getName(), translateFor(node, log));
    }

    public JDK forEnvironment(EnvVars environment) {
        return new JDK(getName(), environment.expand(getHome()));
    }

    
    public static boolean isDefaultJDKValid(Node n) {
        try {
            TaskListener listener = new StreamTaskListener(new NullStream());
            Launcher launcher = n.createLauncher(listener);
            return launcher.launch().cmds("java","-fullversion").stdout(listener).join()==0;
        } catch (IOException e) {
            return false;
        } catch (InterruptedException e) {
            return false;
        }
    }

    @Extension @Symbol("jdk")
    public static class DescriptorImpl extends ToolDescriptor<JDK> {

        public String getDisplayName() {
            return "JDK";         }

        public @Override JDK[] getInstallations() {
            return Jenkins.getInstance().getJDKs().toArray(new JDK[0]);
        }

        public @Override void setInstallations(JDK... jdks) {
            Jenkins.getInstance().setJDKs(Arrays.asList(jdks));
        }

        @Override
        public List<? extends ToolInstaller> getDefaultInstallers() {
            try {
                Class<? extends ToolInstaller> jdkInstallerClass = Jenkins.getInstance().getPluginManager()
                        .uberClassLoader.loadClass("hudson.tools.JDKInstaller").asSubclass(ToolInstaller.class);
                Constructor<? extends ToolInstaller> constructor = jdkInstallerClass.getConstructor(String.class, boolean.class);
                return Collections.singletonList(constructor.newInstance(null, false));
            } catch (ClassNotFoundException e) {
                return Collections.emptyList();
            } catch (Exception e) {
                LOGGER.log(Level.WARNING, "Unable to get default installer", e);
                return Collections.emptyList();
            }
        }

        
        @Override protected FormValidation checkHomeDirectory(File value) {
            File toolsJar = new File(value,"lib/tools.jar");
            File mac = new File(value,"lib/dt.jar");

                        File javac = new File(value, "bin/javac");
            File javacExe = new File(value, "bin/javac.exe");
            if(!toolsJar.exists() && !mac.exists() && !javac.exists() && !javacExe.exists())
                return FormValidation.error(Messages.Hudson_NotJDKDir(value));

            return FormValidation.ok();
        }

    }

    public static class ConverterImpl extends ToolConverter {
        public ConverterImpl(XStream2 xstream) { super(xstream); }
        @Override protected String oldHomeField(ToolInstallation obj) {
            return ((JDK)obj).javaHome;
        }
    }

    private static final Logger LOGGER = Logger.getLogger(JDK.class.getName());
}
