
package com.alibaba.dubbo.rpc.service;


public interface GenericService {

    
    Object $invoke(String method, String[] parameterTypes, Object[] args) throws GenericException;

}