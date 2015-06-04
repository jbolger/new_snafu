//symbolic constants used by snafu and snafu_player

#define SNAFU_RANDOM 0
#define SNAFU_UP 1
#define SNAFU_DOWN 2
#define SNAFU_LEFT 4
#define SNAFU_RIGHT 8

//typedefs

//the snafu_player_direction is an 8-bit integer representing 
//directions for snafu_player
//snafu_player_directions use the above flags to determine directions
//it may appear to use a weird design:  this is because
//hypothetically in the future, i want this typedef to be able 
//to represent diagonal directions by using SNAFU_UP | SNAFU_LEFT for example
typedef guint8 snafu_player_direction;

//the snafu_player is an individual contender in the game of snafu
//
//x and y are the current coordinate location of the snafu_player to a 
//particular board of a snafu.
//_x and _y are the initial coodinates a snafu_player has when a game starts
//alive and humand contain gbooleans determining whether the snafu_player 
//is alive and/or human respectively
//cell_value is the board_cell which will be used to represent the snafu_player
// on the board
//direction is the snafu_player_direction the snafu_player will 
//attempt to move in
//
//name is an allocated string representing what markup will should be 
//used to depict the player's name on the user interface. it needs to be freed with g_free.
//snafu_free will free snafu->players->name
//
//score is the snafu_player's score, and score_board is a GtkLabel 
//which will be used to display the score.  
//this is an optional feature, score_board may be set to NULL
typedef struct _snafu_player{
   guint8 direction;
   board_cell cell_value;
   guint x;
   guint y;
   guint _x;
   guint _y;
   gboolean alive;
   gboolean human;
   guint score;
   GtkWidget *score_board;
   gchar *name;
} snafu_player;


//the snafu represents a game through all stages of the game's lifetime
//
//number_players is the number of snafu_players playing the game of snafu
//players is an array of snafu_players
//play_area ia board which the game will be drawn to
//death_count is the number of snafu_players with alive set to FALSE
//frequency is in fact not a measure of frequency, rather a measure of 
//periodicty.  snafu_players will advance on the board every frequency 
//miliseconds
//
//active contains a gboolean indicating if a snafu is initialized and ready 
//to be start
//started contains a gboolean indicating if a game of snafu has already been
//started
//
//timeout_func_ref is a refference to the snafu timeout function.  the ref 
//is used in the event that a timeout needs to be cancelled for whatever reason
//
//snafu needs to be freed with snafu_free and players needs to be freed with 
//g_free.  snafu_free frees players.  play_area needs to be freed with 
//board_free
typedef struct _snafu{
   guint number_players;
   snafu_player *players;
   board *play_area;
   gboolean started;
   gboolean active;
   guint frequency;
   guint death_count;
   gint timeout_func_ref;
   GtkWidget *message_area;
} snafu;

/***
 *snafu_player functions
 *preface: all functions named snafu_player_* will either accept a snafu
 *    pointer and a snafu_player pointer or only a snafu_player pointer
 **/

//accepts a snafu_player_directions intended to contain at most two 
//   direction flags set
//if no flags are set, a random direction is returned
//if a single flag is set, it will return that direction
//if two flags are set, it will return one of those directions, 
//   randomly selected
//if more than two flags are set, it will return one of the outer-bit
//   directions.  inner bits will never be returned
snafu_player_direction snafu_player_direction_new(
   snafu_player_direction directions);

//returns an allocated snafu_player whith properties determined by
//the value of i
snafu_player snafu_player_new(snafu *game, gint i);

//this function is called at the end of a game, restoring a snafu_player
//to play in a new game
void snafu_player_end(snafu_player *player);

//returns an allocated string representing the value to be set to score_board
//the gchar* is a string to be used as markup to a GtkLabel
//return value should be freed with g_free
gchar *snafu_player_get_score_string(snafu_player *player);

//this function is called when a player's score is to be increased
//'score' in this case is used a verb and is not intended to 
//   refer to snafu_player->score
void snafu_player_score(snafu_player *player);

//sets the player's score to the value specified by score
void snafu_player_set_score(snafu_player *player, guint score);

//this function is called when a player dies
void snafu_player_die(snafu *game, snafu_player *player);

