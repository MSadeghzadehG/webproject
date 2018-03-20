
package hudson;

import hudson.security.ACLContext;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.StandardOpenOption;
import jenkins.util.SystemProperties;
import com.thoughtworks.xstream.converters.reflection.PureJavaReflectionProvider;
import com.thoughtworks.xstream.core.JVM;
import hudson.model.Hudson;
import hudson.security.ACL;
import hudson.util.BootFailure;
import jenkins.model.Jenkins;
import hudson.util.HudsonIsLoading;
import hudson.util.IncompatibleServletVersionDetected;
import hudson.util.IncompatibleVMDetected;
import hudson.util.InsufficientPermissionDetected;
import hudson.util.NoHomeDir;
import hudson.util.RingBufferLogHandler;
import hudson.util.NoTempDir;
import hudson.util.IncompatibleAntVersionDetected;
import hudson.util.HudsonFailedToLoad;
import hudson.util.ChartUtil;
import hudson.util.AWTProblem;
import jenkins.util.JenkinsJVM;
import org.jvnet.localizer.LocaleProvider;
import org.kohsuke.stapler.jelly.JellyFacet;
import org.apache.tools.ant.types.FileSet;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.servlet.ServletContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import javax.servlet.ServletResponse;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Date;
import java.util.Locale;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.security.Security;
import java.util.logging.LogRecord;

import static java.util.logging.Level.*;


public class WebAppMain implements ServletContextListener {

        private static final int DEFAULT_RING_BUFFER_SIZE = SystemProperties.getInteger(RingBufferLogHandler.class.getName() + ".defaultSize", 256);

