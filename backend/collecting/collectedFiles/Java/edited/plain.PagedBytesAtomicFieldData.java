
package org.elasticsearch.index.fielddata.plain;

import org.apache.lucene.index.SortedSetDocValues;
import org.apache.lucene.util.Accountable;
import org.apache.lucene.util.Accountables;
import org.apache.lucene.util.BytesRef;
import org.apache.lucene.util.PagedBytes;
import org.apache.lucene.util.packed.PackedLongValues;
import org.elasticsearch.index.fielddata.ordinals.Ordinals;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

public class PagedBytesAtomicFieldData extends AbstractAtomicOrdinalsFieldData {

    private final PagedBytes.Reader bytes;
    private final PackedLongValues termOrdToBytesOffset;
    protected final Ordinals ordinals;

    public PagedBytesAtomicFieldData(PagedBytes.Reader bytes, PackedLongValues termOrdToBytesOffset, Ordinals ordinals) {
        super(DEFAULT_SCRIPT_FUNCTION);
        this.bytes = bytes;
        this.termOrdToBytesOffset = termOrdToBytesOffset;
        this.ordinals = ordinals;
    }

    @Override
    public void close() {
    }

    @Override
    public long ramBytesUsed() {
        long size = ordinals.ramBytesUsed();
                size += bytes.ramBytesUsed();
                size += termOrdToBytesOffset.ramBytesUsed();
        return size;
    }

    @Override
    public Collection<Accountable> getChildResources() {
        List<Accountable> resources = new ArrayList<>();
        resources.add(Accountables.namedAccountable("ordinals", ordinals));
        resources.add(Accountables.namedAccountable("term bytes", bytes));
        resources.add(Accountables.namedAccountable("term offsets", termOrdToBytesOffset));
        return Collections.unmodifiableList(resources);
    }

    @Override
    public SortedSetDocValues getOrdinalsValues() {
        return ordinals.ordinals(new ValuesHolder(bytes, termOrdToBytesOffset));
    }

    private static class ValuesHolder implements Ordinals.ValuesHolder {

        private final BytesRef scratch = new BytesRef();
        private final PagedBytes.Reader bytes;
        private final PackedLongValues termOrdToBytesOffset;

        ValuesHolder(PagedBytes.Reader bytes, PackedLongValues termOrdToBytesOffset) {
            this.bytes = bytes;
            this.termOrdToBytesOffset = termOrdToBytesOffset;
        }

        @Override
        public BytesRef lookupOrd(long ord) {
            assert ord >= 0;
            bytes.fill(scratch, termOrdToBytesOffset.get(ord));
            return scratch;
        }

    }

}
