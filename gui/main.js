const { app, BrowserWindow, ipcMain, Notification } = require('electron');
const path = require('path');
const net = require('net');
const fs = require('fs');

let mainWindow;
let tcpClient;
let imageWindow;
let canvasContext;
let imageBuffer = [];
let imageCanvas;

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
    } else {
      mainWindow.webContents.send('tcp-message', message);
    }
  });

  tcpClient.on('data', (data) => {
    const message = data.toString();

    if (message.startsWith('lb{0x0007}')) {
      if (!imageWindow) {
        imageWindow = new BrowserWindow({
          width: 500,
          height: 300,
          autoHideMenuBar: true,
          frame: true,
          alwaysOnTop: false,
          webPreferences: {
            contextIsolation: false,
            nodeIntegration: true,
          }
        });

        imageWindow.loadURL('data:text/html,<html><body><canvas id="screen" style="width:100%;height:100%;"></canvas></body></html>');

        imageWindow.webContents.on('did-finish-load', () => {
          imageWindow.webContents.executeJavaScript(`
            window.canvas = document.getElementById('screen');
            window.context = canvas.getContext('2d');
          `).then(() => {
            canvasContext = imageWindow.webContents;
          });
        });
      }

      imageBuffer = []; 
    } else if (message.startsWith('lb{0x0008}')) {
      console.log('Screen sharing ended');
      mainWindow.webContents.send('tcp-message', 'Screen sharing ended.');
      if (imageWindow) {
        imageWindow.close();
        imageWindow = null;
      }
      imageBuffer = [];
    } else {
      imageBuffer.push(message);

      const base64Image = imageBuffer.join('');
      if (canvasContext) {
        canvasContext.executeJavaScript(`
          const img = new Image();
          img.onload = function() {
            canvas.width = img.width;
            canvas.height = img.height;
            context.drawImage(img, 0, 0);
          };
          img.src = 'data:image/jpeg;base64,${base64Image}';
        `);
      }
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

app.whenReady().then(() => {
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
    connectToServer(host, port);
  });

  ipcMain.on('disconnect-from-server', disconnectFromServer);

  ipcMain.on('request-file-transfer', (event, { sourcePath, destinationPath }) => {
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

  ipcMain.on('close-me', () => {
    app.quit();
  });

  ipcMain.on('send-message', (event, message) => {
    if (tcpClient) {
      tcpClient.write(message);
    }
  });
});
