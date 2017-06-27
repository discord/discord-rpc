const net = require('net');
const RpcMessage = require('./rpc-message');

let PipePrefix;
let PipePostfix;
if (process.platform == 'win32') {
    PipePrefix = '\\\\.\\pipe\\';
    PipePostfix = '';
}
else {
    PipePrefix = "/tmp";
    PipePostfix = '.pipe';
}

const PipePath = PipePrefix + "DiscordRpcServer" + PipePostfix;

function sendMesg(testUpdatesToSend, stream) {
    const msgObj = {
        name: 'My Awesome Game',
        state: (testUpdatesToSend % 2 == 0) ? 'In a match' : 'In Lobby'
    };
    console.log('Client: send update:', msgObj);
    stream.write(RpcMessage.serialize(msgObj));
}

function sendMessageLoop(testUpdatesToSend, interval, stream) {
    sendMesg(testUpdatesToSend, stream);
    if (testUpdatesToSend > 1) {
        setTimeout(() => {sendMessageLoop(testUpdatesToSend - 1, interval, stream)}, interval);
    } else {
        shutdown();
    }
}

const client = net.connect(PipePath, function(stream) {
    console.log('Client: on connection');

    sendMessageLoop(5, 3000, client);
});

client.on('data', function(data) {
    const msgObj = RpcMessage.deserialize(data);
    if (msgObj != null) {
        console.log('Client: got data:', msgObj);
    } else {
        console.log('Client: got some data');
    }
});

client.on('end', function() {
    console.log('Client: on end');
});

function shutdown() {
    client.end();
}