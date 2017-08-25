class Player {
  constructor() {
    this.x = 0;
    this.y = 0;
    this.gold = 0;

    this.img = new Image();
    this.img.src = 'hero.png';
  }

  draw(canvas) {
    var ctx = canvas.getContext('2d');
    // TODO put somewhere accessible
    var s = 12;
    ctx.drawImage(this.img, this.x * s, this.y * s);
  }
}

class Game {
  constructor() {
    this.message = '';
    this.floors = [];
    this.floor = 0;
    this.player = new Player();
    this.putPlayerOn('up');
  }

  getDungeonFloor(floor) {
    while (floor >= this.floors.length) {
      var d = new Dungeon(79, 39);
      d.generateAll();
      this.floors.push(d);
    }
    return this.floors[floor];
  }

  getCurrentFloor() {
    return this.getDungeonFloor(this.floor);
  }

  putPlayerOn(tile) {
    var pos = this.getCurrentFloor().findTile(tile);
    if (pos != null) {
      this.player.x = pos.x;
      this.player.y = pos.y;
    }
  }

  draw(canvas) {
    this.getCurrentFloor().draw(canvas);
    this.player.draw(canvas);
  }

  movePlayer(dx, dy) {
    var nx = this.player.x + dx;
    var ny = this.player.y + dy;

    var t = this.getCurrentFloor().getCell(nx, ny).tile;

    switch (t) {
      case 'wall':
        return false;

      case 'door':
        this.message = 'You opened the door.';
        this.getCurrentFloor().setCell(nx, ny, 'open', 1);
        return false;

      case 'treasure':
        var gold = Math.floor(Math.random() * 25 + 10);
        this.message = 'You picked up ' + gold + ' gold pieces.';
        this.getCurrentFloor().setCell(nx, ny, 'room', 1);
        this.player.gold += gold;
        break;

      case 'down':
        this.message = 'You walked down the stairs';
        this.floor++;
        this.putPlayerOn('up');
        return true;

      case 'up':
        if (this.floor == 0) {
          this.message = 'Those stairs lead outside, and who wants to go there?';
          break;
        } else {
          this.message = 'You walked up the stairs';
          this.floor--;
          this.putPlayerOn('down');
          return true;
        }
    }

    this.player.x = nx;
    this.player.y = ny;

    return true;
  }
}

var canvas = document.getElementById('c');
var game = new Game();

setTimeout(function() { game.draw(canvas); }, 100);

document.addEventListener('keydown', function(e) {
  switch (e.key) {
    case 'w':
      game.movePlayer(0, -1);
      break;

    case 'a':
      game.movePlayer(-1, 0);
      break;

    case 's':
      game.movePlayer(0, 1);
      break;

    case 'd':
      game.movePlayer(1, 0);
      break;

    default:
      console.log('unknown key: ' + e.key);
  }

  game.draw(canvas);
  document.getElementById('messages').innerHTML = game.message;
  document.getElementById('gold').innerHTML = game.player.gold;
});
