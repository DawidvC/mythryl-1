/* gtk-driver.c
 *
 * This file handles the C side
 * of the Mythryl <-> C interface
 * layer for the Mythryl in-process
 * Gtk binding.  The Mythryl side
 * is implemented by
 *
 *     src/lib/src/gtk-driver.pkg
 *
 */

#include "../../config.h"

#include <string.h>

#if HAVE_GTK_2_0_GTK_GTK_H
#include <gtk-2.0/gtk/gtk.h>
#elif HAVE_GTK_GTK_H
#include <gtk/gtk.h>
#endif

#include "runtime-base.h"
#include "runtime-values.h"
#include "runtime-heap.h"
#include "cfun-proto-list.h"
#include "../lib7-c.h"

#define MAX_WIDGETS 1024
static GtkWidget* widget[ MAX_WIDGETS ];	/* XXX BUGGO FIXME Should expand in size as needed.	*/


static char text_buf[ 1024 ];
static void
die(void)
{
    printf( "FATAL src/runtime/c-libs/lib7-gtk/gtk-driver.c: %s  exit(1)ing.\n", text_buf );
    exit(1);
}




/* We do not want to call Mythryl
 * code directly from the C level;
 * that would lead to messy problems.
 *
 * Consequently when Gtk issues a
 * widget callback we queue up the
 * event and then read it in response
 * to calls from the Mythryl side.
 *
 * Here we implement that queue.
 */

#define MAX_CALLBACK_QUEUE 1024

/* Define the different types of
 * callback queue entries supported.
 *
 * WARNING! Must be kept in sync
 * with matching declarations in
 *
 *     src/lib/src/gtk-driver.pkg 
 */
#define          QUEUED_VOID_CALLBACK   1
#define          QUEUED_BOOL_CALLBACK   2
#define         QUEUED_FLOAT_CALLBACK   3
#define  QUEUED_BUTTON_PRESS_CALLBACK   4
#define     QUEUED_KEY_PRESS_CALLBACK   5
#define QUEUED_MOTION_NOTIFY_CALLBACK   6
#define        QUEUED_EXPOSE_CALLBACK   7
#define     QUEUED_CONFIGURE_CALLBACK   8

typedef struct {
    int callback_number;	/* This identifies which closure to call on the Mythryl side.	*/
    int callback_type;		/* This will be one of the QUEUED_*_CALLBACK values above.	*/
    union {

        int    bool_value;

        double float_value; 

        struct {
	    int    widget_id;
	    int    button;
	    double x;
	    double y;
	    int    time;
	    int    modifiers;
        }                     button_press;

        struct {
	    int  key;
	    int  keycode;
	    int  time;
	    int  modifiers;
        }                     key_press;

        struct {
	    int    widget_id;

	    int    time;
	    double x;
	    double y;
	    int    modifiers;
	    int    is_hint;
        }                     motion_notify;

        struct {
	    int    widget_id;

	    int    count;
	    int    area_x;
	    int    area_y;
	    int    area_wide;
	    int    area_high;
        }                     expose;

        struct {
	    int    widget_id;

	    int    x;
	    int    y;
	    int    wide;
	    int    high;
        }                     configure;

    } entry;

} Callback_Queue_Entry;

/* Here is the cat-and-mouse queue proper.
 * Queue is empty when cat==rat.
 * Next slot to read  is  callback_queue[ callback_queue_cat ].
 * Next slot to write is  callback_queue[ callback_queue_rat ].
 */
static int          callback_queue_cat = 0;
static int          callback_queue_rat = 0;
static Callback_Queue_Entry  callback_queue[ MAX_CALLBACK_QUEUE ];

static int  callback_queue_bump (int at) {
    if (at < (MAX_CALLBACK_QUEUE-1))   return at + 1;
    return 0;
}

static int  callback_queue_is_empty () {
    return  callback_queue_cat == callback_queue_rat;
}

static int  number_of_queued_callbacks() {
    if         (callback_queue_rat >= callback_queue_cat) {
        return  callback_queue_rat -  callback_queue_cat;
    }
    return (callback_queue_rat + MAX_CALLBACK_QUEUE) - callback_queue_cat;
}

static int  type_of_next_queued_callback () {
    if (callback_queue_is_empty()) {
        strcpy( text_buf, "type_of_next_queued_callback(): Callback queue is empty.\n" );
        die();
    }
    return callback_queue[ callback_queue_cat ].callback_type;
}

static Callback_Queue_Entry  get_next_queued_callback () {
    int cat = callback_queue_cat;
    if (callback_queue_is_empty()) {
        strcpy( text_buf, "get_next_queued_callback(): Callback queue is empty.\n" );
        die();
    }
    callback_queue_cat = callback_queue_bump( callback_queue_cat );
    return callback_queue[ cat ];
}

static void  queue_up_callback (Callback_Queue_Entry entry) {
    int rat = callback_queue_rat;
    callback_queue_rat = callback_queue_bump( callback_queue_rat );
    if (callback_queue_is_empty()) {
        strcpy( text_buf, "queue_up_callback(): Callback queue overflowed.\n" );
        die();
    }
    callback_queue[ rat ] = entry;
}

static void  queue_up_void_callback (int callback) {
    Callback_Queue_Entry e;
    e.callback_type   =  QUEUED_VOID_CALLBACK;
    e.callback_number =  callback;
    queue_up_callback(e);
}

static void  queue_up_bool_callback (int callback, int bool_value) {
    Callback_Queue_Entry e;
    e.callback_type    =  QUEUED_BOOL_CALLBACK;
    e.callback_number  =  callback;
    e.entry.bool_value =  bool_value;
    queue_up_callback(e);
}

static void  queue_up_float_callback (int callback, double float_value) {
    Callback_Queue_Entry e;
    e.callback_type     =  QUEUED_FLOAT_CALLBACK;
    e.callback_number   =  callback;
    e.entry.float_value =  float_value;
    queue_up_callback(e);
}

static int
find_free_widget_slot(void)
{
    int i;
    for (i = 1; i < MAX_WIDGETS; ++i) {		/* We reserve 0 as an always-invalid analog to NULL.	*/
        if (!widget[i])  return i;
    }
    sprintf(text_buf, "find_free_widget_slot: All slots full."); die();
    return 0;	/* Can't happen, but keeps gcc quiet. */
}

static int
find_widget_id( GtkWidget* query_widget )
{   int  i;
    for (i = 1;   i < MAX_WIDGETS;   ++i) {

        if (widget[i] == query_widget)   return i;
    }
    return 0;
}

static int
get_widget_id( GtkWidget* query_widget )
{
    int slot = find_widget_id( query_widget );

    if (!slot) {
	 slot = find_free_widget_slot ();

	 widget[slot] = (GtkWidget*) query_widget;
    }

    return slot;
}

static int
find_free_callback_id ()
{
    static int next_callback_id = 1;

    return next_callback_id++;
}


/* These are operationally identical, but for debugging
 * it is convenient to keep them separate:
 */
static void run_clicked_callback(     GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data ); }
static void run_pressed_callback(     GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data ); }
static void run_enter_callback  (     GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data ); }
static void run_leave_callback  (     GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data ); }
static void run_release_callback(     GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data ); }
static void run_activate_callback(    GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data ); }

