/*
Copyright (C) 2013-2019 Alessio Garzi <gun101@email.it>
Copyright (C) 2013-2019 Francesco Minà <mina.francesco@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include <guake-indicator.h>
#include <guake-indicator-notify.h>
#include <guake-indicator-ayatana.h>
#include <string.h>
#include <stdlib.h>

AppIndicator *GLOBAL_CI = NULL;

void build_menu_ayatana(int argc, char **argv,GtkInfo* gtkinfo)
{

	apply_css();

	GLOBAL_CI = app_indicator_new ("guake-indicator",DEFAULT_ICON,APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_assert (IS_APP_INDICATOR (GLOBAL_CI));
	g_assert (G_IS_OBJECT (GLOBAL_CI));

    app_indicator_set_status (GLOBAL_CI, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_icon_full(GLOBAL_CI,"guake-indicator","guake-indicator");
	app_indicator_set_title (GLOBAL_CI, "Guake indicator");
    //g_signal_connect (ci, "scroll-event",G_CALLBACK (scroll_event_cb), NULL);

    
    //gtk_style_context_get_padding (menu_context, gtk_style_context_get_state (menu_context), &padding);
    //gtk_style_context_get_border (menu_context, gtk_style_context_get_state (menu_context), &border);

    //printf("\n\n%d %d %d %d %d %d\n",padding.left,border.left,padding.top,border.top,padding.bottom,border.bottom);


    gtk3_build_menu(gtkinfo);
    mainloop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainloop);

    return ;
}

void apply_css()
{
	GtkCssProvider *provider;

	provider = gtk_css_provider_new ();
    GtkWidget *window;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_css_provider_load_from_data (provider, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen (gtk_widget_get_screen (window),GTK_STYLE_PROVIDER (provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
}

static void
activate_clicked_cb (GtkWidget *widget, gpointer data)
{
    AppIndicator * ci = APP_INDICATOR(data);

    if (active) {
        app_indicator_set_status (ci, APP_INDICATOR_STATUS_ATTENTION);
        gtk_menu_item_set_label(GTK_MENU_ITEM(widget), "I'm okay now");
        active = FALSE;
    } else {
        app_indicator_set_status (ci, APP_INDICATOR_STATUS_ACTIVE);
        gtk_menu_item_set_label(GTK_MENU_ITEM(widget), "Get Attention");
        active = TRUE;
    }

}

static void
local_icon_toggle_cb (GtkWidget *widget, gpointer data)
{
    AppIndicator * ci = APP_INDICATOR(data);

    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
        app_indicator_set_icon_full(ci, "simple-client-test-icon.png", "Local Icon");
    } else {
        app_indicator_set_icon_full(ci, "indicator-messages", "System Icon");
    }

    return;
}

static void
scroll_event_cb (AppIndicator * ci, gint delta, guint direction, gpointer data)
{
    g_print("Got scroll event! delta: %d, direction: %d\n", delta, direction);
}


static void append_submenu (GtkWidget *menu,Host* ptr)
{
    GtkWidget *mi;
    GtkStyleContext *menu_context;
    gchar* menu_desc;
    void (*funct_ptr)(GtkAction*,gpointer);

    //printf("-----------%s %d\n",ptr->menu_name,ptr->open_in_tab==NULL);
    
    if (ptr->open_in_tab==NULL)
        mi = gtk_menu_item_new_with_label (ptr->menu_name);
    else
    {
        if (atol((char*)ptr->open_in_tab)==-1)
            menu_desc=g_strjoin(NULL,ptr->menu_name," (Current Tab)",NULL);
        else
            menu_desc=g_strjoin(NULL,ptr->menu_name," (Tab ",ptr->open_in_tab,")",NULL);
        mi = gtk_menu_item_new_with_label (menu_desc);
        g_free(menu_desc);
    }

    menu_context = gtk_widget_get_style_context (mi);
    gtk_style_context_add_class (menu_context, "titlebar");
    if (ptr->label==TRUE) gtk_widget_set_sensitive ((GtkWidget *)mi,FALSE);
    else 
    {
        // If open all is clicked then the call back function is group guake open
        if (ptr->group_head!=NULL)
            funct_ptr=group_guake_open;
        else
            if (ptr->dont_show_guake==NULL || g_strcmp0(ptr->dont_show_guake,"yes"))
                funct_ptr=guake_open_with_show;
            else
                funct_ptr=guake_open;

        ptr->right_click_funct_ptr=funct_ptr;

        g_signal_connect(mi,"button-press-event",G_CALLBACK(gtk3_detect_clickbutton),(gpointer) ptr);
        g_signal_connect (mi, "activate",G_CALLBACK (funct_ptr), (gpointer) ptr);
    }
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), mi);
}

// Detect click button
gboolean gtk3_detect_clickbutton(GtkWidget *btn, GdkEventButton *event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3)
    {
    	GtkWindow *main_app_window; // Window the dialog should show up on
		GtkWidget *dialog;
		//GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
		GtkDialogFlags flags = GTK_DIALOG_MODAL ;
		dialog = gtk_dialog_new_with_buttons ("Chose open method",
                                      main_app_window,
                                      flags,
                                      ("Open in new vertical split on current tab"),
                                      0,
                                      ("Open in new horizontal split on current tab"),
                                      1,
									  ("Execute on all terminals on current tab"),
                                      2,
                                      ("Execute only on focused terminal on current tab"),
                                      3,
                                      NULL);

		gtk_widget_show_all(dialog);
		//g_signal_connect(dialog, "destroy", G_CALLBACK(guake_open), userdata);
		//g_signal_connect(dialog, "destroy", G_CALLBACK(guake_open), userdata);
		switch (gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			case 0:
				((Host*)userdata)->force_current_tab=TRUE;
				((Host*)userdata)->vertical_split_current_tab=TRUE;
				((Host*)userdata)->horizontal_split_current_tab=FALSE;
                ((Host*)userdata)->force_current_split=FALSE;
				gtk_widget_destroy (dialog);
                ((Host*)userdata)->right_click_funct_ptr(NULL,userdata);
				return TRUE;
			case 1:
				((Host*)userdata)->force_current_tab=TRUE;
				((Host*)userdata)->vertical_split_current_tab=FALSE;
				((Host*)userdata)->horizontal_split_current_tab=TRUE;
                ((Host*)userdata)->force_current_split=FALSE;
				gtk_widget_destroy (dialog);
                ((Host*)userdata)->right_click_funct_ptr(NULL,userdata);
				return TRUE;
			case 2:
				((Host*)userdata)->force_current_tab=TRUE;
				((Host*)userdata)->vertical_split_current_tab=FALSE;
				((Host*)userdata)->horizontal_split_current_tab=FALSE;
				((Host*)userdata)->force_current_split=FALSE;
				gtk_widget_destroy (dialog);
                ((Host*)userdata)->right_click_funct_ptr(NULL,userdata);
                 return TRUE;
            case 3:
				((Host*)userdata)->force_current_tab=TRUE;
				((Host*)userdata)->vertical_split_current_tab=FALSE;
				((Host*)userdata)->horizontal_split_current_tab=FALSE;
				((Host*)userdata)->force_current_split=TRUE;
				gtk_widget_destroy (dialog);
                ((Host*)userdata)->right_click_funct_ptr(NULL,userdata);
                return TRUE;
			default:
				gtk_widget_destroy (dialog);
				return TRUE;
		}
    }
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 1)
    {
    	((Host*)userdata)->force_current_tab=FALSE;
    	((Host*)userdata)->vertical_split_current_tab=FALSE;
		((Host*)userdata)->horizontal_split_current_tab=FALSE;
        ((Host*)userdata)->force_current_split=FALSE;
        return 0;
    }
    return 0;
}


// Function to close the indicator
static void gtk3_close_guake_indicator ( GtkAction *action, gpointer user_data)
{
        GArray* grouphostlist= ((GtkInfo*)user_data)->grouphostlist;
        grouphostlist_free(grouphostlist);
        g_main_loop_quit(mainloop);
        //gtk_widget_destroy((GtkWidget *)menu);
        gtk_main_quit();
        return ;
}

void refresh_indicator_ayatana(gpointer user_data)
{
    gtk3_reload (NULL,user_data);
}

static void gtk3_build_menu(GtkInfo* gtkinfo)
{
    GtkWidget *menu = NULL;
    GtkStyleContext *menu_context;
    GtkWidget *item;
    unsigned int i;
    GtkWidget *submenu = NULL;
    Host* ptr;
    void (*funct_ptr)(GtkAction*,gpointer);

    menu = gtk_menu_new ();

    for (i=0;gtkinfo->grouphostlist!=NULL && i<gtkinfo->grouphostlist->len;i++)
    {
        HostGroup* hostgroup = g_array_index (gtkinfo->grouphostlist, HostGroup* , i);
        if (hostgroup->title)
        {
            GtkWidget *toggle_item = gtk_menu_item_new_with_label (hostgroup->title);
            menu_context = gtk_widget_get_style_context (toggle_item);
            gtk_style_context_add_class (menu_context, "titlebar");
            if (hostgroup->label==TRUE) gtk_widget_set_sensitive ((GtkWidget *)toggle_item,FALSE);
            else
            {
                submenu = gtk_menu_new ();
                for (ptr=hostgroup->hostarray;ptr;ptr=ptr->next)
                {
                    //printf("%s\n",ptr->menu_name);
                    append_submenu (submenu,ptr);
                }
                gtk_widget_show_all (submenu);
                gtk_menu_item_set_submenu (GTK_MENU_ITEM (toggle_item), submenu);
            }
            gtk_menu_shell_append (GTK_MENU_SHELL (menu), toggle_item);
            gtk_widget_show (toggle_item);
        }
        else
        {
            for (ptr=hostgroup->hostarray;ptr;ptr=ptr->next)
            {
                if (ptr->open_in_tab==NULL)
                {
                    item = gtk_menu_item_new_with_label (ptr->menu_name);
                    menu_context = gtk_widget_get_style_context (item);
                    gtk_style_context_add_class (menu_context, "titlebar");
                }
                else
                {
                    gchar* menu_desc;
                    if (atol((char*)ptr->open_in_tab)==-1)
                        menu_desc=g_strjoin(NULL,ptr->menu_name," (Current Tab)",NULL);
                    else
                        menu_desc=g_strjoin(NULL,ptr->menu_name," (Tab ",ptr->open_in_tab,")",NULL);
                    
                    item = gtk_menu_item_new_with_label (menu_desc);
                    menu_context = gtk_widget_get_style_context (item);
                    gtk_style_context_add_class (menu_context, "titlebar");
                    g_free(menu_desc);
                }

                if (ptr->label==TRUE) gtk_widget_set_sensitive ((GtkWidget *)item,FALSE);
                else 
                {
                    if (ptr->dont_show_guake==NULL || g_strcmp0(ptr->dont_show_guake,"yes"))
                        funct_ptr=guake_open_with_show;
                    else
                        funct_ptr=guake_open;
                    
                    ptr->right_click_funct_ptr=funct_ptr;
                    g_signal_connect(item,"button-press-event",G_CALLBACK(gtk3_detect_clickbutton),(gpointer) ptr);
                    g_signal_connect (item, "activate",G_CALLBACK (funct_ptr), (gpointer) ptr);
                }
                
                gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
                gtk_widget_show (item);
            }

        }
    }

    // Build default actions
    item=gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    gtk_widget_show (item);
    for (i=0;i<GUAKE_INDICATOR_DEFAULT_MENUITEMS_ARRAY_SIZE;i++)
    {
        item = gtk_menu_item_new_with_label (GUAKE_INDICATOR_DEFAULT_MENUITEMS_ARRAY_GTK3[i].label);
        menu_context = gtk_widget_get_style_context (item);
        gtk_style_context_add_class (menu_context, "titlebar");
        if (GUAKE_INDICATOR_DEFAULT_MENUITEMS_ARRAY_GTK3[i].action_function) g_signal_connect (item, "activate",G_CALLBACK (GUAKE_INDICATOR_DEFAULT_MENUITEMS_ARRAY_GTK3[i].action_function), (gpointer) gtkinfo);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
        gtk_widget_show (item);
    }

    app_indicator_set_menu (GLOBAL_CI, GTK_MENU (menu));
}

static void gtk3_reload (GtkAction* action,gpointer user_data)
{
    reload((GtkInfo*)user_data);
    gtk3_build_menu((GtkInfo*)user_data);
    guake_notify("Guake indicator","Reload completed");
    return ;
}

static void gtk3_about (GtkAction* action,gpointer data)
{
    GError *error = NULL;
    const gchar *authors[] = {
        "Alessio Garzi <gun101@email.it>",
        "Francesco Minà <mina.francesco@gmail.com>",
        NULL
    };
    const gchar* license ="Guake indicator is free software; you can redistribute it and/or\n"
                    " modify it under the terms of the GNU General Public License\n"
                    " as published by the Free Software Foundation; either version 2\n"
                    " of the License, or (at your option) any later version.\n\n"

                    "This program is distributed in the hope that it will be useful,\n"
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                    " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                    " GNU General Public License for more details.\n\n"

                    "You should have received a copy of the GNU General Public License\n"
                    "along with this program; if not, write to the Free Software\n"
                    "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.";
    
    GdkPixbuf *logo = gdk_pixbuf_new_from_file (DATADIR"/"GUAKE_INDICATOR_ICON_DIR"/guake-indicator.png", &error);  
    if (error != NULL)
    {
        if (error->domain == GDK_PIXBUF_ERROR)
            g_print ("GdkPixbufError: %s\n", error->message);
        else if (error->domain == G_FILE_ERROR)
            g_print ("GFileError: %s\n", error->message);
        else
            g_print ("An error in the domain: %d has occurred!\n", error->domain);
        g_error_free (error);
    }

    gtk_show_about_dialog(NULL,
                            "program-name", "Guake indicator",
                            "authors", authors,
                            "comments", "A simple indicator that lets you send custom commands to Guake/Guake3.",
                            "copyright", "(C) 2013-2020 Alessio Garzi\n(C) 2013-2020 Francesco Mina\n\nDedicated to my daughters\n Ludovica and Mariavittoria",
                            "logo", logo,
                            "version", GUAKE_INDICATOR_VERSION, 
                            "website", "http://guake-indicator.ozzyboshi.com",
                            "license",license,
                            NULL);
}
