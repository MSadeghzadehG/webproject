
package com.alibaba.dubbo.qos.textui;

import org.apache.commons.lang3.StringUtils;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import static java.lang.System.currentTimeMillis;
import static org.apache.commons.lang3.StringUtils.EMPTY;
import static org.apache.commons.lang3.StringUtils.repeat;


public class TTree implements TComponent {

    private static final String STEP_FIRST_CHAR = "`---";
    private static final String STEP_NORMAL_CHAR = "+---";
    private static final String STEP_HAS_BOARD = "|   ";
    private static final String STEP_EMPTY_BOARD = "    ";

        private final boolean isPrintCost;

        private final Node root;

        private Node current;


    public TTree(boolean isPrintCost, String title) {
        this.root = new Node(title).markBegin().markEnd();
        this.current = root;
        this.isPrintCost = isPrintCost;
    }


    @Override
    public String rendering() {

        final StringBuilder treeSB = new StringBuilder();
        recursive(0, true, "", root, new Callback() {

            @Override
            public void callback(int deep, boolean isLast, String prefix, Node node) {

                final boolean hasChild = !node.children.isEmpty();
                final String stepString = isLast ? STEP_FIRST_CHAR : STEP_NORMAL_CHAR;
                final int stepStringLength = StringUtils.length(stepString);
                treeSB.append(prefix).append(stepString);

                int costPrefixLength = 0;
                if (hasChild) {
                    treeSB.append("+");
                }
                if (isPrintCost
                        && !node.isRoot()) {
                    final String costPrefix = String.format("[%s,%sms]", (node.endTimestamp - root.beginTimestamp), (node.endTimestamp - node.beginTimestamp));
                    costPrefixLength = StringUtils.length(costPrefix);
                    treeSB.append(costPrefix);
                }

                final Scanner scanner = new Scanner(new StringReader(node.data.toString()));
                try {
                    boolean isFirst = true;
                    while (scanner.hasNextLine()) {
                        if (isFirst) {
                            treeSB.append(scanner.nextLine()).append("\n");
                            isFirst = false;
                        } else {
                            treeSB
                                    .append(prefix)
                                    .append(repeat(' ', stepStringLength))
                                    .append(hasChild ? "|" : EMPTY)
                                    .append(repeat(' ', costPrefixLength))
                                    .append(scanner.nextLine())
                                    .append("\n");
                        }
                    }
                } finally {
                    scanner.close();
                }

            }

        });

        return treeSB.toString();
    }

    
    private void recursive(int deep, boolean isLast, String prefix, Node node, Callback callback) {
        callback.callback(deep, isLast, prefix, node);
        if (!node.isLeaf()) {
            final int size = node.children.size();
            for (int index = 0; index < size; index++) {
                final boolean isLastFlag = index == size - 1;
                final String currentPrefix = isLast ? prefix + STEP_EMPTY_BOARD : prefix + STEP_HAS_BOARD;
                recursive(
                        deep + 1,
                        isLastFlag,
                        currentPrefix,
                        node.children.get(index),
                        callback
                );
            }
        }
    }

    public boolean isTop() {
        return current.isRoot();
    }

    
    public TTree begin(Object data) {
        current = new Node(current, data);
        current.markBegin();
        return this;
    }

    public TTree begin() {
        return begin(null);
    }

    public Object get() {
        if (current.isRoot()) {
            throw new IllegalStateException("current node is root.");
        }
        return current.data;
    }

    public TTree set(Object data) {
        if (current.isRoot()) {
            throw new IllegalStateException("current node is root.");
        }
        current.data = data;
        return this;
    }

    
    public TTree end() {
        if (current.isRoot()) {
            throw new IllegalStateException("current node is root.");
        }
        current.markEnd();
        current = current.parent;
        return this;
    }


    
    private static class Node {

        
        final Node parent;

        
        Object data;

        
        final List<Node> children = new ArrayList<Node>();

        
        private long beginTimestamp;

        
        private long endTimestamp;

        
        private Node(Object data) {
            this.parent = null;
            this.data = data;
        }

        
        private Node(Node parent, Object data) {
            this.parent = parent;
            this.data = data;
            parent.children.add(this);
        }

        
        boolean isRoot() {
            return null == parent;
        }

        
        boolean isLeaf() {
            return children.isEmpty();
        }

        Node markBegin() {
            beginTimestamp = currentTimeMillis();
            return this;
        }

        Node markEnd() {
            endTimestamp = currentTimeMillis();
            return this;
        }

    }


    
    private interface Callback {

        void callback(int deep, boolean isLast, String prefix, Node node);

    }

}
