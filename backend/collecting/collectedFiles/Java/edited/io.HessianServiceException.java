

package com.alibaba.com.caucho.hessian.io;


public class HessianServiceException extends Exception {
    private String code;
    private Object detail;

    
    public HessianServiceException() {
    }

    
    public HessianServiceException(String message, String code, Object detail) {
        super(message);
        this.code = code;
        this.detail = detail;
    }

    
    public String getCode() {
        return code;
    }

    
    public Object getDetail() {
        return detail;
    }
}
