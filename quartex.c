#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define test

#define tile_count 55
#define corner_count 4
#define config_count 8
#define hand_limit 5
#define player_limit 5

typedef enum bool bool;
enum bool { false, true };

typedef enum corner_t corner_t;
enum corner_t { RED, YELLOW, PURPLE, BLUE };

typedef enum dir_t dir_t;
enum dir_t { N, E, S, W };

/*
   0 1
   3 2
*/
typedef struct tile_t tile_t;
struct tile_t {
  int id;
  int encoded; // in bits, store number of colors on the tile. we'll use this for quick comparison.
  corner_t configs[config_count][corner_count];
};

typedef struct placed_tile_t placed_tile_t;
struct placed_tile_t {
  int x, y;
  tile_t *tile;
  corner_t *config;
  placed_tile_t *n, *e, *s, *w;

  // Meta
  bool marked_for_deletion;
  bool visited;
};

typedef struct game_t game_t;
struct game_t {
  int red_tokens, yellow_tokens, purple_tokens, blue_tokens;
  int tiles_remaining;
  tile_t **tile_bag;
  int player_count;
  int hand_count[hand_limit];
  tile_t hands[player_limit][hand_limit];
  placed_tile_t *origin; // TODO: set this up correctly (see tests)
};

// TODO: convert this to use function pointers
#define find_most(dir, comp) \
  placed_tile_t *find_most_##dir(placed_tile_t *origin) {\
    if (origin == NULL) return NULL;\
    if (origin->visited) return NULL;\
    origin->visited = true;\
    placed_tile_t *most = origin;\
    placed_tile_t *result = NULL;\
    result = find_most_##dir(origin->n);\
    if (result != NULL && comp) most = result;\
    result = find_most_##dir(origin->e);\
    if (result != NULL && comp) most = result;\
    result = find_most_##dir(origin->s);\
    if (result != NULL && comp) most = result;\
    result = find_most_##dir(origin->w);\
    if (result != NULL && comp) most = result;\
    origin->visited = false;\
    return most;\
  }

find_most(north, result->y > most->y);
find_most(south, result->y < most->y);
find_most(east, result->x > most->x);
find_most(west, result->x < most->x);

placed_tile_t *find_placed_tile(placed_tile_t *origin, int x, int y) {
  if (origin == NULL) return NULL;
  if (origin->visited) return NULL;

  placed_tile_t *result = NULL;

  origin->visited = true;
  if (origin->x == x && origin->y == y) {
    result = origin;
  } else {
    if (result == NULL) result = find_placed_tile(origin->n, x, y);
    if (result == NULL) result = find_placed_tile(origin->e, x, y);
    if (result == NULL) result = find_placed_tile(origin->s, x, y);
    if (result == NULL) result = find_placed_tile(origin->w, x, y);
  }
  origin->visited = false;

  return result;
}

placed_tile_t *place_tile(placed_tile_t *origin, dir_t dir, tile_t *tile, corner_t *config) {
  int x = origin->x;
  int y = origin->y;

  switch (dir) {
    case N: y += 1; break;
    case E: x += 1; break;
    case S: y -= 1; break;
    case W: x -= 1; break;
  }

  placed_tile_t *new_placed_tile = malloc(sizeof(placed_tile_t));

  new_placed_tile->x = x;
  new_placed_tile->y = y;
  new_placed_tile->tile = tile;
  new_placed_tile->config = config;
  new_placed_tile->n = NULL;
  new_placed_tile->e = NULL;
  new_placed_tile->s = NULL;
  new_placed_tile->w = NULL;
  new_placed_tile->marked_for_deletion = false;
  new_placed_tile->visited = false;

  switch (dir) {
    case N:
      new_placed_tile->s = origin;
      origin->n = new_placed_tile;
      break;
    case E:
      new_placed_tile->w = origin;
      origin->e = new_placed_tile;
      break;
    case S:
      new_placed_tile->n = origin;
      origin->s = new_placed_tile;
      break;
    case W:
      new_placed_tile->e = origin;
      origin->w = new_placed_tile;
      break;
  }

  return new_placed_tile;
}

void free_placed_tiles(placed_tile_t *origin) {
  if (origin != NULL && !origin->marked_for_deletion) {
    origin->marked_for_deletion = true;
    free_placed_tiles(origin->n);
    free_placed_tiles(origin->e);
    free_placed_tiles(origin->s);
    free_placed_tiles(origin->w);
    free(origin);
  }
}

placed_tile_t *origin_tile(tile_t *tile, corner_t *config) {
  placed_tile_t *origin = malloc(sizeof(placed_tile_t));

  origin->x = 0;
  origin->y = 0;
  origin->tile = tile;
  origin->config = config;
  origin->n = NULL;
  origin->e = NULL;
  origin->s = NULL;
  origin->w = NULL;
  origin->marked_for_deletion = false;
  origin->visited = false;

  return origin;
}

void print_tile(tile_t *tile) {
  corner_t *base = tile->configs[0];
  printf("%i%i\n%i%i\n\n", base[0], base[1], base[2], base[3]);
}

void print_tiles(tile_t **tiles, int count) {
  for (int i = 0; i < count; i++) {
    print_tile(tiles[i]);
  }
}

void rotate_corners(corner_t *corners) {
  corner_t temp = corners[3];
  corners[3] = corners[2];
  corners[2] = corners[1];
  corners[1] = corners[0];
  corners[0] = temp;
}

void flip_corners(corner_t *corners) {
  corner_t temp1 = corners[0];
  corner_t temp2 = corners[3];
  corners[0] = corners[1];
  corners[3] = corners[2];
  corners[1] = temp1;
  corners[2] = temp2;
}

