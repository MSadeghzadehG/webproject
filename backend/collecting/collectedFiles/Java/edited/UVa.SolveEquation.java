


import java.util.Scanner;

public class SolveEquation {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);

		int numberOfTestCases = input.nextInt();
		while (numberOfTestCases != 0) {

			int A = input.nextInt();
			int B = input.nextInt();
			int C = input.nextInt();
			boolean hasSolution = false;

			for (int x = -22; x <= 22 && !hasSolution; x++) {
				if (x * x <= C) {
					for (int y = -100; y <= 100 && !hasSolution; y++) {
						if (x != y && y * y <= C && (x * x + y * y <= C)) {
							int z = A - x - y;
							if ((z != y && z != x && x * x + y * y + z * z == C)
									&& x * y * z == B) {
								hasSolution = true;
								System.out.println(x + " " + y + " " + z);
							}
						}
					}
				}
			}
			if (!hasSolution) {
				System.out.println("No solution.");
			}

			numberOfTestCases--;
		}

	}
}
