const { webFrame } = require('electron');
const snek = document.getElementById('snek');
const counter = document.getElementById('boops');

webFrame.setZoomLevelLimits(1, 1);

window.boops = 0;
function boop() {
  window.boops++;
  counter.innerHTML = `${window.boops} BOOPS`;
}

snek.onmousedown = () => {
  snek.style['font-size'] = '550%';
  boop();
};

snek.onmouseup = () => {
  snek.style['font-size'] = '500%';
};