static gboolean run_destroy_callback( GtkObject* object, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_realize_callback( GtkWidget* widget, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }

static gboolean run_button_press_event_callback		( GtkWidget* widget, GdkEventButton* event, gpointer user_data)
{
    int type = 0;
    switch (event->type) {
    case GDK_BUTTON_PRESS:  type = 1; break;
    case GDK_2BUTTON_PRESS: type = 2; break;
    case GDK_3BUTTON_PRESS: type = 3; break;
    default:
	sprintf (text_buf, "run_button_press_event: type value '%d' is not supported.\n", event->type); die();
    }

    {   Callback_Queue_Entry e;

	e.callback_type    =  QUEUED_BUTTON_PRESS_CALLBACK;
	e.callback_number  =  (int) user_data;

	e.entry.button_press.widget_id = get_widget_id( (GtkWidget*) event->window );
	e.entry.button_press.button    =                             event->button;
	e.entry.button_press.x         =                             event->x;
	e.entry.button_press.y         =                             event->y;
	e.entry.button_press.time      =                             event->time;
	e.entry.button_press.modifiers =                             event->state;

        queue_up_callback(e);

	return TRUE;
    }
}

static gboolean run_motion_notify_event_callback	( GtkWidget* widget, GdkEventMotion* event, gpointer user_data)
{
    Callback_Queue_Entry e;

    e.callback_type    =  QUEUED_MOTION_NOTIFY_CALLBACK;
    e.callback_number  =  (int) user_data;


    e.entry.motion_notify.widget_id = get_widget_id ((GtkWidget*) (event->window));

    e.entry.motion_notify.time      = event->time;
    e.entry.motion_notify.x         = event->x;
    e.entry.motion_notify.y         = event->y;
    e.entry.motion_notify.modifiers = event->state; 
    e.entry.motion_notify.is_hint   = event->is_hint;

    queue_up_callback(e);

    return TRUE;
}

static gboolean run_expose_event_callback		( GtkWidget* w, GdkEventExpose* event, gpointer user_data)
{
    Callback_Queue_Entry e;

    e.callback_type    =  QUEUED_EXPOSE_CALLBACK;
    e.callback_number  =  (int) user_data;


    e.entry.expose.widget_id =  find_widget_id( w );

    e.entry.expose.count     =  event->count;
    e.entry.expose.area_x    =  event->area.x;
    e.entry.expose.area_y    =  event->area.y;
    e.entry.expose.area_wide =  event->area.width;
    e.entry.expose.area_high =  event->area.height;

    queue_up_callback(e);

    return TRUE;
}

static gboolean run_configure_event_callback		( GtkWidget* widget, GdkEventConfigure* event, gpointer user_data)
{
    Callback_Queue_Entry e;

    e.callback_type    =  QUEUED_CONFIGURE_CALLBACK;
    e.callback_number  =  (int) user_data;


    e.entry.configure.widget_id =  find_widget_id( widget );

    e.entry.configure.x         =  event->x;
    e.entry.configure.y         =  event->y;
    e.entry.configure.wide      =  event->width;
    e.entry.configure.high      =  event->height;


    queue_up_callback(e);

    return TRUE;
}

static gboolean run_key_press_event_callback		( GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    Callback_Queue_Entry e;

    e.callback_type    =  QUEUED_KEY_PRESS_CALLBACK;
    e.callback_number  =  (int) user_data;


    e.entry.key_press.key       =  event->keyval;
    e.entry.key_press.keycode   =  event->hardware_keycode;
    e.entry.key_press.time      =  event->time;
    e.entry.key_press.modifiers =  event->state;


    queue_up_callback(e);

    return TRUE;
}

static gboolean run_button_release_event_callback	(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_scroll_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_delete_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_key_release_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_enter_notify_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_leave_notify_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_focus_in_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_focus_out_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_map_event_callback			(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_unmap_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_property_notify_event_callback	(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_selection_clear_event_callback	(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_selection_request_event_callback	(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_selection_notify_event_callback	(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_proximity_in_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_proximity_out_event_callback	(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_client_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_no_expose_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }
static gboolean run_window_state_event_callback		(GtkWidget* widget, GdkEvent* event, gpointer user_data) {  queue_up_void_callback( (int)user_data );  return TRUE; }

/* This one returns a boolean value:
 */
static void
run_toggled_callback ( GtkToggleButton* widget, gpointer user_data)
{
    gboolean is_set = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(widget) );

    queue_up_bool_callback( (int)user_data, is_set );
}

/* This one returns a double value:
 */
static void
run_value_changed_callback ( GtkAdjustment* adjustment, gpointer user_data)
{
    double value = gtk_adjustment_get_value( GTK_ADJUSTMENT(adjustment) );

    queue_up_float_callback( (int)user_data, value );
}



/* _lib7_Gtk_gtk_init : Void -> Void
 *
 *
 */
lib7_val_t

_lib7_Gtk_gtk_init (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    extern char	**commandline_arguments;

    int argc;
    while (commandline_arguments[argc]) ++argc;    

    if (!gtk_init_check( &argc, &commandline_arguments )) {

        return RAISE_ERROR(lib7_state, "gtk_init: failed to initialize GUI support",__LINE__);
    }

    /* XXX BUGGO FIXME: gtk_init_check installs gtk default signal handlers,
                        which most likely screws up Mythryl's own signal
                        handling no end.  At some point should put some work
                        into keeping both happy.
    */

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}

static int
int_to_arrow_direction (int arrow_direction)
{
    switch (arrow_direction) {
    case 0:  return GTK_ARROW_UP;    
    case 1:  return GTK_ARROW_DOWN;    
    case 2:  return GTK_ARROW_LEFT;    
    case 3:  return GTK_ARROW_RIGHT;
    default:
      sprintf( text_buf, "int_to_arrow_direction: bad arg %d.", arrow_direction );
      die();
    }
    return 0;	/* Just to quiet compilers. */
}

static int
int_to_shadow_style (int shadow_style)
{
    switch (shadow_style) {
    case 0:  return GTK_SHADOW_NONE;
    case 1:  return GTK_SHADOW_IN;
    case 2:  return GTK_SHADOW_OUT;
    case 3:  return GTK_SHADOW_ETCHED_IN;
    case 4:  return GTK_SHADOW_ETCHED_OUT;
    default:
      sprintf( text_buf, "int_to_shadow_style: bad arg %d.", shadow_style );
      die();
    }
    return 0;	/* Just to quiet compilers. */
}

static int
int_to_policy (int policy)
{
    switch (policy) {
    case 0:  return GTK_POLICY_AUTOMATIC;
    case 1:  return GTK_POLICY_ALWAYS;
    default:
      sprintf( text_buf, "int_to_policy: bad arg %d.", policy );
      die();
    }
    return 0;	/* Just to quiet compilers. */
}

static int int_to_event_mask( int i1 ) {

    int        mask      = 0;

    if (i1 & (1 <<  0))    mask |= GDK_EXPOSURE_MASK;
    if (i1 & (1 <<  1))    mask |= GDK_POINTER_MOTION_MASK;
    if (i1 & (1 <<  2))    mask |= GDK_POINTER_MOTION_HINT_MASK;
    if (i1 & (1 <<  3))    mask |= GDK_BUTTON_MOTION_MASK;
    if (i1 & (1 <<  4))    mask |= GDK_BUTTON1_MOTION_MASK;
    if (i1 & (1 <<  5))    mask |= GDK_BUTTON2_MOTION_MASK;
    if (i1 & (1 <<  6))    mask |= GDK_BUTTON3_MOTION_MASK;
    if (i1 & (1 <<  7))    mask |= GDK_BUTTON_PRESS_MASK;
    if (i1 & (1 <<  8))    mask |= GDK_BUTTON_RELEASE_MASK;
    if (i1 & (1 <<  9))    mask |= GDK_KEY_PRESS_MASK;
    if (i1 & (1 << 10))    mask |= GDK_KEY_RELEASE_MASK;
    if (i1 & (1 << 11))    mask |= GDK_ENTER_NOTIFY_MASK;
    if (i1 & (1 << 12))    mask |= GDK_LEAVE_NOTIFY_MASK;
    if (i1 & (1 << 13))    mask |= GDK_FOCUS_CHANGE_MASK;
    if (i1 & (1 << 14))    mask |= GDK_STRUCTURE_MASK;
    if (i1 & (1 << 15))    mask |= GDK_PROPERTY_CHANGE_MASK;
    if (i1 & (1 << 16))    mask |= GDK_PROXIMITY_IN_MASK;
    if (i1 & (1 << 17))    mask |= GDK_PROXIMITY_OUT_MASK;

    return mask;
}

static int int_to_justification( int i1 ) {

    switch (i1) {
    case 0:	i1 = GTK_JUSTIFY_LEFT;	break;
    case 1:	i1 = GTK_JUSTIFY_RIGHT;	break;
    case 2:	i1 = GTK_JUSTIFY_CENTER;	break;
    case 3:	i1 = GTK_JUSTIFY_FILL;	break;
    default:
      sprintf( text_buf, "do_set_label_justification: bad arg %d.", i1 );
      die();
    }

    return i1;
}

static int int_to_position( i1 ) {

    switch (i1) {
    case 0:	i1 = GTK_POS_LEFT;	break;
    case 1:	i1 = GTK_POS_RIGHT;	break;
    case 2:	i1 = GTK_POS_TOP;	break;
    case 3:	i1 = GTK_POS_BOTTOM;	break;
    default:
      sprintf( text_buf, "do_set_scale_value_position: bad position arg %d.", i1 );
      die();
    }

    return i1;
}

static int int_to_metric( i1 ) {

    switch (i1) {
    case 0:	i1 = GTK_PIXELS;		break;
    case 1:	i1 = GTK_INCHES;		break;
    case 2:	i1 = GTK_CENTIMETERS;	break;
    default:
      sprintf( text_buf, "do_set_ruler_metric: bad arg %d.", i1 );
      die();
    }

    return i1;
}

static int int_to_range_update_policy( int i1 ) {

    switch (i1) {
    case 0: i1 = GTK_UPDATE_CONTINUOUS;	break;
    case 1: i1 = GTK_UPDATE_DISCONTINUOUS;	break;
    case 2: i1 = GTK_UPDATE_DELAYED;		break;
    default:
      sprintf( text_buf, "do_set_range_update_policy: bad policy arg %d.", i1 );
      die();
    }

    return i1;
}


/* _lib7_Gtk_callback_queue_is_empty
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> Bool
 */
lib7_val_t

_lib7_Gtk_callback_queue_is_empty   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    return  callback_queue_is_empty()
              ?  LIB7_true
              : LIB7_false;
}

/* _lib7_Gtk_number_of_queued_callbacks
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> Int
 */
lib7_val_t

_lib7_Gtk_number_of_queued_callbacks   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    int result = number_of_queued_callbacks();

    return INT_CtoLib7( result );
}

/* _lib7_Gtk_type_of_next_queued_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> Int
 */
lib7_val_t

_lib7_Gtk_type_of_next_queued_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    int result = type_of_next_queued_callback();

    return INT_CtoLib7( result );
}

/* _lib7_Gtk_get_queued_void_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> Int
 */
lib7_val_t

_lib7_Gtk_get_queued_void_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_VOID_CALLBACK) {
        strcpy( text_buf, "get_queued_void_callback: Next callback not Void." );
        die();
    }

    return INT_CtoLib7( e.callback_number );
}

/* _lib7_Gtk_get_queued_bool_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int, Bool)
 */
lib7_val_t

_lib7_Gtk_get_queued_bool_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_BOOL_CALLBACK) {
        strcpy( text_buf, "get_queued_bool_callback: Next callback not Bool." );
        die();
    }

    LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 2));
    LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number ));
    LIB7_AllocWrite(  lib7_state, 2, e.entry.bool_value ?  LIB7_true : LIB7_false );
    return LIB7_Alloc(lib7_state, 2);
}

