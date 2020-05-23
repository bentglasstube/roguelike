const tileSize = 4;
const screenWidth = 16;
const screenHeight = 12;
const mapWidth = 16;
const mapHeight = 12;

class Overworld {
  constructor(width, height) {
    this.width = width;
    this.height = height;
    this.reset();

    const t = ['grass', 'water', 'sand', 'tree', 'rock'];
    this.imgs = {};
    for (var i = 0; i < t.length; ++i) {
      this.imgs[t[i]] = new Image();
      this.imgs[t[i]].src = t[i] + '.png';
    }
  }

  reset() {
    this.points = [];
    this.voronoi = null;
    this.hcell = null;

    this.cells = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.cells[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.cells[y][x] = { tile: 'grass' };
      }
    }
  }

  setCell(x, y, tile) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    this.cells[y][x].tile = tile;
  }

  getCell(x, y) {
    if (x < 0 || x >= this.width) return { tile: 'oob' };
    if (y < 0 || y >= this.height) return { tile: 'oob' };
    return this.cells[y][x];
  }

  randRange(low, high) {
    return Math.floor(Math.random() * (high - low)) + low;
  }

  addPoints(count) {
    for (var i = 0; i < count; ++i) {
      const p = {
        x: this.width * Math.random(),
        y: this.height * Math.random(),
      };
      this.points.push(p);
    }

    this.voronoize();
  }

  relax(iterations) {
    if (!this.voronoi) return;

    for (var h = 0; h < iterations; ++h) {
      this.points = [];
      for (var i = 0; i < this.voronoi.cells.length; ++i) {
        const cell = this.voronoi.cells[i];

        var x = 0, y = 0, a = 0;
        for (var j = 0; j < cell.halfedges.length; ++j) {
          const edge = cell.halfedges[j];
          const p1 = edge.getStartpoint();
          const p2 = edge.getEndpoint();
          const v = p1.x * p2.y - p2.x * p1.y;
          x += (p1.x + p2.x) * v;
          y += (p1.y + p2.y) * v;
          a += p1.x * p2.y - p1.y * p2.x;
        }

        this.points.push({x: x / a / 3, y: y / a / 3});
      }
      this.voronoize();
    }
  }

  voronoize() {
    const bbox = {xl: 0, xr: this.width, yt: 0, yb: this.height };
    var v = new Voronoi();
    this.voronoi = v.compute(this.points, bbox);
    this.hcell = null;

    if (isNaN(this.voronoi.cells[0].site.x)) {
      throw "Voronoi cell x is NaN";
    }
  }

  noiseOctave(x, y, octaves, gen) {
    var freq = 1;
    var amp = 1;
    var max = 0;
    var val = 0;

    for (var i = 0; i < octaves; ++i) {
      val += gen(x * freq, y * freq) * amp;
      max += amp;
      amp /= 2;
      freq *= 2;
    }

    return val / max;
  }

  generateMap(params, seed, property, mask) {
    if (!this.voronoi) return;

    console.log('Generating map for ' + property + ' with seed ' + seed);

    noise.seed(seed);
    for (var c of this.voronoi.cells) {
      const cx = c.site.x / this.width * params.baseFreq;
      const cy = c.site.y / this.height * params.baseFreq;
      const n = this.noiseOctave(cx, cy, params.octaves, noise.simplex2);
      c[property] = mask ? mask(n, c.site) : n;
    }
  }

  generateMaps(params) {
    this.generateMap(params, params.seed, 'height', (n, p) => {
      const g = 1 - 2 * p.y / this.height;
      const gs = params.gradientScale / 10;
      return g * gs + n + params.gradientOffset / 20;
    });
    this.generateMap(params, params.seed * 233, 'moisture');
    this.generateMap(params, params.seed * 181, 'temp');

    for (var c of this.voronoi.cells) c.water = 0;
  }

  getVoronoiCell(x, y) {
    if (!this.voronoi) return;

    var min = 99999;
    var found = null;
    for (var i = 0; i < this.voronoi.cells.length; ++i) {
      const cell = this.voronoi.cells[i];
      const dx = cell.site.x - x;
      const dy = cell.site.y - y;
      const d = dx * dx + dy * dy;
      if (d < min) {
        min = d;
        found = cell;
      }
    }

    return found;
  }

  rasterize() {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        const c = this.getVoronoiCell(x, y);
        this.setCell(x, y, this.tile(c));
      }
    }
  }

  generateVoronoi(params) {
    this.reset(params);
    this.addPoints(2 ** params.detail);
    this.relax(params.relax);
  }

  generateTerrain(params) {
    this.generateMaps(params);
    this.rasterize();
  }

  addRain() {
    if (!this.voronoi) return;
    for (var c of this.voronoi.cells) {
      if (c.height > 0 && c.moisture > 0) {
        c.water = Math.min(1, c.water + Math.random() * c.moisture);
      }
    }
  }

  flowWater() {
    if (!this.voronoi) return;
    for (var c of this.voronoi.cells) {
      var min = c.height;
      var drain = c;
      for (var id of c.getNeighborIds()) {
        var n = this.voronoi.cells[id];
        if (n.height > 0 && n.height < min && n.water < 1) {
          min = n.height;
          drain = n;
        }
      }

      if (min < c.height) {
        const w = Math.min(c.water * (c.height - drain.height), 1 - drain.water);
        c.water -= w;
        drain.water += w;
      }
    }
  }

  makeRiver() {
    var cell = undefined;

    while (true) {
      var r = Math.floor(Math.random() * this.voronoi.cells.length);
      cell = this.voronoi.cells[r];
      if (cell.height > 0) break;
    }

    while (cell.height > 0 && cell.water < 1) {
      cell.water = 1;
      var min = 9;
      var next = cell;
      for (var i of cell.getNeighborIds()) {
        var n = this.voronoi.cells[i];

        if (n.height < min) {
          min = n.height;
          next = n;
        }
      }

      cell = next;
    }
  }

  rgb(r, g, b) {
    return 'rgb(' + Math.floor(r * 255) + ',' + Math.floor(g * 255) + ',' + Math.floor(b * 255) + ')';
  }

  biome(cell) {
    if (cell.height < 0) return 'ocean';
    if (cell.height < 0.15) return 'beach';
    if (cell.height > 1) return 'mountain';

    const t = Math.floor((cell.temp + 1) / 2 * 100);
    const p = Math.floor((cell.moisture + 1) / 2 * 10);

    return 'grass';
  }

  color(cell) {
    const h = cell.height;
    const w = cell.water;
    const t = cell.temp;

    if (h < 0) return this.rgb(0, 0, h / 2 + 0.75);
    if (w > 1) return this.rgb(1, 0, 0);
    return this.rgb(h / 2 * (t + 1) / 2, 0.1 + h * (0.4 + w / 2), w);


    var r = 0;
    switch (this.biome(cell)) {
      case 'ocean':    return this.rgb(0, 0, 0.6);
      case 'beach':    return this.rgb(0.7, 0.7, 0.1);
      case 'mountain': return this.rgb(0.4, 0.4, 0.4);
      case 'grass':    return this.rgb(0.2, 0.7, 0.3);
      case 'snow':     return this.rgb(0.9, 0.9, 0.9);
    }
  }

  heightColor(cell) {
    const h = cell.height;

    if (h < 0) return this.rgb(0, 0, h / 4 + 0.75);
    if (h > 1.2) return this.rgb(h / 2, h / 2, h / 2);

    if (h < 0.15) {
      const v = h * 2 + 0.5;
      return this.rgb(v, v, v / 2);
    }

    if (h < 0.5) return this.rgb(0, h / 2 + 0.25, 0);
    return this.rgb(h, h, h);
  }

  tile(cell) {
    if (cell.height < 0) return 'water';
    if (cell.height > 1.2) return 'rock';
    if (cell.height < 0.15 || cell.moisture < -0.25) return 'sand';
    if (cell.height < 0.5) return 'grass';

    return Math.random() * Math.random() < cell.moisture / 2 ? 'tree' : 'grass';
  }

  draw(canvas) {
    var ctx = canvas.getContext('2d');

    canvas.setAttribute('width', this.width * tileSize);
    canvas.setAttribute('height', this.height * tileSize);

    ctx.fillStyle = '#222';
    ctx.fillRect(0, 0, this.width * tileSize, this.height * tileSize);

    if (document.getElementById('rasterize').checked) {
      for (var y = 0; y < this.height; ++y) {
        for (var x = 0; x < this.width; ++x) {
          const c = this.getCell(x, y);
          ctx.drawImage(this.imgs[c.tile], x * tileSize, y * tileSize);
        }
      }
    } else {

      for (var i = 0; i < this.voronoi.cells.length; ++i) {
        const cell = this.voronoi.cells[i];

        if (cell.height) {
          ctx.beginPath();
          const start = cell.halfedges[0].getStartpoint();
          ctx.moveTo(start.x * tileSize, start.y * tileSize);
          for (var j = 0; j < cell.halfedges.length; ++j) {
            const e = cell.halfedges[j];
            const p = e.getEndpoint();
            ctx.lineTo(p.x * tileSize, p.y * tileSize);
          }

          const val = cell.height > 0 ? 255 : 0;
          ctx.fillStyle = this.color(cell);
          ctx.strokeStyle = 'rgba(0, 0, 0, 0.2)';
          ctx.fill();
          ctx.stroke();
        }
      }

    }

    for (var y = 0; y < this.height / screenHeight; ++y) {
      for (var x = 0; x < this.width / screenWidth; ++x) {
        ctx.strokeStyle = '#000';
        ctx.strokeRect(x * tileSize * screenWidth, y * tileSize * screenHeight, tileSize * screenWidth, tileSize * screenHeight);
      }
    }
  }

  drawScreen(camera, canvas) {
    var ctx = canvas.getContext('2d');

    canvas.setAttribute('width', 16 * screenWidth);
    canvas.setAttribute('height', 16 * screenHeight);

    ctx.fillStyle = '#222';
    ctx.fillRect(0, 0, 16 * screenWidth, 16 * screenHeight);

    for (var y = 0; y < screenHeight; ++y) {
      for (var x = 0; x < screenWidth; ++x) {
        const c = this.getCell(camera.x + x, camera.y + y);
        ctx.drawImage(this.imgs[c.tile], x * 16, y * 16);
      }
    }

  }
}

class Player {
  constructor() {
    this.x = 0;
    this.y = 0;
    this.img = new Image();
    this.img.src = 'owhero.png';
  }

  draw(camera, canvas) {
    var ctx = canvas.getContext('2d');
    ctx.drawImage(this.img, (this.x - camera.x) * 16, (this.y - camera.y) * 16);
  }
}
