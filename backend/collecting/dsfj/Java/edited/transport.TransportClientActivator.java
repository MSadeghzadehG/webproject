

package org.elasticsearch.wildfly.transport;

import javax.ws.rs.ApplicationPath;
import javax.ws.rs.core.Application;

import java.util.Collections;
import java.util.Set;

@ApplicationPath("/transport")
public class TransportClientActivator extends Application {

    @Override
    public Set<Class<?>> getClasses() {
        return Collections.singleton(TransportClientEmployeeResource.class);
    }

}
