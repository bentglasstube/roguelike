var dungeon = new Dungeon(79, 39);
var canvas = document.getElementById('c');
dungeon.draw(canvas);

function clicked(id, func) {
  document.getElementById(id).addEventListener('click', func);
}

var i = null;
function runUntilDone(interval, func) {
  i = setInterval(function() {
    if (!func.bind(dungeon)()) clearInterval(i);
    dungeon.draw(canvas);
  }, interval);
}

clicked('reset', function(e) {
  clearInterval(i);
  dungeon.reset();
  dungeon.draw(canvas);
});

clicked('rooms', function(e) {
  dungeon.placeRooms();
  dungeon.draw(canvas);
});

clicked('halls', function(e) {
  runUntilDone(10, dungeon.step)
});

clicked('connect', function(e) {
  runUntilDone(250, dungeon.connectRegions);
});

clicked('clean', function(e) {
  runUntilDone(10, dungeon.cleanDeadEnds);
});

clicked('populate', function(e) {
  dungeon.placeTreasures();
  dungeon.placeStairs();
  dungeon.draw(canvas);
});

clicked('stop', function(e) {
  clearInterval(i);
  dungeon.draw(canvas);
});

clicked('all', function(e) {
  dungeon.generateAll();
  dungeon.draw(canvas);
});
