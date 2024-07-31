const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electron', {
  sendMessage: (message) => ipcRenderer.send('send-message', message),
  onTcpMessage: (callback) => ipcRenderer.on('tcp-message', (event, message) => callback(message)),
  quit: () => ipcRenderer.send('close-me'),
  closeConnection: () => ipcRenderer.send('close')
});
