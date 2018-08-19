
package domainapp.integtests.tests.modules.simple;

import static org.assertj.core.api.Assertions.assertThat;

import com.google.common.base.Throwables;
import domainapp.dom.modules.simple.SimpleObject;
import domainapp.dom.modules.simple.SimpleObjects;
import domainapp.fixture.modules.simple.SimpleObjectsTearDown;
import domainapp.fixture.scenarios.RecreateSimpleObjects;
import domainapp.integtests.tests.SimpleAppIntegTest;
import java.sql.SQLIntegrityConstraintViolationException;
import java.util.List;
import javax.inject.Inject;
import org.apache.isis.applib.fixturescripts.FixtureScript;
import org.apache.isis.applib.fixturescripts.FixtureScripts;
import org.hamcrest.Description;
import org.hamcrest.Matcher;
import org.hamcrest.TypeSafeMatcher;
import org.junit.Test;


public class SimpleObjectsIntegTest extends SimpleAppIntegTest {

  @Inject
  FixtureScripts fixtureScripts;
  @Inject
  SimpleObjects simpleObjects;

  
  public static class ListAll extends SimpleObjectsIntegTest {

    @Test
    public void happyCase() throws Exception {

            RecreateSimpleObjects fs = new RecreateSimpleObjects();
      fixtureScripts.runFixtureScript(fs, null);
      nextTransaction();

            final List<SimpleObject> all = wrap(simpleObjects).listAll();

            assertThat(all).hasSize(fs.getSimpleObjects().size());

      SimpleObject simpleObject = wrap(all.get(0));
      assertThat(simpleObject.getName()).isEqualTo(fs.getSimpleObjects().get(0).getName());
    }

    @Test
    public void whenNone() throws Exception {

            FixtureScript fs = new SimpleObjectsTearDown();
      fixtureScripts.runFixtureScript(fs, null);
      nextTransaction();

            final List<SimpleObject> all = wrap(simpleObjects).listAll();

            assertThat(all).hasSize(0);
    }
  }


  
  public static class Create extends SimpleObjectsIntegTest {

    @Test
    public void happyCase() throws Exception {

            FixtureScript fs = new SimpleObjectsTearDown();
      fixtureScripts.runFixtureScript(fs, null);
      nextTransaction();

            wrap(simpleObjects).create("Faz");

            final List<SimpleObject> all = wrap(simpleObjects).listAll();
      assertThat(all).hasSize(1);
    }

    @Test
    public void whenAlreadyExists() throws Exception {

            FixtureScript fs = new SimpleObjectsTearDown();
      fixtureScripts.runFixtureScript(fs, null);
      nextTransaction();
      wrap(simpleObjects).create("Faz");
      nextTransaction();

            expectedExceptions.expectCause(causalChainContains(SQLIntegrityConstraintViolationException.class));

            wrap(simpleObjects).create("Faz");
      nextTransaction();
    }

    private static Matcher<? extends Throwable> causalChainContains(final Class<?> cls) {
      return new TypeSafeMatcher<Throwable>() {
        @Override
        protected boolean matchesSafely(Throwable item) {
          final List<Throwable> causalChain = Throwables.getCausalChain(item);
          for (Throwable throwable : causalChain) {
            if (cls.isAssignableFrom(throwable.getClass())) {
              return true;
            }
          }
          return false;
        }

        @Override
        public void describeTo(Description description) {
          description.appendText("exception with causal chain containing " + cls.getSimpleName());
        }
      };
    }
  }

}
