package lib;

import java.awt.*;
import java.awt.TrayIcon.MessageType;
import java.io.PrintWriter;

/**
 * lb{0x0004} - Notify
 * Here client sends lb{0x0004} [message]
 * and server display a notification to the victim
 * 
 * @example
 *          Input: lb{0x0004} You got hacked!!!
 * @example
 *          Output: "lb{0x0004} [SUCCESS] Notify sent."
 */
public class Notify {

    public static PrintWriter out;

    public Notify(PrintWriter out_) {
        Notify.out = out_;
    }

    /* todo: use os notification without awt old lib */
    public static void send(String title, String message) {
        try {
            if (SystemTray.isSupported()) {
                SystemTray tray = SystemTray.getSystemTray();
                Image image = Toolkit.getDefaultToolkit().createImage("icon.png"); /* todo: add image to tray */

                TrayIcon trayIcon = new TrayIcon(image, "Java Notification");
                trayIcon.setImageAutoSize(true);
                trayIcon.setToolTip("Luther Blissett Notification");
                tray.add(trayIcon);

                Notify.out.println("lb{0x0004} [SUCCESS] Notify sent.");
                System.out.println("lb{0x0004} [SUCCESS] Notify sent.");

                trayIcon.displayMessage(title, message, MessageType.INFO);
            } else {
                Notify.out.println("lb{0x0004} [ERROR] System tray not supported!");
                System.err.println("lb{0x0004} [ERROR] System tray not supported!");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
