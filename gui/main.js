const { app, BrowserWindow, ipcMain, Notification } = require('electron');
const path = require('path');
const net = require('net');
const fs = require('fs');

let mainWindow;
let tcpClient;

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
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      enableRemoteModule: false,
      nodeIntegration: false,

    }
  });

  mainWindow.loadFile('index.html');



  ipcMain.on('connect-to-server', (ev, host, port) => {
    console.log(port)
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
    app.quit()
  })

  ipcMain.on('send-message', (event, message) => {
    if (tcpClient) {
      tcpClient.write(message);
    }
  });


});
