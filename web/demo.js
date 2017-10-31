var dungeon = new Dungeon(79, 39, makeParams());
var canvas = document.getElementById('c');
dungeon.draw(canvas);

function clicked(id, func) {
  document.getElementById(id).addEventListener('click', func);
}

var stage = 0;
var i = false;
function runUntilDone(func, mode='anim') {
  if (i) {
    console.log('Still animating last phase');
    return;
  }

  switch (mode) {
    case 'anim':
      i = setInterval(function() {
        if (!func.bind(dungeon)()) {
          clearInterval(i);
          i = false;
        }
        dungeon.draw(canvas);
      }, 10);
      ++stage;
      break;

    case 'step':
      if (!func.bind(dungeon)()) ++stage;
      dungeon.draw(canvas);
      break;

    case 'fast':
      while (func.bind(dungeon)()) {};
      dungeon.draw(canvas);
      ++stage;
      break;
  }
}

function makeParams() {
  var params = {};
  var keys = [ 'sections', 'room_density', 'straightness', 'extra_doors' ];

  for (var i = 0; i < keys.length; ++i) {
    var p = keys[i];
    var v = document.getElementById(p).value;
    params[p] = v / (p == 'sections' ? 1 : 100);
  }

  return params;
}

function reset() {
  dungeon.reset(makeParams());
  dungeon.reveal();
  dungeon.draw(canvas);
  stage = 0;
}

clicked('reset', reset);

clicked('step', function(e) {
  switch (stage) {
    case 0:
      reset();
      runUntilDone(dungeon.placeRoom, 'fast');
      break;

    case 1:
      runUntilDone(dungeon.step, 'anim');
      break;

    case 2:
      runUntilDone(dungeon.connectRegions, 'step');
      break;

    case 3:
      runUntilDone(dungeon.connectSections, 'step');
      break;

    case 4:
      runUntilDone(dungeon.cleanDeadEnds, 'anim');
      break;
  }

  dungeon.draw(canvas);
});

clicked('all', function(e) {
  dungeon.generateAll(makeParams());
  dungeon.reveal();
  dungeon.draw(canvas);
  stage = 9;
});

reset();
