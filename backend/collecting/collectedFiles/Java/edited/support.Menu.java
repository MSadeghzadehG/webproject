
package com.alibaba.dubbo.rpc.cluster.support;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Menu {

    private Map<String, List<String>> menus = new HashMap<String, List<String>>();

    public Menu() {
    }

    public Menu(Map<String, List<String>> menus) {
        for (String key : menus.keySet()) {
            this.menus.put(key, new ArrayList<String>(menus.get(key)));
        }
    }

    public void putMenuItem(String menu, String item) {
        List<String> items = menus.get(menu);
        if (item == null) {
            items = new ArrayList<String>();
            menus.put(menu, items);
        }
        items.add(item);
    }

    public void addMenu(String menu, List<String> items) {
        List<String> menuItems = menus.get(menu);
        if (menuItems == null) {
            menus.put(menu, new ArrayList<String>(items));
        } else {
            menuItems.addAll(new ArrayList<String>(items));
        }
    }

    public Map<String, List<String>> getMenus() {
        return Collections.unmodifiableMap(menus);
    }

    public void merge(Menu menu) {
        for (Map.Entry<String, List<String>> entry : menu.menus.entrySet()) {
            addMenu(entry.getKey(), entry.getValue());
        }
    }

}
