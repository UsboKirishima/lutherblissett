const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electron', {
  connectToServer: (host, port) => ipcRenderer.send('connect-to-server', host, port),
  disconnectFromServer: () => ipcRenderer.send('disconnect-from-server'),
  sendMessage: (message) => ipcRenderer.send('send-message', message),
  onTcpMessage: (callback) => ipcRenderer.on('tcp-message', (event, message) => callback(message)),
  onConnectionStatus: (callback) => ipcRenderer.on('connection-status', (event, status) => callback(status)),
  quit: () => ipcRenderer.send('close-me'),
  requestFileTransfer: (sourcePath, destinationPath) => {
    ipcRenderer.send('request-file-transfer', { sourcePath, destinationPath });
  },
});
