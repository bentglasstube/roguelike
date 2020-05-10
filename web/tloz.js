'use strict';

const tileSize = 16;

const tileNames = ['grass', 'sand', 'water', 'rock', 'tree'];
const tileImages = [];
for (var t of tileNames) {
  tileImages[t] = new Image();
  tileImages[t].src = t + '.png';
}

const tileColors = {
  'oob': '#222',
  'grass': '#74cf45',
  'sand': '#ba936b',
  'water': '#68a8c8',
  'rock': '#594848',
  'tree': '#376a25',
};

class Player {
  constructor() {
    this.x = 8;
    this.y = 6;
    this.img = new Image();
    this.img.src = 'owhero.png';
  }

  draw(c, xo, yo) {
    c.drawImage(this.img, (this.x - xo) * 16, (this.y - yo) * 16);
  }
}

class PRNG {
  constructor(seed) {
    this.state = seed;
  }

  value() {
    var t = this.state += 0x6d2b79f5;
    t = Math.imul(t ^ t >>> 15, t | 1);
    t ^= t + Math.imul(t ^ t >>> 7, t | 61);
    return ((t^t >>> 14) >>> 0) / 4294967296;
  }

  below(max) {
    return Math.floor(this.value() * max);
  }

  range(min, max) {
    return Math.floor(this.value() * (max - min)) + min;
  }

  element(list) {
    return list[this.below(list.length)];
  }
}

class Screen {
  constructor() {
    this.width = 16;
    this.height = 12;
    this.region = 'oob';
    this.walls = [0, 0, 0, 0];
    this.seen = false;

    this.tiles = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.tiles[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.tiles[y][x] = 'oob';
      }
    }
  }

  inBounds(x, y) {
    if (x < 0 || x >= this.width) return false;
    if (y < 0 || y >= this.height) return false;
    return true;
  }

  getTile(x, y) {
    return this.inBounds(x, y) ? this.tiles[y][x] : 'oob';
  }

  setTile(x, y, tile) {
    if (this.inBounds(x, y)) this.tiles[y][x] = tile;
  }

  setHRange(x1, x2, y, tile) {
    for (var x = x1; x <= x2; ++x) {
      this.setTile(x, y, tile);
    }
  }

  setVRange(x, y1, y2, tile) {
    for (var y = y1; y <= y2; ++y) {
      this.setTile(x, y, tile);
    }
  }

  draw(c, xo, yo) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        const tile = this.getTile(x, y);
        c.drawImage(tileImages[tile], xo + x * tileSize, yo + y * tileSize);
      }
    }

    this.seen = true;
  }

  drawMap(c, xo, yo) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        const tile = this.getTile(x, y);
        c.fillStyle = this.seen ? tileColors[tile] : '#555';
        c.fillRect(x + xo, y + yo, 1, 1);
      }
    }
  }

  edgeTile() {
    switch (this.region) {
      case 'water':
        return 'water';

      case 'rock':
      case 'sand':
      case 'grass':
        return 'rock';

      case 'tree':
        return 'tree';
    }

    return 'oob';
  }

  groundTile() {
    switch (this.region) {
      case 'water': return 'sand';
      case 'rock':  return 'grass';
      case 'sand':  return 'sand';
      case 'grass': return 'grass';
      case 'tree':  return 'grass';
    }
  }

  openWideWalls() {
    const cx = this.width / 2;
    const cy = this.height / 2;

    if (this.walls[0] > 0) {
      const s = Math.floor(this.walls[0] / 2);
      this.setHRange(cx - s, cx + s, 0, this.groundTile());
      this.setHRange(cx - s, cx + s, 1, this.groundTile());
    }

    if (this.walls[1] > 0) {
      const s = Math.floor(this.walls[1] / 2);
      this.setVRange(this.width - 1, cy - s, cy + s, this.groundTile());
    }

    if (this.walls[2] > 0) {
      const s = Math.floor(this.walls[2] / 2);
      this.setHRange(cx - s, cx + s, this.height - 1, this.groundTile());
    }

    if (this.walls[3] > 0) {
      const s = Math.floor(this.walls[3] / 2);
      this.setVRange(0, cy - s, cy + s, this.groundTile());
    }
  }

  cellWalls(x, y) {
    const n = [
      {x: x - 1, y: y - 1},
      {x: x,     y: y - 1},
      {x: x + 1, y: y - 1},
      {x: x - 1, y: y    },
      {x: x + 1, y: y    },
      {x: x - 1, y: y + 1},
      {x: x,     y: y + 1},
      {x: x + 1, y: y + 1},
    ];

    var count = 0;
    for (var p of n) {
      if (this.getTile(p.x, p.y) != this.groundTile()) ++count;
    }

    return count;
  }

  automata(rng) {
    var rounds = 8;

    for (var y = 2; y < this.height - 1; ++y) {
      for (var x = 1; x < this.width; ++x) {
        if (rng.value() < 0.45) this.setTile(x, y, this.edgeTile());
      }
    }

    while (rounds > 0) {
      --rounds;

      var births = [];
      var deaths = [];

      for (var y = 2; y < this.height - 1; ++y) {
        for (var x = 1; x < this.width - 1; ++x) {
          const walls = this.cellWalls(x, y);
          const ground = this.tiles[y][x] == this.groundTile();

          if (ground && walls >= 5) births.push({x: x, y: y});
          if (!ground && walls < 5) deaths.push({x: x, y: y});
        }
      }

      for (var b of births) {
        this.setTile(b.x, b.y, this.edgeTile());
      }

      for (var d of deaths) {
        this.setTile(d.x, d.y, this.groundTile());
      }
    }
  }

  generate(r) {
    for (var y = 0; y < this.height; ++y) {
      this.tiles[y][0] = this.tiles[y][this.width - 1] = this.edgeTile();
    }

    for (var x = 0; x < this.width; ++x) {
      this.tiles[0][x] = this.edgeTile();
      this.tiles[1][x] = this.edgeTile();
      this.tiles[this.height - 1][x] = this.edgeTile();
    }

    this.openWideWalls();

    switch (this.region) {
      case 'tree':
      case 'rock':
        this.sprinkle(r, this.region);
        break;

      case 'grass':
      case 'water':
        this.automata(r);
        break;
    }
  }

  sprinkle(rng, tile) {
    const cy = this.height / 2;

    for (var x = 2; x < this.width - 2; x += 2) {
      const dx = x + (x >= this.width / 2 ? 1 : 0);

      const n = rng.below(4);
      switch (n) {
        case 2:
          this.setTile(dx, cy - 1, tile);
          this.setTile(dx, cy + 1, tile);
          break;

        case 3:
          this.setTile(dx, cy - 2, tile);
          this.setTile(dx, cy,     tile);
          this.setTile(dx, cy + 2, tile);
          break;
      }
    }
  }

  hasRegion() {
    return this.region != 'oob';
  }

  setRegion(region) {
    this.region = region;
    this.replace('oob', this.groundTile());
  }

  replace(source, dest) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        if (this.tiles[y][x] == source) {
          this.tiles[y][x] = dest;
        }
      }
    }
  }

  openWall(dir, size) {
    const cx = this.width / 2;
    const cy = this.height / 2;

    switch (dir) {
      case 'U':
        this.walls[0] = size;
        break;

      case 'D':
        this.walls[2] = size;
        break;

      case 'L':
        this.walls[3] = size;
        break;

      case 'R':
        this.walls[1] = size;
        break;
    }
  }
}

