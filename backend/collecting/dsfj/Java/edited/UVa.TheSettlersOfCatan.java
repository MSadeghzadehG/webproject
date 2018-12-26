



import java.io.*;


public class TheSettlersOfCatan {

    public static int[][] matrix = new int[30][30];
    public static int answer;

    public static void main(String args[]) throws Exception {

                BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line;

                while(!(line = br.readLine()).equals("0 0")) {

                        int nodes = Integer.parseInt(line.split(" ")[0]);
            int edges = Integer.parseInt(line.split(" ")[1]);

                        for(int i = 0; i < edges; i++) {

                                String[] current = br.readLine().split(" ");
                int x = Integer.parseInt(current[0]);
                int y = Integer.parseInt(current[1]);

                                matrix[x][y] = 1;
                matrix[y][x] = 1;

            }

                        answer = 0;

                        for(int i = 0; i < nodes; i++) {

                dfs(i, 0, nodes);

            }

                        System.out.println(answer);

                        matrix = new int[30][30];

        }

    }

    public static void dfs(int nd, int l, int nodes) {

                if(l > answer) {

            answer = l;

        }

        for(int i = 0; i < nodes; i++) {

            if(matrix[nd][i] > 0) {
                                matrix[nd][i] = 0;
                matrix[i][nd] = 0;
                                dfs(i, l + 1, nodes);
                                matrix[nd][i] = 1;
                matrix[i][nd] = 1;

            }

        }

    }

}

