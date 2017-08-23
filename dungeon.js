class Dungeon {
  constructor(width, height) {
    this.width = width;
    this.height = height;
    this.reset();
  }

  reset() {
    this.region = 1;
    this.stack = [];
    this.connectors = [];
    this.cells = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.cells[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.setCell(x, y, 0);
      }
    }
  }

  generateAll() {
    this.reset();
    this.placeRooms();
    while (this.step()) 1;
    while (this.connectRegions()) 1;
    while (this.cleanDeadEnds()) 1;
    this.placeTreasures();
    this.placeStairs();
  }

  randomOdd(min, max) {
    if (min % 2 == 0) ++min;
    if (max % 2 == 0) --max;
    return 2 * Math.floor(Math.random() * (max - min) / 2) + min;
  }

  get cellSize() {
    return 12;
  }

  get straightness() {
    return 0.75;
  }

  get bonusDoorChance() {
    return 0.02;
  }

  get roomAttempts() {
    return 100;
  }

  get treasureCount() {
    return 15;
  }

  findTile(tile) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        if (this.getCell(x, y) == tile) return [x, y];
      }
    }

    return [];
  }

  setCell(x, y, value) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    this.cells[y][x] = value;
  }

  getCell(x, y) {
    if (x < 0 || x >= this.width) return -1;
    if (y < 0 || y >= this.height) return -1;
    return this.cells[y][x];
  }

  findOpenSpace() {
    for (var y = 1; y < this.height; y += 2) {
      for (var x = 1; x < this.width; x += 2) {
        if (this.getCell(x, y) == 0) {
          this.setCell(x, y, this.region);
          return [x, y];
        }
      }
    }
    return [];
  }

  step() {
    var pos;
    if (this.stack.length == 0) {
      this.region++;
      pos = this.findOpenSpace();
    } else {
      pos = this.pop();
    }

    if (pos.length == 0) return false;

    var x = pos[0];
    var y = pos[1];

    var dirs = [];
    if (this.getCell(x, y - 2) == 0) dirs.push('n');
    if (this.getCell(x - 2, y) == 0) dirs.push('w');
    if (this.getCell(x, y + 2) == 0) dirs.push('s');
    if (this.getCell(x + 2, y) == 0) dirs.push('e');

    if (dirs.length > 1) this.push(x, y);

    if (dirs.length > 0) {
      var dir = '';
      if (dirs.includes(this.lastDir) && Math.random() < this.straightness) {
        dir = this.lastDir;
      } else {
        dir = dirs[Math.floor(Math.random() * dirs.length)];
        this.lastDir = dir;
      }

      if (dir == 'n') {
        this.setCell(x, y - 1, this.region);
        this.setCell(x, y - 2, this.region);
        this.push(x, y - 2);
      } else if (dir == 'w') {
        this.setCell(x - 1, y, this.region);
        this.setCell(x - 2, y, this.region);
        this.push(x - 2, y);
      } else if (dir == 's') {
        this.setCell(x, y + 1, this.region);
        this.setCell(x, y + 2, this.region);
        this.push(x, y + 2);
      } else if (dir == 'e') {
        this.setCell(x + 1, y, this.region);
        this.setCell(x + 2, y, this.region);
        this.push(x + 2, y);
      }
    }

    return true;
  }

  placeRooms() {
    for (var i = 0; i < this.roomAttempts; ++i) this.placeRoom();
  }

  placeRoom() {
    var x = this.randomOdd(1, this.width);
    var y = this.randomOdd(1, this.height);

    var size = this.randomOdd(3, 13);
    var h = size;
    var w = size;

    var increase = this.randomOdd(1, 3) + 1;
    Math.random() < 0.5 ? h += increase : w += increase;

    if (x + w >= this.width) x -= w - 1;
    if (y + h >= this.height) y -= h - 1;

    for (var iy = 0; iy < h; ++iy) {
      for (var ix = 0; ix < w; ++ix) {
        if (this.getCell(x + ix, y + iy) != 0) return false;
      }
    }

    for (var iy = 0; iy < h; ++iy) {
      for (var ix = 0; ix < w; ++ix) {
        this.setCell(x + ix, y + iy, this.region);
      }
    }
    this.region++;
  }

  isConnector(x, y) {
    if (this.getCell(x, y) != 0) return 0;

    var addIfNew = function(a, v) {
      if (v > 0 && !a.includes(v)) a.push(v);
    };

    var near = [];
    addIfNew(near, this.getCell(x, y - 1));
    addIfNew(near, this.getCell(x - 1, y));
    addIfNew(near, this.getCell(x, y + 1));
    addIfNew(near, this.getCell(x + 1, y));

    if (near.includes(1) && near.length > 1) {
      return near[0] == 1 ? near[1] : near[0];
    }
    return 0;
  }

  replace(from, to) {
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        if (this.getCell(x, y) == from) this.setCell(x, y, to);
      }
    }
  }

  connectRegions() {
    var connectors = [];
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        var other = this.isConnector(x, y);
        if (other > 0) connectors.push([x, y, other]);
      }
    }

    if (connectors.length == 0) return false;

    var i = Math.floor(Math.random() * connectors.length);
    var door = connectors[i];

    this.replace(door[2], 1);
    this.setCell(door[0], door[1], -2);

    for (var i = 0; i < connectors.length; ++i) {
      if (connectors[i][2] == door[2] && Math.random() < this.bonusDoorChance) {
        this.setCell(connectors[i][0], connectors[i][1], -2);
      }
    }

    return true;
  }

  isDeadEnd(x, y) {
    if (this.getCell(x, y) == 0) return false;

    var walls = 0;
    if (this.getCell(x, y - 1) == 0) ++walls;
    if (this.getCell(x - 1, y) == 0) ++walls;
    if (this.getCell(x, y + 1) == 0) ++walls;
    if (this.getCell(x + 1, y) == 0) ++walls;

    return walls == 3;
  }

  cleanDeadEnds() {
    var remove = [];
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        if (this.isDeadEnd(x, y)) remove.push([x, y]);
      }
    }

    for (var i = 0; i < remove.length; ++i) {
      this.setCell(remove[i][0], remove[i][1], 0);
    }

    return remove.length > 0;
  }

  isInRoom(x, y) {
    if (this.getCell(x, y) != 1) return false;

    var open = 0;
    if (this.getCell(x, y - 1) == 1) ++open;
    if (this.getCell(x - 1, y) == 1) ++open;
    if (this.getCell(x, y + 1) == 1) ++open;
    if (this.getCell(x + 1, y) == 1) ++open;

    return open == 4
  }

  placeInRoom(tile) {
    while (true) {
      var x = Math.floor(Math.random() * this.width - 2) + 1;
      var y = Math.floor(Math.random() * this.height - 2) + 1;
      if (this.isInRoom(x, y)) {
        this.setCell(x, y, tile);
        break;
      }
    }
  }

  placeTreasures() {
    for (var i = 0; i < this.treasureCount; ++i) {
      this.placeInRoom(-3);
    }
  }

  placeStairs() {
    this.placeInRoom(-4);
    this.placeInRoom(-5);
  }

  draw(canvas) {
    var ctx = canvas.getContext('2d');
    var size = this.cellSize;

    canvas.setAttribute('width', this.width * size - 1);
    canvas.setAttribute('height', this.height * size - 1);

    ctx.fillStyle = '#fff';
    ctx.fillRect(0, 0, this.width * size - 1, this.height * size - 1);

    ctx.strokeStyle = '#333';
    ctx.fillStyle = '#000';

    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        ctx.fillStyle = this.color(this.getCell(x, y));
        ctx.fillRect(x * size, y * size, size, size);

        if (size >= 4) ctx.strokeRect(x * size, y * size, size, size);

        if (this.isConnector(x, y) > 0) {
          var inset = 4;
          ctx.fillStyle = '#ff0';
          ctx.fillRect(x * size + inset, y * size + inset, size - inset * 2, size - inset * 2);
        }
      }
    }
  }

  color(region) {
    switch (region) {
      case 0: return '#000';
      case 1: return '#fff';

      case -2: return '#840';
      case -3: return '#ff0';
      case -4: return '#800';
      case -5: return '#080';
    }

    var b = (region % 4) * 64 + 32;
    var g = ((region / 4) % 4) * 64 + 32;
    var r = ((region / 16) % 4) * 64 + 32;

    return 'rgb(' + r + ',' + g + ',' + b + ')';
  }

  push(x, y) {
    this.stack.push([x, y]);
  }

  pop() {
    return this.stack.length > 0 ? this.stack.pop() : [];
  }
}