class Overworld {
  constructor() {
    this.width = 16;
    this.height = 8;

    this.screens = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.screens[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.screens[y][x] = new Screen();
      }
    }
  }

  drawMap(c, xo, yo) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        this.screens[y][x].drawMap(c, xo + x * 16, yo + y * 12);
      }
    }
  }

  getTile(x, y) {
    const s = this.getScreen({x: Math.floor(x / 16), y: Math.floor(y / 12)});

    if (s == null) return 'oob';
    return s.getTile(x % 16, y % 12);
  }

  inBounds(x, y) {
    if (x < 0 || x >= this.width) return false;
    if (y < 0 || y >= this.height) return false;
    return true;
  }

  visited(x, y) {
    return this.inBounds(x, y) ? this.screens[y][x].visited : true;
  }

  randomPoint() {
    return {
      x: this.rand.below(this.width),
      y: this.rand.below(this.height),
    }
  }

  generate(seed) {
    console.log('Generating world #' + seed);

    this.rand = new PRNG(seed);
    this.generateRegions();
    this.generateMaze();
  }

  generateRegions() {
    this.openCount = this.width * this.height;

    var mountains = this.startEdgeRegion('rock', 0);
    var ocean = this.startEdgeRegion('water', this.height - 1);

    var grass = this.startRegion('grass');
    var desert = this.startRegion('sand');
    var forest1 = this.startRegion('tree');
    var forest2 = this.startRegion('tree');

    this.expandRegion(desert, 7);
    this.expandRegion(mountains, 16);
    this.expandRegion(ocean, 6);

    while (true) {
      var expanded = false;

      expanded = this.expandRegion(grass, 2)   || expanded;
      expanded = this.expandRegion(forest1, 1) || expanded;
      expanded = this.expandRegion(forest2, 1) || expanded;

      if (!expanded) {
        expanded = this.expandRegion(desert,    2) || expanded;
        expanded = this.expandRegion(mountains, 2) || expanded;
        expanded = this.expandRegion(ocean,     1) || expanded;

        if (!expanded) break;
      }
    }

    console.log('Done building regions');
  }

  getScreen(p) {
    if (!this.inBounds(p.x, p.y)) return null;
    return this.screens[p.y][p.x];
  }

  isOpen(p) {
    return this.inBounds(p.x, p.y) && this.screens[p.y][p.x].region == 'oob';
  }

  startEdgeRegion(region, y) {
    var length = this.rand.range(Math.floor(this.width / 4), Math.floor(this.width / 2));
    var start = this.rand.below(this.width - length);

    var screens = [];
    for (var x = start; x < start + length; ++x) {
      const p = {x: x, y: y};
      if (this.isOpen(p)) {
        this.getScreen(p).setRegion(region);
        this.openCount--;
        screens.push(p);
      }
    }

    return screens;
  }

  startRegion(region) {
    while (true) {
      var p = this.randomPoint();

      if (region == 'rock') p.y = 0;
      if (region == 'water') p.y = this.height - 1;

      if (this.isOpen(p)) {
        this.getScreen(p).setRegion(region);
        this.openCount--;
        return [p];
      }
    }
  }

  openNeighbors(p) {
    const n = [
      { x: p.x, y: p.y - 1 },
      { x: p.x + 1, y: p.y },
      { x: p.x, y: p.y + 1 },
      { x: p.x - 1, y: p.y },
    ];
    return n.filter(p => this.isOpen(p));
  }

  expandRegion(region, count) {
    const tile = this.getScreen(region[0]).region;

    var neighbors = [];
    for (var p of region) {
      neighbors = neighbors.concat(this.openNeighbors(p));
    }

    while (count > 0 && neighbors.length > 0) {
      const x = this.rand.element(neighbors);
      this.getScreen(x).setRegion(tile);
      region.push(x);
      this.openCount--;

      neighbors = neighbors.filter( p => this.isOpen(p) );
      neighbors = neighbors.concat(this.openNeighbors(x));
      count--;
    }

    return neighbors.length > 0;
  }

  generateMaze() {
    this.stack = [this.randomPoint()];

    while (this.stack.length > 0) {
      var p = this.stack.pop();

      this.screens[p.y][p.x].visited = true;

      var dirs = [];
      if (!this.visited(p.x - 1, p.y)) dirs.push('L');
      if (!this.visited(p.x + 1, p.y)) dirs.push('R');
      if (!this.visited(p.x, p.y - 1)) dirs.push('U');
      if (!this.visited(p.x, p.y + 1)) dirs.push('D');

      if (dirs.length > 1) this.stack.push(p)

      if (dirs.length > 0) {
        this.stack.push(this.openWall(p, this.rand.element(dirs)));
      }
    }

    for (var i = 0; i < 50; ++i) {
      const p = this.randomPoint();
      var dirs = [];
      if (this.inBounds(p.x - 1, p.y)) dirs.push('L');
      if (this.inBounds(p.x + 1, p.y)) dirs.push('R');
      if (this.inBounds(p.x, p.y - 1)) dirs.push('U');
      if (this.inBounds(p.x, p.y + 1)) dirs.push('D');
      this.openWall(p, this.rand.element(dirs));
    }

    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        this.screens[y][x].generate(this.rand);
      }
    }
  }

  openWall(p, dir) {
    var op = {};
    var od = '';

    var s = this.rand.range(1, 4) * 2;

    switch (dir) {
      case 'U':
        op = {x: p.x, y: p.y - 1};
        od = 'D';
        break;

      case 'D':
        op = {x: p.x, y: p.y + 1};
        od = 'U';
        break;

      case 'L':
        op = {x: p.x - 1, y: p.y};
        od = 'R';
        --s;
        break;

      case 'R':
        op = {x: p.x + 1, y: p.y};
        od = 'L';
        --s;
        break;
    }

    this.getScreen(p).openWall(dir, s);
    this.getScreen(op).openWall(od, s);

    return op;
  }
}

