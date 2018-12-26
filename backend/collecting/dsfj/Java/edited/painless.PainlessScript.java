

package org.elasticsearch.painless;

import org.elasticsearch.script.ScriptException;

import java.util.ArrayList;
import java.util.BitSet;
import java.util.List;
import java.util.Map;


public interface PainlessScript {

    
    String getName();

    
    String getSource();

    
    BitSet getStatements();

    
    default ScriptException convertToScriptException(Throwable t, Map<String, List<String>> extraMetadata) {
                List<String> scriptStack = new ArrayList<>();
        for (StackTraceElement element : t.getStackTrace()) {
            if (WriterConstants.CLASS_NAME.equals(element.getClassName())) {
                                int offset = element.getLineNumber();
                if (offset == -1) {
                    scriptStack.add("<<< unknown portion of script >>>");
                } else {
                    offset--;                     int startOffset = getPreviousStatement(offset);
                    if (startOffset == -1) {
                        assert false;                         startOffset = 0;
                    }
                    int endOffset = getNextStatement(startOffset);
                    if (endOffset == -1) {
                        endOffset = getSource().length();
                    }
                                        String snippet = getSource().substring(startOffset, endOffset);
                    scriptStack.add(snippet);
                    StringBuilder pointer = new StringBuilder();
                    for (int i = startOffset; i < offset; i++) {
                        pointer.append(' ');
                    }
                    pointer.append("^---- HERE");
                    scriptStack.add(pointer.toString());
                }
                break;
                        } else if (!shouldFilter(element)) {
                scriptStack.add(element.toString());
            }
        }
                final String name;
        if (PainlessScriptEngine.INLINE_NAME.equals(getName())) {
            name = getSource();
        } else {
            name = getName();
        }
        ScriptException scriptException = new ScriptException("runtime error", t, scriptStack, name, PainlessScriptEngine.NAME);
        for (Map.Entry<String, List<String>> entry : extraMetadata.entrySet()) {
            scriptException.addMetadata(entry.getKey(), entry.getValue());
        }
        return scriptException;
    }

    
    default boolean shouldFilter(StackTraceElement element) {
        return element.getClassName().startsWith("org.elasticsearch.painless.") ||
               element.getClassName().startsWith("java.lang.invoke.") ||
               element.getClassName().startsWith("sun.invoke.");
    }

    
    default int getPreviousStatement(int offset) {
        return getStatements().previousSetBit(offset);
    }

    
    default int getNextStatement(int offset) {
        return getStatements().nextSetBit(offset + 1);
    }
}
