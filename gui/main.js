const { app, BrowserWindow, ipcMain, Notification, nativeImage } = require('electron');
const path = require('path');
const net = require('net');
const fs = require('fs');

let mainWindow;
let tcpClient;
let imageWindow;

function showNotification(title, body) {
  new Notification({ title, body }).show();
}

function connectToServer(host, port) {
  tcpClient = new net.Socket();
  const TCP_PORT = Number(port);

  tcpClient.connect(TCP_PORT, host, () => {
    console.log('Connected to TCP server');
    mainWindow.webContents.send('connection-status', 'connected');
  });

  tcpClient.on('data', (data) => {
    const message = data.toString();
    console.log('Message received from server:', message);

    if (message.startsWith('START_FILE_TRANSFER')) {
      const [_, destinationPath] = message.split(' ');
      if (typeof destinationPath !== 'string' || destinationPath.trim() === '') {
        console.error('Invalid destination path:', destinationPath);
        mainWindow.webContents.send('tcp-message', 'Invalid destination path received from server.');
        return;
      }
      const fileStream = fs.createWriteStream(destinationPath.trim());

      tcpClient.on('data', (chunk) => {
        if (chunk.toString().endsWith('END_FILE_TRANSFER')) {
          fileStream.end();
          mainWindow.webContents.send('tcp-message', 'File transfer completed');
          console.log('File transfer completed');
          showNotification('File Transfer', `File saved to ${destinationPath.trim()}`);
        } else {
          fileStream.write(chunk);
        }
      });
    } else if (message.startsWith('lb{0x0007}')) {
      // Handling the display of the remote desktop
      const widthBuffer = data.slice(0, 4);
      const heightBuffer = data.slice(4, 8);
      const imageData = data.slice(8);

      const width = widthBuffer.readInt32BE(0);
      const height = heightBuffer.readInt32BE(0);

      const image = nativeImage.createFromBuffer(imageData, { width, height });
      
      if (!imageWindow) {
        imageWindow = new BrowserWindow({
          width: width,
          height: height,
          autoHideMenuBar: true,
          frame: true,
          alwaysOnTop: false,
          webPreferences: {
            contextIsolation: false,
            nodeIntegration: true,
          }
        });
        imageWindow.loadURL(`data:text/html,
          <html><body style="margin:0;overflow:hidden;">
          <img id="remote-desktop" style="width:100%;height:100%;" src="${image.toDataURL()}" />
          </body></html>`);
      } else {
        imageWindow.webContents.executeJavaScript(
          `document.getElementById('remote-desktop').src = "${image.toDataURL()}";`
        );
      }
    } else {
      mainWindow.webContents.send('tcp-message', message);
    }
  });

  tcpClient.on('close', () => {
    console.log('TCP connection closed');
    mainWindow.webContents.send('connection-status', 'disconnected');
  });

  tcpClient.on('error', (err) => {
    console.error('Error during TCP connection:', err);
    mainWindow.webContents.send('connection-status', 'error');
  });
}

function disconnectFromServer() {
  if (tcpClient) {
    tcpClient.end();
    tcpClient.destroy();
    tcpClient = null;
    console.log('Disconnected from TCP server');
    mainWindow.webContents.send('connection-status', 'disconnected');
  }
}

app.on('ready', () => {
  mainWindow = new BrowserWindow({
    width: 1000,
    height: 600,
    autoHideMenuBar: true,
    frame: false,
    alwaysOnTop: true,
    transparent: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      enableRemoteModule: false,
      nodeIntegration: false,
    }
  });

  mainWindow.loadFile('index.html');

  ipcMain.on('connect-to-server', (ev, host, port) => {
    console.log(port);
    connectToServer(host, port);
  });
  ipcMain.on('disconnect-from-server', disconnectFromServer);

  ipcMain.on('request-file-transfer', (event, { sourcePath, destinationPath }) => {
    console.log("Received sourcePath: " + sourcePath);
    console.log("Received destinationPath: " + destinationPath);

    if (tcpClient) {
      if (typeof sourcePath === 'string' && typeof destinationPath === 'string') {
        const command = `lb{0x0005} ${sourcePath} ${destinationPath}`;
        tcpClient.write(command);

        mainWindow.webContents.send('tcp-message', `Preparing to save file to ${destinationPath}`);
        const fileStream = fs.createWriteStream(destinationPath);

        tcpClient.on('data', (chunk) => {
          if (chunk.toString().endsWith('END_FILE_TRANSFER')) {
            fileStream.end();
            mainWindow.webContents.send('tcp-message', 'File transfer completed');
            showNotification('File Transfer', `File saved to ${destinationPath}`);
          } else {
            fileStream.write(chunk);
          }
        });
      } else {
        console.error('Invalid source or destination path.');
      }
    }
  });

  ipcMain.on('close-me', (evt, arg) => {
    app.quit();
  });

  ipcMain.on('send-message', (event, message) => {
    if (tcpClient) {
      tcpClient.write(message);
    }
  });
});