bool corners_equal(corner_t *a, corner_t *b) {
  for (int i = 0; i < corner_count; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

corner_t int_to_corner(int i) {
  switch (i) {
    case 0: return RED;
    case 1: return YELLOW;
    case 2: return PURPLE;
    case 3: return BLUE;
  }
  return 0;
}

void free_shallow_tiles(tile_t **tiles) {
  free(tiles);
}

void free_tiles(tile_t **tiles) {
  for (int i = 0; i < tile_count; i++) {
    free(tiles[i]);
  }
  free(tiles);
}

tile_t **generate_tiles() {
  tile_t **tiles = malloc(sizeof(tile_t *) * tile_count);

  int i = 0;
  corner_t a_corner, b_corner, c_corner, d_corner;
  for (int a = 0; a < corner_count; a++) {
    a_corner = int_to_corner(a);
    for (int b = 0; b < corner_count; b++) {
      b_corner = int_to_corner(b);
      for (int c = 0; c < corner_count; c++) {
        c_corner = int_to_corner(c);
        for (int d = 0; d < corner_count; d++) {
          d_corner = int_to_corner(d);

          tile_t *tile = malloc(sizeof(tile_t));
          tile->id = i;

          for (int x = 0; x < config_count; x++) {
            tile->configs[x][0] = a_corner;
            tile->configs[x][1] = b_corner;
            tile->configs[x][2] = c_corner;
            tile->configs[x][3] = d_corner;
          }

          rotate_corners(tile->configs[1]);

          rotate_corners(tile->configs[2]);
          rotate_corners(tile->configs[2]);

          rotate_corners(tile->configs[3]);
          rotate_corners(tile->configs[3]);
          rotate_corners(tile->configs[3]);

          rotate_corners(tile->configs[4]);

          rotate_corners(tile->configs[5]);
          rotate_corners(tile->configs[5]);

          rotate_corners(tile->configs[6]);
          rotate_corners(tile->configs[6]);
          rotate_corners(tile->configs[6]);

          flip_corners(tile->configs[4]);
          flip_corners(tile->configs[5]);
          flip_corners(tile->configs[6]);
          flip_corners(tile->configs[7]);

          bool duplicate = false;
          for (int j = 0; j < i; j++) {
            for (int z = 0; z < config_count; z++) {
              if (corners_equal(tiles[j]->configs[z], tile->configs[0])) {
                duplicate = true;
                break;
              }
            }
            if (duplicate) break;
          }
          if (duplicate) {
            free(tile);
          } else {
            tiles[i++] = tile;
          }
        }
      }
    }
  }

  assert(i == tile_count);

  return tiles;
}

tile_t **shallow_copy_tiles(tile_t **tiles) {
  tile_t **copy = malloc(sizeof(tile_t *) * tile_count);
  for (int i = 0; i < tile_count; i++) {
    copy[i] = tiles[i];
  }
  return copy;
}

void shuffle_tiles(tile_t **tiles) {
  for (int i = tile_count - 1; i >= 0; i--) {
    int j = rand() % (i + 1);
    tile_t *temp = tiles[i];
    tiles[i] = tiles[j];
    tiles[j] = temp;
  }
}

void simulate_game(tile_t **original) {
  tile_t **tiles = shallow_copy_tiles(original);
  shuffle_tiles(tiles);

  game_t game = {
    .red_tokens = 10,
    .blue_tokens = 10,
    .yellow_tokens = 10,
    .purple_tokens = 10,
    .tiles_remaining = tile_count,
    .tile_bag = tiles,
    .player_count = 2,
    .hand_count = { 0, 0, 0, 0, 0 }
  };

  if (game.player_count < 4) {
    game.tiles_remaining -= 1;
  } else if (game.player_count < 5) {
    game.tiles_remaining -= 3;
  }

  free_shallow_tiles(tiles);
}

int main(int argc, char **argv) {
  srand(time(NULL));

#ifdef test

  printf("Running tests...\n");

  tile_t **tiles = generate_tiles();
  tile_t **copy = shallow_copy_tiles(tiles);

  shuffle_tiles(copy);

  print_tiles(copy, tile_count);

  placed_tile_t *origin = origin_tile(copy[0], copy[0]->configs[0]);
  placed_tile_t *pt1 = place_tile(origin, N, copy[1], copy[1]->configs[0]);
  placed_tile_t *pt2 = place_tile(origin, E, copy[2], copy[2]->configs[0]);
  placed_tile_t *pt3 = place_tile(origin, S, copy[3], copy[3]->configs[0]);
  placed_tile_t *pt4 = place_tile(origin, W, copy[4], copy[4]->configs[0]);

  assert(origin->x == 0);
  assert(origin->y == 0);
  assert(pt1->x == 0);
  assert(pt1->y == 1);
  assert(pt1->s == origin);
  assert(origin->n == pt1);

  placed_tile_t *result = find_placed_tile(origin, 0, 1);
  assert(result->x == 0);
  assert(result->y == 1);

  placed_tile_t *most_north = find_most_north(origin);
  assert(most_north->x == 0);
  assert(most_north->y == 1);

  placed_tile_t *most_south = find_most_south(origin);
  assert(most_south->x == 0);
  assert(most_south->y == -1);

  placed_tile_t *most_east = find_most_east(origin);
  assert(most_east->x == 1);
  assert(most_east->y == 0);

  placed_tile_t *most_west = find_most_west(origin);
  assert(most_west->x == -1);
  assert(most_west->y == 0);

  free_placed_tiles(origin);
  free_shallow_tiles(copy);
  free_tiles(tiles);

  return 0;

#else

  printf("Quartex Simulation v0.0\n\n");

  tile_t **tiles = generate_tiles();
  simulate_game(tiles);
  free_tiles(tiles);

  return 0;

#endif

}
