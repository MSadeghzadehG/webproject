

package org.elasticsearch.painless.spi;

import java.util.List;
import java.util.Map;

import org.elasticsearch.script.ScriptContext;

public interface PainlessExtension {

    Map<ScriptContext<?>, List<Whitelist>> getContextWhitelists();
}
