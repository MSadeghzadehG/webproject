
package com.alibaba.dubbo.common.json;

import java.io.IOException;


@Deprecated
interface JSONNode {
    
    void writeJSON(JSONConverter jc, JSONWriter jb, boolean writeClass) throws IOException;
}