
package com.alibaba.dubbo.qos.textui;

import org.apache.commons.lang3.StringUtils;

import java.util.Scanner;


public class TKv implements TComponent {

    private final TTable tTable;

    public TKv() {
        this.tTable = new TTable(new TTable.ColumnDefine[]{
                new TTable.ColumnDefine(TTable.Align.RIGHT),
                new TTable.ColumnDefine(TTable.Align.RIGHT),
                new TTable.ColumnDefine(TTable.Align.LEFT)
        })
                .padding(0);
        this.tTable.getBorder().set(TTable.Border.BORDER_NON);
    }

    public TKv(TTable.ColumnDefine keyColumnDefine, TTable.ColumnDefine valueColumnDefine) {
        this.tTable = new TTable(new TTable.ColumnDefine[]{
                keyColumnDefine,
                new TTable.ColumnDefine(TTable.Align.RIGHT),
                valueColumnDefine
        })
                .padding(0);
        this.tTable.getBorder().set(TTable.Border.BORDER_NON);
    }

    public TKv add(final Object key, final Object value) {
        tTable.addRow(key, " : ", value);
        return this;
    }

    @Override
    public String rendering() {
        return filterEmptyLine(tTable.rendering());
    }

    private String filterEmptyLine(String content) {
        final StringBuilder sb = new StringBuilder();
        Scanner scanner = null;
        try {
            scanner = new Scanner(content);
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                if (line != null) {
                                        line = StringUtils.stripEnd(line, " ");
                    if (line.isEmpty()) {
                        line = " ";
                    }
                }
                sb.append(line).append('\n');
            }
        } finally {
            if (null != scanner) {
                scanner.close();
            }
        }

        return sb.toString();
    }
}
