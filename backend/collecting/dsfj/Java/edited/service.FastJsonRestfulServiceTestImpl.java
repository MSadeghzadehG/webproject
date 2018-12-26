
package com.alibaba.json.bvt.support.jaxrs.mock.service;

import java.util.List;

import org.springframework.stereotype.Service;

import com.alibaba.fastjson.JSONObject;
import com.alibaba.json.bvt.support.spring.mock.entity.FastJsonParentTestVO;
import com.alibaba.json.bvt.support.spring.mock.entity.FastJsonTestVO;


@Service("fastJsonRestful")
public class FastJsonRestfulServiceTestImpl implements FastJsonRestfulServiceTest {

	@Override
	public JSONObject test1(FastJsonTestVO vo) {

		JSONObject jsonObj = new JSONObject();
		
		jsonObj.put("id", vo.getId());
		
		jsonObj.put("name", vo.getName());
		
		return jsonObj;
	}

	@Override
	public JSONObject test2(List<FastJsonParentTestVO> vos) {

		JSONObject jsonObj = new JSONObject();
		
		for (FastJsonParentTestVO fastJsonParentTestVO : vos) {
			
			jsonObj.put(fastJsonParentTestVO.getName(), fastJsonParentTestVO.getSonList().size());
		}
		
		return jsonObj;
	}
}
