
package domainapp.integtests.specglue;

import org.apache.isis.core.specsupport.specs.CukeGlueAbstract;

import cucumber.api.java.Before;
import domainapp.fixture.scenarios.RecreateSimpleObjects;


public class CatalogOfFixturesGlue extends CukeGlueAbstract {

  @Before(value = {"@integration", "@SimpleObjectsFixture"}, order = 20000)
  public void integrationFixtures() throws Throwable {
    scenarioExecution().install(new RecreateSimpleObjects());
  }
}
