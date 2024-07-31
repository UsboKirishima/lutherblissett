const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const net = require('net');

let mainWindow;
let tcpClient;

function connectToServer(host, port) {
  tcpClient = new net.Socket();
  const TCP_PORT = Number(port);

  tcpClient.connect(TCP_PORT, host, () => {
    console.log('Connected to TCP server');
    mainWindow.webContents.send('connection-status', 'connected');
  });

  tcpClient.on('data', (data) => {
    console.log('Message received from the server TCP:', data.toString());
    mainWindow.webContents.send('tcp-message', data.toString());
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

  ipcMain.on('close-me', (evt, arg) => {
    app.quit()
  })

  ipcMain.on('send-message', (event, message) => {
    if (tcpClient) {
      tcpClient.write(message);
    }
  });
});
