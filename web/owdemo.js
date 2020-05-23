var overworld = new Overworld(mapWidth * screenWidth, mapHeight * screenHeight, {});
var canvas = document.getElementById('c');

function randomSeed() {
  document.getElementById('seed').value = Math.floor(Math.random() * 65536);
}

function redoAll() {
  const p = makeParams();

  overworld.generateVoronoi(p);
  overworld.generateTerrain(p);
  overworld.draw(canvas);
}

function redoMap() {
  const p = makeParams();

  overworld.generateTerrain(p);
  overworld.draw(canvas);
}

function redraw() {
  overworld.draw(canvas);
}

function makeParams() {
  var params = {};
  var keys = [ 'seed', 'baseFreq', 'octaves', 'detail', 'relax', 'gradientOffset', 'gradientScale' ];

  for (var i = 0; i < keys.length; ++i) {
    var p = keys[i];
    var v = parseInt(document.getElementById(p).value);
    params[p] = v;
  }

  return params;
}

function change(id, f) {
  document.getElementById(id).addEventListener('change', f);
}

function click(id, f) {
  document.getElementById(id).addEventListener('click', f);
}

window.addEventListener('DOMContentLoaded', (event) => {
  click('generate', redoAll);
  click('randSeed', () => { randomSeed(); redoMap() });

  change('detail', redoAll);
  change('relax', redoAll);
  change('seed', redoMap);
  change('baseFreq', redoMap);
  change('octaves', redoMap);
  change('gradientOffset', redoMap);
  change('gradientScale', redoMap);
  change('rasterize', redraw);

  randomSeed();
  redoAll();

  canvas.addEventListener('mousemove', e => {
    return;

    const x = e.offsetX;
    const y = e.offsetY;
    const c = overworld.getVoronoiCell(x, y);

    console.log('show info for cell at ' + x + ', ' + y);

    for (var p of ['height', 'moisture', 'temp']) {
      var dd = document.getElementById(p);
      dd.innerText = c[p];
    }
  });
});

setInterval(function() {
  overworld.flowWater();
  redraw();
}, 250);
