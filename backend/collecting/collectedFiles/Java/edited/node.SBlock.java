

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import static java.util.Collections.emptyList;


public final class SBlock extends AStatement {

    private final List<AStatement> statements;

    public SBlock(Location location, List<AStatement> statements) {
        super(location);

        this.statements = Collections.unmodifiableList(statements);
    }

    @Override
    void extractVariables(Set<String> variables) {
        for (AStatement statement : statements) {
            statement.extractVariables(variables);
        }
    }

    @Override
    void analyze(Locals locals) {
        if (statements == null || statements.isEmpty()) {
            throw createError(new IllegalArgumentException("A block must contain at least one statement."));
        }

        AStatement last = statements.get(statements.size() - 1);

        for (AStatement statement : statements) {
                                    if (allEscape) {
                throw createError(new IllegalArgumentException("Unreachable statement."));
            }

            statement.inLoop = inLoop;
            statement.lastSource = lastSource && statement == last;
            statement.lastLoop = (beginLoop || lastLoop) && statement == last;

            statement.analyze(locals);

            methodEscape = statement.methodEscape;
            loopEscape = statement.loopEscape;
            allEscape = statement.allEscape;
            anyContinue |= statement.anyContinue;
            anyBreak |= statement.anyBreak;
            statementCount += statement.statementCount;
        }
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        for (AStatement statement : statements) {
            statement.continu = continu;
            statement.brake = brake;
            statement.write(writer, globals);
        }
    }

    @Override
    public String toString() {
        return multilineToString(emptyList(), statements);
    }
}
