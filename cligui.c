/* $Id: cligui.c,v 1.9 2013/12/02 10:24:36 moonsdad Exp $ */

/******************************************************************************
 * Warlords And Scumbags                                                      *
 *      cligui.c    --  Image and Pixbuf Creation helper Functions.           *
 ******************************************************************************/

#include "client.h"

/******************************************************************************/
/* Function:    load_pixbuf                                                   */
/* Parameters:  const gchar ptr     filename    -- Image to load              */
/* Returns:     GdkPixbuf ptr                   -- The loaded immage          */
/* WARNING: On Fail, just uses default img and displays warning in terminal.  */
/******************************************************************************/
GdkPixbuf* load_pixbuf( const gchar * filename )
{
   GdkPixbuf* pixbuf = NULL;
   GError* error = NULL;

   if( !(pixbuf = gdk_pixbuf_new_from_file( filename, &error )) ) {
      fprintf( stderr, "\nWARNING: %s\n", error->message );
      g_error_free( error );
   }/* End If */

   return pixbuf;
}/* End load_pixbuf Func */


/******************************************************************************/
/* Function:    create_image                                                  */
/* Parameters:  const gchar ptr     filename    -- Image to load              */
/*              GCallback           func        -- Called when image clicked  */
/*              gpointer            data        -- Passed on to func          */
/* Returns:     GtkWidget ptr                   -- Depends see below          */
/* Creates a gtk image and shows it. If a callback function is given it also  */
/* creates an event box, packs the image into it, and connects the function.  */
/* Either the image is returned or the event box if one was needed.           */
/* Modified From Example: http://www.gtk.org/api/2.6/gtk/GtkImage.html        */
/******************************************************************************/
GtkWidget* create_image( const gchar * filename, GCallback func, gpointer data )
{
    GtkWidget* image = gtk_image_new_from_file( filename );
    GtkWidget* event_box;

    if( func ) {
        event_box = gtk_event_box_new();

        gtk_container_add( GTK_CONTAINER (event_box), image );

        g_signal_connect( G_OBJECT (event_box), "button_press_event",
                          G_CALLBACK (func), data );

        gtk_widget_show(event_box); gtk_widget_show(image);
        return event_box;
    } /* End callback func If */

    gtk_widget_show(image);
    return image;
}/* End create_image Func */


/******************************************************************************/
/* Function:    widget_destroy -- Wrapper for gtk_widget_destroy              */
/******************************************************************************/
void widget_destroy( GtkWidget* widget, gpointer data )
{
    gtk_widget_destroy( widget );
}/* End Func */


/************************************EOF***************************************/
