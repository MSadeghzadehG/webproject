
package org.kohsuke.stapler;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import javax.annotation.Nonnull;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;
import jenkins.model.Jenkins;
import org.eclipse.jetty.server.Request;
import org.jvnet.hudson.test.JenkinsRule;
import org.mockito.Mockito;
import org.springframework.web.multipart.support.DefaultMultipartHttpServletRequest;


public class MockStaplerRequestBuilder{
    
    private final JenkinsRule r;
    
    private final List<AncestorImpl> ancestors = new ArrayList<>();
    private final TokenList tokens;
    final Map<String,Object> getters = new HashMap<>();
    private Stapler stapler;
    
    public MockStaplerRequestBuilder(@Nonnull JenkinsRule r, String url) {
        this.r = r;
        this.tokens = new TokenList(url);
    }
    
    public MockStaplerRequestBuilder withStapler(Stapler stapler) {
        this.stapler = stapler;
        return this;
    }
    
    public MockStaplerRequestBuilder withGetter(String objectName, Object object) {
        this.getters.put(objectName, object);
        return this;
    }
    
    public MockStaplerRequestBuilder withAncestor(AncestorImpl ancestor) {
        this.ancestors.add(ancestor);
        return this;
    }
       
    public StaplerRequest build() throws AssertionError {        
        HttpServletRequest rawRequest = Mockito.mock(HttpServletRequest.class);
        return new RequestImpl(stapler != null ? stapler : new Stapler(), rawRequest, ancestors, tokens);
    }
       
}
