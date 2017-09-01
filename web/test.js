var dungeon = new Dungeon(25, 25, {});
var player = new Player();
var canvas = document.getElementById('c');

for (var y = 1; y < dungeon.height - 1; ++y) {
  for (var x = 1; x < dungeon.width - 1; ++x) {
    dungeon.setCell(x, y, 'room');
  }
}

player.x = player.y = 12;

setTimeout(function() {
  dungeon.draw(canvas);
  dungeon.reveal();
  dungeon.setVisibilityFrom(player.x, player.y);
}, 250);

canvas.addEventListener('click', function(e) {
  var rect = canvas.getBoundingClientRect();
  var tx = Math.floor((e.clientX - rect.left) / 12);
  var ty = Math.floor((e.clientY - rect.top) / 12);

  var c = dungeon.getCell(tx, ty);
  c.tile = c.tile == 'room' ? 'wall' : 'room';

  dungeon.setVisibilityFrom(player.x, player.y);
  dungeon.draw(canvas);
  player.draw(canvas);
});
