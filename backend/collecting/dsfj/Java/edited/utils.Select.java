

package com.badlogic.gdx.utils;

import java.util.Comparator;


public class Select {
	private static Select instance;
	private QuickSelect quickSelect;

	
	public static Select instance () {
		if (instance == null) instance = new Select();
		return instance;
	}

	public <T> T select (T[] items, Comparator<T> comp, int kthLowest, int size) {
		int idx = selectIndex(items, comp, kthLowest, size);
		return items[idx];
	}

	public <T> int selectIndex (T[] items, Comparator<T> comp, int kthLowest, int size) {
		if (size < 1) {
			throw new GdxRuntimeException("cannot select from empty array (size < 1)");
		} else if (kthLowest > size) {
			throw new GdxRuntimeException("Kth rank is larger than size. k: " + kthLowest + ", size: " + size);
		}
		int idx;
				if (kthLowest == 1) {
						idx = fastMin(items, comp, size);
		} else if (kthLowest == size) {
						idx = fastMax(items, comp, size);
		} else {
						if (quickSelect == null) quickSelect = new QuickSelect();
			idx = quickSelect.select(items, comp, kthLowest, size);
		}
		return idx;
	}

	
	private <T> int fastMin (T[] items, Comparator<T> comp, int size) {
		int lowestIdx = 0;
		for (int i = 1; i < size; i++) {
			int comparison = comp.compare(items[i], items[lowestIdx]);
			if (comparison < 0) {
				lowestIdx = i;
			}
		}
		return lowestIdx;
	}

	
	private <T> int fastMax (T[] items, Comparator<T> comp, int size) {
		int highestIdx = 0;
		for (int i = 1; i < size; i++) {
			int comparison = comp.compare(items[i], items[highestIdx]);
			if (comparison > 0) {
				highestIdx = i;
			}
		}
		return highestIdx;
	}
}
