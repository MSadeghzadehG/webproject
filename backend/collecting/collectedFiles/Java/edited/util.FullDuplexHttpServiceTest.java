
package jenkins.util;

import hudson.cli.FullDuplexHttpStream;
import hudson.model.RootAction;
import hudson.security.csrf.CrumbExclusion;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.junit.Test;
import static org.junit.Assert.*;
import org.junit.Rule;
import org.jvnet.hudson.test.JenkinsRule;
import org.jvnet.hudson.test.LoggerRule;
import org.jvnet.hudson.test.TestExtension;
import org.kohsuke.stapler.HttpResponse;
import org.kohsuke.stapler.StaplerRequest;

public class FullDuplexHttpServiceTest {

    @Rule
    public JenkinsRule r = new JenkinsRule();

    @Rule
    public LoggerRule logging = new LoggerRule().record(FullDuplexHttpService.class, Level.FINE).record(FullDuplexHttpStream.class, Level.FINE);

    @Test
    public void smokes() throws Exception {
        logging.record("org.eclipse.jetty", Level.ALL);
        FullDuplexHttpStream con = new FullDuplexHttpStream(r.getURL(), "test/", null);
        InputStream is = con.getInputStream();
        OutputStream os = con.getOutputStream();
        os.write(33);
        os.flush();
        Logger.getLogger(FullDuplexHttpServiceTest.class.getName()).info("uploaded initial content");
        assertEquals(0, is.read());         assertEquals(66, is.read());
    }
    @TestExtension("smokes")
    public static class Endpoint implements RootAction {
        private transient final Map<UUID, FullDuplexHttpService> duplexServices = new HashMap<>();
        @Override
        public String getUrlName() {
            return "test";
        }
        @Override
        public String getIconFileName() {
            return null;
        }
        @Override
        public String getDisplayName() {
            return null;
        }
        public HttpResponse doIndex() {
            return new FullDuplexHttpService.Response(duplexServices) {
                @Override
                protected FullDuplexHttpService createService(StaplerRequest req, UUID uuid) throws IOException, InterruptedException {
                    return new FullDuplexHttpService(uuid) {
                        @Override
                        protected void run(InputStream upload, OutputStream download) throws IOException, InterruptedException {
                            int x = upload.read();
                            download.write(x * 2);
                        }
                    };
                }
            };
        }
    }
    @TestExtension("smokes")
    public static class EndpointCrumbExclusion extends CrumbExclusion {
        @Override
        public boolean process(HttpServletRequest request, HttpServletResponse response, FilterChain chain) throws IOException, ServletException {
            if ("/test/".equals(request.getPathInfo())) {
                chain.doFilter(request, response);
                return true;
            }
            return false;
        }
    }

}
