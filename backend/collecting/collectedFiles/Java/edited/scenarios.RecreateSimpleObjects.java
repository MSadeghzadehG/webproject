

package domainapp.fixture.scenarios;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import com.google.common.collect.Lists;

import org.apache.isis.applib.fixturescripts.FixtureScript;

import domainapp.dom.modules.simple.SimpleObject;
import domainapp.fixture.modules.simple.SimpleObjectCreate;
import domainapp.fixture.modules.simple.SimpleObjectsTearDown;



public class RecreateSimpleObjects extends FixtureScript {

  public final List<String> names = Collections.unmodifiableList(Arrays.asList("Foo", "Bar", "Baz",
      "Frodo", "Froyo", "Fizz", "Bip", "Bop", "Bang", "Boo"));

    private Integer number;

  
    private final List<SimpleObject> simpleObjects = Lists.newArrayList();

  public RecreateSimpleObjects() {
    withDiscoverability(Discoverability.DISCOVERABLE);
  }

  
  public Integer getNumber() {
    return number;
  }

  public RecreateSimpleObjects setNumber(final Integer number) {
    this.number = number;
    return this;
  }

  
  public List<SimpleObject> getSimpleObjects() {
    return simpleObjects;
  }

  
  @Override
  protected void execute(final ExecutionContext ec) {

        final int paramNumber = defaultParam("number", ec, 3);

        if (paramNumber < 0 || paramNumber > names.size()) {
      throw new IllegalArgumentException(String.format("number must be in range [0,%d)",
          names.size()));
    }

                ec.executeChild(this, new SimpleObjectsTearDown());

    for (int i = 0; i < paramNumber; i++) {
      final SimpleObjectCreate fs = new SimpleObjectCreate().setName(names.get(i));
      ec.executeChild(this, fs.getName(), fs);
      simpleObjects.add(fs.getSimpleObject());
    }
  }
}
