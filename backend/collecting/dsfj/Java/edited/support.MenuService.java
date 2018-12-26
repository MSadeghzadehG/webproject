
package com.alibaba.dubbo.rpc.cluster.support;

import java.util.List;

public interface MenuService {

    public Menu getMenu();

    public void addMenu(String menu, List<String> items);

}
