

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import com.google.gwt.user.client.rpc.core.java.util.Collection_CustomFieldSerializerBase;
import java.util.List;


public class RegularImmutableSet_CustomFieldSerializer {

  public static void deserialize(SerializationStreamReader reader, ImmutableSet<?> instance) {}

  public static ImmutableSet<Object> instantiate(SerializationStreamReader reader)
      throws SerializationException {
    List<Object> elements = Lists.newArrayList();
    Collection_CustomFieldSerializerBase.deserialize(reader, elements);
    return ImmutableSet.copyOf(elements);
  }

  public static void serialize(SerializationStreamWriter writer, ImmutableSet<?> instance)
      throws SerializationException {
    Collection_CustomFieldSerializerBase.serialize(writer, instance);
  }
}
