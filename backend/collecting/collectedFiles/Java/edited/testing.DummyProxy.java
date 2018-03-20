

package com.google.common.testing;

import static com.google.common.base.Preconditions.checkNotNull;
import static com.google.common.testing.NullPointerTester.isNullable;

import com.google.common.annotations.GwtIncompatible;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.Sets;
import com.google.common.reflect.AbstractInvocationHandler;
import com.google.common.reflect.Invokable;
import com.google.common.reflect.Parameter;
import com.google.common.reflect.TypeToken;
import java.io.Serializable;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.Set;


@GwtIncompatible
abstract class DummyProxy {

  
  final <T> T newProxy(TypeToken<T> interfaceType) {
    Set<Class<?>> interfaceClasses = Sets.newLinkedHashSet();
    interfaceClasses.addAll(interfaceType.getTypes().interfaces().rawTypes());
        interfaceClasses.add(Serializable.class);
    Object dummy =
        Proxy.newProxyInstance(
            interfaceClasses.iterator().next().getClassLoader(),
            interfaceClasses.toArray(new Class<?>[interfaceClasses.size()]),
            new DummyHandler(interfaceType));
    @SuppressWarnings("unchecked")     T result = (T) dummy;
    return result;
  }

  
  abstract <R> R dummyReturnValue(TypeToken<R> returnType);

  private class DummyHandler extends AbstractInvocationHandler implements Serializable {
    private final TypeToken<?> interfaceType;

    DummyHandler(TypeToken<?> interfaceType) {
      this.interfaceType = interfaceType;
    }

    @Override
    protected Object handleInvocation(Object proxy, Method method, Object[] args) {
      Invokable<?, ?> invokable = interfaceType.method(method);
      ImmutableList<Parameter> params = invokable.getParameters();
      for (int i = 0; i < args.length; i++) {
        Parameter param = params.get(i);
        if (!isNullable(param)) {
          checkNotNull(args[i]);
        }
      }
      return dummyReturnValue(interfaceType.resolveType(method.getGenericReturnType()));
    }

    @Override
    public int hashCode() {
      return identity().hashCode();
    }

    @Override
    public boolean equals(Object obj) {
      if (obj instanceof DummyHandler) {
        DummyHandler that = (DummyHandler) obj;
        return identity().equals(that.identity());
      } else {
        return false;
      }
    }

    private DummyProxy identity() {
      return DummyProxy.this;
    }

    @Override
    public String toString() {
      return "Dummy proxy for " + interfaceType;
    }

            private Object writeReplace() {
      return new DummyHandler(TypeToken.of(interfaceType.getRawType()));
    }
  }
}
