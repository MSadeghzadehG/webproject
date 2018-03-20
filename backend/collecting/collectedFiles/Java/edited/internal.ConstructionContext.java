

package org.elasticsearch.common.inject.internal;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.List;


public class ConstructionContext<T> {

    T currentReference;
    boolean constructing;

    List<DelegatingInvocationHandler<T>> invocationHandlers;

    public T getCurrentReference() {
        return currentReference;
    }

    public void removeCurrentReference() {
        this.currentReference = null;
    }

    public void setCurrentReference(T currentReference) {
        this.currentReference = currentReference;
    }

    public boolean isConstructing() {
        return constructing;
    }

    public void startConstruction() {
        this.constructing = true;
    }

    public void finishConstruction() {
        this.constructing = false;
        invocationHandlers = null;
    }

    public Object createProxy(Errors errors, Class<?> expectedType) throws ErrorsException {
                        
        if (!expectedType.isInterface()) {
            throw errors.cannotSatisfyCircularDependency(expectedType).toException();
        }

        if (invocationHandlers == null) {
            invocationHandlers = new ArrayList<>();
        }

        DelegatingInvocationHandler<T> invocationHandler
                = new DelegatingInvocationHandler<>();
        invocationHandlers.add(invocationHandler);

                        ClassLoader classLoader = expectedType.getClassLoader() == null ? ClassLoader.getSystemClassLoader() : expectedType.getClassLoader();
        return expectedType.cast(Proxy.newProxyInstance(classLoader,
                new Class[]{expectedType}, invocationHandler));
    }

    public void setProxyDelegates(T delegate) {
        if (invocationHandlers != null) {
            for (DelegatingInvocationHandler<T> handler : invocationHandlers) {
                handler.setDelegate(delegate);
            }
        }
    }

    static class DelegatingInvocationHandler<T> implements InvocationHandler {

        T delegate;

        @Override
        public Object invoke(Object proxy, Method method, Object[] args)
                throws Throwable {
            if (delegate == null) {
                throw new IllegalStateException("This is a proxy used to support"
                        + " circular references involving constructors. The object we're"
                        + " proxying is not constructed yet. Please wait until after"
                        + " injection has completed to use this object.");
            }

            try {
                                return method.invoke(delegate, args);
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            } catch (IllegalArgumentException e) {
                throw new RuntimeException(e);
            } catch (InvocationTargetException e) {
                throw e.getTargetException();
            }
        }

        void setDelegate(T delegate) {
            this.delegate = delegate;
        }
    }
}