//this function is called when the next iteration of a game in progress occurs
//if human is set, the snafu_player will attempt to move in the direction
//    of direction and die if that cell is occupied on the board
//if human is not set, the snafu_player will use a crude ai to attempt to
//   change to a random direction if it detects a collision.  
//   if all directions fail, the snafu_player will die
void snafu_player_next(snafu *game, snafu_player *player);

/**
 *snafu functions
 *preface: all functions named snafu_* accept a snafu pointer as its 
 *   first parameter
 */

//called to end a game in progress
void snafu_end(snafu *game);

//called to go through the next iteration of a game in progress
gboolean snafu_next(snafu *game);

//accepts a gchar* which will be used on game->message_area, a GtkLabel, 
//if the label is not NULL
void snafu_display_message(snafu *game, gchar *message);

//score_board, a GTKBox, will be initialized with labels for each 
//snafu_player in players
void snafu_score_board_init(snafu *game, GtkWidget *score_board);

//called to start a game of snafu
void snafu_start(snafu *game);

//returns an allocated snafu pointer with number_players snafu_players and
//frequency used as a timeout interval for the game
snafu *snafu_new(board *play_area, guint number_players, guint frequency);

//frees a snafu, including snafu_players and snafu_player->names
//snafu->play_area still needs to be freed with board_free
void snafu_free(snafu *game);

/********/

snafu_player_direction snafu_player_direction_new(
   snafu_player_direction directions){
   snafu_player_direction direction = 0;

   if(!directions){
      return(
         (direction = g_random_int_range(1, 5)) && direction == 3?8:direction
      );
   }

   for(gint i = 0, j = g_random_int_range(0, 2);
      (j && ((direction = (directions & 1))) || 
         (direction = (directions & 128))) || TRUE;
      (i++, j?(directions >>= 1):(directions <<= 1))){
      if(direction){
        return(j?direction << i:direction >> i);
      }
   }
}

snafu_player snafu_player_new(snafu *game, gint i){
   snafu_player new_snafu_player;

   switch(i){
      case(0):{
         new_snafu_player.x = 6;
         new_snafu_player.y = 15;
         new_snafu_player.cell_value = board_cell_new_with_flags(1, 255, 0, 0);
         break;
      }
      case(1):{
         new_snafu_player.x = 39;
         new_snafu_player.y = 15;
         new_snafu_player.cell_value = board_cell_new_with_flags(1, 0, 0, 255);
         break;
      }
      case(2):{
         new_snafu_player.x = 22;
         new_snafu_player.y = 6;
         new_snafu_player.cell_value = board_cell_new_with_flags(1, 0, 80, 0);
         break;
      }
      case(3):{
         new_snafu_player.x = 22;
         new_snafu_player.y = 24;
         new_snafu_player.cell_value = board_cell_new_with_flags(
            1, 255, 0, 255
         );
         break;
      }
   } 

//if more than 4 players are desired, below is a crude algorithm to place
//players on the board with somewhat even distribution
//   new_snafu_player.x = 5 + (i * ((game->play_area->width - 10) / game->number_players));
//   new_snafu_player.y = game->play_area->height / 2;

   new_snafu_player._x = new_snafu_player.x;
   new_snafu_player._y = new_snafu_player.y;

   new_snafu_player.direction = snafu_player_direction_new(SNAFU_RANDOM);
   new_snafu_player.alive = TRUE;
   new_snafu_player.human = FALSE;
   new_snafu_player.score = 0;

   new_snafu_player.score_board = NULL;

   gint color_interval = 0xff / game->number_players;

//if more than 4 players are desired, below is a crude algorithm to color
//players on the board with somewhat even distribution
//   new_snafu_player.cell_value = board_cell_new_with_flags(1, ~(i * color_interval), (i * color_interval), ~(i * color_interval) >> 1);

   new_snafu_player.name = g_strdup_printf(
      "<b><span color='#%006X'>Player %d</span></b>",
       new_snafu_player.cell_value & (~BOARD_CELL_FLAGS_MASK), i + 1);

   return(new_snafu_player);
}

void snafu_player_end(snafu_player *player){
   player->x = player->_x;
   player->y = player->_y;

   player->direction = snafu_player_direction_new(SNAFU_RANDOM);

   player->alive = TRUE;
   player->human = FALSE;
}

