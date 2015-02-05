#include <math.h>
#include <pebble.h>
//mine
#define VOR_CELLS 15
#define FAT 1
#define WID 144
#define HEI 168
#define DIA 222

float speed = 1;
float vor_on = 0;
float border = 150;
float min_distances[2];// = (float *)(malloc(sizeof(float)) * 2);
//min_distances[0] = float(DIA);
//min_distances[1] = float(DIA);
int cell_on = 0;
int sec;
int min;
int hou;
   
static Window *window;
static Layer *canvas;
static AppTimer *timer;
const ResHandle *f_handle;
static uint32_t delta = 33;
static int visible_stars = 0;
static char* time_text;

struct Cell {
  int x;
  int y;
  int id;
  int radius;
  bool is_on;
};
  
struct Cell *cells[VOR_CELLS];

float my_pow( float num )
{
  return num * num;
};

float my_abs( float num )
{
  if (num < 0){return -num;}
  else{return num;}
};

static void applog(const char* message)
{
  app_log(APP_LOG_LEVEL_INFO, "renderer.c", 0, message);
}

static void setup() {
  //sec_x = 77;
  //sec_y = 0;
  //speed = 1;
  //vor_on = 0;
  //border = 1;
  //min_distances = (float *)(malloc(sizeof(float)) * 2);
  min_distances[0] = DIA;
  min_distances[1] = DIA;
  //cell_on = 0;
}

/********************************** Cell Lifecycle *****************************/
static struct Cell* create_cell(int iid)
{
  struct Cell *this = malloc(sizeof(struct Cell));
  this->x = rand() % 144;
  this->y =  rand() % 168;
  this->id = iid;
  this->is_on = true;
  return this;
}

float distance(float x1, float y1, float x2, float y2)
{
  return my_pow(x1 - x2)  + my_pow(y1 - y2);
}

static void create_cells()
{
  for(int i = 0; i < VOR_CELLS; i++)
  {
    cells[i] = create_cell(i);
    vor_on ++;
  }
}

static void turn_off(int i)
{
  if (cells[i]->is_on == true)
  {
    cells[i]->is_on = false; 
  }
}

static void turn_on(int i)
{
  if (cells[i]->is_on == false)
  {
    cells[i]->x = rand() % 144;
    cells[i]->y = rand() % 168;
    cells[i]->is_on = true; 
  }
}

static void destroy_cells()
{
  int i = 0;
  for(int i = 0; i < VOR_CELLS; i++)
  {
    free(cells[i]);    
  }
}
/*
static void update_position(int i)
{
  //Update all existing
  if (cells[i]->is_on == true)
  {
    float disttemp;
    float dist = DIA * DIA;
    float distx = WID * WID;
    float disty = HEI * HEI;
    bool moved = false;
    
    if (cells[i]->x - cells[i]->radius  < 1){
      cells[i]->x = cells[i]->radius + 1;
      moved = true;
    }
    if (cells[i]->x + cells[i]->radius > WID){
      cells[i]->x = WID - cells[i]->radius - 1; 
      moved = true;
    }
    if (cells[i]->y - cells[i]->radius < 1){
      cells[i]->y = cells[i]->radius + 1;
      moved = true;
    }
    if (cells[i]->y + cells[i]->radius > HEI){
      cells[i]->y = HEI - cells[i]->radius - 1;
      moved = true;
    }   
    //varredura para encontrar ponto mais proximo
    if (moved == false)
    {
      for (int j=0; j < VOR_CELLS; j++)
      {
        if (i != j){
          disttemp = distance(cells[i]->x, cells[j]->x, cells[i]->y, cells[j]->y);
          if (disttemp < dist)
          {
            dist = disttemp;
            distx = cells[i]->x - cells[j]->x;
            disty = cells[i]->y - cells[j]->y;
          }
        }
      }
      //repulsion closest point
      float power = (1 - (dist / DIA)) * FAT; 
      float powerx = (distx/dist) * power;
      float powery = (disty/dist) * power;
      
      cells[i]->x += powerx; 
      cells[i]->y += powery;
    }
  }
}
*/

