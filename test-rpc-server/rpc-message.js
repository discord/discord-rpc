const path = require('path');

const VERSION = 1;

const OPCODES = {
  HANDSHAKE: 0,
  FRAME: 1,
  CLOSE: 2,
};

let PipePath;
if (process.platform == 'win32') {
    PipePath = '\\\\?\\pipe\\discord-ipc';
}
else {
    const temp = process.env.XDG_RUNTIME_DIR || process.env.TMPDIR || process.env.TMP || process.env.TEMP || '/tmp';
    PipePath = path.join(temp, 'discord-ipc');
}

class RpcMessage {

    static serialize(opcode, obj) {
        const serializedJson = JSON.stringify(obj);
        const msgLen = serializedJson.length;
        let buff = Buffer.alloc(8 + msgLen);
        buff.writeInt32LE(opcode, 0);
        buff.writeInt32LE(msgLen, 4);
        buff.write(serializedJson, 8, serializedJson.length, 'utf-8');
        return buff;
    }

    static handshake(id) {
        const opcode = OPCODES.HANDSHAKE;
        return RpcMessage.serialize(opcode, {
            client_id: id,
            v: VERSION
        });
    }

    static send(obj) {
        const opcode = OPCODES.FRAME;
        return RpcMessage.serialize(opcode, obj);
    }

    static sendClose(code, message) {
        const opcode = OPCODES.CLOSE;
        return RpcMessage.serialize(opcode, {code, message});
    }

    static deserialize(buff) {
        const opcode = buff.readInt32LE(0);
        const msgLen = buff.readInt32LE(4);
        if (buff.length < (msgLen + 8)) {
            return null;
        }
        const msg = buff.toString('utf-8', 8, msgLen + 8);
        try {
            return {opcode, data: JSON.parse(msg)};
        } catch(e) {
            console.log(`failed to parse "${msg}"`);
            console.error(e);
            return {opcode: OPCODES.CLOSE, message: e.message};
        }
    }
};

module.exports = {OPCODES, PipePath, RpcMessage};
