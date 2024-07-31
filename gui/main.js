const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const net = require('net');

let mainWindow;
let tcpClient;

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

  // Configura il client TCP
  tcpClient = new net.Socket();

  const TCP_PORT = 8080;

  tcpClient.connect(TCP_PORT, '127.0.0.1', () => {
    console.log('Connesso al server TCP');
  });

  tcpClient.on('data', (data) => {
    console.log('Messaggio ricevuto dal server TCP:', data.toString());
    // Invia i dati ricevuti dal server TCP alla finestra
    mainWindow.webContents.send('tcp-message', data.toString());
  });

  tcpClient.on('close', () => {
    console.log('Connessione al server TCP chiusa');
  });

  tcpClient.on('error', (err) => {
    console.error('Errore nella connessione TCP:', err);
  });

  // Gestisci i messaggi inviati dalla finestra
  ipcMain.on('send-message', (event, message) => {
    if (tcpClient) {
      tcpClient.write(message);
    }
  });

  ipcMain.on('close-me', (evt, arg) => {
    app.quit()
  })
});
