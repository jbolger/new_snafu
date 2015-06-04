/******************************************************************************
Title         : New SNAFU
Author        : John Bolger
Created on    : December 10, 2011
Description   : An implimentation of the 1981 game SNAFU.  User can adjust spe-
                ed of players, causing the players' scores to be reset.  Playe-
                rs constantly increase in size, limitting the amount of availa-
                ble spaces on the board.  When a player collides with itself, 
                the board boundary, or another player, the player dies, increa-
                sing the score of each living player by 1.  The last living pl-
                ayer wins.
Pupose        : To impliment the game of SNAFU in a way which is cross-platfor-
                m and offer a subset of options and conditions which are not f-
                ound in the 1981 original.
Usage         : Press play to start a game.  Control of Player 1 is determined
                by the Up/Down/Left/Right keys on the keyboard.  Control of pl-
                ayer 2 is determined by the W/S/A/D keys on the keyboard.  Bot-
                h human controllable players will be controlled by ai until ap-
                propriate input is detected
Build with    : gcc -o snafu -std=c99 -Wall -g `pkg-config --cflags \
   --libs gtk+-2.0` main.c
Modifications :
******************************************************************************/
   
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <cairo.h>
#include "board.h"
#include "snafu.h"

#define PADDING 25

#define BOARD_WIDTH 45
#define BOARD_HEIGHT 30
#define BOARD_CELL_HEIGHT 15
#define BOARD_CELL_WIDTH 15

#define FREQUENCY 85
#define NUMBER_PLAYERS 4

#define FREQUENCY_MAX 500
#define FREQUENCY_MIN 2
 
//the game of snafu
//global for convinience purposes
static snafu *game;

//the slider which determines play speed,
//global for convinience purposes
static GtkWidget *speed_slider;

//destroy signal handler
static void destroy(GtkWidget*, gpointer);

//returns a GtkWindow pointer of type type with title title
GtkWidget *gtk_window_new_init(GtkWindowType type, gchar *title);

//returns a GtkButton pointer with label label, mnemonic is mnemonic is TRUE
GtkWidget *gtk_button_new_init(gchar *label, gboolean mnemonic);

//keyboard key-press-event signal handler
//will change the direction of player 1 or player 2 if input is detected
static gboolean keyboard_press(GtkWidget *widget, GdkEventKey *event);

//resets the score for all snafu_players of game
void score_reset(snafu *game);

//score reset button clicked signal handler
static void score_reset_button_press(GtkButton *button, snafu *game);

//start button clicked signal handler
//will start a game of snafu using game, ends a game in progress if there is one
//will reset score if the speed slider bares a new value
static void start_button_press(GtkButton *button, snafu *game);

//main function
int main (int argc, char *argv[]){
   gtk_init(&argc, &argv);

   //get window and initialize
   GtkWidget *window = gtk_window_new_init(GTK_WINDOW_TOPLEVEL,
     "New Snafu");
   gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
   gtk_container_set_border_width(GTK_CONTAINER(window), PADDING);

   GdkColor widget_bg_color;
   gdk_color_parse("#00ff00", &widget_bg_color);
   gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &widget_bg_color);

   //connect window to destroy signal
   g_signal_connect(G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);

   //create containing vbox
   GtkWidget *vbox = gtk_vbox_new(FALSE, PADDING);

   //create board + play area widget + snafu
   GtkWidget *drawing_area = gtk_event_box_new();
   gtk_widget_set_size_request(drawing_area, BOARD_WIDTH * BOARD_CELL_WIDTH, 
      BOARD_HEIGHT * BOARD_CELL_HEIGHT);

   board *brd = board_new(drawing_area, BOARD_WIDTH, BOARD_HEIGHT, 
      BOARD_CELL_WIDTH, BOARD_CELL_HEIGHT, board_cell_new_with_color(128, 128, 
      128));

   game = snafu_new(brd, NUMBER_PLAYERS, FREQUENCY);

   //create score board
   GtkWidget *score_board = gtk_event_box_new();
   GtkWidget *score_board_hbox = gtk_hbox_new(TRUE, PADDING);

   gtk_container_add(GTK_CONTAINER(score_board), score_board_hbox);

   gdk_color_parse("#808080", &widget_bg_color);
   gtk_widget_modify_bg(score_board, GTK_STATE_NORMAL, &widget_bg_color);

   snafu_score_board_init(game, score_board_hbox);

   //create button hbox
   GtkWidget *buttons_hbox = gtk_hbox_new(FALSE, PADDING);

   //start button
   GtkWidget *start_button = gtk_button_new_init("Play", FALSE);
 
   g_signal_connect(start_button, "clicked", G_CALLBACK(start_button_press), 
      game);

   //message label
   GtkWidget *message_label = gtk_label_new(
      "Adjust speed below.  Press Play to start!");   
   game->message_area = message_label;

   //play speed slider
   speed_slider = gtk_hscale_new_with_range(FREQUENCY_MIN, FREQUENCY_MAX, 50);

   gtk_range_set_value(GTK_RANGE(speed_slider), FREQUENCY);
   gtk_scale_add_mark(GTK_SCALE(speed_slider), FREQUENCY, GTK_POS_BOTTOM, 
      "Default Speed");

   //score reset button
   GtkWidget *score_reset_button = gtk_button_new_init("Reset Score", FALSE);
 
   g_signal_connect(score_reset_button, "clicked", 
      G_CALLBACK(score_reset_button_press), game);

   //attach buttons_hbox
   gtk_box_pack_start(GTK_BOX(buttons_hbox), start_button, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(buttons_hbox), message_label, TRUE, TRUE, PADDING);

   gtk_box_pack_start(GTK_BOX(buttons_hbox), score_reset_button, FALSE, FALSE,
      0);

   //accept user keyboard input
   g_signal_connect(window, "key-press-event", G_CALLBACK(keyboard_press), 
      NULL);

   //attach vbox
   gtk_container_add(GTK_CONTAINER(vbox), drawing_area);
   gtk_container_add(GTK_CONTAINER(vbox), score_board);
   gtk_container_add(GTK_CONTAINER(vbox), buttons_hbox);
   gtk_container_add(GTK_CONTAINER(vbox), speed_slider);

   //attach window
   gtk_container_add(GTK_CONTAINER(window), vbox);

   gtk_widget_show_all(window);

   gtk_main();

   return(0);
}

