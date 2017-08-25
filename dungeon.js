class Dungeon {
  constructor(width, height) {
    this.width = width;
    this.height = height;
    this.reset();

    var t = ['wall', 'room', 'hall', 'door', 'open', 'up', 'down', 'treasure'];
    this.imgs = {};
    for (var i = 0; i < t.length; ++i) {
      this.imgs[t[i]] = new Image();
      this.imgs[t[i]].src = t[i] + '.png';
    }
  }

  reset() {
    this.region = 1;
    this.stack = [];
    this.cells = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.cells[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.setCell(x, y, 'wall', 0);
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
        if (this.getCell(x, y).tile == tile) return { x: x, y: y };
      }
    }

    return [];
  }

  setCell(x, y, tile, region) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    this.cells[y][x] = { tile: tile, region: region };
  }

  getCell(x, y) {
    if (x < 0 || x >= this.width) return { tile: 'oob', region: 0 };
    if (y < 0 || y >= this.height) return { tile: 'oob', region: 0 };
    return this.cells[y][x];
  }

  findOpenSpace() {
    for (var y = 1; y < this.height; y += 2) {
      for (var x = 1; x < this.width; x += 2) {
        if (this.getCell(x, y).tile == 'wall') {
          this.setCell(x, y, 'hall', this.region);
          return { x: x, y: y };
        }
      }
    }
    return null;
  }

  step() {
    var pos;
    if (this.stack.length == 0) {
      this.region++;
      pos = this.findOpenSpace();
    } else {
      pos = this.pop();
    }

    if (pos == null) return false;

    var dirs = [];
    if (this.getCell(pos.x, pos.y - 2).tile == 'wall') dirs.push('n');
    if (this.getCell(pos.x - 2, pos.y).tile == 'wall') dirs.push('w');
    if (this.getCell(pos.x, pos.y + 2).tile == 'wall') dirs.push('s');
    if (this.getCell(pos.x + 2, pos.y).tile == 'wall') dirs.push('e');

    if (dirs.length > 1) this.push(pos.x, pos.y);

    if (dirs.length > 0) {
      var dir = '';
      if (dirs.includes(this.lastDir) && Math.random() < this.straightness) {
        dir = this.lastDir;
      } else {
        dir = dirs[Math.floor(Math.random() * dirs.length)];
        this.lastDir = dir;
      }

      if (dir == 'n') {
        this.setCell(pos.x, pos.y - 1, 'hall', this.region);
        this.setCell(pos.x, pos.y - 2, 'hall', this.region);
        this.push(pos.x, pos.y - 2);
      } else if (dir == 'w') {
        this.setCell(pos.x - 1, pos.y, 'hall', this.region);
        this.setCell(pos.x - 2, pos.y, 'hall', this.region);
        this.push(pos.x - 2, pos.y);
      } else if (dir == 's') {
        this.setCell(pos.x, pos.y + 1, 'hall', this.region);
        this.setCell(pos.x, pos.y + 2, 'hall', this.region);
        this.push(pos.x, pos.y + 2);
      } else if (dir == 'e') {
        this.setCell(pos.x + 1, pos.y, 'hall', this.region);
        this.setCell(pos.x + 2, pos.y, 'hall', this.region);
        this.push(pos.x + 2, pos.y);
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
        if (this.getCell(x + ix, y + iy).tile != 'wall') return false;
      }
    }

    for (var iy = 0; iy < h; ++iy) {
      for (var ix = 0; ix < w; ++ix) {
        this.setCell(x + ix, y + iy, 'room', this.region);
      }
    }
    this.region++;
  }

  isConnector(x, y) {
    if (this.getCell(x, y).tile != 'wall') return 0;

    var addIfNew = function(a, v) {
      if (v > 0 && !a.includes(v)) a.push(v);
    };

    var near = [];
    addIfNew(near, this.getCell(x, y - 1).region);
    addIfNew(near, this.getCell(x - 1, y).region);
    addIfNew(near, this.getCell(x, y + 1).region);
    addIfNew(near, this.getCell(x + 1, y).region);

    if (near.includes(1) && near.length > 1) {
      return near[0] == 1 ? near[1] : near[0];
    }
    return 0;
  }

  replaceRegion(from, to) {
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        var c = this.getCell(x, y);
        if (c.region == from) this.setCell(x, y, c.tile, to);
      }
    }
  }

  connectRegions() {
    var connectors = [];
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        var other = this.isConnector(x, y);
        if (other > 0) connectors.push({ x: x, y: y, region: other })
      }
    }

    if (connectors.length == 0) return false;

    var i = Math.floor(Math.random() * connectors.length);
    var door = connectors[i];

    this.replaceRegion(door.region, 1);
    this.setCell(door.x, door.y, 'door', 1);

    for (var i = 0; i < connectors.length; ++i) {
      if (connectors[i].region != door.region) continue;
      if (this.adjacentCount(door.x, door.y, 'door') > 0) continue;
      if (Math.random() < this.bonusDoorChance) {
        this.setCell(connectors[i].x, connectors[i].y, 'door', 1);
      }
    }

    return true;
  }

  adjacentCount(x, y, tile) {
    var n = 0;
    if (this.getCell(x, y - 1).tile == tile) ++n;
    if (this.getCell(x - 1, y).tile == tile) ++n;
    if (this.getCell(x, y + 1).tile == tile) ++n;
    if (this.getCell(x + 1, y).tile == tile) ++n;
    return n;
  }

  isDeadEnd(x, y) {
    return this.getCell(x, y).tile != 'wall' && this.adjacentCount(x, y, 'wall') == 3;
  }

  cleanDeadEnds() {
    var remove = [];
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        if (this.isDeadEnd(x, y)) remove.push({ x: x, y: y });
      }
    }

    for (var i = 0; i < remove.length; ++i) {
      this.setCell(remove[i].x, remove[i].y, 'wall', 0);
    }

    return remove.length > 0;
  }

  isInRoom(x, y) {
    return this.getCell(x, y).tile == 'room' && this.adjacentCount(x, y, 'room') == 4;
  }

  placeInRoom(tile) {
    while (true) {
      var x = Math.floor(Math.random() * this.width - 2) + 1;
      var y = Math.floor(Math.random() * this.height - 2) + 1;
      if (this.isInRoom(x, y)) {
        this.setCell(x, y, tile, 1);
        break;
      }
    }
  }

  placeTreasures() {
    for (var i = 0; i < this.treasureCount; ++i) {
      this.placeInRoom('treasure');
    }
  }

  placeStairs() {
    this.placeInRoom('down');
    this.placeInRoom('up');
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

  drawSprites(canvas) {
    var ctx = canvas.getContext('2d');
    var size = this.cellSize;

    canvas.setAttribute('width', this.width * size - 1);
    canvas.setAttribute('height', this.height * size - 1);

    ctx.fillStyle = '#fff';
    ctx.fillRect(0, 0, this.width * size - 1, this.height * size - 1);

    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        var t = this.getCell(x, y).tile;
        ctx.drawImage(this.imgs[t], x * size, y * size);
      }
    }
  }

  color(cell) {
    switch (cell.tile) {
      case 'wall': return '#000';
      case 'door': return '#840';
      case 'open': return '#fed';
      case 'treasure': return '#ff0';
      case 'down': return '#800';
      case 'up': return '#080';
    }

    if (cell.region > 1) {
      var b = (cell.region % 4) * 64 + 32;
      var g = ((cell.region / 4) % 4) * 64 + 32;
      var r = ((cell.region / 16) % 4) * 64 + 32;
      return 'rgb(' + r + ',' + g + ',' + b + ')';
    }
    
    return cell.tile == 'room' ? '#fff' : '#bbb';
  }

  push(x, y) {
    this.stack.push({ x: x, y: y });
  }

  pop() {
    return this.stack.length > 0 ? this.stack.pop() : [];
  }
}
