
package com.alibaba.json.bvt.support.jaxrs.mock.entity;

import java.util.List;


public class FastJsonParentTestVO {

	private String name;
	
	private List<FastJsonSonTestVO> sonList;
	
	
	public String getName() {
		return name;
	}

	
	public void setName(String name) {
		this.name = name;
	}

	
	public List<FastJsonSonTestVO> getSonList() {
		return sonList;
	}

	
	public void setSonList(List<FastJsonSonTestVO> sonList) {
		this.sonList = sonList;
	}
	
}