/* _lib7_Gtk_get_queued_float_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int, Float)
 */
lib7_val_t

_lib7_Gtk_get_queued_float_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_FLOAT_CALLBACK) {
        strcpy( text_buf, "get_queued_float_callback: Next callback not Float." );
        die();
    }

    {   double d = e.entry.float_value;

        lib7_val_t	         boxed_double;
        REAL64_ALLOC(lib7_state, boxed_double, d);	/* Defined in src/runtime/include/runtime-heap.h */

	LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 2));
	LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number ));
	LIB7_AllocWrite(  lib7_state, 2, boxed_double );
	return LIB7_Alloc(lib7_state, 2);
    }
}

/* _lib7_Gtk_get_queued_button_press_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int,     Int,   Int,   Float, Float, Int, Int)
 *                                 callback widget button x      y      time modifiers
 */
lib7_val_t

_lib7_Gtk_get_queued_button_press_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_BUTTON_PRESS_CALLBACK) {
        strcpy( text_buf, "get_queued_button_press_callback: Next callback not Button_Press." );
        die();
    }

    {   double x = e.entry.button_press.x;
        double y = e.entry.button_press.y;

        lib7_val_t	         boxed_x;
        lib7_val_t	         boxed_y;

        REAL64_ALLOC(lib7_state, boxed_x, x);
        REAL64_ALLOC(lib7_state, boxed_y, y);

	LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 7)                    );
	LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number              ));
	LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( e.entry.button_press.widget_id ));
	LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( e.entry.button_press.button    ));
	LIB7_AllocWrite(  lib7_state, 4, boxed_x                                      );
	LIB7_AllocWrite(  lib7_state, 5, boxed_y                                      );
	LIB7_AllocWrite(  lib7_state, 6, INT_CtoLib7( e.entry.button_press.time      ));
	LIB7_AllocWrite(  lib7_state, 7, INT_CtoLib7( e.entry.button_press.modifiers ));
	return LIB7_Alloc(lib7_state, 7);
    }
}


/* _lib7_Gtk_get_queued_key_press_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int,     Int,   Int,    Int, Int)
 *                                 callback key    keycode time modifiers
 */
lib7_val_t

_lib7_Gtk_get_queued_key_press_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_KEY_PRESS_CALLBACK) {
        strcpy( text_buf, "get_queued_key_press_callback: Next callback not Key_Press." );
        die();
    }

    LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 5)                 );
    LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number           ));
    LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( e.entry.key_press.key       ));
    LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( e.entry.key_press.keycode   ));
    LIB7_AllocWrite(  lib7_state, 4, INT_CtoLib7( e.entry.key_press.time      ));
    LIB7_AllocWrite(  lib7_state, 5, INT_CtoLib7( e.entry.key_press.modifiers ));
    return LIB7_Alloc(lib7_state, 5);
}


/* _lib7_Gtk_get_queued_motion_notify_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int,     Int,  Float, Float, Int,      Bool)
 *                                 callback time  x      y      modifiers is_hint
 */
lib7_val_t

_lib7_Gtk_get_queued_motion_notify_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_MOTION_NOTIFY_CALLBACK) {
        strcpy( text_buf, "get_queued_motion_notify_callback: Next callback not Motion_Notify." );
        die();
    }

    {   double x = e.entry.motion_notify.x;
        double y = e.entry.motion_notify.y;

        lib7_val_t	         boxed_x;
        lib7_val_t	         boxed_y;

        REAL64_ALLOC(lib7_state, boxed_x, x);
        REAL64_ALLOC(lib7_state, boxed_y, y);

	LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 7)                     );
	LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number               ));
	LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( e.entry.motion_notify.widget_id ));
	LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( e.entry.motion_notify.time      ));
	LIB7_AllocWrite(  lib7_state, 4, boxed_x                                       );
	LIB7_AllocWrite(  lib7_state, 5, boxed_y                                       );
	LIB7_AllocWrite(  lib7_state, 6, INT_CtoLib7( e.entry.motion_notify.modifiers ));
	LIB7_AllocWrite(  lib7_state, 7, e.entry.motion_notify.is_hint ? LIB7_true : LIB7_false );
	return LIB7_Alloc(lib7_state, 7);
    }
}

/* _lib7_Gtk_get_queued_expose_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int,     Int,   Int,  Int,   Int,   Int,      Int)
 *                                 callback widget count area_x area_y area_wide area_high
 */
lib7_val_t