class Game {
  constructor() {

    this.seed = parseInt(window.location.hash.substring(1));
    if (isNaN(this.seed)) {
      this.seed = Math.floor(Math.random() * 1000000000);
    }

    this.player = new Player();
    this.world = new Overworld();

    this.world.generate(this.seed);
    document.getElementById('seed').innerText = this.seed;

    var map = document.getElementById('map');
    map.setAttribute('width', 256);
    map.setAttribute('height', 96);
    this.mapContext = map.getContext('2d');

    var screen = document.getElementById('screen');
    screen.setAttribute('width', 256);
    screen.setAttribute('height', 192);
    this.screenContext = screen.getContext('2d');
  }

  draw() {
    const p = {
      x: Math.floor(this.player.x / 16),
      y: Math.floor(this.player.y / 12),
    };

    this.world.getScreen(p).draw(this.screenContext, 0, 0);
    this.player.draw(this.screenContext, p.x * 16, p.y * 12);

    this.world.drawMap(this.mapContext, 0, 0);
    this.mapContext.strokeStyle = '#ffffff';
    this.mapContext.strokeRect(p.x * 16 + 1, p.y * 12 + 1, 14, 10);
  }

  movePlayer(dx, dy) {
    var nx = this.player.x + dx;
    var ny = this.player.y + dy;

    var t = this.world.getTile(nx, ny);
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
}

var game = new Game();
game.draw();

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

  game.draw();
});

setTimeout(() => game.draw(), 250);