    private final RingBufferLogHandler handler = new RingBufferLogHandler(DEFAULT_RING_BUFFER_SIZE) {
        @Override public synchronized void publish(LogRecord record) {
            if (record.getLevel().intValue() >= Level.INFO.intValue()) {
                super.publish(record);
            }
        }
    };
    private static final String APP = "app";
    private boolean terminated;
    private Thread initThread;

    
    public void contextInitialized(ServletContextEvent event) {
        JenkinsJVMAccess._setJenkinsJVM(true);
        final ServletContext context = event.getServletContext();
        File home=null;
        try {

                        LocaleProvider.setProvider(new LocaleProvider() {
                public Locale get() {
                    return Functions.getCurrentLocale();
                }
            });

                        JVM jvm;
            try {
                jvm = new JVM();
                new URLClassLoader(new URL[0],getClass().getClassLoader());
            } catch(SecurityException e) {
                throw new InsufficientPermissionDetected(e);
            }

            try {                Security.removeProvider("SunPKCS11-Solaris");
            } catch (SecurityException e) {
                            }

            installLogger();

            final FileAndDescription describedHomeDir = getHomeDir(event);
            home = describedHomeDir.file.getAbsoluteFile();
            home.mkdirs();
            System.out.println("Jenkins home directory: "+home+" found at: "+describedHomeDir.description);

                        if (!home.exists())
                throw new NoHomeDir(home);

            recordBootAttempt(home);

                        if(jvm.bestReflectionProvider().getClass()==PureJavaReflectionProvider.class) {
                throw new IncompatibleVMDetected();             }


                        try {
                ServletResponse.class.getMethod("setCharacterEncoding",String.class);
            } catch (NoSuchMethodException e) {
                throw new IncompatibleServletVersionDetected(ServletResponse.class);
            }

                        try {
                FileSet.class.getMethod("getDirectoryScanner");
            } catch (NoSuchMethodException e) {
                throw new IncompatibleAntVersionDetected(FileSet.class);
            }

                        if(ChartUtil.awtProblemCause!=null) {
                throw new AWTProblem(ChartUtil.awtProblemCause);
            }

                                                try {
                File f = File.createTempFile("test", "test");
                f.delete();
            } catch (IOException e) {
                throw new NoTempDir(e);
            }

                                    try {
                TransformerFactory.newInstance();
                            } catch (TransformerFactoryConfigurationError x) {
                                LOGGER.log(WARNING, "XSLT not configured correctly. Hudson will try to fix this. See http:                System.setProperty(TransformerFactory.class.getName(),"com.sun.org.apache.xalan.internal.xsltc.trax.TransformerFactoryImpl");
                try {
                    TransformerFactory.newInstance();
                    LOGGER.info("XSLT is set to the JAXP RI in JRE");
                } catch(TransformerFactoryConfigurationError y) {
                    LOGGER.log(SEVERE, "Failed to correct the problem.");
                }
            }

            installExpressionFactory(event);

            context.setAttribute(APP,new HudsonIsLoading());

            final File _home = home;
            initThread = new Thread("Jenkins initialization thread") {
                @Override
                public void run() {
                    boolean success = false;
                    try {
                        Jenkins instance = new Hudson(_home, context);

                                                if (Thread.interrupted())
                            throw new InterruptedException();

                        context.setAttribute(APP, instance);

                        BootFailure.getBootFailureFile(_home).delete();

                                                LOGGER.info("Jenkins is fully up and running");
                        success = true;
                    } catch (Error e) {
                        new HudsonFailedToLoad(e).publish(context,_home);
                        throw e;
                    } catch (Exception e) {
                        new HudsonFailedToLoad(e).publish(context,_home);
                    } finally {
                        Jenkins instance = Jenkins.getInstanceOrNull();
                        if(!success && instance!=null)
                            instance.cleanUp();
                    }
                }
            };
            initThread.start();
        } catch (BootFailure e) {
            e.publish(context, home);
        } catch (Error | RuntimeException e) {
            LOGGER.log(SEVERE, "Failed to initialize Jenkins",e);
            throw e;
        }
    }

    public void joinInit() throws InterruptedException {
        initThread.join();
    }

    
    private void recordBootAttempt(File home) {
        try (OutputStream o=Files.newOutputStream(BootFailure.getBootFailureFile(home).toPath(), StandardOpenOption.CREATE, StandardOpenOption.APPEND)) {
            o.write((new Date().toString() + System.getProperty("line.separator", "\n")).toString().getBytes());
        } catch (IOException | InvalidPathException e) {
            LOGGER.log(WARNING, "Failed to record boot attempts",e);
        }
    }

    public static void installExpressionFactory(ServletContextEvent event) {
        JellyFacet.setExpressionFactory(event, new ExpressionFactory2());
    }

	
    @edu.umd.cs.findbugs.annotations.SuppressFBWarnings("LG_LOST_LOGGER_DUE_TO_WEAK_REFERENCE")
    private void installLogger() {
        Jenkins.logRecords = handler.getView();
        Logger.getLogger("").addHandler(handler);
    }

    
    public static class FileAndDescription {
        public final File file;
        public final String description;
        public FileAndDescription(File file,String description) {
            this.file = file;
            this.description = description;
        }
    }

    
    public FileAndDescription getHomeDir(ServletContextEvent event) {
                for (String name : HOME_NAMES) {
            try {
                InitialContext iniCtxt = new InitialContext();
                Context env = (Context) iniCtxt.lookup("java:comp/env");
                String value = (String) env.lookup(name);
                if(value!=null && value.trim().length()>0)
                    return new FileAndDescription(new File(value.trim()),"JNDI/java:comp/env/"+name);
                                value = (String) iniCtxt.lookup(name);
                if(value!=null && value.trim().length()>0)
                    return new FileAndDescription(new File(value.trim()),"JNDI/"+name);
            } catch (NamingException e) {
                            }
        }

                for (String name : HOME_NAMES) {
            String sysProp = SystemProperties.getString(name);
            if(sysProp!=null)
                return new FileAndDescription(new File(sysProp.trim()),"SystemProperties.getProperty(\""+name+"\")");
        }

                for (String name : HOME_NAMES) {
            String env = EnvVars.masterEnvVars.get(name);
            if(env!=null)
                return new FileAndDescription(new File(env.trim()).getAbsoluteFile(),"EnvVars.masterEnvVars.get(\""+name+"\")");
        }

        
        String root = event.getServletContext().getRealPath("/WEB-INF/workspace");
        if(root!=null) {
            File ws = new File(root.trim());
            if(ws.exists())
                                                                return new FileAndDescription(ws,"getServletContext().getRealPath(\"/WEB-INF/workspace\")");
        }

        File legacyHome = new File(new File(System.getProperty("user.home")),".hudson");
        if (legacyHome.exists()) {
            return new FileAndDescription(legacyHome,"$user.home/.hudson");         }

        File newHome = new File(new File(System.getProperty("user.home")),".jenkins");
        return new FileAndDescription(newHome,"$user.home/.jenkins");
    }

    public void contextDestroyed(ServletContextEvent event) {
        try (ACLContext old = ACL.as(ACL.SYSTEM)) {
            Jenkins instance = Jenkins.getInstanceOrNull();
            try {
                if (instance != null) {
                    instance.cleanUp();
                }
            } catch (Exception e) {
                LOGGER.log(Level.SEVERE, "Failed to clean up. Restart will continue.", e);
            }

            terminated = true;
            Thread t = initThread;
            if (t != null && t.isAlive()) {
                LOGGER.log(Level.INFO, "Shutting down a Jenkins instance that was still starting up", new Throwable("reason"));
                t.interrupt();
            }

                                    Logger.getLogger("").removeHandler(handler);
        } finally {
            JenkinsJVMAccess._setJenkinsJVM(false);
        }
    }

    private static final Logger LOGGER = Logger.getLogger(WebAppMain.class.getName());

    private static final class JenkinsJVMAccess extends JenkinsJVM {
        private static void _setJenkinsJVM(boolean jenkinsJVM) {
            JenkinsJVM.setJenkinsJVM(jenkinsJVM);
        }
    }

    private static final String[] HOME_NAMES = {"JENKINS_HOME","HUDSON_HOME"};
}
