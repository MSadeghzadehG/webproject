

package com.google.common.reflect;

import static com.google.common.base.Preconditions.checkArgument;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;


abstract class TypeCapture<T> {

  
  final Type capture() {
    Type superclass = getClass().getGenericSuperclass();
    checkArgument(superclass instanceof ParameterizedType, "%s isn't parameterized", superclass);
    return ((ParameterizedType) superclass).getActualTypeArguments()[0];
  }
}
