package lib;

import java.awt.AWTException;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Base64;
import javax.imageio.ImageIO;

/**
 * lb{0x0007} - ScreenSharing
 * lb{0x0008} - Stop ScreenSharing
 * 
 * Here, the server receives lb{0x0007}.
 * Subsequently, the server sends lb{0x0007}.
 * The server then repeatedly sends screen captures.
 * These captures are compressed into JPEG format to achieve high compression, 
 * which, however, reduces the image quality.
 * The server stops only when it receives lb{0x0008}, 
 * which it will then send back to the client as lb{0x0008}.
 * 
 * fix: still buggy maybe in the screenshot section,
 *      probably take a screenshot require more than few ms.
 * 
 * todo: find a new method to take screenshot 
 */
public class ScreenSharing {

    private static final int CHUNK_SIZE = 8192; 

    public void share(Socket socket) throws IOException, InterruptedException {
        DataOutputStream dos = new DataOutputStream(socket.getOutputStream());

        while (true) {
            BufferedImage screenImage = ScreenCapture.captureScreen();
            byte[] imageBytes = ImageCompressor.compressImage(screenImage);
            String base64Image = Base64.getEncoder().encodeToString(imageBytes);

            int length = base64Image.length();
            int offset = 0;


            while (offset < length) {
                int end = Math.min(offset + CHUNK_SIZE, length);
                String chunk = base64Image.substring(offset, end);
                dos.writeUTF(chunk); 
                offset = end;
            }

            
            dos.flush();
            Thread.sleep(100); 
        }
    }
}

class ScreenCapture {
    public static BufferedImage captureScreen() {
        BufferedImage screenImage = null;
        try {
            Robot robot = new Robot();
            Rectangle screenRect = new Rectangle(Toolkit.getDefaultToolkit().getScreenSize());
            screenImage = robot.createScreenCapture(screenRect);
        } catch (AWTException e) {
            e.printStackTrace();
        }
        return screenImage;
    }
}

class ImageCompressor {
    public static byte[] compressImage(BufferedImage image) {
        byte[] imageBytes = null;
        try (ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
            ImageIO.write(image, "jpg", baos);
            baos.flush();
            imageBytes = baos.toByteArray();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return imageBytes;
    }
}
