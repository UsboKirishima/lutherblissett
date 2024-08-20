import java.io.*;
import java.net.*;

import lib.GetInfo;

public class Server {
    public static final int PORT = 8080;

    public static void main(String[] args) throws IOException {
        ServerSocket s = new ServerSocket();

        s.setReuseAddress(true);
        s.bind(new InetSocketAddress(PORT));

        System.out.println("Started: " + s);

        try {
            Socket socket = s.accept();
            try {
                System.out.println("Connection accepted: " + socket);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(
                        new BufferedWriter(
                                new OutputStreamWriter(socket.getOutputStream())),
                        true);

                /**
                 * Handling Commands
                 */
                while (true) {
                    String str = in.readLine();
                    if (str.equals("END"))
                        break;

                    if (str.startsWith("lb{0x0001}")) {
                        GetInfo info = new GetInfo();
                        info.run(str, out);
                    }

                    // System.out.println("Echoing: " + str + "\n");
                    // out.println(str);
                }
            } finally {
                System.out.println("closing...");
                socket.close();
            }
        } finally {
            s.close();
        }
    }
}