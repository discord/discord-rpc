const binding = require('bindings')('discord-rpc');

let interval;

function intervalLoop() {
  binding.runCallbacks();
}

const constants = {
  REPLY_YES: binding.DISCORD_REPLY_YES,
  REPLY_NO: binding.DISCORD_REPLY_NO,
  REPLY_IGNORE: binding.DISCORD_REPLY_IGNORE,
};

module.exports = {
  constants,
  initialize(id, {
    handlers = {},
    register = true,
    steam = '',
    runCallbacks = true,
  } = {}) {
    binding.initialize(id, handlers, register, steam);
    if (runCallbacks)
      setInterval(intervalLoop, 500);
    return true;
  },
  shutdown() {
    if (interval)
      clearInterval(interval);
    return binding.shutdown();
  },
  runCallbacks: binding.runCallbacks,
  updatePresence(presence) {
    if (presence.startTimestamp instanceof Date)
      presence.startTimestamp = Math.round(presence.startTimestamp.getTime() / 1000);
    if (presence.endTimestamp instanceof Date)
      presence.endTimestamp = Math.round(presence.endTimestamp.getTime() / 1000);

    return binding.updatePresence(presence);
  },
  respond(id, reason) {
    if (typeof reason === 'string') {
      reason = reason.toUpperCase();
      if (reason in constants)
        reason = constants[reason];
      else if (`REPLY_${reason}` in constants)
        reason = constants[`REPLY_${reason}`];
    }
    return binding.respond(id, reason);
  },
  register(id, command = '') {
    return binding.register(id, command);
  },
  registerSteamGame: binding.registerSteamGame,
};
