


import java.math.BigInteger;
import java.util.Scanner;

public class BigMod {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			BigInteger b = input.nextBigInteger();
			BigInteger p = input.nextBigInteger();
			BigInteger m = input.nextBigInteger();
			System.out.println(b.modPow(p, m));
		}
	}
}
