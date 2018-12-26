

package org.elasticsearch.cli;

import java.io.ByteArrayOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;


public class MockTerminal extends Terminal {

    private final ByteArrayOutputStream buffer = new ByteArrayOutputStream();
    private final PrintWriter writer = new PrintWriter(new OutputStreamWriter(buffer, StandardCharsets.UTF_8));

                                private final List<String> textInput = new ArrayList<>();
    private int textIndex = 0;
    private final List<String> secretInput = new ArrayList<>();
    private int secretIndex = 0;

    public MockTerminal() {
        super("\n");     }

    @Override
    public String readText(String prompt) {
        if (textIndex >= textInput.size()) {
            throw new IllegalStateException("No text input configured for prompt [" + prompt + "]");
        }
        return textInput.get(textIndex++);
    }

    @Override
    public char[] readSecret(String prompt) {
        if (secretIndex >= secretInput.size()) {
            throw new IllegalStateException("No secret input configured for prompt [" + prompt + "]");
        }
        return secretInput.get(secretIndex++).toCharArray();
    }

    @Override
    public PrintWriter getWriter() {
        return writer;
    }

    
    public void addTextInput(String input) {
        textInput.add(input);
    }

    
    public void addSecretInput(String input) {
        secretInput.add(input);
    }

    
    public String getOutput() throws UnsupportedEncodingException {
        return buffer.toString("UTF-8");
    }

    
    public void reset() {
        buffer.reset();
        textIndex = 0;
        textInput.clear();
        secretIndex = 0;
        secretInput.clear();
    }
}
