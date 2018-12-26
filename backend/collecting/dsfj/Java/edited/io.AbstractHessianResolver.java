

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;


public class AbstractHessianResolver implements HessianRemoteResolver {
    
    public Object lookup(String type, String url)
            throws IOException {
        return new HessianRemote(type, url);
    }
}