gchar *snafu_player_get_score_string(snafu_player *player){
   return(g_strdup_printf("<b><span color='#%006X'>%d</span></b>",
      player->cell_value & (~BOARD_CELL_FLAGS_MASK), player->score));
}

void snafu_player_score(snafu_player *player){
   if(!player->alive){
      return;
   }

   player->score++;

   if(player->score_board == NULL){
      return;
   }

   gchar *score_string = snafu_player_get_score_string(player);

   gtk_label_set_markup(GTK_LABEL(player->score_board), score_string);

   g_free(score_string);
}

void snafu_player_set_score(snafu_player *player, guint score){
   player->score = score;

   if(player->score_board == NULL){
      return;
   }   

   gchar *score_string = snafu_player_get_score_string(player);

   gtk_label_set_markup(GTK_LABEL(player->score_board), score_string);

   g_free(score_string);   
}

void snafu_player_die(snafu *game, snafu_player *player){
   if(!player->alive){
      return;
   }

   player->alive = FALSE;
   game->death_count++;

   for(gint i = 0; i < game->number_players; i++){
      snafu_player_score(game->players + i);
   }

   gchar *message_string = g_strdup_printf("%s Dies!", player->name);

   snafu_display_message(game, message_string);

   g_free(message_string);
}

void snafu_player_next(snafu *game, snafu_player *player){
   if(!player->alive){
      return;
   }

   gint advance_x = 0, advance_y = 0;

   switch(player->direction){
      case(SNAFU_UP):{
         board_cell advance_cell = board_get_cell_flags(game->play_area, 
            advance_x = player->x, advance_y = player->y - 1);

         if(advance_cell){
            if(player->human){
               snafu_player_die(game, player);
               break;
            }

            gint random_direction = g_random_int_range(0, 2)?1:-1;

            if(advance_cell = board_get_cell_flags(game->play_area, 
               advance_x = player->x + random_direction, 
               advance_y = player->y)){
               if(advance_cell = board_get_cell_flags(game->play_area, 
                  advance_x = player->x - random_direction, 
                  advance_y = player->y)){
                  snafu_player_die(game, player);
               }else{
                  player->direction = random_direction > 0?
                     SNAFU_LEFT:SNAFU_RIGHT;
               }
            }else{
               player->direction = random_direction > 0?SNAFU_RIGHT:SNAFU_LEFT;
            }
         }

         break;
      }
      case(SNAFU_DOWN):{
         board_cell advance_cell = board_get_cell_flags(game->play_area, 
            advance_x = player->x, advance_y = player->y + 1);

         if(advance_cell){
            if(player->human){
               snafu_player_die(game, player);
               break;
            }

            gint random_direction = g_random_int_range(0, 2)?1:-1;

            if(advance_cell = board_get_cell_flags(game->play_area, 
               advance_x = player->x + random_direction, 
               advance_y = player->y)){
               if(advance_cell = board_get_cell_flags(game->play_area, 
                  advance_x = player->x - random_direction, 
                  advance_y = player->y)){
                  snafu_player_die(game, player);
               }else{
                  player->direction = random_direction > 0?
                     SNAFU_LEFT:SNAFU_RIGHT;
               }
            }else{
               player->direction = random_direction > 0?SNAFU_RIGHT:SNAFU_LEFT;
            }
         }

         break;
      }
      case(SNAFU_LEFT):{
         board_cell advance_cell = board_get_cell_flags(game->play_area, 
            advance_x = player->x - 1, advance_y = player->y);

         if(advance_cell){
            if(player->human){
               snafu_player_die(game, player);
               break;
            }

            gint random_direction = g_random_int_range(0, 2)?1:-1;

            if(advance_cell = board_get_cell_flags(game->play_area, 
               advance_x = player->x, 
               advance_y = player->y + random_direction)){
               if(advance_cell = board_get_cell_flags(game->play_area, 
                  advance_x = player->x, 
                  advance_y = player->y - random_direction)){
                  snafu_player_die(game, player);
               }else{
                  player->direction = random_direction > 0?SNAFU_UP:SNAFU_DOWN;
               }
            }else{
               player->direction = random_direction > 0?SNAFU_DOWN:SNAFU_UP;
            }
         }

         break;
      }
      case(SNAFU_RIGHT):{
         board_cell advance_cell = board_get_cell_flags(game->play_area, 
            advance_x = player->x + 1, advance_y = player->y);

         if(advance_cell){
            if(player->human){
               snafu_player_die(game, player);
               break;
            }

            gint random_direction = g_random_int_range(0, 2)?1:-1;

            if(advance_cell = board_get_cell_flags(game->play_area, 
               advance_x = player->x, 
               advance_y = player->y + random_direction)){
               if(advance_cell = board_get_cell_flags(game->play_area, 
                  advance_x = player->x, 
                  advance_y = player->y - random_direction)){
                  snafu_player_die(game, player);
               }else{
                  player->direction = random_direction > 0?SNAFU_UP:SNAFU_DOWN;
               }
            }else{
               player->direction = random_direction > 0?SNAFU_DOWN:SNAFU_UP;
            }
         }

         break;
      }
      default:{
         player->direction = snafu_player_direction_new(SNAFU_RANDOM);
         snafu_player_next(game, player);
         return;
      }
   }

   if(!player->alive){
      return;
   }

   player->x = advance_x;
   player->y = advance_y;

   board_set_cell(game->play_area, advance_x, advance_y, player->cell_value);
}

