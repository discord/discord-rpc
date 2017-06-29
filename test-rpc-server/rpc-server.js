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
const PipePath = PipePrefix + 'DiscordRpcServer' + PipePostfix;
let connections = 0;

const server = net.createServer(function(sock) {
    connections += 1;
    console.log('Server: on connection', connections);
    let myConnection = connections;

    sock.on('data', function(data) {
        const msgObj = RpcMessage.deserialize(data);
        if (msgObj != null) {
            console.log('Server: on data:', myConnection, msgObj);
        }
        else {
            console.log('Server: got some data', data.toString());
        }
    });
    
    sock.on('end', function() {
        connections -= 1;
        console.log('Server: on end', connections);
    });
});

server.on('close', function(){
    console.log('Server: on close');
})

try {
    server.listen(PipePath, function(){
        console.log('Server: on listening');
    });
} catch(e) {
    console.error('could not start server:', e);
}
