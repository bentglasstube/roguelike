var tileSize = 12;

class Shadow {
  constructor(start, end) {
    this.start = start;
    this.end = end;
  }

  contains(other) {
    return this.start <= other.start && this.end >= other.end;
  }
}

class ShadowLine {
  constructor() {
    this.shadows = [];
  }

  isShadowed(projection) {
    for (var i = 0; i < this.shadows.length; ++i) {
      if (this.shadows[i].contains(projection)) return true;
    }
    return false;
  }

  add(shadow) {
    var i;
    for (i = 0; i < this.shadows.length; ++i) {
      if (this.shadows[i].start >= shadow.start) break;
    }

    var prev = (i > 0 && this.shadows[i - 1].end > shadow.start) ? this.shadows[i - 1] : null;
    var next = (i < this.shadows.length && this.shadows[i].start < shadow.end) ? this.shadows[i] : null;

    if (next != null) {
      if (prev != null) {
        prev.end = next.end;
        this.shadows.splice(i, 1);
      } else {
        next.start = shadow.start;
      }
    } else {
      if (prev != null) {
        prev.end = shadow.end;
      } else {
        this.shadows.splice(i, 0, shadow);
      }
    }
  }
}

class Player {
  constructor() {
    this.x = 0;
    this.y = 0;
    this.gold = 0;
    this.keys = 0;

    this.img = new Image();
    this.img.src = 'hero.png';
  }

  draw(canvas) {
    var ctx = canvas.getContext('2d');
    // TODO put somewhere accessible
    ctx.drawImage(this.img, this.x * tileSize, this.y * tileSize);
  }
}

class Dungeon {
  constructor(width, height, params) {
    this.width = width;
    this.height = height;
    this.reset(params);
    this.sections = 2;

    var t = ['wall', 'room', 'hall', 'door', 'open', 'up', 'down', 'treasure', 'key', 'locked'];
    this.imgs = {};
    for (var i = 0; i < t.length; ++i) {
      this.imgs[t[i]] = new Image();
      this.imgs[t[i]].src = t[i] + '.png';
    }
  }