static void destroy(GtkWidget *window, gpointer data){
   gtk_main_quit ();
}

GtkWidget *gtk_window_new_init(GtkWindowType type, gchar *title){

   GtkWidget *window = gtk_window_new(type);
   *title?gtk_window_set_title(GTK_WINDOW (window), title):NULL;

   return(window);
}

GtkWidget *gtk_button_new_init(gchar *label, gboolean mnemonic){
   return(mnemonic?gtk_button_new_with_mnemonic(label):
      gtk_button_new_with_label(label));
}

static gboolean keyboard_press(GtkWidget *widget, GdkEventKey *event){
   if(!game->started){
      return(FALSE);
   }

   if(event->type != GDK_KEY_PRESS){
      return(TRUE);
   }

   switch(event->keyval){
      case(GDK_KEY_Up):{
         game->players->human || (game->players->human = TRUE);
         game->players->direction != SNAFU_DOWN && 
            (game->players->direction = SNAFU_UP);
         break;
      }
      case(GDK_KEY_Down):{
         game->players->human || (game->players->human = TRUE);
         game->players->direction != SNAFU_UP && 
            (game->players->direction = SNAFU_DOWN);
         break;
      }
      case(GDK_KEY_Right):{
         game->players->human || (game->players->human = TRUE);
         game->players->direction != SNAFU_LEFT && 
            (game->players->direction = SNAFU_RIGHT);
         break;
      }
      case(GDK_KEY_Left):{
         game->players->human || (game->players->human = TRUE);
         game->players ->direction != SNAFU_RIGHT && 
            (game->players->direction = SNAFU_LEFT);
         break;
      }
      case(GDK_KEY_w):
      case(GDK_KEY_W):{
         (game->players + 1)->human || ((game->players + 1)->human = TRUE);
         (game->players + 1)->direction != SNAFU_DOWN && 
            ((game->players + 1)->direction = SNAFU_UP);
         break;
      }
      case(GDK_KEY_s):
      case(GDK_KEY_S):{
         (game->players + 1)->human || ((game->players + 1)->human = TRUE);
         (game->players + 1)->direction != SNAFU_UP && 
            ((game->players + 1)->direction = SNAFU_DOWN);
         break;
      }
      case(GDK_KEY_d):
      case(GDK_KEY_D):{
         (game->players + 1)->human || ((game->players + 1)->human = TRUE);
         (game->players + 1)->direction != SNAFU_LEFT && 
            ((game->players + 1)->direction = SNAFU_RIGHT);
         break;
      }
      case(GDK_KEY_a):
      case(GDK_KEY_A):{
         (game->players + 1)->human || ((game->players + 1)->human = TRUE);
         (game->players + 1)->direction != SNAFU_RIGHT && 
            ((game->players + 1)->direction = SNAFU_LEFT);
         break;
      }
      default:{

      }
   }

   return(TRUE);
}

void score_reset(snafu *game){
   for(gint i = 0; i < game->number_players; i++){
      snafu_player_set_score(game->players + i, 0);
   }
}

static void score_reset_button_press(GtkButton *button, snafu *game){
   score_reset(game);
}

static void start_button_press(GtkButton *button, snafu *game){
   static guint last_speed = 0;
   guint _last_speed = 0;
   
   if(game->started){
      snafu_end(game);
   }

   if(last_speed != 
      (_last_speed = gtk_range_get_value(GTK_RANGE(speed_slider)))){

      gtk_scale_clear_marks(GTK_SCALE(speed_slider));

      last_speed = _last_speed;

      game->frequency = last_speed;

      score_reset(game);

      gtk_scale_add_mark(GTK_SCALE(speed_slider), last_speed, GTK_POS_BOTTOM, "Current Speed");
   }

   snafu_start(game);
}

