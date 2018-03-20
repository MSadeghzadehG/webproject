package com.alibaba.json.test.vans;

import com.alibaba.fastjson.annotation.JSONType;

import java.io.Serializable;
import java.util.ArrayList;


@JSONType(orders = {"uuid","type","matrix","children"})
public class VansObject implements Serializable {
    public String uuid;
    public String type;
    public ArrayList<VansObjectChildren> children;
    public float[] matrix;
}