_lib7_Gtk_get_queued_expose_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_EXPOSE_CALLBACK) {
        strcpy( text_buf, "get_queued_expose_callback: Next callback not Expose." );
        die();
    }

    LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 7)              );
    LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number        ));
    LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( e.entry.expose.widget_id ));
    LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( e.entry.expose.count     ));
    LIB7_AllocWrite(  lib7_state, 4, INT_CtoLib7( e.entry.expose.area_x    ));
    LIB7_AllocWrite(  lib7_state, 5, INT_CtoLib7( e.entry.expose.area_y    ));
    LIB7_AllocWrite(  lib7_state, 6, INT_CtoLib7( e.entry.expose.area_wide ));
    LIB7_AllocWrite(  lib7_state, 7, INT_CtoLib7( e.entry.expose.area_high ));
    return LIB7_Alloc(lib7_state, 7);
}

/* _lib7_Gtk_get_queued_configure_callback
 *
 * gtk.api        type:   (None -- not exported to gtk.api level.)
 * gtk-driver.api type:   Void -> (Int,     Int,   Int, Int, Int, Int)
 *                                 callback widget x    y    wide high
 */
lib7_val_t

_lib7_Gtk_get_queued_configure_callback   (lib7_state_t *lib7_state, lib7_val_t arg)
{
    Callback_Queue_Entry e = get_next_queued_callback ();

    if (e.callback_type != QUEUED_CONFIGURE_CALLBACK) {
        strcpy( text_buf, "get_queued_configure_callback: Next callback not Configure." );
        die();
    }

    LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 6)                 );
    LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( e.callback_number           ));
    LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( e.entry.configure.widget_id ));
    LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( e.entry.configure.x         ));
    LIB7_AllocWrite(  lib7_state, 4, INT_CtoLib7( e.entry.configure.y         ));
    LIB7_AllocWrite(  lib7_state, 5, INT_CtoLib7( e.entry.configure.wide      ));
    LIB7_AllocWrite(  lib7_state, 6, INT_CtoLib7( e.entry.configure.high      ));
    return LIB7_Alloc(lib7_state, 6);
}

/*  _lib7_Gtk_get_widget_allocation : Widget -> (Int, Int, Int, Int)
 */
lib7_val_t

_lib7_Gtk_get_widget_allocation (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    w0 = GTK_WIDGET( w0 );		/* Verify user gave us something appropriate. */

    int x    =  w0->allocation.x;
    int y    =  w0->allocation.y;
    int wide =  w0->allocation.width;
    int high =  w0->allocation.height;

    LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 4));
    LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( x          ));
    LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( y          ));
    LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( wide       ));
    LIB7_AllocWrite(  lib7_state, 4, INT_CtoLib7( high       ));
    return LIB7_Alloc(lib7_state, 4);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}

/*  _lib7_Gtk_get_window_pointer : Widget -> (Int, Int, Int)       # (x, y, modifiers)
 */
lib7_val_t

_lib7_Gtk_get_window_pointer (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    w0 = GTK_WIDGET( w0 );		/* Verify user gave us something appropriate. */

    {
	int             x;
	int             y;
	GdkModifierType modifiers;

/*      GdkWindow* result_window = */  gdk_window_get_pointer (GDK_WINDOW(w0), &x, &y, &modifiers); 

	LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 3));
	LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( x          ));
	LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( y          ));
	LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( modifiers  ));
	return LIB7_Alloc(lib7_state, 3);
    }
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}

/*  _lib7_Gtk_make_dialog : Void -> (Int, Int, Int)       # (dialog, vbox, action_area)
 */
lib7_val_t

_lib7_Gtk_make_dialog (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    int dialog;
    int vbox;
    int action_area;

    dialog      = find_free_widget_slot ();   widget[dialog]      = gtk_dialog_new();
    vbox        = find_free_widget_slot ();   widget[vbox]        = GTK_DIALOG( widget[dialog] )->vbox;
    action_area = find_free_widget_slot ();   widget[action_area] = GTK_DIALOG( widget[dialog] )->action_area;

    LIB7_AllocWrite(  lib7_state, 0, MAKE_DESC(DTAG_record, 3));
    LIB7_AllocWrite(  lib7_state, 1, INT_CtoLib7( dialog     ));
    LIB7_AllocWrite(  lib7_state, 2, INT_CtoLib7( vbox       ));
    LIB7_AllocWrite(  lib7_state, 3, INT_CtoLib7( action_area));
    return LIB7_Alloc(lib7_state, 3);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}


/*  _lib7_Gtk_unref_object : Int -> Void       # Widget -> Void
 */
lib7_val_t

_lib7_Gtk_unref_object (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    g_object_unref( G_OBJECT( w0 ) );

    widget[ get_widget_id( w0 ) ] = 0;

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}


/*  _lib7_Gtk_run_eventloop_once : Bool -> Bool       # Bool -> Bool
 */
lib7_val_t

_lib7_Gtk_run_eventloop_once (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    int block_until_event = INT_LIB7toC(arg);

    int quit_called = gtk_main_iteration_do( block_until_event );

    return quit_called ? LIB7_true : LIB7_false;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}





/* Do not edit this or following lines -- they are autogenerated by make-gtk-glue. */
/* _lib7_Gtk_make_window
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_window   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_window_new( GTK_WINDOW_TOPLEVEL );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_label
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_label_new( /*label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_status_bar_context_id
 *
 * gtk.api        type:   (Session, Widget, String) -> Int
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Int
 */
lib7_val_t

