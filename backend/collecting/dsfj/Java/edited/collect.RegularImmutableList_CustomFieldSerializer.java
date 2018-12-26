

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import com.google.gwt.user.client.rpc.core.java.util.Collection_CustomFieldSerializerBase;
import java.util.ArrayList;
import java.util.List;


public class RegularImmutableList_CustomFieldSerializer {

  public static void deserialize(
      SerializationStreamReader reader, RegularImmutableList<?> instance) {}

  public static RegularImmutableList<Object> instantiate(SerializationStreamReader reader)
      throws SerializationException {
    List<Object> elements = new ArrayList<>();
    Collection_CustomFieldSerializerBase.deserialize(reader, elements);
    
    return (RegularImmutableList<Object>) ImmutableList.copyOf(elements);
  }

  public static void serialize(SerializationStreamWriter writer, RegularImmutableList<?> instance)
      throws SerializationException {
    Collection_CustomFieldSerializerBase.serialize(writer, instance);
  }
}
