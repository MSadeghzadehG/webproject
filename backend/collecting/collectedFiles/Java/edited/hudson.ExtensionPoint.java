
package hudson;

import jenkins.model.Jenkins;

import static java.lang.annotation.ElementType.TYPE;
import java.lang.annotation.Retention;
import static java.lang.annotation.RetentionPolicy.RUNTIME;
import java.lang.annotation.Target;
import jenkins.util.io.OnMaster;


public interface ExtensionPoint {
    
    @Target(TYPE)
    @Retention(RUNTIME)
    @interface LegacyInstancesAreScopedToHudson {}
}
