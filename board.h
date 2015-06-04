//symbolic constants used with board and board_cell

//masks used to isolate the individual components of a board_cell
#define BOARD_CELL_FLAGS_MASK 0xff000000
#define BOARD_CELL_RED_MASK 0x00ff0000
#define BOARD_CELL_GREEN_MASK 0x0000ff00
#define BOARD_CELL_BLUE_MASK 0x000000ff

//the value returned at sometimes to indicate that an (x, y) 
//coordinate is out of bounds on a board
#define BOARD_CELL_OUT_OF_BOUNDS 0xffffffff

////////////
//typedefs//
////////////

//the board_cell is the singular unit of the grid represented by the board
//
//board_cell is a four byte integer with 4 8-bit regions representing 
//the contents of the cell
//
//11111111 00000000 00000000 00000000 represents the cell's flags
//  the flags determine the color-blind contents of the grid cell
//  a cell with no flags is considered empty, however empty cells 
//  may still be drawn with a color as defined by the next 12 bits
//   of the board_cell
//
//00000000 11111111 11111111 11111111
//  each 8-bit region represents the r, g and b components of a color
//  in the common 24-bit hexadecimal format
//  at some parts in this code, a single r, g or b component is referred
//  to as board_cell_color
//
//manual bit manipulation of board_cells is not generally required, as
//   there are several functions which make board_cells easy to use
//sometimes manual bit manipulation is desired however rarely necesary
typedef guint32 board_cell;

//the board representing a grid of board_cells
//
//cells is a board_cell array which represent a grid with dimensions 
//width and height
//boards can be drawn using various functions.  drawing occurs on 
//widget.  when a cell is cleared it is drawn with background_color
//to facilitate faster drawing, changed_cells is a colon-delimited 
//gchar* of indices to cells, which are redrawn selectively by an 
//incremental drawing function
//
//changed_cells is an alocated string and must be freed with g_free
//boards must be freed with board_free.  board_free also frees changed_cells
typedef struct _board {
   gint height;  //height of grid
   gint width;   //width of grid
   
   gint cell_height; //height in pixels of an individual cell
   gint cell_width;  //width in pixels of an individual cell
   
   GtkWidget *widget; //the widget to use to draw on
 
   board_cell *cells; //the grid of cells

   board_cell background_color; //the color to set cleared cells to

   gchar *changed_cells; //a colon delimited list of cells 
                         //to facilitate faster drawing
} board;


/****
 *board_cell functions
 *preface:  board_cells are never allocated and never need to be freed. 
 ****/

//returns a board_cell set up without flags
//red is an integer between 0x00 and 0xff representing the red component
//green is an integer between 0x00 and 0xff representing the green component
//blue is an integer between 0x00 and 0xff representing the blue component
board_cell board_cell_new_with_color(guint8 red, guint8 green, guint8 blue);

//returns a board_cell set up with flags
//flags is an integer between 0x00 and 0xff representing the 
//flags component of the board_cell
board_cell board_cell_new_with_flags(guint8 flags, guint8 red, guint8 green,
   guint8 blue);

//returns the gfloat equivalent of a particular board_cell_color,
//which can be passed to gdk color functions
gfloat board_cell_color_get_float(guint8 board_cell_color);

//accepts a board_cell and three gfloat pointers
//the rgb values of board_cell will be mapped to gfloats r, g and b respectively
void board_cell_get_rgb(board_cell cell, gfloat *r, gfloat *g, gfloat *b);
/****
 *board functions
 *preface:  all functions named board_* accept board pointer brd as 
 *   the first parameter
 *   new boards must be freed with board_free
 ****/

//accepts x and y coordinates and returns true if the coordinates are 
//within the bounds of brd
gboolean board_check_coords_in_bounds(board *brd, gint x, gint y);
//appends the cell coordinates (x, y) to brd->changed_cells, marking the
//cells for redrawing
void board_mark_cell_changed(board *brd, gint x, gint y);

//changes board_cell (x, y) to value
void board_set_cell(board *brd, gint x, gint y, board_cell value);

//clears board_cell (x, y) including the color
void board_clear_cell(board *brd, gint x, gint y);

//clears board_cell (x, y) however leaving behind the color
void board_clear_cell_leave_color(board *brd, gint x, gint y);

//same as board_set_cell however board_cell (x, y) is not marked changed
void board_set_cell_dont_mark_changed(board *brd, gint x, gint y,
   board_cell value);

//same as board_clear_cell however board_cell (x, y) is not marked changed
void board_clear_cell_dont_mark_changed(board *brd, gint x, gint y);

//same as board_clear_cell_leave_color however board_cell (x, y) is not 
//marked changed
void board_clear_cell_leave_color_dont_mark_changed(board *brd, gint x, gint y);

