


import java.math.BigInteger;
import java.util.Scanner;

public class WhatBaseIsThis {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			String x = input.next();
			String y = input.next();
			boolean found = false;
			for (int i = 2; i < 37 && !found; i++) {
				BigInteger xConvertedToBase;
				try {
					xConvertedToBase = new BigInteger(x, i);
				} catch (Exception e) {
					continue;
				}
				for (int j = 2; j < 37; j++) {
					BigInteger yConvertedToBase;
					try {
						yConvertedToBase = new BigInteger(y, j);
					} catch (Exception e) {
						continue;
					}
					if (xConvertedToBase.equals(yConvertedToBase)) {
						System.out.println(x + " (base " + i + ") = " + y
								+ " (base " + j + ")");
						found = true;
						break;
					}
				}
			}
			if (!found) {
				System.out.println(x + " is not equal to " + y
						+ " in any base 2..36");
			}
		}
	}
}
