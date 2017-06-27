
module.exports = class RpcMessage {
    static serialize(obj) {
        const serializedJson = JSON.stringify(obj);
        const msgLen = 4 + serializedJson.length;
        let buff = Buffer.alloc(msgLen);
        buff.writeInt32LE(msgLen, 0);
        buff.write(serializedJson, 4, serializedJson.length, 'utf-8');
        return buff;
    }

    static deserialize(buff) {
        const msgLen = buff.readInt32LE(0);
        if (buff.length < msgLen) {
            return null;
        }
        const msg = buff.toString('utf-8', 4, msgLen);
        try {
            return JSON.parse(msg);
        } catch(e) {
            return null;
        }
    }
};
