'use strict';

class Screen {
  constructor(up, left, rng) {
    this.tileSet = [
      [ 0, 0, 0, 1 ],
      [ 2, 0, 2, 1 ],
      [ 0, 1, 1, 1 ],
      [ 3, 2, 0, 2 ],
      [ 2, 2, 3, 2 ],
      [ 3, 3, 0, 3 ],
      [ 0, 1, 2, 3 ],
      [ 2, 3, 2, 0 ],
      [ 2, 0, 3, 0 ],
      [ 1, 1, 2, 0 ],
      [ 0, 3, 0, 1 ],
      [ 0, 1, 1, 0 ],
      [ 1, 0, 0, 1 ],
      [ 1, 1, 1, 3 ],
      [ 2, 0, 0, 3 ],
      [ 2, 0, 0, 1 ],
      [ 3, 0, 1, 0 ],
      [ 3, 2, 0, 1 ],
      [ 0, 1, 0, 2 ],
      [ 1, 2, 0, 2 ],
    ];

    this.tileImages = new Array(this.tileSet.length);
    for (var i = 0; i < this.tileSet.length; ++i) {
      this.tileImages[i] = new Image();
      this.tileImages[i].src = "tile" + i + ".png";
    }

    const possible = this.tileSet.filter(t => t[0] == up && t[3] == left);

    if (possible.length == 0) {
      throw 'Impossible constraints: ' + up + ',' + left;
    }

    this.tile = rng.element(possible);
    this.image = this.tileImages[this.tileSet.indexOf(this.tile)];
  }

  draw(context, x, y) {
    context.drawImage(this.image, x, y);
  }

  get top() { return this.tile[0]; }
  get right() { return this.tile[1]; }
  get bottom() { return this.tile[2]; }
  get left() { return this.tile[3]; }
}

class Overworld {
  constructor(seed) {
    this.width = 16;
    this.height = 8;

    this.rand = new PRNG(seed);

    this.screens = new Array(this.height);
    for (var y = 0; y < this.height; ++y) {
      this.screens[y] = new Array(this.width);
      for (var x = 0; x < this.width; ++x) {
        const up = this.upConstraint(x, y);
        const left = this.leftConstraint(x, y);

        console.log('Picking tile for ' + x + ',' + y);

        this.screens[y][x] = new Screen(up, left, this.rand);

      }
    }
  }

  getScreen(x, y) {
    return this.screens[y][x];
  }

  upConstraint(x, y) {
    if (y == 0) return 0;
    return this.getScreen(x, y - 1).bottom;
  }

  leftConstraint(x, y) {
    if (x == 0) return 0;
    return this.getScreen(x - 1, y).right;
  }

  draw(context) {
    for (var y = 0; y < this.height; ++y) {
      for (var x = 0; x < this.width; ++x) {
        this.getScreen(x, y).draw(context, x * 16, y * 12);
      }
    }
  }
}

var canvas = document.getElementById('c');
canvas.setAttribute('width', 256);
canvas.setAttribute('height', 96);

var world = new Overworld(Math.floor(Math.random() * 328749823749));

setTimeout( () => world.draw(canvas.getContext('2d')), 1000);
