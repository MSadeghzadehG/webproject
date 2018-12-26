

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.spi.InjectionPoint;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;


interface ConstructionProxy<T> {

    
    T newInstance(Object... arguments) throws InvocationTargetException;

    
    InjectionPoint getInjectionPoint();

    
    Constructor<T> getConstructor();
}
