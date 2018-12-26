
package scripts;

import com.gargoylesoftware.htmlunit.ScriptResult;
import com.gargoylesoftware.htmlunit.html.HtmlPage;
import org.jvnet.hudson.test.Issue;
import org.jvnet.hudson.test.HudsonTestCase;


public class BehaviorTest extends HudsonTestCase {
    public void testCssSelectors() throws Exception {
        HtmlPage p = createWebClient().goTo("self/testCssSelectors");

                assertEquals(2,asInt(p.executeJavaScript("findElementsBySelector($('test1'),'.a',true).length")));
        assertEquals(1,asInt(p.executeJavaScript("findElementsBySelector($('test1'),'.a',false).length")));

                assertEquals(1,asInt(p.executeJavaScript("findElementsBySelector($('test2'),'.a .b',true).length")));
        assertEquals(1,asInt(p.executeJavaScript("findElementsBySelector($('test2'),'.a .b',false).length")));

                assertEquals(1,asInt(p.executeJavaScript("findElementsBySelector($('test3'),'P.a',true).length")));
        assertEquals(1,asInt(p.executeJavaScript("findElementsBySelector($('test3'),'P.a',false).length")));
    }

    private int asInt(ScriptResult r) {
        return ((Double)r.getJavaScriptResult()).intValue();
    }

    @Issue("JENKINS-14495")
    public void testDuplicateRegistrations() throws Exception {
        HtmlPage p = createWebClient().goTo("self/testDuplicateRegistrations");
        ScriptResult r = p.executeJavaScript("document.getElementsBySelector('DIV.a')[0].innerHTML");
        assertEquals("initial and appended yet different", r.getJavaScriptResult().toString());
    }

    public void testSelectorOrdering() throws Exception {
        HtmlPage p = createWebClient().goTo("self/testSelectorOrdering");
        ScriptResult r = p.executeJavaScript("document.getElementsBySelector('DIV.a')[0].innerHTML");
        assertEquals("initial early counted! generic weevils! late", r.getJavaScriptResult().toString());
    }

}