void snafu_end(snafu *game){
   game->active = FALSE;
   game->started = FALSE;

   game->death_count = 0;

   g_source_remove(game->timeout_func_ref);

   board_clear(game->play_area, TRUE);

   for(gint i = 0; i < game->number_players; i++){
      snafu_player_end(game->players + i);
   }
   
}

gboolean snafu_next(snafu *game){
   if(!game->active){
      return(FALSE);
   }

   for(gint i = 0; i < game->number_players; i++){
      snafu_player_next(game, (game->players + i)); 
   }

   if(game->death_count >= game->number_players - 1){
      game->active = FALSE;

      if(game->death_count == game->number_players){
         snafu_display_message(game, "No winner!");
      }else{
         for(gint i = 0; i < game->number_players; i++){
            if((game->players + i)->alive){
               gchar *message_string = g_strdup_printf("%s Wins!", 
                  (game->players + i)->name);

               snafu_display_message(game, message_string);

               g_free(message_string);
            }
         }
      }
   }

   board_incremental_draw(game->play_area);

   return(game->active);
}

void snafu_score_board_init(snafu *game, GtkWidget *score_board){
   for(gint i = 0; i< game->number_players; i++){
      gchar *score_string = snafu_player_get_score_string(game->players + i);

      GtkWidget *score_label = gtk_label_new(NULL);
      gtk_label_set_markup(GTK_LABEL(score_label), score_string);

      g_free(score_string);

      (game->players + i)->score_board = score_label;

      gtk_container_add(GTK_CONTAINER(score_board), score_label);
   }
}

void snafu_display_message(snafu *game, gchar *message){
   if(game->message_area == NULL){
      return;
   }

   gtk_label_set_markup(GTK_LABEL(game->message_area), message);
}

void snafu_start(snafu *game){
   if(game->started){
      return;
   }

   for(gint i = 0; i < game->number_players; i++){
      board_set_cell(game->play_area, (game->players + i)->x, 
         (game->players + i)->y, (game->players + i)->cell_value);
   }

   game->started = TRUE;
   game->active = TRUE;

   snafu_display_message(game, "<b>GO!</b>");

   game->timeout_func_ref = g_timeout_add(game->frequency, 
      (GSourceFunc) snafu_next, game);
}

snafu *snafu_new(board *play_area, guint number_players, guint frequency){
   snafu *new_snafu = g_new(snafu, 1);

   new_snafu->play_area = play_area;
   new_snafu->number_players = number_players;
   new_snafu->frequency = frequency;

   new_snafu->started = FALSE;
   new_snafu->active = FALSE;

   new_snafu->death_count = 0;

   new_snafu->players = g_new(snafu_player, number_players);

   board_clear(play_area, FALSE);

   for(gint i = 0; i < number_players; i++){
      *(new_snafu->players + i) = snafu_player_new(new_snafu, i);
   }

   new_snafu->message_area = NULL;

   return(new_snafu);
}

void snafu_free(snafu *game){
   for(gint i = 0; i < game->number_players; i++){
      g_free((game->players + i)->name);
   }

   g_free(game->players);

   g_free(game);
}
