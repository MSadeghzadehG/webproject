
package domainapp.webapp;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

import com.google.common.base.Joiner;
import com.google.common.io.Resources;
import com.google.inject.AbstractModule;
import com.google.inject.Module;
import com.google.inject.name.Names;
import com.google.inject.util.Modules;
import com.google.inject.util.Providers;

import org.apache.wicket.Session;
import org.apache.wicket.request.IRequestParameters;
import org.apache.wicket.request.Request;
import org.apache.wicket.request.Response;
import org.apache.wicket.request.http.WebRequest;

import org.apache.isis.viewer.wicket.viewer.IsisWicketApplication;
import org.apache.isis.viewer.wicket.viewer.integration.wicket.AuthenticatedWebSessionForIsis;

import de.agilecoders.wicket.core.Bootstrap;
import de.agilecoders.wicket.core.settings.IBootstrapSettings;
import de.agilecoders.wicket.themes.markup.html.bootswatch.BootswatchTheme;
import de.agilecoders.wicket.themes.markup.html.bootswatch.BootswatchThemeProvider;



public class SimpleApplication extends IsisWicketApplication {

  private static final long serialVersionUID = 1L;

  
  private static final boolean DEMO_MODE_USING_CREDENTIALS_AS_QUERYARGS = false;


  @Override
  protected void init() {
    super.init();

    IBootstrapSettings settings = Bootstrap.getSettings();
    settings.setThemeProvider(new BootswatchThemeProvider(BootswatchTheme.Flatly));
  }

  @Override
  public Session newSession(final Request request, final Response response) {
    if (!DEMO_MODE_USING_CREDENTIALS_AS_QUERYARGS) {
      return super.newSession(request, response);
    }

        final AuthenticatedWebSessionForIsis s =
        (AuthenticatedWebSessionForIsis) super.newSession(request, response);
    IRequestParameters requestParameters = request.getRequestParameters();
    final org.apache.wicket.util.string.StringValue user =
        requestParameters.getParameterValue("user");
    final org.apache.wicket.util.string.StringValue password =
        requestParameters.getParameterValue("pass");
    s.signIn(user.toString(), password.toString());
    return s;
  }

  @Override
  public WebRequest newWebRequest(HttpServletRequest servletRequest, String filterPath) {
    if (!DEMO_MODE_USING_CREDENTIALS_AS_QUERYARGS) {
      return super.newWebRequest(servletRequest, filterPath);
    }

        try {
      String uname = servletRequest.getParameter("user");
      if (uname != null) {
        servletRequest.getSession().invalidate();
      }
    } catch (Exception e) {
      System.out.println(e);
    }
    return super.newWebRequest(servletRequest, filterPath);
  }

  @Override
  protected Module newIsisWicketModule() {
    final Module isisDefaults = super.newIsisWicketModule();

    final Module overrides = new AbstractModule() {
      @Override
      protected void configure() {
        bind(String.class).annotatedWith(Names.named("applicationName")).toInstance("Simple App");
        bind(String.class).annotatedWith(Names.named("applicationCss")).toInstance(
            "css/application.css");
        bind(String.class).annotatedWith(Names.named("applicationJs")).toInstance(
            "scripts/application.js");
        bind(String.class).annotatedWith(Names.named("welcomeMessage")).toInstance(
            readLines(getClass(), "welcome.html"));
        bind(String.class).annotatedWith(Names.named("aboutMessage")).toInstance("Simple App");
        bind(InputStream.class).annotatedWith(Names.named("metaInfManifest")).toProvider(
            Providers.of(getServletContext().getResourceAsStream("/META-INF/MANIFEST.MF")));
      }
    };

    return Modules.override(isisDefaults).with(overrides);
  }

  private static String readLines(final Class<?> contextClass, final String resourceName) {
    try {
      List<String> readLines =
          Resources.readLines(Resources.getResource(contextClass, resourceName),
              Charset.defaultCharset());
      return Joiner.on("\n").join(readLines);
    } catch (IOException e) {
      return "This is a simple app";
    }
  }

}
