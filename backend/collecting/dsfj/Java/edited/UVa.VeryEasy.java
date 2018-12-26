



import java.math.BigInteger;
import java.util.Scanner;

public class VeryEasy {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			BigInteger sum = BigInteger.ZERO;
			int N = input.nextInt();
			int A = input.nextInt();
			BigInteger aAsBigInteger = BigInteger.valueOf(A);
			BigInteger product = BigInteger.ONE;
			for (int i = 1; i < N + 1; i++) {
				product = BigInteger.valueOf(i).multiply(aAsBigInteger.pow(i));
				sum = sum.add(product);
			}
			System.out.println(sum);
		}
	}
}
