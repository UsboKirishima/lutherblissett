
const sendButton = document.getElementById('send-button');
const messageInput = document.getElementById('message-input');
const messagesDiv = document.getElementById('messages');

const closeButton = document.getElementById('close-button');
const connButton = document.getElementById('connect-disconnect-button');
const connStatus = document.getElementById('conn-status');

const homeIndex = document.getElementById('home');
const infoIndex = document.getElementById('info');
const exploitsIndex = document.getElementById('exploits');
const shellIndex = document.getElementById('shell');
const settingsIndex = document.getElementById('settings');

const homeButton = document.getElementById('homeButton');
const infoButton = document.getElementById('infoButton');
const exploitsButton = document.getElementById('exploitsButton');
const shellButton = document.getElementById('shellButton');
const settingsButton = document.getElementById('settingsButton');

const hostInput = document.getElementById('host-input');
const portInput = document.getElementById('port-input');

const errorElm = document.getElementById('error')

function showPage(page) {
    homeIndex.classList.remove('active');
    infoIndex.classList.remove('active');
    exploitsIndex.classList.remove('active');
    shellIndex.classList.remove('active');
    settingsIndex.classList.remove('active');

    if (page === 'home') {
        homeIndex.classList.add('active');
    } else {
        shellIndex.classList.add('active');
    }
}

function newError(error) {
    if(!error) {
        errorElm.innerHTML = '';
        errorElm.style.display = 'none'
    } else {
        errorElm.style.display = 'flex'
        errorElm.innerHTML = `<svg xmlns="http://www.w3.org/2000/svg" height="18x" viewBox="0 -960 960 960" width="18px" fill="currentColor"><path="M480-280q17 0 28.5-11.5T520-320q0-17-11.5-28.5T480-360q-17 0-28.5 11.5T440-320q0 17 11.5 28.5T480-280Zm-40-160h80v-240h-80v240Zm40 360q-83 0-156-31.5T197-197q-54-54-85.5-127T80-480q0-83 31.5-156T197-763q54-54 127-85.5T480-880q83 0 156 31.5T763-763q54 54 85.5 127T880-480q0 83-31.5 156T763-197q-54 54-127 85.5T480-80Zm0-80q134 0 227-93t93-227q0-134-93-227t-227-93q-134 0-227 93t-93 227q0 134 93 227t227 93Zm0-320Z" /></svg>${error}`
    }
}

homeButton.addEventListener('click', () => showPage('home'))
infoButton.addEventListener('click', () => showPage('info'))
exploitsButton.addEventListener('click', () => showPage('exploits'))
shellButton.addEventListener('click', () => showPage('shell'))
settingsButton.addEventListener('click', () => showPage('settings'))

let tcpClient;

connButton.addEventListener('click', () => {
    newError();
    if (connButton.innerHTML.includes("disconnect")) {
        window.electron.disconnectFromServer();
    }
    else {
        if (hostInput.value == '' || portInput.value == '') {
            newError("Missing HOST or PORT"); 
        } else {
            window.electron.connectToServer(hostInput.value, portInput.value);
        }
    }
})

window.electron.onConnectionStatus((status) => {
    if (status === 'connected') {
        connButton.innerHTML = "disconnect";
        connStatus.innerHTML = `<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="currentColor"><path d="M196-276q-57-60-86.5-133T80-560q0-78 29.5-151T196-844l48 48q-48 48-72 110.5T148-560q0 63 24 125.5T244-324l-48 48Zm96-96q-39-39-59.5-88T212-560q0-51 20.5-100t59.5-88l48 48q-30 27-45 64t-15 76q0 36 15 73t45 67l-48 48ZM280-80l135-405q-16-14-25.5-33t-9.5-42q0-42 29-71t71-29q42 0 71 29t29 71q0 23-9.5 42T545-485L680-80h-80l-26-80H387l-27 80h-80Zm133-160h134l-67-200-67 200Zm255-132-48-48q30-27 45-64t15-76q0-36-15-73t-45-67l48-48q39 39 58 88t22 100q0 51-20.5 100T668-372Zm96 96-48-48q48-48 72-110.5T812-560q0-63-24-125.5T716-796l48-48q57 60 86.5 133T880-560q0 78-28 151t-88 133Z"/></svg>CONNECTED`
        connStatus.style.color = "green";
        connButton.style.background = "red"
    } else {
        connButton.innerHTML = "connect";
        connStatus.innerHTML = `<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="currentColor"><path d="m684-389-49-49q22-26 33.5-57t11.5-65q0-40-16-76t-44-64l48-48q38 38 59 86t21 102q0 48-17 91.5T684-389ZM565-508 428-645q12-7 25-11t27-4q42 0 71 29t29 71q0 14-4 27t-11 25Zm215 214-48-48q40-45 60-101.5T812-560q0-66-24.5-127.5T716-796l48-48q55 58 85.5 131T880-560q0 74-25.5 142.5T780-294Zm11 238L520-327v207h-80v-287L280-566v6q0 40 16 76t44 64l-48 48q-38-38-59-86t-21-102q0-17 2-33t7-33l-51-51q-11 29-16.5 58t-5.5 59q0 66 24.5 127.5T244-324l-48 48q-55-58-85.5-131T80-560q0-44 9.5-86.5T118-729l-62-62 56-57 736 736-57 56Z"/></svg>DISCONNECTED`
        connStatus.style.color = "red";
        connButton.style.background = "green"
    }
});

closeButton.addEventListener('click', () => {
    window.electron.disconnectFromServer();
    window.electron.quit();
});

sendButton.addEventListener('click', () => {
    const message = messageInput.value;
    if (message.trim()) {
        window.electron.sendMessage(message + ' 2>&1');
        messageInput.value = '';
    }
});


window.electron.onTcpMessage((message) => {
    messagesDiv.innerHTML = '';
    const messageElement = document.createElement('div');
    messageElement.textContent = `${message}`;
    messagesDiv.appendChild(messageElement);
    messagesDiv.scrollTop = messagesDiv.scrollHeight;
});