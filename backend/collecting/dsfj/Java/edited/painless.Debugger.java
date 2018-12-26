

package org.elasticsearch.painless;

import org.elasticsearch.painless.spi.Whitelist;
import org.objectweb.asm.util.Textifier;

import java.io.PrintWriter;
import java.io.StringWriter;


final class Debugger {

    
    static String toString(final String source) {
        return toString(GenericElasticsearchScript.class, source, new CompilerSettings());
    }

    
    static String toString(Class<?> iface, String source, CompilerSettings settings) {
        StringWriter output = new StringWriter();
        PrintWriter outputWriter = new PrintWriter(output);
        Textifier textifier = new Textifier();
        try {
            new Compiler(iface, new Definition(Whitelist.BASE_WHITELISTS))
                    .compile("<debugging>", source, settings, textifier);
        } catch (RuntimeException e) {
            textifier.print(outputWriter);
            e.addSuppressed(new Exception("current bytecode: \n" + output));
            throw e;
        }

        textifier.print(outputWriter);
        return output.toString();
    }
}
