class Overworld {
  constructor(width, height) {
    this.width = width;
    this.height = height;
    this.reset()
  }

  reset() {
    this.cells = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.cells[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        this.cells[y][x] = {};
      }
    }
  }

  setCell(x, y, value) {
    if (x < 0 || x >= this.width) return;
    if (y < 0 || y >= this.height) return;
    this.cells[y][x] = value;
  }

  getCell(x, y) {
    if (x < 0 || x >= this.width) return { height: 0 };
    if (y < 0 || y >= this.height) return { height: 0 };
    return this.cells[y][x];
  }

  noiseOctave(x, y) {
    var freq = 8;
    var amp = 2;
    var max = 0;
    var val = amp / 2;

    for (var i = 0; i < 8; ++i) {
      val += noise.perlin2(x / this.width * freq, y / this.height * freq) * amp;
      max += amp;
      amp /= 2;
      freq *= 2;
    }

    return val / max;
  }

  generateMap(seed) {
    console.log('Generating height map with seed ' + seed);
    noise.seed(seed);
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        this.cells[y][x].height = this.noiseOctave(x, y)
      }
    }
  }

  applyGradient() {
    const fraction = 4;

    console.log('Applying gradient');
    for (var y = 0; y < this.height; ++y) {
      var yr = y / this.height;
      var g = 0;

      if (yr < 1 / fraction) {
        g = 1 - yr * fraction;
      } else if (yr > (1 - 1 / fraction)) {
        g = (fraction - 1) - yr * fraction;
      }

      for (var x = 0; x < this.width; ++x) {
        var h = this.cells[y][x].height;
        this.cells[y][x].height = (g + h) / 2;
      }
    }
  }

  maxNeighbor(x, y) {
    var max = this.getCell(x, y).height;
    for (var ix = x - 1; ix < x + 1; ++ix) {
      for (var iy = y - 1; iy < y + 1; ++iy) {
        var h = this.getCell(ix, iy).height;
        if (h > max) max = h
      }
    }
    return h;
  }

  tile(height) {
    if (height < 0.1) {
      return '#00f';
    } else if (height < 0.2) {
      return '#fb8';
    } else if (height < 0.6) {
      return '#2b2';
    } else if (height < 0.8) {
      return '#999';
    } else if (height < 0.9) {
      return '#bbb';
    } else {
      return '#fff';
    }
  }

  stratify() {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        var h = this.cells[y][x].height;
        this.cells[y][x].color = h < 0 ? '#00f' : this.rgb(h, h, h);
      }
    }
  }

  identifyWaterBodies() {
    // TODO denote each body of water with an id and note the largest and northernmost ones
  }

  flowRiver() {
    // TODO Flow river toward ocean
  }

  rgb(r, g, b) {
    return 'rgb(' + Math.floor(255 * r) + ', ' + Math.floor(255 * g) + ', ' + Math.floor(255 * b) + ')';
  }

  draw(canvas) {
    var ctx = canvas.getContext('2d');

    canvas.setAttribute('width', this.width * 8);
    canvas.setAttribute('height', this.height * 8);

    ctx.fillStyle = '#222';
    ctx.fillRect(0, 0, this.width * 8, this.height * 8);

    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        ctx.fillStyle = this.cells[y][x].color;
        ctx.fillRect(x * 8, y * 8, 8, 8);
      }
    }
  }
}

var ow = new Overworld(256, 128);
var canvas = document.getElementById('c');

var seed = Math.floor(Math.random() * 65536 * 65536);
ow.generateMap(seed);
ow.applyGradient();
ow.stratify();

function redraw() {
  ow.draw(canvas);
  requestAnimationFrame(redraw);
}
requestAnimationFrame(redraw);
