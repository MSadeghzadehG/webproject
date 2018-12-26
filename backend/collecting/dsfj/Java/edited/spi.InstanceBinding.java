

package org.elasticsearch.common.inject.spi;

import org.elasticsearch.common.inject.Binding;

import java.util.Set;


public interface InstanceBinding<T> extends Binding<T>, HasDependencies {

    
    T getInstance();

    
    Set<InjectionPoint> getInjectionPoints();

}
