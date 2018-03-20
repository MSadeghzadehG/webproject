
package com.alibaba.dubbo.qos.textui;

import java.util.ArrayList;
import java.util.List;

import static org.apache.commons.lang3.StringUtils.repeat;


public class TLadder implements TComponent {

        private static final String LADDER_CHAR = "`-";

        private static final String STEP_CHAR = " ";

        private static final int INDENT_STEP = 2;

    private final List<String> items = new ArrayList<String>();


    @Override
    public String rendering() {
        final StringBuilder ladderSB = new StringBuilder();
        int deep = 0;
        for (String item : items) {

                        if (deep == 0) {
                ladderSB
                        .append(item)
                        .append("\n");
            }

                        else {
                ladderSB
                        .append(repeat(STEP_CHAR, deep * INDENT_STEP))
                        .append(LADDER_CHAR)
                        .append(item)
                        .append("\n");
            }

            deep++;

        }
        return ladderSB.toString();
    }

    
    public TLadder addItem(String item) {
        items.add(item);
        return this;
    }

}
