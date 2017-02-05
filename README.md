# PUTMOP

PUTMOP ("Putmop Updates The Memory Of Putmop") is an unfriendly
rogue-like ascii game where the game world contains almost all of the working
memory of the game, which the player can update and manipulate.

The main objective (apart from trying to have fun
playing this game) is to exit the game gracefully, thus, if
the program segfaults, you lose.

The world is also populated with other turing-machine-like bots,
unknowing of the perils of C's undefined behavior.

Upon each start the game world is randomized, so
the placement of the different parts of
game data is random, and the bots program's are
also random.

This game is far from finished...

## TODOs

* Implement the rest of the bot's commands
* Make it possible to configure init-values via command line args.
* Add a high-score based on the sum of the
  cell-wise diff between the original world and the current world
  upon graceful exit. So more cells edited gives a higher score.
