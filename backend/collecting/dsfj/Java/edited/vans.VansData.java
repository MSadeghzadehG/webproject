package com.alibaba.json.test.vans;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;


public class VansData implements Serializable{

    public int[] textures;
    public ArrayList<String> images;
    public VansObject object;
    public VansMetaData metadata;
    public ArrayList<VansGeometry> geometries;
    public ArrayList<VansAnimation> animations;
    public Object materials;

}