_lib7_Gtk_make_status_bar_context_id   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    int result = gtk_statusbar_get_context_id( GTK_STATUSBAR(/*status_bar*/w0), /*description*/s1);

    return INT_CtoLib7(result);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_menu
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_menu   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_menu_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_option_menu
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_option_menu   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_option_menu_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_menu_bar
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_menu_bar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_menu_bar_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_combo_box
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_combo_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_combo_box_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_text_combo_box
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_text_combo_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_combo_box_new_text ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_frame
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_frame   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_frame_new (*/*label*/s0 ? /*label*/s0 : NULL);

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_button
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_button_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_button_with_label
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_button_with_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_button_new_with_label( /*label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime,__LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_button_with_mnemonic
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_button_with_mnemonic   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_button_new_with_mnemonic( /*mnemonic_label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_toggle_button
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_toggle_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_toggle_button_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_toggle_button_with_label
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_toggle_button_with_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_toggle_button_new_with_label( /*label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_toggle_button_with_mnemonic
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_toggle_button_with_mnemonic   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_toggle_button_new_with_mnemonic( /*mnemonic_label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_check_button
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_check_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_check_button_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_check_button_with_label
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_check_button_with_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_check_button_new_with_label ( /*label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_check_button_with_mnemonic
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_check_button_with_mnemonic   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_check_button_new_with_mnemonic( /*mnemonic_label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_menu_item
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_menu_item   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_menu_item_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_menu_item_with_label
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_menu_item_with_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_menu_item_new_with_label( /*label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_menu_item_with_mnemonic
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_menu_item_with_mnemonic   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_menu_item_new_with_mnemonic( /*mnemonic_label*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_first_radio_button
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_first_radio_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_radio_button_new (NULL);

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_next_radio_button
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_next_radio_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON(/*sib*/w0));

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_first_radio_button_with_label
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_first_radio_button_with_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_radio_button_new_with_label(NULL,/*label*/s0);

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_next_radio_button_with_label
 *
 * gtk.api        type:   (Session, Widget, String) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_next_radio_button_with_label   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON(/*sib*/w0), /*label*/s1 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_first_radio_button_with_mnemonic
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_first_radio_button_with_mnemonic   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_radio_button_new_with_mnemonic(NULL,/*label*/s0);

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_next_radio_button_with_mnemonic
 *
 * gtk.api        type:   (Session, Widget, String) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_next_radio_button_with_mnemonic   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_radio_button_new_with_mnemonic_from_widget ( GTK_RADIO_BUTTON(/*sib*/w0), /*label*/s1 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_arrow
 *
 * gtk.api        type:   (Session, Arrow_Direction, Shadow_Style) -> Widget
 * gtk-driver.api type:   (Session, Int, Int) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_arrow   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    int               i0 =                              REC_SELINT( arg, 1);
    int               i1 =                              REC_SELINT( arg, 2);

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_arrow_new( int_to_arrow_direction(/*arrow_direction_to_int arrow_direction*/i0), int_to_shadow_style(/*shadow_style_to_int shadow_style*/i1) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_arrow
 *
 * gtk.api        type:   (Session, Widget, Arrow_Direction, Shadow_Style) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_arrow   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_arrow_set( GTK_ARROW(/*arrow*/w0), int_to_arrow_direction(/*arrow_direction_to_int arrow_direction*/i1), int_to_shadow_style(/*shadow_style_to_int shadow_style*/i2) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_box
 *
 * gtk.api        type:   (Session, Bool, Int)   ->   Widget
 * gtk-driver.api type:   (Session, Bool, Int) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    int               b0 =                              REC_SEL(    arg, 1) == LIB7_true;
    int               i1 =                              REC_SELINT( arg, 2);

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hbox_new ( /*homogeneous*/b0, /*spacing*/i1 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_box
 *
 * gtk.api        type:   (Session, Bool, Int)   ->   Widget
 * gtk-driver.api type:   (Session, Bool, Int) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    int               b0 =                              REC_SEL(    arg, 1) == LIB7_true;
    int               i1 =                              REC_SELINT( arg, 2);

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vbox_new ( /*homogeneous*/b0, /*spacing*/i1 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_button_box
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_button_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hbutton_box_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_button_box
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_button_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vbutton_box_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_table
 *
 * gtk.api        type:    { session: Session,   rows: Int,   cols: Int,   homogeneous: Bool }   ->   Widget
 * gtk-driver.api type:   (Session, Int, Int, Bool) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_table   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    int               i0 =                              REC_SELINT( arg, 1);
    int               i1 =                              REC_SELINT( arg, 2);
    int               b2 =                              REC_SEL(    arg, 3) == LIB7_true;

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_table_new ( /*rows*/i0, /*cols*/i1, /*homogeneous*/b2 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_event_box
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_event_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_event_box_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_image_from_file
 *
 * gtk.api        type:   (Session, String) -> Widget
 * gtk-driver.api type:   (Session, String) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_image_from_file   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    char*             s0 =                 STR_LIB7toC (REC_SEL(    arg, 1));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_image_new_from_file( /*filename*/s0 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_separator
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_separator   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hseparator_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_separator
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_separator   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vseparator_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_layout_container
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_layout_container   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_layout_new (NULL, NULL);

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_layout_put
 *
 * gtk.api        type:    { session: Session,  layout: Widget,  kid: Widget,  x: Int,  y: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_layout_put   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);

    gtk_layout_put( GTK_LAYOUT(/*layout*/w0), GTK_WIDGET(/*kid*/w1), /*x*/i2, /*y*/i3);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_layout_move
 *
 * gtk.api        type:    { session: Session, layout: Widget,  kid: Widget,  x: Int,  y: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_layout_move   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);

    gtk_layout_move( GTK_LAYOUT(/*layout*/w0), GTK_WIDGET(/*kid*/w1), /*x*/i2, /*y*/i3);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_fixed_container
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_fixed_container   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_fixed_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_fixed_put
 *
 * gtk.api        type:    { session: Session, layout: Widget,  kid: Widget,  x: Int,  y: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_fixed_put   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);

    gtk_fixed_put(   GTK_FIXED(/*layout*/w0), GTK_WIDGET(/*kid*/w1), /*x*/i2, /*y*/i3);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_fixed_move
 *
 * gtk.api        type:    { session: Session, layout: Widget,  kid: Widget,  x: Int,  y: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_fixed_move   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);

    gtk_fixed_move(  GTK_FIXED(/*layout*/w0), GTK_WIDGET(/*kid*/w1), /*x*/i2, /*y*/i3);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_adjustment
 *
 * gtk.api        type:    { session: Session,   value: Float,   lower: Float,   upper: Float,   step_increment: Float,   page_increment: Float,   page_size: Float }   ->   Widget
 * gtk-driver.api type:   (Session, Float, Float, Float, Float, Float, Float) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_adjustment   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    double            f0 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 1)));
    double            f1 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 2)));
    double            f2 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 3)));
    double            f3 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 4)));
    double            f4 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 5)));
    double            f5 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 6)));

    int slot = find_free_widget_slot ();

    widget[slot] = (GtkWidget*) gtk_adjustment_new ( /*value*/f0, /*lower*/f1, /*upper*/f2, /*step_increment*/f3, /*page_increment*/f4, /*page_size*/f5 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_viewport
 *
 * gtk.api        type:    { session: Session, horizontal_adjustment: Null_Or(Widget), vertical_adjustment: Null_Or(Widget) } -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_viewport   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_viewport_new( GTK_ADJUSTMENT(/*null_or_widget_to_int horizontal_adjustment*/w0), GTK_ADJUSTMENT(/*null_or_widget_to_int vertical_adjustment*/w1) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_scrolled_window
 *
 * gtk.api        type:    { session: Session, horizontal_adjustment: Null_Or(Widget), vertical_adjustment: Null_Or(Widget) } -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_scrolled_window   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_scrolled_window_new( GTK_ADJUSTMENT(/*null_or_widget_to_int horizontal_adjustment*/w0), GTK_ADJUSTMENT(/*null_or_widget_to_int vertical_adjustment*/w1) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_ruler
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_ruler   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hruler_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_ruler
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_ruler   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vruler_new ();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_scrollbar
 *
 * gtk.api        type:   (Session, Null_Or(Widget)) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_scrollbar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vscrollbar_new( GTK_ADJUSTMENT(/*null_or_widget_to_int adjustment*/w0) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_scrollbar
 *
 * gtk.api        type:   (Session, Null_Or(Widget)) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_scrollbar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hscrollbar_new( GTK_ADJUSTMENT(/*null_or_widget_to_int adjustment*/w0) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_scale
 *
 * gtk.api        type:   (Session, Null_Or(Widget)) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_scale   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vscale_new( GTK_ADJUSTMENT(/*null_or_widget_to_int adjustment*/w0) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_scale
 *
 * gtk.api        type:   (Session, Null_Or(Widget)) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_scale   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hscale_new( GTK_ADJUSTMENT(/*null_or_widget_to_int adjustment*/w0) );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_vertical_scale_with_range
 *
 * gtk.api        type:    { session: Session, min: Float, max: Float, step: Float } -> Widget
 * gtk-driver.api type:   (Session, Float, Float, Float) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_vertical_scale_with_range   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    double            f0 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 1)));
    double            f1 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 2)));
    double            f2 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 3)));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_vscale_new_with_range( /*min*/f0, /*max*/f1, /*step*/f2 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_horizontal_scale_with_range
 *
 * gtk.api        type:    { session: Session, min: Float, max: Float, step: Float } -> Widget
 * gtk-driver.api type:   (Session, Float, Float, Float) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_horizontal_scale_with_range   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    double            f0 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 1)));
    double            f1 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 2)));
    double            f2 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 3)));

    int slot = find_free_widget_slot ();

    widget[slot] = gtk_hscale_new_with_range( /*min*/f0, /*max*/f1, /*step*/f2 );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_drawing_area
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_drawing_area   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_drawing_area_new();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_pixmap
 *
 * gtk.api        type:    { session: Session, window: Widget, wide: Int, high: Int } -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_pixmap   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    int slot = find_free_widget_slot ();

    widget[slot] = (GtkWidget*) gdk_pixmap_new( GDK_DRAWABLE(/*window*/w0), /*wide*/i1, /*high*/i2, -1);

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_make_status_bar
 *
 * gtk.api        type:    Session -> Widget
 * gtk-driver.api type:   (Session) -> Int (*new Widget*)
 */
lib7_val_t

_lib7_Gtk_make_status_bar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)


    int slot = find_free_widget_slot ();

    widget[slot] = gtk_statusbar_new();

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_push_text_on_status_bar
 *
 * gtk.api        type:   (Session, Widget, Int, String) -> Int
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, String) -> Int
 */
