
package org.elasticsearch.xpack.ml.job.process.autodetect.writer;

import org.elasticsearch.xpack.core.ml.job.process.autodetect.writer.RecordWriter;
import org.supercsv.io.CsvListWriter;
import org.supercsv.prefs.CsvPreference;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.List;


public class CsvRecordWriter implements RecordWriter {
    private final CsvListWriter writer;

    
    public CsvRecordWriter(OutputStream os) {
        writer = new CsvListWriter(new OutputStreamWriter(os, StandardCharsets.UTF_8), CsvPreference.STANDARD_PREFERENCE);
    }

    @Override
    public void writeRecord(String[] record) throws IOException {
        writer.write(record);
    }

    @Override
    public void writeRecord(List<String> record) throws IOException {
        writer.write(record);
    }

    @Override
    public void flush() throws IOException {
        writer.flush();
    }

}
