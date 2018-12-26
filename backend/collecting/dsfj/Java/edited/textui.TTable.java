
package com.alibaba.dubbo.qos.textui;

import org.apache.commons.lang3.StringUtils;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import static java.lang.Math.abs;
import static java.lang.Math.max;
import static java.lang.String.format;
import static org.apache.commons.lang3.StringUtils.EMPTY;
import static org.apache.commons.lang3.StringUtils.length;
import static org.apache.commons.lang3.StringUtils.repeat;


public class TTable implements TComponent {

        private final ColumnDefine[] columnDefineArray;

        private final Border border = new Border();

        private int padding;

    public TTable(ColumnDefine[] columnDefineArray) {
        this.columnDefineArray = null == columnDefineArray
                ? new ColumnDefine[0]
                : columnDefineArray;
    }

    public TTable(int columnNum) {
        this.columnDefineArray = new ColumnDefine[columnNum];
        for (int index = 0; index < this.columnDefineArray.length; index++) {
            columnDefineArray[index] = new ColumnDefine();
        }
    }


    @Override
    public String rendering() {
        final StringBuilder tableSB = new StringBuilder();

                final int[] widthCacheArray = new int[getColumnCount()];
        for (int index = 0; index < widthCacheArray.length; index++) {
            widthCacheArray[index] = abs(columnDefineArray[index].getWidth());
        }

        final int rowCount = getRowCount();
        for (int rowIndex = 0; rowIndex < rowCount; rowIndex++) {

            final boolean isFirstRow = rowIndex == 0;
            final boolean isLastRow = rowIndex == rowCount - 1;

                        if (isFirstRow
                    && border.has(Border.BORDER_OUTER_TOP)) {
                tableSB.append(drawSeparationLine(widthCacheArray)).append("\n");
            }

                        if (!isFirstRow
                    && border.has(Border.BORDER_INNER_H)) {
                tableSB.append(drawSeparationLine(widthCacheArray)).append("\n");
            }

                        tableSB.append(drawRow(widthCacheArray, rowIndex));


                        if (isLastRow
                    && border.has(Border.BORDER_OUTER_BOTTOM)) {
                tableSB.append(drawSeparationLine(widthCacheArray)).append("\n");
            }

        }


        return tableSB.toString();
    }


    private String drawRow(int[] widthCacheArray, int rowIndex) {

        final StringBuilder rowSB = new StringBuilder();
        final Scanner[] scannerArray = new Scanner[getColumnCount()];
        try {
            boolean hasNextLine;
            do {

                hasNextLine = false;
                final StringBuilder segmentSB = new StringBuilder();

                for (int colIndex = 0; colIndex < getColumnCount(); colIndex++) {


                    final int width = widthCacheArray[colIndex];
                    final boolean isFirstColOfRow = colIndex == 0;
                    final boolean isLastColOfRow = colIndex == widthCacheArray.length - 1;

                    final String borderChar;
                    if (isFirstColOfRow
                            && border.has(Border.BORDER_OUTER_LEFT)) {
                        borderChar = "|";
                    } else if (!isFirstColOfRow
                            && border.has(Border.BORDER_INNER_V)) {
                        borderChar = "|";
                    } else {
                        borderChar = EMPTY;
                    }


                    if (null == scannerArray[colIndex]) {
                        scannerArray[colIndex] = new Scanner(
                                new StringReader(wrap(getData(rowIndex, columnDefineArray[colIndex]), width)));
                    }
                    final Scanner scanner = scannerArray[colIndex];

                    final String data;
                    if (scanner.hasNextLine()) {
                        data = scanner.nextLine();
                        hasNextLine = true;
                    } else {
                        data = EMPTY;
                    }

                    if (width > 0) {
                        final ColumnDefine columnDefine = columnDefineArray[colIndex];
                        final String dataFormat = getDataFormat(columnDefine, width, data);
                        final String paddingChar = repeat(" ", padding);
                        segmentSB.append(format(borderChar + paddingChar + dataFormat + paddingChar, data));
                    }

                    if (isLastColOfRow) {
                        if (border.has(Border.BORDER_OUTER_RIGHT)) {
                            segmentSB.append("|");
                        }
                        segmentSB.append("\n");
                    }

                }

                if (hasNextLine) {
                    rowSB.append(segmentSB);
                }

            } while (hasNextLine);

            return rowSB.toString();
        } finally {
            for (Scanner scanner : scannerArray) {
                if (null != scanner) {
                    scanner.close();
                }
            }
        }

    }

    private String getData(int rowIndex, ColumnDefine columnDefine) {
        return columnDefine.getRowCount() <= rowIndex
                ? EMPTY
                : columnDefine.rows.get(rowIndex);
    }

