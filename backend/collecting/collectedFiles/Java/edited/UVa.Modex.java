


import java.math.BigInteger;
import java.util.Scanner;

public class Modex {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		int numberOfTestCases = input.nextInt();
		while (numberOfTestCases != 0) {
			BigInteger x = input.nextBigInteger();
			BigInteger y = input.nextBigInteger();
			BigInteger n = input.nextBigInteger();
			BigInteger result = x.modPow(y, n);
			System.out.println(result);
			numberOfTestCases--;
		}
	}
}

