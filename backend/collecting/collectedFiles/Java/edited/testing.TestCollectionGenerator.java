

package com.google.common.collect.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.Collection;


@GwtCompatible
public interface TestCollectionGenerator<E> extends TestContainerGenerator<Collection<E>, E> {}
