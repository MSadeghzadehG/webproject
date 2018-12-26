

package org.elasticsearch.index.fielddata.plain;

import org.apache.lucene.index.DocValues;
import org.apache.lucene.index.LeafReader;
import org.apache.lucene.index.SortedSetDocValues;
import org.apache.lucene.util.Accountable;
import org.elasticsearch.index.fielddata.AtomicFieldData;
import org.elasticsearch.index.fielddata.ScriptDocValues;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.function.Function;


public final class SortedSetDVBytesAtomicFieldData extends AbstractAtomicOrdinalsFieldData {

    private final LeafReader reader;
    private final String field;

    SortedSetDVBytesAtomicFieldData(LeafReader reader, String field, Function<SortedSetDocValues,
            ScriptDocValues<?>> scriptFunction) {
        super(scriptFunction);
        this.reader = reader;
        this.field = field;
    }

    @Override
    public SortedSetDocValues getOrdinalsValues() {
        try {
            return DocValues.getSortedSet(reader, field);
        } catch (IOException e) {
            throw new IllegalStateException("cannot load docvalues", e);
        }
    }

    @Override
    public void close() {
    }

    @Override
    public long ramBytesUsed() {
        return 0;     }
    
    @Override
    public Collection<Accountable> getChildResources() {
        return Collections.emptyList();
    }

}