static void update()
{
  if((vor_on == 0) || (vor_on >= VOR_CELLS))
  {
    for (int c = 2; c < VOR_CELLS; c ++){
      turn_off(c);
    }
  }
  else if ((vor_on > 0) && (vor_on < VOR_CELLS))
  {
    turn_on(vor_on);
  }   
}


static void evaluate_pixel(float x, float y)
{
  float temp_dist = 0;
  
  min_distances[0] = DIA * DIA;
  min_distances[1] = DIA * DIA;

  if (vor_on > 0){
    for (int d = 0; d < vor_on; d++){
      temp_dist = distance(cells[d]->x, cells[d]->y, x, y);
      if (temp_dist < min_distances[0]){
        min_distances[0] = temp_dist;
      }
      else if(temp_dist < min_distances[1]){
        min_distances[1] = temp_dist;
      }
    }
  }
}

static void draw_pixels(GContext * ctx){
  for (int x = 0; x < WID; x += 2){
    for (int y = 0; y < HEI; y += 2){
      evaluate_pixel(x, y);
      if (my_abs(min_distances[0] - min_distances[1]) < border){
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_draw_pixel (ctx, GPoint(x, y));
        graphics_draw_pixel (ctx, GPoint(x + 1, y));
        graphics_draw_pixel (ctx, GPoint(x + 1, y + 1));
        graphics_draw_pixel (ctx, GPoint(x, y + 1));
      }
    }
  }
}


/****************************** Renderer Lifecycle *****************************/

/*
* Render re-scheduling
*/
static void timer_callback(void *data) 
{
  //Do logic
  //update();
  //Render
  layer_mark_dirty(canvas);
  //Register next render
  timer = app_timer_register(delta, (AppTimerCallback) timer_callback, 0);
}
/*
* Start rendering loop
*/
static void start()
{
  timer = app_timer_register(delta, (AppTimerCallback) timer_callback, 0);
}
/*
* Rendering
*/
static void render(Layer *layer, GContext* ctx)
{
  //Setup
  draw_pixels(ctx);
  //Draw time
}
/****************************** Window Lifecycle *****************************/
static void set_time_display(struct tm *t)
{
  int size = sizeof("00:00");
  //Time string
  if(time_text == NULL)
  {
    time_text = malloc(size);
  }
  if(clock_is_24h_style())
  {
    strftime(time_text, size, "%H:%M", t);
  }
  else
  {
    strftime(time_text, size, "%I:%M", t);
  }
  // text_layer_set_text(colonLayer, hour_text);
}

static void window_load(Window *window)
{
  //Setup window
  window_set_background_color(window, GColorWhite);
  APP_LOG(APP_LOG_LEVEL_INFO, "I am here");
  //Get Font
  //f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_38);
  //Setup canvas
  canvas = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(canvas, (LayerUpdateProc) render);
  layer_add_child(window_get_root_layer(window), canvas);
  create_cells();
  //Set initial time so display isn't blank
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);
  //Works to here
  set_time_display(t);
  //Start rendering
  
  start();
}
static void window_unload(Window *window)
{
  //Cancel timer
  app_timer_cancel(timer);
  //Destroy canvas
  layer_destroy(canvas);
}
/****************************** App Lifecycle *****************************/
static void tick_handler(struct tm *t, TimeUnits units_changed)
{
  
  vor_on ++;
  if (vor_on >= VOR_CELLS) { vor_on = 0;}
  update();
}

static void init(void) 
{
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  //Prepare stars memory
  //Tick tock
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
  //Finally
  window_stack_push(window, true);
}

static void deinit(void)
  {
  //De-init stars
  destroy_cells();
  //No more ticks
  tick_timer_service_unsubscribe();
  //Finally
  window_destroy(window);
}
      
int main(void)
{
  setup();
  init();
  app_event_loop();
  deinit();
}
