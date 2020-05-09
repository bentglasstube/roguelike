class Game {
  constructor() {
    this.player = new Player();
    this.overworld = new Overworld(16 * 16, 12 * 12);

    const params = {
      detail: 12,
      relax: 2,
      seed: 42069,
      baseFreq: 2,
      octaves: 8,
      gradientOffset: 10,
      gradientScale: 10,
    };

    this.overworld.generateVoronoi(params);
    this.overworld.generateTerrain(params);

    const px = Math.floor(this.overworld.width / 2) + 8;
    for (var py = 0; py < this.overworld.height; ++py) {
      const pt = this.overworld.getCell(px, py).tile;
      if (pt == 'water') {
        this.player.x = px;
        this.player.y = py - 1;
        break;
      }
    }
  }

  draw(canvas) {
    this.overworld.drawScreen(this.player.x, this.player.y, canvas);
    this.player.draw(canvas);
  }

  movePlayer(dx, dy) {
    var nx = this.player.x + dx;
    var ny = this.player.y + dy;

    var t = this.overworld.getCell(nx, ny).tile;
    switch (t) {
      case 'water':
      case 'rock':
      case 'tree':
      case 'oob':
        return false;

      case 'grass':
      case 'sand':
        this.player.x = nx;
        this.player.y = ny;
        return 'true';

      default:
        console.log('Unknown tile: ' + t);
        return 'false';
    }

  }

  update() {
    // nothing
  }
}

var canvas = document.getElementById('c');
var game = new Game();

setTimeout(function() {
  game.update();
  game.draw(canvas);
}, 250);

function update() {
  game.update();
  game.draw(canvas);
}

document.addEventListener('gamepadconnected', function(e) {
  console.log("Gamepad connected: %s", e.gamepad.id);
});

document.addEventListener('gamepaddisconnected', function(e) {
  console.log("Gamepad disconnected: %s", e.gamepad.id);
});

document.addEventListener('keydown', function(e) {
  switch (e.key) {
    case 'ArrowUp':
    case 'w':
      game.movePlayer(0, -1);
      e.preventDefault();
      break;

    case 'ArrowLeft':
    case 'a':
      game.movePlayer(-1, 0);
      e.preventDefault();
      break;

    case 'ArrowDown':
    case 's':
      game.movePlayer(0, 1);
      e.preventDefault();
      break;

    case 'ArrowRight':
    case 'd':
      game.movePlayer(1, 0);
      e.preventDefault();
      break;
  }

  update();
});

setInterval(function() {
  var dx = 0;
  var dy = 0;

  var gamepads = navigator.getGamepads();

  for (var i = 0; i < gamepads.length; ++i) {
    const g = gamepads[i];
    if (g) {
      if (g.axes[0] > 0.5) dx = 1;
      if (g.axes[0] < -0.5) dx = -1;
      if (g.axes[1] > 0.5) dy = 1;
      if (g.axes[1] < -0.5) dy = -1;
    }
  }

  if (dx != 0 || dy != 0) {
    game.movePlayer(dx, dy);
    update();
  }
}, 100);
