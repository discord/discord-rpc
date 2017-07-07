const net = require('net');
const {OPCODES, PipePath, RpcMessage} = require('./rpc-message');

const APP_ID = '12345678910';
global.isConnected = false;
global.timeoutId = null;

function sendMesg(testUpdatesToSend, stream) {
    const msgObj = {
        state: (testUpdatesToSend % 2 == 0) ? 'In a match' : 'In Lobby',
        details: 'Excited'
    };
    console.log('Client: send update:', msgObj);
    stream.write(RpcMessage.send(msgObj));
}

function sendMessageLoop(testUpdatesToSend, interval, stream) {
    global.timeoutId = null;
    if (!global.isConnected) {
        return;
    }
    sendMesg(testUpdatesToSend, stream);
    if (testUpdatesToSend > 1) {
        global.timeoutId = setTimeout(() => {sendMessageLoop(testUpdatesToSend - 1, interval, stream)}, interval);
    } else {
        shutdown();
    }
}

const client = net.connect(PipePath, function(stream) {
    console.log('Client: on connection');
    global.isConnected = true;
    client.write(RpcMessage.handshake(APP_ID));
    sendMessageLoop(10, 3000, client);
});

client.on('data', function(data) {
    const msgObj = RpcMessage.deserialize(data);
    if (msgObj != null) {
        const {opcode, data} = msgObj;
        console.log(`Client: got opcode: ${opcode}, data: ${JSON.stringify(data)}`);

        if (opcode == OPCODES.CLOSE) {
            shutdown();
        }

    } else {
        console.log('Client: got some data', data);
    }
});

client.on('end', function() {
    global.isConnected = false;
    console.log('Client: on end');
});

function shutdown() {
    if (global.timeoutId !== null) {
        clearTimeout(global.timeoutId);
        global.timeoutId = null;
    }
    client.end();
}