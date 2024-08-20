package lib;

import java.io.*;
import java.text.DecimalFormat;

/**
 * lb{0x0001} - GetInfo
 * Here the client sends the command lb{0x0001} to the server
 * and expects to receive a string containing
 * user, os, kernel, cpu, mem_u, mem_t, uptime,
 * separated by the character 'ù'.
 * 
 * @example
 *          Input: lb{0x0001}
 * @example
 *          Output: "Arch LinuxùKernel 6.10ù..."
 */
public class GetInfo {

    String username = System.getProperty("user.name");
    String osName = System.getProperty("os.name");
    String osVersion = System.getProperty("os.version");

    private String getOsName() {
        String distroName = "";

        try {
            BufferedReader reader = new BufferedReader(new FileReader("/etc/os-release"));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.startsWith("PRETTY_NAME=")) {
                    distroName = line.split("=")[1].replace("\"", "");
                    break;
                }
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return distroName;
    }

    private String getCpuModel() {
        String cpuModel = "";

        try {
            BufferedReader reader = new BufferedReader(new FileReader("/proc/cpuinfo"));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.startsWith("model name")) {
                    cpuModel = line.split(":")[1];
                    break;
                }
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return cpuModel;
    }

    private double convertKBToMB(long kb) {
        return kb / 1024.0;
    }

    private String getMemTotal() {
        long totalMem = 0;
        double convertedMem;
        String totalMemString = "";

        try {
            BufferedReader reader = new BufferedReader(new FileReader("/proc/meminfo"));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.startsWith("MemTotal:")) {
                    totalMemString = line.split(":")[1].trim().split(" ")[0];
                    totalMem = Long.parseLong(totalMemString);
                    convertedMem = convertKBToMB(totalMem);
                    DecimalFormat df = new DecimalFormat("#");
                    totalMemString = df.format(convertedMem);
                    break;
                }
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return totalMemString;
    }

    private String getMemUsed() {
        long freeMem = 0, totalMem = 0, usedMem;
        double convertedMem;
        String usedMemString = "";

        try {
            BufferedReader reader = new BufferedReader(new FileReader("/proc/meminfo"));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.startsWith("MemTotal:")) {
                    totalMem = Long.parseLong(line.split(":")[1].trim().split(" ")[0]);
                }
                if (line.startsWith("MemFree:")) {
                    String freeMemString = line.split(":")[1].trim().split(" ")[0];
                    freeMem = Long.parseLong(freeMemString);
                }
                if (totalMem > 0 && freeMem >= 0) {
                    usedMem = totalMem - freeMem;
                    convertedMem = convertKBToMB(usedMem);
                    DecimalFormat df = new DecimalFormat("#");
                    usedMemString = df.format(convertedMem);
                    break;
                }
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return usedMemString;
    }

    private String getUptime() {
        double uptime;
        String uptimeString = "";
        try {
            BufferedReader reader = new BufferedReader(new FileReader("/proc/uptime"));
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(" ");

                DecimalFormat df = new DecimalFormat("#");
                uptime = Double.parseDouble(parts[0]);
                uptimeString = df.format(uptime);

                if (uptime >= 86400.0)
                    uptimeString = df.format(uptime / 86400.0) + " days";
                else if (uptime >= 3600.0)
                    uptimeString = df.format(uptime / 3600.0) + " hours";
                else if (uptime >= 60.0)
                    uptimeString = df.format(uptime / 60.0) + " minutes";
                else
                    uptimeString = df.format(uptime) + " seconds";

            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return uptimeString;
    }

    public void run(String command, PrintWriter out) {

        final char separator = 'ù';
        String responseFormatted = username
                + separator
                + getOsName() /* Os Name */
                + separator
                + osVersion /* Kernel */
                + separator
                + getCpuModel() /* Cpu Model */
                + separator
                + getMemTotal() /* Memory Total */
                + separator
                + getMemUsed() /* Memory Used */
                + separator
                + getUptime(); /* Formatted Uptime */

        System.out.println(responseFormatted);
        out.println("Test!!!");
    }
}
