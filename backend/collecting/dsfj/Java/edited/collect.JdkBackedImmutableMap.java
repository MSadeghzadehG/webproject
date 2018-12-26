

package com.google.common.collect;

import java.util.Map;


final class JdkBackedImmutableMap<K, V> extends ForwardingImmutableMap<K, V> {
  JdkBackedImmutableMap(Map<? extends K, ? extends V> delegate) {
    super(delegate);
  }
}