lib7_val_t

_lib7_Gtk_push_text_on_status_bar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    char*             s2 =                 STR_LIB7toC (REC_SEL(    arg, 3));

    int result = gtk_statusbar_push( GTK_STATUSBAR(/*status_bar*/w0), /*context*/i1, /*text*/s2);

    return INT_CtoLib7(result);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_pop_text_off_status_bar
 *
 * gtk.api        type:   (Session, Widget, Int) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_pop_text_off_status_bar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_statusbar_pop(GTK_STATUSBAR(/*status_bar*/w0), /*context*/i1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_remove_text_from_status_bar
 *
 * gtk.api        type:    { session: Session,   status_bar: Widget,   context: Int,   message: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_remove_text_from_status_bar   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_statusbar_remove( GTK_STATUSBAR(/*status_bar*/w0), /*context*/i1, /*message*/i2);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_pack_box
 *
 * gtk.api        type:    { session: Session,   box: Widget,   kid: Widget,   pack: Pack_From,   expand: Bool,   fill: Bool,   padding: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Bool, Bool, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_pack_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               b3 =                              REC_SEL(    arg, 4) == LIB7_true;
    int               b4 =                              REC_SEL(    arg, 5) == LIB7_true;
    int               i5 =                              REC_SELINT( arg, 6);

    if (!/*pack_to_int pack*/i2)  gtk_box_pack_start(   GTK_BOX(/*box*/w0), GTK_WIDGET(/*kid*/w1), /*expand*/b3, /*fill*/b4, /*padding*/i5 ); else gtk_box_pack_end( GTK_BOX(/*box*/w0), GTK_WIDGET(/*kid*/w1), /*expand*/b3, /*fill*/b4, /*padding*/i5 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_menu_shell_append
 *
 * gtk.api        type:    { session: Session,   menu: Widget,   kid: Widget } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_menu_shell_append   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    gtk_menu_shell_append( GTK_MENU_SHELL(/*menu*/w0), GTK_WIDGET(/*kid*/w1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_menu_bar_append
 *
 * gtk.api        type:    { session: Session,   menu: Widget,   kid: Widget } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_menu_bar_append   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    gtk_menu_bar_append( GTK_MENU_SHELL(/*menu*/w0), GTK_WIDGET(/*kid*/w1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_append_text_to_combo_box
 *
 * gtk.api        type:   (Session, Widget, String) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Void
 */
lib7_val_t

_lib7_Gtk_append_text_to_combo_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    gtk_combo_box_append_text( GTK_COMBO_BOX(/*combo_box*/w0), /*text*/s1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_option_menu_menu
 *
 * gtk.api        type:    { session: Session,   option_menu: Widget,   menu: Widget } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_set_option_menu_menu   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    gtk_option_menu_set_menu( GTK_OPTION_MENU(/*option_menu*/w0), GTK_WIDGET(/*menu*/w1) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_text_tooltip_on_widget
 *
 * gtk.api        type:   (Session, Widget, String) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Void
 */
lib7_val_t

_lib7_Gtk_set_text_tooltip_on_widget   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    gtk_widget_set_tooltip_text( GTK_WIDGET(/*widget*/w0), /*text*/s1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_ruler_metric
 *
 * gtk.api        type:   (Session, Widget, Metric) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_ruler_metric   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_ruler_set_metric( GTK_RULER(/*ruler*/w0), int_to_metric(/*metric_to_int metric*/i1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_ruler_range
 *
 * gtk.api        type:    { session: Session,   ruler: Widget,   lower: Float,   upper: Float,   position: Float,   max_size: Float } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Float, Float, Float, Float) -> Void
 */
lib7_val_t

_lib7_Gtk_set_ruler_range   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    double            f1 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 2)));
    double            f2 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 3)));
    double            f3 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 4)));
    double            f4 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 5)));

    gtk_ruler_set_range( GTK_RULER(/*ruler*/w0), /*lower*/f1, /*upper*/f2, /*position*/f3, /*max_size*/f4);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_scrollbar_policy
 *
 * gtk.api        type:    { session: Session,   window: Widget,   horizontal_scrollbar: Scrollbar_Policy,   vertical_scrollbar: Scrollbar_Policy } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_scrollbar_policy   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(/*window*/w0), int_to_policy(/*scrollbar_policy_to_int horizontal_scrollbar*/i1), int_to_policy(/*scrollbar_policy_to_int vertical_scrollbar*/i2) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_draw_rectangle
 *
 * gtk.api        type:    { session: Session,   drawable: Widget,   gcontext: Widget,   filled:	Bool,   x: Int,   y: Int,   wide: Int,   high: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Bool, Int, Int, Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_draw_rectangle   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               b2 =                              REC_SEL(    arg, 3) == LIB7_true;
    int               i3 =                              REC_SELINT( arg, 4);
    int               i4 =                              REC_SELINT( arg, 5);
    int               i5 =                              REC_SELINT( arg, 6);
    int               i6 =                              REC_SELINT( arg, 7);

    gdk_draw_rectangle(   GDK_DRAWABLE(/*drawable*/w0), GDK_GC(/*gcontext*/w1), /*filled*/b2, /*x*/i3, /*y*/i4, /*wide*/i5, /*high*/i6);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_draw_drawable
 *
 * gtk.api        type:    { session: Session,   drawable: Widget,   gcontext: Widget,   from: Widget,   from_x:	Int,   from_y: Int,   to_x: Int,   to_y: Int,   wide: Int,   high: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int(*Widget*), Int, Int, Int, Int, Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_draw_drawable   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    GtkWidget*        w2 =    (GtkWidget*)      widget[ REC_SELINT( arg, 3) ];
    int               i3 =                              REC_SELINT( arg, 4);
    int               i4 =                              REC_SELINT( arg, 5);
    int               i5 =                              REC_SELINT( arg, 6);
    int               i6 =                              REC_SELINT( arg, 7);
    int               i7 =                              REC_SELINT( arg, 8);
    int               i8 =                              REC_SELINT( arg, 9);

    gdk_draw_drawable(   GDK_DRAWABLE(/*drawable*/w0), GDK_GC(/*gcontext*/w1), GDK_DRAWABLE(/*from*/w2), /*from_x*/i3, /*from_y*/i4, /*to_x*/i5, /*to_y*/i6, /*wide*/i7, /*high*/i8);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_queue_redraw
 *
 * gtk.api        type:    { session: Session,   widget:	Widget,   x: Int,   y: Int,   wide: Int,   high: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int, Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_queue_redraw   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);
    int               i4 =                              REC_SELINT( arg, 5);

    gtk_widget_queue_draw_area( GTK_WIDGET(/*widget*/w0), /*x*/i1, /*y*/i2, /*wide*/i3, /*high*/i4);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_press_button
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_press_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_button_pressed(  GTK_BUTTON(/*widget*/w0) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_release_button
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_release_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_button_released( GTK_BUTTON(/*widget*/w0) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_click_button
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_click_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_button_clicked(  GTK_BUTTON(/*widget*/w0) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_enter_button
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_enter_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_button_enter(    GTK_BUTTON(/*widget*/w0) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_leave_button
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_leave_button   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_button_leave(    GTK_BUTTON(/*widget*/w0) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_show_widget
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_show_widget   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_widget_show( /*widget*/w0 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_show_widget_tree
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_show_widget_tree   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_widget_show_all( /*widget*/w0 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_destroy_widget
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_destroy_widget   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_widget_destroy( /*widget*/w0 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_emit_changed_signal
 *
 * gtk.api        type:   (Session, Widget)   -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_emit_changed_signal   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    g_signal_emit_by_name( GTK_OBJECT(/*widget*/w0), "changed");

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_pop_up_combo_box
 *
 * gtk.api        type:   (Session, Widget)   -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_pop_up_combo_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_combo_box_popup(   GTK_COMBO_BOX(/*widget*/w0));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_pop_down_combo_box
 *
 * gtk.api        type:   (Session, Widget) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_pop_down_combo_box   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    gtk_combo_box_popdown( GTK_COMBO_BOX(/*widget*/w0));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_combo_box_title
 *
 * gtk.api        type:   (Session, Widget, String)   -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Void
 */
lib7_val_t

_lib7_Gtk_set_combo_box_title   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    gtk_combo_box_set_title( GTK_COMBO_BOX(/*widget*/w0), /*title*/s1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_window_title
 *
 * gtk.api        type:   (Session, Widget, String) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Void
 */
lib7_val_t

_lib7_Gtk_set_window_title   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    gtk_window_set_title( GTK_WINDOW(/*window*/w0), /*title*/s1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_window_default_size
 *
 * gtk.api        type:   (Session, Widget, (Int,Int)) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_window_default_size   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_window_set_default_size( GTK_WINDOW(/*widget*/w0), /*wide*/i1, /*high*/i2);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_minimum_widget_size
 *
 * gtk.api        type:   (Session, Widget, (Int,Int)) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_minimum_widget_size   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_widget_set_size_request( GTK_WIDGET(/*widget*/w0), /*wide*/i1, /*high*/i2);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_border_width
 *
 * gtk.api        type:   (Session, Widget, Int) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_border_width   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_container_set_border_width(GTK_CONTAINER(/*widget*/w0), /*width*/i1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_event_box_visibility
 *
 * gtk.api        type:   (Session, Widget, Bool) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Bool) -> Void
 */
lib7_val_t

_lib7_Gtk_set_event_box_visibility   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               b1 =                              REC_SEL(    arg, 2) == LIB7_true;

    gtk_event_box_set_visible_window(GTK_EVENT_BOX(/*event_box*/w0),/*visibility*/b1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_widget_alignment
 *
 * gtk.api        type:    { session: Session, widget: Widget, x: Float, y: Float } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Float, Float) -> Void
 */
lib7_val_t

_lib7_Gtk_set_widget_alignment   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    double            f1 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 2)));
    double            f2 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 3)));

    gtk_misc_set_alignment(GTK_MISC(/*widget*/w0), /*x*/f1, /*y*/f2);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_widget_events
 *
 * gtk.api        type:   (Session, Widget, List( Event_Mask )) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_widget_events   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_widget_set_events( GTK_WIDGET(/*widget*/w0), int_to_event_mask(/*events_to_int events*/i1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_widget_name
 *
 * gtk.api        type:   (Session, Widget, String) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Void
 */
lib7_val_t

_lib7_Gtk_set_widget_name   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    gtk_widget_set_name( GTK_WIDGET(/*widget*/w0), /*name*/s1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_label_justification
 *
 * gtk.api        type:   (Session, Widget, Justification) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_label_justification   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_label_set_justify( GTK_LABEL(/*label*/w0), int_to_justification(/*justification_to_int justification*/i1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_label_line_wrapping
 *
 * gtk.api        type:   (Session, Widget, Bool) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Bool) -> Void
 */
lib7_val_t

_lib7_Gtk_set_label_line_wrapping   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               b1 =                              REC_SEL(    arg, 2) == LIB7_true;

    gtk_label_set_line_wrap( GTK_LABEL(/*label*/w0), /*wrap_lines*/b1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_label_underlines
 *
 * gtk.api        type:   (Session, Widget, String) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), String) -> Void
 */
lib7_val_t

_lib7_Gtk_set_label_underlines   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    char*             s1 =                 STR_LIB7toC (REC_SEL(    arg, 2));

    gtk_label_set_pattern( GTK_LABEL(/*label*/w0), /*underlines*/s1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_scale_value_position
 *
 * gtk.api        type:   (Session, Widget, Position_Type) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_scale_value_position   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_scale_set_value_pos( GTK_SCALE(/*scale*/w0), int_to_position(/*position_to_int position*/i1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_draw_scale_value
 *
 * gtk.api        type:   (Session, Widget, Bool) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Bool) -> Void
 */
lib7_val_t

_lib7_Gtk_set_draw_scale_value   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               b1 =                              REC_SEL(    arg, 2) == LIB7_true;

    gtk_scale_set_draw_value( GTK_SCALE(/*scale*/w0), /*draw_value*/b1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_scale_value_digits_shown
 *
 * gtk.api        type:   (Session, Widget) -> Int
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int
 */
lib7_val_t

_lib7_Gtk_get_scale_value_digits_shown   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int result = gtk_scale_get_digits( GTK_SCALE(/*scale*/w0) );

    return INT_CtoLib7(result);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_scale_value_digits_shown
 *
 * gtk.api        type:   (Session, Widget, Int)  -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_scale_value_digits_shown   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_scale_set_digits( GTK_SCALE(/*scale*/w0), /*digits*/i1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_range_update_policy
 *
 * gtk.api        type:   (Session, Widget, Update_Policy) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_range_update_policy   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_range_set_update_policy( GTK_RANGE(/*scale*/w0), /*policy*/int_to_range_update_policy(/*update_policy_to_int policy*/i1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_toggle_button_state
 *
 * gtk.api        type:   (Session, Widget) -> Bool
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Bool
 */
lib7_val_t

_lib7_Gtk_get_toggle_button_state   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    int result = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(/*toggle_button*/w0) );

    return  result ? LIB7_true : LIB7_false;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_toggle_button_state
 *
 * gtk.api        type:   (Session, Widget, Bool) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Bool) -> Void
 */
lib7_val_t

_lib7_Gtk_set_toggle_button_state   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               b1 =                              REC_SEL(    arg, 2) == LIB7_true;

    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(/*toggle_button*/w0), /*state*/b1 != 0 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_adjustment_value
 *
 * gtk.api        type:   (Session, Widget) -> Float
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Float
 */
lib7_val_t

_lib7_Gtk_get_adjustment_value   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    double d = gtk_adjustment_get_value( GTK_ADJUSTMENT(/*adjustment*/w0) );

    lib7_val_t               result;
    REAL64_ALLOC(lib7_state, result, d);
    return                   result;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_adjustment_value
 *
 * gtk.api        type:   (Session, Widget, Float) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Float) -> Void
 */
lib7_val_t

_lib7_Gtk_set_adjustment_value   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    double            f1 =        *(PTR_LIB7toC(double, REC_SEL(    arg, 2)));

    gtk_adjustment_set_value( GTK_ADJUSTMENT(/*adjustment*/w0), /*value*/f1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_white_graphics_context
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_white_graphics_context   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) /*widget*/w0->style->white_gc;

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_black_graphics_context
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_black_graphics_context   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) /*widget*/w0->style->black_gc;

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_current_foreground_graphics_context
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_current_foreground_graphics_context   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) w0->style->fg_gc[ GTK_WIDGET_STATE(/*widget*/w0) ];

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_current_background_graphics_context
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_current_background_graphics_context   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) w0->style->bg_gc[ GTK_WIDGET_STATE(/*widget*/w0) ];

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_widget_window
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_widget_window   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) /*widget*/w0->window;

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_add_kid
 *
 * gtk.api        type:    { session: Session,   mom: Widget,   kid: Widget } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_add_kid   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    gtk_container_add( GTK_CONTAINER(/*mom*/w0), GTK_WIDGET(/*kid*/w1));

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_add_scrolled_window_kid
 *
 * gtk.api        type:    { session: Session,   window: Widget,   kid: Widget } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*)) -> Void
 */
lib7_val_t

_lib7_Gtk_add_scrolled_window_kid   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];

    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(/*window*/w0), GTK_WIDGET(/*kid*/w1) );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_add_table_kid
 *
 * gtk.api        type:    { session: Session,   table: Widget,   kid: Widget,   left: Int,   right: Int,   top: Int,   bottom: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Int, Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_add_table_kid   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);
    int               i4 =                              REC_SELINT( arg, 5);
    int               i5 =                              REC_SELINT( arg, 6);

    gtk_table_attach_defaults( GTK_TABLE(/*table*/w0), GTK_WIDGET(/*kid*/w1), /*left*/i2, /*right*/i3, /*top*/i4, /*bottom*/i5 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_add_table_kid2
 *
 * gtk.api        type:    { session: Session,   table: Widget,   kid: Widget,   left: Int,   right: Int,   top: Int,   bottom: Int,   xoptions: List( Table_Attach_Option ),   yoptions: List( Table_Attach_Option ),   xpadding: Int,   ypadding: Int }   ->   Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int(*Widget*), Int, Int, Int, Int, Int, Int, Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_add_table_kid2   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    GtkWidget*        w1 =    (GtkWidget*)      widget[ REC_SELINT( arg, 2) ];
    int               i2 =                              REC_SELINT( arg, 3);
    int               i3 =                              REC_SELINT( arg, 4);
    int               i4 =                              REC_SELINT( arg, 5);
    int               i5 =                              REC_SELINT( arg, 6);
    int               i6 =                              REC_SELINT( arg, 7);
    int               i7 =                              REC_SELINT( arg, 8);
    int               i8 =                              REC_SELINT( arg, 9);
    int               i9 =                              REC_SELINT( arg, 10);

    gtk_table_attach( GTK_TABLE(/*table*/w0), GTK_WIDGET(/*kid*/w1), /*left*/i2, /*right*/i3, /*top*/i4, /*bottom*/i5, /*sum_table_attach_options xoptions*/i6, /*sum_table_attach_options yoptions*/i7, /*xpadding*/i8, /*ypadding*/i9 );

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_viewport_vertical_adjustment
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_viewport_vertical_adjustment   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) gtk_viewport_get_vadjustment( GTK_VIEWPORT(/*viewport*/w0) );

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_get_viewport_horizontal_adjustment
 *
 * gtk.api        type:   (Session, Widget) -> Widget
 * gtk-driver.api type:   (Session, Int(*Widget*)) -> Int (*Widget*)
 */
lib7_val_t

_lib7_Gtk_get_viewport_horizontal_adjustment   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];

    GtkWidget* widget = (GtkWidget*) gtk_viewport_get_hadjustment( GTK_VIEWPORT(/*viewport*/w0) );

    int slot = get_widget_id( widget );

    return INT_CtoLib7(slot);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_table_row_spacing
 *
 * gtk.api        type:    { session: Session, table: Widget, row: Int, spacing: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_table_row_spacing   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_table_set_row_spacing( GTK_TABLE(/*table*/w0), /*row*/i1, /*spacing*/i2);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_table_col_spacing
 *
 * gtk.api        type:    { session: Session, table: Widget, col: Int, spacing: Int } -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int, Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_table_col_spacing   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);
    int               i2 =                              REC_SELINT( arg, 3);

    gtk_table_set_col_spacing( GTK_TABLE(/*table*/w0), /*col*/i1, /*spacing*/i2);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_table_row_spacings
 *
 * gtk.api        type:   (Session, Widget, Int) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_table_row_spacings   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_table_set_row_spacings( GTK_TABLE(/*table*/w0), /*spacing*/i1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */


/* _lib7_Gtk_set_table_col_spacings
 *
 * gtk.api        type:   (Session, Widget, Int) -> Void
 * gtk-driver.api type:   (Session, Int(*Widget*), Int) -> Void
 */
lib7_val_t

_lib7_Gtk_set_table_col_spacings   (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ REC_SELINT( arg, 1) ];
    int               i1 =                              REC_SELINT( arg, 2);

    gtk_table_set_col_spacings( GTK_TABLE(/*table*/w0), /*spacing*/i1);

    return LIB7_void;
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_plain_fun. */




/*  _lib7_Gtk_set_clicked_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_clicked_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "clicked", G_CALLBACK( run_clicked_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_pressed_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_pressed_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "pressed", G_CALLBACK( run_pressed_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_release_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_release_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "release", G_CALLBACK( run_release_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_enter_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_enter_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "enter", G_CALLBACK( run_enter_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_leave_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_leave_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "leave", G_CALLBACK( run_leave_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_activate_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_activate_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( GTK_MENU_ITEM(widget), "activate", G_CALLBACK( run_activate_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_destroy_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_destroy_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "destroy", G_CALLBACK( run_destroy_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_realize_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_realize_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "realize", G_CALLBACK( run_realize_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_button_press_event_callback : Session -> Widget -> Button_Event_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_button_press_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "button_press_event", G_CALLBACK( run_button_press_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_button_release_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_button_release_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "button_release_event", G_CALLBACK( run_button_release_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_scroll_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_scroll_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "scroll_event", G_CALLBACK( run_scroll_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_motion_notify_event_callback : Session -> Widget -> Motion_Event_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_motion_notify_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "motion_notify_event", G_CALLBACK( run_motion_notify_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_delete_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_delete_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "delete_event", G_CALLBACK( run_delete_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_expose_event_callback : Session -> Widget -> Expose_Event_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_expose_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "expose_event", G_CALLBACK( run_expose_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_key_press_event_callback : Session -> Widget -> Key_Event_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_key_press_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "key_press_event", G_CALLBACK( run_key_press_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_key_release_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_key_release_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "key_release_event", G_CALLBACK( run_key_release_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_enter_notify_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_enter_notify_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "enter_notify_event", G_CALLBACK( run_enter_notify_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_leave_notify_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_leave_notify_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "leave_notify_event", G_CALLBACK( run_leave_notify_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_configure_event_callback : Session -> Widget -> Configure_Event_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_configure_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "configure_event", G_CALLBACK( run_configure_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_focus_in_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_focus_in_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "focus_in_event", G_CALLBACK( run_focus_in_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_focus_out_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_focus_out_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "focus_out_event", G_CALLBACK( run_focus_out_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_map_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_map_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "map_event", G_CALLBACK( run_map_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_unmap_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_unmap_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "unmap_event", G_CALLBACK( run_unmap_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_property_notify_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_property_notify_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "property_notify_event", G_CALLBACK( run_property_notify_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_selection_clear_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_selection_clear_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "selection_clear_event", G_CALLBACK( run_selection_clear_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_selection_request_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_selection_request_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "selection_request_event", G_CALLBACK( run_selection_request_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_selection_notify_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_selection_notify_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "selection_notify_event", G_CALLBACK( run_selection_notify_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_proximity_in_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_proximity_in_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "proximity_in_event", G_CALLBACK( run_proximity_in_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_proximity_out_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_proximity_out_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "proximity_out_event", G_CALLBACK( run_proximity_out_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_client_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_client_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "client_event", G_CALLBACK( run_client_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_no_expose_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_no_expose_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "no_expose_event", G_CALLBACK( run_no_expose_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_window_state_event_callback : Session -> Widget -> Void_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_window_state_event_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "window_state_event", G_CALLBACK( run_window_state_event_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_toggled_callback : Session -> Widget -> Bool_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_toggled_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "toggled", G_CALLBACK( run_toggled_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */




/*  _lib7_Gtk_set_value_changed_callback : Session -> Widget -> Float_Callback -> Void
 */
lib7_val_t

_lib7_Gtk_set_value_changed_callback (lib7_state_t *lib7_state, lib7_val_t arg)
{
#if (HAVE_GTK_2_0_GTK_GTK_H || HAVE_GTK_GTK_H)

    GtkWidget*        w0 =    (GtkWidget*)      widget[ INT_LIB7toC(arg) ];

    int id   =  find_free_callback_id ();

    g_signal_connect( G_OBJECT(widget), "value_changed", G_CALLBACK( run_value_changed_callback ), (void*)id );

    return INT_CtoLib7(id);
#else
    extern char* no_gtk_support_in_runtime;
    return RAISE_ERROR(lib7_state, no_gtk_support_in_runtime, __LINE__);
#endif
}
/* Above fn generated by write_gtk_driver_c_set_callback_fun. */


/* Do not edit this or preceding lines -- they are autogenerated by make-gtk-glue. */




/* Code by Jeff Prothero: Copyright (c) 2010,
 * released under Gnu Public Licence version 3.
 */
