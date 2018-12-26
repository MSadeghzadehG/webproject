

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import com.google.gwt.user.client.rpc.core.java.util.Collection_CustomFieldSerializerBase;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;


public class RegularImmutableSortedSet_CustomFieldSerializer {
  public static void deserialize(
      SerializationStreamReader reader, RegularImmutableSortedSet<?> instance) {}

  public static RegularImmutableSortedSet<Object> instantiate(SerializationStreamReader reader)
      throws SerializationException {
    
    @SuppressWarnings("unchecked")
    Comparator<Object> comparator = (Comparator<Object>) reader.readObject();

    List<Object> elements = new ArrayList<>();
    Collection_CustomFieldSerializerBase.deserialize(reader, elements);
    
    return (RegularImmutableSortedSet<Object>) ImmutableSortedSet.copyOf(comparator, elements);
  }

  public static void serialize(
      SerializationStreamWriter writer, RegularImmutableSortedSet<?> instance)
      throws SerializationException {
    writer.writeObject(instance.comparator());

    Collection_CustomFieldSerializerBase.serialize(writer, instance);
  }
}