//returns a pointer to board_cell (x, y)
//remember to call board_mark_cell_changed(brd,x,y) if you modify this pointer!
board_cell *board_get_cell(board *brd, gint x, gint y);

//returns a copy of board_cell (x, y)
board_cell board_get_cell_copy(board *brd, gint x, gint y);

//returns only the flags of board_cell (x, y)
board_cell board_get_cell_flags(board *brd, gint x, gint y);
//returns only the color of board_cell (x, y)
board_cell board_get_cell_color(board *brd, gint x, gint y);

//accepts a cairo_t as its first parameter and uses it to draw 
//board_cell (x, y) to brd
void board_draw_cell_with_cairo_t(board *brd, cairo_t *cr, gint x, gint y);

//same as board_draw_cell_with_cairo_t except it does not accept a 
//cairo_t to draw to brd
//it creates a new cairo_t and destroys it each time
//this function is not recomended for repetative draws
void board_draw_cell(board *brd, gint x, gint y);

//draws only the cells marked changed in brd->changed_cells
//allows for the board to be incrementally redrawn as opposed 
//to redrawn from scratch
//recomended for frequent board drawing, it is much faster than 
//complete board redraws
//if brd goes off screen, it will need to be completely redrawn
//this function will cause incomplete board renderrings in that case 
void board_incremental_draw(board *brd);

//draws the complete board
//recomended for use when every cell in the board needs to be drawn, 
//such as on expose
//not recomended for frequent draws, such as animation
void board_draw(board *brd);

//clears an entire board, also redrawing it if draw_after is set to TRUE
void board_clear(board *brd, gboolean draw_after);

//same as board clear however only the flags of cells will be cleared
void board_clear_leave_color(board *brd, gboolean draw_after);
//debug function
void board_dump(board *brd);

//callback for expose events
void board_expose(board *brd, GtkWidget *drawing_area);

//returns a pointer to an allocated board
//accepts a widget which will be used to draw the board on
//width and height of the board
//width and heigh in pixels of individual cells
//the background color to apply to cleared cells
//returned board must be freed with board_free
board *board_new(GtkWidget *widget, gint width, gint height, gint cell_height, 
   gint cell_width, board_cell background_color);

//frees the allocated board
void board_free(board *brd);

/********/

board_cell board_cell_new_with_color(guint8 red, guint8 green, guint8 blue){
   board_cell new_board_cell = red;
   new_board_cell <<= 8;
   new_board_cell |= green;
   new_board_cell <<= 8;
   new_board_cell |= blue;

   return(new_board_cell);
}

board_cell board_cell_new_with_flags(guint8 flags, guint8 red, guint8 green, 
   guint8 blue){
   board_cell new_board_cell = flags;
   new_board_cell <<= 8;
   new_board_cell |= red;
   new_board_cell <<= 8;
   new_board_cell |= green;
   new_board_cell <<= 8;
   new_board_cell |= blue;

   return(new_board_cell);
}

gfloat board_cell_color_get_float(guint8 board_cell_color){
   return(board_cell_color / 256.0);
}

void board_cell_get_rgb(board_cell cell, gfloat *r, gfloat *g, gfloat *b){
   *r = board_cell_color_get_float((cell & BOARD_CELL_RED_MASK) >> 16);
   *g = board_cell_color_get_float((cell & BOARD_CELL_GREEN_MASK) >> 8);
   *b = board_cell_color_get_float(cell & BOARD_CELL_BLUE_MASK);
}

gboolean board_check_coords_in_bounds(board *brd, gint x, gint y){
   if((x < 0) || (y < 0) || (x >= brd->width) || (y >= brd->height)){
      return(FALSE);
   }

   return(TRUE);
}

void board_mark_cell_changed(board *brd, gint x, gint y){
   gchar *cell_string = g_strdup_printf(":%d", (brd->width * y) + x);

   brd->changed_cells = g_strconcat(brd->changed_cells, cell_string, NULL);

   g_free(cell_string);
}

void board_set_cell(board *brd, gint x, gint y, board_cell value){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   *(brd->cells + ((brd->width * y) + x)) = value;

   board_mark_cell_changed(brd, x, y);
}

void board_clear_cell(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   *(brd->cells + ((brd->width * y) + x)) = 
      (brd->background_color & (~BOARD_CELL_FLAGS_MASK));

   board_mark_cell_changed(brd, x, y);
}

void board_clear_cell_leave_color(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   *(brd->cells + ((brd->width * y) + x)) &= (~BOARD_CELL_FLAGS_MASK);

   board_mark_cell_changed(brd, x, y);
}

void board_set_cell_dont_mark_changed(board *brd, gint x, gint y, 
   board_cell value){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   *(brd->cells + ((brd->width * y) + x)) = value;
}

