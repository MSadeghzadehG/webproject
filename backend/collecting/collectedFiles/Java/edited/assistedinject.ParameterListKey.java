

package org.elasticsearch.common.inject.assistedinject;

import org.elasticsearch.common.inject.TypeLiteral;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


class ParameterListKey {

    private final List<Type> paramList;

    ParameterListKey(List<Type> paramList) {
        this.paramList = new ArrayList<>(paramList);
    }

    ParameterListKey(Type[] types) {
        this(Arrays.asList(types));
    }

    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof ParameterListKey)) {
            return false;
        }
        ParameterListKey other = (ParameterListKey) o;
        return paramList.equals(other.paramList);
    }

    @Override
    public int hashCode() {
        return paramList.hashCode();
    }

    @Override
    public String toString() {
        return paramList.toString();
    }
}
