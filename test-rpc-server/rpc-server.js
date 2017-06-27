const net = require('net');
const RpcMessage = require('./rpc-message');

console.log('Start up');

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

var server = net.createServer(function(stream) {
    console.log('Server: on connection');

    stream.on('data', function(data) {
        const msgObj = RpcMessage.deserialize(data);
        if (msgObj != null) {
            console.log('Server: on data:', msgObj);
        }
        else {
            console.log('Server: got some data');
        }
    });

    stream.on('end', function() {
        console.log('Server: on end')
        server.close();
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