

package org.elasticsearch.example.painlesswhitelist;

import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.elasticsearch.painless.spi.PainlessExtension;
import org.elasticsearch.painless.spi.Whitelist;
import org.elasticsearch.painless.spi.WhitelistLoader;
import org.elasticsearch.script.ScriptContext;
import org.elasticsearch.script.SearchScript;


public class ExampleWhitelistExtension implements PainlessExtension {

    private static final Whitelist WHITELIST =
        WhitelistLoader.loadFromResourceFiles(ExampleWhitelistExtension.class, "example_whitelist.txt");

    @Override
    public Map<ScriptContext<?>, List<Whitelist>> getContextWhitelists() {
        return Collections.singletonMap(SearchScript.CONTEXT, Collections.singletonList(WHITELIST));
    }
}