void board_clear_cell_dont_mark_changed(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   *(brd->cells + ((brd->width * y) + x)) = 
      (brd->background_color & (~BOARD_CELL_FLAGS_MASK));
}

void board_clear_cell_leave_color_dont_mark_changed(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   *(brd->cells + ((brd->width * y) + x)) &= (~BOARD_CELL_FLAGS_MASK);
}

board_cell *board_get_cell(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return(NULL);
   }

   return(brd->cells + ((brd->width * y) + x));
}

board_cell board_get_cell_copy(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return(BOARD_CELL_OUT_OF_BOUNDS);
   }

   return(*(brd->cells + ((brd->width * y) + x)));
}

board_cell board_get_cell_flags(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return(BOARD_CELL_OUT_OF_BOUNDS);
   }

   return(*(brd->cells + ((brd->width * y) + x)) & BOARD_CELL_FLAGS_MASK);
}

board_cell board_get_cell_color(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return(BOARD_CELL_OUT_OF_BOUNDS);
   }

   return(*(brd->cells + ((brd->width * y) + x)) & (~BOARD_CELL_FLAGS_MASK));
}

void board_draw_cell_with_cairo_t(board *brd, cairo_t *cr, gint x, gint y){
   gfloat r, g, b;
   board_cell_get_rgb(*(brd->cells + ((brd->width * y) + x)), &r, &g, &b);   

   cairo_set_source_rgb(cr, r, g, b);

   cairo_rectangle(cr, x * brd->cell_width, y * brd->cell_height, 
      brd->cell_width, brd->cell_height);

   cairo_fill(cr);
}

void board_draw_cell(board *brd, gint x, gint y){
   if(!board_check_coords_in_bounds(brd, x, y)){
      return;
   }

   cairo_t *cr = gdk_cairo_create(brd->widget->window);

   board_draw_cell_with_cairo_t(brd, cr, x, y);

   cairo_destroy(cr);
}

void board_incremental_draw(board *brd){
   cairo_t *cr = gdk_cairo_create(brd->widget->window);

   gchar **changed_cells = g_strsplit(brd->changed_cells, ":", 0);

   for(gint i = 1; *(changed_cells + i) != NULL; i++){
      gint cell_number = g_ascii_strtoll(*(changed_cells + i), NULL, 10);

      board_draw_cell_with_cairo_t(brd, cr, cell_number % brd->width, 
         cell_number / brd->width);
   }

   cairo_destroy(cr);

   g_strfreev(changed_cells);

   g_free(brd->changed_cells);

   brd->changed_cells = g_strdup("");
}

void board_draw(board *brd){
   cairo_t *cr = gdk_cairo_create(brd->widget->window);

   for(gint i = 0; i < (brd->width * brd->height); i++){
      board_draw_cell_with_cairo_t(brd, cr, i % brd->width, i / brd->width);
   }

   cairo_destroy(cr);

   g_free(brd->changed_cells);

   brd->changed_cells = g_strdup("");
}

void board_clear(board *brd, gboolean draw_after){
   for(gint i = 0; i < (brd->width * brd->height); i++){
      board_clear_cell_dont_mark_changed(brd, i % brd->width, i / brd->width);
   }

   if(draw_after){
      board_draw(brd);
   }
}

void board_clear_leave_color(board *brd, gboolean draw_after){
   for(gint i = 0; i < (brd->width * brd->height); i++){
      board_clear_cell_leave_color_dont_mark_changed(brd, i / brd->width, 
         i % brd->width);
   }

   if(draw_after){
      board_draw(brd);
   }
}

void board_dump(board *brd){
/*   g_printf("\nheight: %d\nwidth: %d\ncell_height: %d\ncell_width: %d\n", 
        brd->height, brd->width, brd->cell_height, brd->cell_width);*/
}

void board_expose(board *brd, GtkWidget *drawing_area){
   board_draw(brd);
}

board *board_new(GtkWidget *widget, gint width, gint height, gint cell_height, 
   gint cell_width, board_cell background_color){
   board *new_board = g_new(board, 1);

   new_board->widget = widget;

   new_board->height = height;
   new_board->width = width;

   new_board->cell_height = cell_height;
   new_board->cell_width = cell_width;

   new_board->cells = g_new(board_cell, height * width);

   new_board->background_color = background_color & (~BOARD_CELL_FLAGS_MASK);

   board_clear(new_board, FALSE);

   new_board->changed_cells = g_strdup("");

   g_signal_connect_swapped(widget, "expose-event", G_CALLBACK(board_expose), 
      new_board);

   return(new_board);   
}

void board_free(board *brd){
   g_free(brd->cells);

   g_free(brd->changed_cells);

   g_free(brd);
}