  reset(params) {
    this.params = params;
    this.region = 1;
    this.rooms = 0;
    this.stack = [];
    this.cells = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.cells[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.cells[y][x] = {
          tile: 'wall',
          region: 0,
          visible: false,
          seen: false,
        };
      }
    }
  }

  reveal() {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        this.setVisible(x, y, true);
      }
    }
  }

  setVisibilityFrom(vx, vy) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        this.setVisible(x, y, false);
      }
    }

    var transform = function(r, c, oct) {
      switch (oct) {
        case 0: return { c:  c, r: -r };
        case 1: return { c:  r, r: -c };
        case 2: return { c:  r, r:  c };
        case 3: return { c:  c, r:  r };
        case 4: return { c: -c, r:  r };
        case 5: return { c: -r, r:  c };
        case 6: return { c: -r, r: -c };
        case 7: return { c: -c, r: -r };
      }
    }

    this.setVisible(vx, vy, true);
    for (var oct = 0; oct < 8; ++oct) {
      var line = new ShadowLine();

      for (var r = 1; r < 9; ++r) {
        for (var c = 0; c <= r; ++c) {
          if (r * r + c * c >= 64) break;
          var projection = new Shadow(c / (r + 1), (c + 1) / r);
          var offset = transform(r, c, oct);
          var x = vx + offset.c;
          var y = vy + offset.r;
          var cell = this.getCell(x, y);
          var visible = !line.isShadowed(projection);

          this.setVisible(x, y, visible);
          if (visible && (cell.tile == 'wall' || cell.tile == 'door' || cell.tile == 'locked')) {
            line.add(projection);
          }
        }
      }
    }
  }

  generateAll(params) {
    this.reset(params);
    while (this.placeRoom()) 1;
    while (this.step()) 1;
    while (this.connectRegions()) 1;
    while (this.cleanDeadEnds()) 1;
  }

  randomOdd(min, max) {
    if (min % 2 == 0) ++min;
    if (max % 2 == 0) --max;
    return 2 * Math.floor(Math.random() * (max - min) / 2) + min;
  }

  findTile(tile) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        if (this.getCell(x, y).tile == tile) return { x: x, y: y };
      }
    }

    return [];
  }

  setCell(x, y, tile) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    this.cells[y][x].tile = tile;
  }

  setRegion(x, y, region) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    this.cells[y][x].region = region;
  }

  setVisible(x, y, visible) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    if (visible) this.cells[y][x].seen = true;
    this.cells[y][x].visible = visible;
  }

  getCell(x, y) {
    if (x < 0 || x >= this.width) return { tile: 'oob', region: 0 };
    if (y < 0 || y >= this.height) return { tile: 'oob', region: 0 };
    return this.cells[y][x];
  }

  carve(x, y, tile) {
    this.setCell(x, y, tile);
    this.setRegion(x, y, this.region);
  }

  findOpenSpace() {
    for (var y = 1; y < this.height; y += 2) {
      for (var x = 1; x < this.width; x += 2) {
        if (this.getCell(x, y).tile == 'wall') {
          this.carve(x, y, 'hall');
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
      if (dirs.includes(this.lastDir) && Math.random() < this.params.straightness) {
        dir = this.lastDir;
      } else {
        dir = dirs[Math.floor(Math.random() * dirs.length)];
        this.lastDir = dir;
      }

      if (dir == 'n') {
        this.carve(pos.x, pos.y - 1, 'hall');
        this.carve(pos.x, pos.y - 2, 'hall');
        this.push(pos.x, pos.y - 2);
      } else if (dir == 'w') {
        this.carve(pos.x - 1, pos.y, 'hall');
        this.carve(pos.x - 2, pos.y, 'hall');
        this.push(pos.x - 2, pos.y);
      } else if (dir == 's') {
        this.carve(pos.x, pos.y + 1, 'hall');
        this.carve(pos.x, pos.y + 2, 'hall');
        this.push(pos.x, pos.y + 2);
      } else if (dir == 'e') {
        this.carve(pos.x + 1, pos.y, 'hall');
        this.carve(pos.x + 2, pos.y, 'hall');
        this.push(pos.x + 2, pos.y);
      }
    }

    return true;
  }

  placeRoom() {
    if (this.region > 3 && this.rooms / Math.floor(this.width * this.height / 2) > this.params.room_density) return false;

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
        if (this.getCell(x + ix, y + iy).tile != 'wall') return true;
      }
    }

    for (var iy = 0; iy < h; ++iy) {
      for (var ix = 0; ix < w; ++ix) {
        this.carve(x + ix, y + iy, 'room');
      }
    }

    if (this.region == 1) {
      this.placeInRoom(x, y, w, h, 'up');
    } else if (this.region == 2) {
      this.placeInRoom(x, y, w, h, 'down');
    } else {
      while (Math.random() < 0.33) this.placeInRoom(x, y, w, h, 'treasure');
    }

    this.region++;
    this.rooms += w * h;

    return true;
  }

  isConnector(x, y, source) {
    if (this.getCell(x, y).tile != 'wall') return 0;

    var addIfNew = function(a, v) {
      if (v > 0 && !a.includes(v)) a.push(v);
    };

    var near = [];
    addIfNew(near, this.getCell(x, y - 1).region);
    addIfNew(near, this.getCell(x - 1, y).region);
    addIfNew(near, this.getCell(x, y + 1).region);
    addIfNew(near, this.getCell(x + 1, y).region);

    if (near.includes(source) && near.length > 1) {
      return near[0] == source ? near[1] : near[0];
    }
    return 0;
  }

  replaceRegion(from, to) {
    for (var y = 1; y < this.height; ++y) {
      for (var x = 1; x < this.width; ++x) {
        var cell = this.getCell(x, y);
        if (cell.region == from) cell.region = to;
      }
    }
  }

  connectToRegion(region, min, lock) {
    var connectors = [];
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        var other = this.isConnector(x, y, region);
        if (other > min) connectors.push({x: x, y: y, region: other });
      }
    }

    if (connectors.length == 0) return false;

    var i = Math.floor(Math.random() * connectors.length);
    var door = connectors[i];

    this.replaceRegion(door.region, region);
    this.setCell(door.x, door.y, lock ? 'locked' : 'door');

    if (!lock) {
      for (var i = 0; i < connectors.length; ++i) {
        if (connectors[i].region != door.region) continue;
        if (this.adjacentCount(connectors[i].x, connectors[i].y, 'door') > 0) continue;
        if (Math.random() < this.params.extra_doors) {
          this.setCell(connectors[i].x, connectors[i].y, 'door');
        }
      }
    }

    return true;
  }

  connectRegions() {
    var anyPlaced = false;
    for (var r = 1; r <= this.sections; ++r) {
      if (this.connectToRegion(r, this.sections, false)) anyPlaced = true;
    }
    return anyPlaced || this.setUpLockAndKey();
  }

  setUpLockAndKey() {
    var rooms = [];
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        var cell = this.getCell(x, y);
        if (cell.region == 1 && cell.tile == 'treasure') rooms.push({ x: x, y: y});
      }
    }

    if (rooms.length > 0) {
      var i = Math.floor(Math.random() * rooms.length);
      var p = rooms[i];

      if (!this.connectToRegion(1, 0, true)) return false;
      this.setCell(rooms[i].x, rooms[i].y, 'key')
      return true;
    } else {
      return this.connectToRegion(1, 0, false);
    }
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
      this.setCell(remove[i].x, remove[i].y, 'wall');
    }

    return remove.length > 0;
  }

  isInRoom(x, y) {
    return this.getCell(x, y).tile == 'room' && this.adjacentCount(x, y, 'room') == 4;
  }

  placeInRoom(x, y, w, h, tile) {
    var px = Math.floor(Math.random() * w) + x;
    var py = Math.floor(Math.random() * h) + y;
    this.setCell(px, py, tile);
  }

  draw(canvas) {
    var ctx = canvas.getContext('2d');

    canvas.setAttribute('width', this.width * tileSize - 1);
    canvas.setAttribute('height', this.height * tileSize - 1);

    ctx.fillStyle = '#222';
    ctx.fillRect(0, 0, this.width * tileSize - 1, this.height * tileSize - 1);

    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        var c = this.getCell(x, y);
        var mark = function(color) {
          ctx.fillStyle = color;
          ctx.fillRect(x * tileSize + 4, y * tileSize + 4, tileSize - 4 * 2, tileSize - 4 * 2);
        };

        if (c.seen) {
          ctx.drawImage(this.imgs[c.tile], x * tileSize, y * tileSize);

          if (this.isConnector(x, y, 1) > 0) {
            mark('#ff0');
          } else if (this.isConnector(x, y, 2) > 0) {
            mark('#0ff');
          }

          ctx.fillStyle = this.color(c);
          ctx.fillRect(x * tileSize, y * tileSize, tileSize, tileSize);
        }
      }
    }
  }

  color(cell) {
    if (cell.region > 1) {
      var b = (cell.region % 4) * 64 + 32;
      var g = ((cell.region / 4) % 4) * 64 + 32;
      var r = ((cell.region / 16) % 4) * 64 + 32;
      return 'rgba(' + r + ',' + g + ',' + b + ', 0.50)';
    } else {
      var alpha = cell.visible ? 0 : 0.5;
      return 'rgba(0, 0, 0, ' + alpha + ')';
    }
  }

  push(x, y) {
    this.stack.push({ x: x, y: y });
  }

  pop() {
    return this.stack.length > 0 ? this.stack.pop() : [];
  }
}