    private String getDataFormat(ColumnDefine columnDefine, int width, String data) {
        switch (columnDefine.align) {
            case MIDDLE: {
                final int length = StringUtils.length(data);
                final int diff = width - length;
                final int left = diff / 2;
                return repeat(" ", diff - left) + "%s" + repeat(" ", left);
            }
            case RIGHT: {
                return "%" + width + "s";
            }
            case LEFT:
            default: {
                return "%-" + width + "s";
            }
        }
    }

    
    private int getRowCount() {
        int rowCount = 0;
        for (ColumnDefine columnDefine : columnDefineArray) {
            rowCount = max(rowCount, columnDefine.getRowCount());
        }
        return rowCount;
    }

    
    private int indexLastCol(final int[] widthCacheArray) {
        for (int colIndex = widthCacheArray.length - 1; colIndex >= 0; colIndex--) {
            final int width = widthCacheArray[colIndex];
            if (width <= 0) {
                continue;
            }
            return colIndex;
        }
        return 0;
    }

    
    private String drawSeparationLine(final int[] widthCacheArray) {
        final StringBuilder separationLineSB = new StringBuilder();

        final int lastCol = indexLastCol(widthCacheArray);
        final int colCount = widthCacheArray.length;
        for (int colIndex = 0; colIndex < colCount; colIndex++) {
            final int width = widthCacheArray[colIndex];
            if (width <= 0) {
                continue;
            }

            final boolean isFirstCol = colIndex == 0;
            final boolean isLastCol = colIndex == lastCol;

            if (isFirstCol
                    && border.has(Border.BORDER_OUTER_LEFT)) {
                separationLineSB.append("+");
            }

            if (!isFirstCol
                    && border.has(Border.BORDER_INNER_V)) {
                separationLineSB.append("+");
            }

            separationLineSB.append(repeat("-", width + 2 * padding));

            if (isLastCol
                    && border.has(Border.BORDER_OUTER_RIGHT)) {
                separationLineSB.append("+");
            }

        }
        return separationLineSB.toString();
    }

    
    public TTable addRow(Object... columnDataArray) {

        if (null != columnDataArray) {
            for (int index = 0; index < columnDefineArray.length; index++) {
                final ColumnDefine columnDefine = columnDefineArray[index];
                if (index < columnDataArray.length
                        && null != columnDataArray[index]) {
                    columnDefine.rows.add(replaceTab(columnDataArray[index].toString()));
                } else {
                    columnDefine.rows.add(EMPTY);
                }
            }
        }

        return this;
    }


    
    public enum Align {

        
        LEFT,

        
        RIGHT,

        
        MIDDLE
    }

    
    public static class ColumnDefine {

                private final int width;

                private final boolean isAutoResize;

                private final Align align;

                private final List<String> rows = new ArrayList<String>();

        public ColumnDefine(int width, boolean isAutoResize, Align align) {
            this.width = width;
            this.isAutoResize = isAutoResize;
            this.align = align;
        }

        public ColumnDefine(Align align) {
            this(0, true, align);
        }

        public ColumnDefine(int width) {
            this(width, false, Align.LEFT);
        }

        public ColumnDefine(int width, Align align) {
            this(width, false, align);
        }

        public ColumnDefine() {
            this(Align.LEFT);
        }

        
        public int getWidth() {

                        if (!isAutoResize) {
                return width;
            }

                        int maxWidth = 0;
            for (String data : rows) {
                maxWidth = max(width(data), maxWidth);
            }

            return maxWidth;
        }

        
        public int getRowCount() {
            return rows.size();
        }

    }

    
    public TTable padding(int padding) {
        this.padding = padding;
        return this;
    }

    
    public int getColumnCount() {
        return columnDefineArray.length;
    }


    
    private static String replaceTab(String string) {
        return StringUtils.replace(string, "\t", "    ");
    }

    
    private static int width(String string) {
        int maxWidth = 0;
        final Scanner scanner = new Scanner(new StringReader(string));
        try {
            while (scanner.hasNextLine()) {
                maxWidth = max(length(scanner.nextLine()), maxWidth);
            }
        } finally {
            scanner.close();
        }
        return maxWidth;
    }

    
    public Border getBorder() {
        return border;
    }

    
    public class Border {

        private int borders = BORDER_OUTER | BORDER_INNER;

        
        public static final int BORDER_OUTER_TOP = 1 << 0;

        
        public static final int BORDER_OUTER_RIGHT = 1 << 1;

        
        public static final int BORDER_OUTER_BOTTOM = 1 << 2;

        
        public static final int BORDER_OUTER_LEFT = 1 << 3;

        
        public static final int BORDER_INNER_H = 1 << 4;

        
        public static final int BORDER_INNER_V = 1 << 5;

        
        public static final int BORDER_OUTER = BORDER_OUTER_TOP | BORDER_OUTER_BOTTOM | BORDER_OUTER_LEFT | BORDER_OUTER_RIGHT;

        
        public static final int BORDER_INNER = BORDER_INNER_H | BORDER_INNER_V;

        
        public static final int BORDER_NON = 0;

        
        public boolean has(int... borderArray) {
            if (null == borderArray) {
                return false;
            }
            for (int b : borderArray) {
                if ((this.borders & b) == b) {
                    return true;
                }
            }
            return false;
        }

        
        public int get() {
            return borders;
        }

        
        public Border set(int border) {
            this.borders = border;
            return this;
        }

        public Border add(int border) {
            return set(get() | border);
        }

        public Border remove(int border) {
            return set(get() ^ border);
        }

    }


    public static String wrap(String string, int width) {
        final StringBuilder sb = new StringBuilder();
        final char[] buffer = string.toCharArray();
        int count = 0;
        for (char c : buffer) {

            if (count == width) {
                count = 0;
                sb.append('\n');
                if (c == '\n') {
                    continue;
                }
            }

            if (c == '\n') {
                count = 0;
            } else {
                count++;
            }

            sb.append(c);

        }
        return sb.toString();
    }

}
