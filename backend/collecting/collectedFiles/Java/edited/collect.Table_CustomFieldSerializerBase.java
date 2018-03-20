

package com.google.common.collect;

import com.google.gwt.user.client.rpc.SerializationException;
import com.google.gwt.user.client.rpc.SerializationStreamReader;
import com.google.gwt.user.client.rpc.SerializationStreamWriter;
import java.util.Map;
import java.util.Map.Entry;


final class Table_CustomFieldSerializerBase {
  static <T extends StandardTable<Object, Object, Object>> T populate(
      SerializationStreamReader reader, T table) throws SerializationException {
    Map<?, ?> hashMap = (Map<?, ?>) reader.readObject();
    for (Entry<?, ?> row : hashMap.entrySet()) {
      table.row(row.getKey()).putAll((Map<?, ?>) row.getValue());
    }
    return table;
  }

  static void serialize(SerializationStreamWriter writer, StandardTable<?, ?, ?> table)
      throws SerializationException {
    
    writer.writeObject(table.backingMap);
  }

  private Table_CustomFieldSerializerBase() {}
}
