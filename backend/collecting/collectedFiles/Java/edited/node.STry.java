

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;
import org.objectweb.asm.Label;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import static java.util.Collections.singleton;


public final class STry extends AStatement {

    private final SBlock block;
    private final List<SCatch> catches;

    public STry(Location location, SBlock block, List<SCatch> catches) {
        super(location);

        this.block = block;
        this.catches = Collections.unmodifiableList(catches);
    }

    @Override
    void extractVariables(Set<String> variables) {
        if (block != null) {
            block.extractVariables(variables);
        }
        for (SCatch expr : catches) {
            expr.extractVariables(variables);
        }
    }

    @Override
    void analyze(Locals locals) {
        if (block == null) {
            throw createError(new IllegalArgumentException("Extraneous try statement."));
        }

        block.lastSource = lastSource;
        block.inLoop = inLoop;
        block.lastLoop = lastLoop;

        block.analyze(Locals.newLocalScope(locals));

        methodEscape = block.methodEscape;
        loopEscape = block.loopEscape;
        allEscape = block.allEscape;
        anyContinue = block.anyContinue;
        anyBreak = block.anyBreak;

        int statementCount = 0;

        for (SCatch catc : catches) {
            catc.lastSource = lastSource;
            catc.inLoop = inLoop;
            catc.lastLoop = lastLoop;

            catc.analyze(Locals.newLocalScope(locals));

            methodEscape &= catc.methodEscape;
            loopEscape &= catc.loopEscape;
            allEscape &= catc.allEscape;
            anyContinue |= catc.anyContinue;
            anyBreak |= catc.anyBreak;

            statementCount = Math.max(statementCount, catc.statementCount);
        }

        this.statementCount = block.statementCount + statementCount;
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        writer.writeStatementOffset(location);

        Label begin = new Label();
        Label end = new Label();
        Label exception = new Label();

        writer.mark(begin);

        block.continu = continu;
        block.brake = brake;
        block.write(writer, globals);

        if (!block.allEscape) {
            writer.goTo(exception);
        }

        writer.mark(end);

        for (SCatch catc : catches) {
            catc.begin = begin;
            catc.end = end;
            catc.exception = catches.size() > 1 ? exception : null;
            catc.write(writer, globals);
        }

        if (!block.allEscape || catches.size() > 1) {
            writer.mark(exception);
        }
    }

    @Override
    public String toString() {
        return multilineToString(singleton(block), catches);
    }
}
