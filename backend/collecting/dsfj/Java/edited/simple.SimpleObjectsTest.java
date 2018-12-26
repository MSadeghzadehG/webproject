
package domainapp.dom.modules.simple;

import static org.assertj.core.api.Assertions.assertThat;

import com.google.common.collect.Lists;
import java.util.List;
import org.apache.isis.applib.DomainObjectContainer;
import org.apache.isis.core.unittestsupport.jmocking.JUnitRuleMockery2;
import org.apache.isis.core.unittestsupport.jmocking.JUnitRuleMockery2.Mode;
import org.jmock.Expectations;
import org.jmock.Sequence;
import org.jmock.auto.Mock;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;


public class SimpleObjectsTest {

  @Rule
  public JUnitRuleMockery2 context = JUnitRuleMockery2.createFor(Mode.INTERFACES_AND_CLASSES);

  @Mock
  DomainObjectContainer mockContainer;

  SimpleObjects simpleObjects;

  @Before
  public void setUp() throws Exception {
    simpleObjects = new SimpleObjects();
    simpleObjects.container = mockContainer;
  }

  
  public static class Create extends SimpleObjectsTest {

    @Test
    public void happyCase() throws Exception {

            final SimpleObject simpleObject = new SimpleObject();

      final Sequence seq = context.sequence("create");
      context.checking(new Expectations() {
        {
          oneOf(mockContainer).newTransientInstance(SimpleObject.class);
          inSequence(seq);
          will(returnValue(simpleObject));

          oneOf(mockContainer).persistIfNotAlready(simpleObject);
          inSequence(seq);
        }
      });

            final SimpleObject obj = simpleObjects.create("Foobar");

            assertThat(obj).isEqualTo(simpleObject);
      assertThat(obj.getName()).isEqualTo("Foobar");
    }

  }

  
  public static class ListAll extends SimpleObjectsTest {

    @Test
    public void happyCase() throws Exception {

            final List<SimpleObject> all = Lists.newArrayList();

      context.checking(new Expectations() {
        {
          oneOf(mockContainer).allInstances(SimpleObject.class);
          will(returnValue(all));
        }
      });

            final List<SimpleObject> list = simpleObjects.listAll();

            assertThat(list).isEqualTo(all);
    }
  }
}
