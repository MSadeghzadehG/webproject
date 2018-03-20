

package org.elasticsearch.cluster.metadata;

import org.elasticsearch.common.collect.Tuple;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;


public interface AliasOrIndex {

    
    boolean isAlias();

    
    List<IndexMetaData> getIndices();

    
    class Index implements AliasOrIndex {

        private final IndexMetaData concreteIndex;

        public Index(IndexMetaData indexMetaData) {
            this.concreteIndex = indexMetaData;
        }

        @Override
        public boolean isAlias() {
            return false;
        }

        @Override
        public List<IndexMetaData> getIndices() {
            return Collections.singletonList(concreteIndex);
        }

        
        public IndexMetaData getIndex() {
            return concreteIndex;
        }

    }

    
    class Alias implements AliasOrIndex {

        private final String aliasName;
        private final List<IndexMetaData> referenceIndexMetaDatas;

        public Alias(AliasMetaData aliasMetaData, IndexMetaData indexMetaData) {
            this.aliasName = aliasMetaData.getAlias();
            this.referenceIndexMetaDatas = new ArrayList<>();
            this.referenceIndexMetaDatas.add(indexMetaData);
        }

        @Override
        public boolean isAlias() {
            return true;
        }

        @Override
        public List<IndexMetaData> getIndices() {
            return referenceIndexMetaDatas;
        }

        
        public Iterable<Tuple<String, AliasMetaData>> getConcreteIndexAndAliasMetaDatas() {
            return new Iterable<Tuple<String, AliasMetaData>>() {
                @Override
                public Iterator<Tuple<String, AliasMetaData>> iterator() {
                    return new Iterator<Tuple<String,AliasMetaData>>() {

                        int index = 0;

                        @Override
                        public boolean hasNext() {
                            return index < referenceIndexMetaDatas.size();
                        }

                        @Override
                        public Tuple<String, AliasMetaData> next() {
                            IndexMetaData indexMetaData = referenceIndexMetaDatas.get(index++);
                            return new Tuple<>(indexMetaData.getIndex().getName(), indexMetaData.getAliases().get(aliasName));
                        }

                        @Override
                        public void remove() {
                            throw new UnsupportedOperationException();
                        }

                    };
                }
            };
        }

        public AliasMetaData getFirstAliasMetaData() {
            return referenceIndexMetaDatas.get(0).getAliases().get(aliasName);
        }

        void addIndex(IndexMetaData indexMetaData) {
            this.referenceIndexMetaDatas.add(indexMetaData);
        }

    }
}
