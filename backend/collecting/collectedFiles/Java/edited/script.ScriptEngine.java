

package org.elasticsearch.script;

import java.io.Closeable;
import java.io.IOException;
import java.util.Map;


public interface ScriptEngine extends Closeable {

    
    String getType();

    
    <FactoryType> FactoryType compile(String name, String code, ScriptContext<FactoryType> context, Map<String, String> params);

    @Override
    default void close() throws IOException {}
}
