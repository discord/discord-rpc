const net = require('net');
const repl = require('repl');
const {PipePath, RpcMessage} = require('./rpc-message');

let connectionNonce = 0;
global.connections = {};

const server = net.createServer(function(sock) {
    connectionNonce += 1;
    console.log('Server: on connection', connectionNonce);
    let myConnection = connectionNonce;
    let messages = 0;

    global.connections[myConnection] = sock;

    sock.on('data', function(data) {
        messages++;
        const msgObj = RpcMessage.deserialize(data);
        if (msgObj != null) {
            const {opcode, data} = msgObj;
            console.log(`\nServer (${myConnection}): got opcode: ${opcode}, data: ${JSON.stringify(data)}`);
        }
        else {
            console.log('\nServer: got some data', data.toString());
        }
    });
    
    sock.on('end', function() {
        delete global.connections[myConnection];
        console.log('\nServer: on end', myConnection);
    });
});

server.on('close', function(){
    console.log('\nServer: on close');
});

try {
    server.listen(PipePath, function(){
        console.log('\nServer: on listening');
    });
} catch(e) {
    console.error('\nServer: could not start:', e);
}

const replServer = repl.start({prompt: '> ', useGlobal: true, breakEvalOnSigint: true});
replServer.defineCommand('kill', {
  help: 'Kill a client',
  action(who) {
    this.bufferedCommand = '';
    who = parseInt(who, 10);
    const sock = global.connections[who];
    if (sock) {
        console.log('killing', who);
        sock.end(RpcMessage.sendClose(123, 'killed'));
    }
    this.displayPrompt();
  }
});

replServer.defineCommand('ping', {
  help: 'Ping all clients',
  action() {
    this.bufferedCommand = '';
    Object.keys(global.connections).forEach((who) => {
        const sock = global.connections[who];
        if (sock) {
            console.log('pinging', who);
            sock.write(RpcMessage.sendPing('hello'));
        }
    })
    this.displayPrompt();
  }
});

