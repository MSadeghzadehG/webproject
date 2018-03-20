

package org.elasticsearch.index.fielddata.plain;

import org.apache.lucene.index.BinaryDocValues;
import org.apache.lucene.index.DocValues;
import org.apache.lucene.index.LeafReader;
import org.apache.lucene.util.Accountable;
import org.elasticsearch.index.fielddata.AtomicFieldData;
import org.elasticsearch.index.fielddata.FieldData;
import org.elasticsearch.index.fielddata.ScriptDocValues;
import org.elasticsearch.index.fielddata.ScriptDocValues.Strings;
import org.elasticsearch.index.fielddata.SortedBinaryDocValues;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;


public class BinaryDVAtomicFieldData implements AtomicFieldData {

    private final LeafReader reader;
    private final String field;

    public BinaryDVAtomicFieldData(LeafReader reader, String field) {
        this.reader = reader;
        this.field = field;
    }

    @Override
    public SortedBinaryDocValues getBytesValues() {
        try {
            final BinaryDocValues values = DocValues.getBinary(reader, field);
            return FieldData.singleton(values);
        } catch (IOException e) {
            throw new IllegalStateException("Cannot load doc values", e);
        }
    }

    @Override
    public Strings getScriptValues() {
        return new ScriptDocValues.Strings(getBytesValues());
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
