#define _POSIX_C_SOURCE 200809L
#include <X11/Xcursor/Xcursor.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ICON_PATH "/usr/share/icons"
#define CONFIG_PATH ".config/sway/config"
#define MAX_THEMES 256

typedef struct {
  char name[256];
  char path[512];
} Theme;

static Theme themes[MAX_THEMES];
static int theme_count = 0;
static int selected_idx = 0;
static int cursor_size = 24;

static GtkWidget *list_box;
static GtkWidget *preview_image;
static GtkWidget *size_label;

static void update_preview() {
  if (selected_idx < 0 || selected_idx >= theme_count)
    return;

  char cursor_file[2048];
  snprintf(cursor_file, sizeof(cursor_file), "%s/cursors/left_ptr",
           themes[selected_idx].path);

  XcursorImage *ximg = XcursorFilenameLoadImage(cursor_file, cursor_size);
  if (ximg) {
    GdkPixbuf *pixbuf =
        gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, ximg->width, ximg->height);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);

    for (int y = 0; y < (int)ximg->height; y++) {
      for (int x = 0; x < (int)ximg->width; x++) {
        XcursorPixel p = ximg->pixels[y * ximg->width + x];
        guchar *ptr = pixels + y * rowstride + x * n_channels;
        ptr[0] = (p >> 16) & 0xFF;
        ptr[1] = (p >> 8) & 0xFF;
        ptr[2] = (p >> 0) & 0xFF;
        ptr[3] = (p >> 24) & 0xFF;
      }
    }

    GdkTexture *texture = gdk_texture_new_for_pixbuf(pixbuf);

    gtk_picture_set_content_fit(GTK_PICTURE(preview_image),
                                GTK_CONTENT_FIT_CONTAIN);

    gtk_widget_set_valign(preview_image, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(preview_image, GTK_ALIGN_CENTER);

    gtk_picture_set_paintable(GTK_PICTURE(preview_image),
                              GDK_PAINTABLE(texture));

    g_object_unref(pixbuf);
    g_object_unref(texture);
    XcursorImageDestroy(ximg);
  } else {
    gtk_picture_set_paintable(GTK_PICTURE(preview_image), NULL);
  }

  char sz_text[16];
  snprintf(sz_text, sizeof(sz_text), "%d", cursor_size);
  gtk_label_set_text(GTK_LABEL(size_label), sz_text);
}

static void load_themes(void) {
  DIR *dir = opendir(ICON_PATH);
  struct dirent *entry;
  if (!dir)
    return;
  while ((entry = readdir(dir)) && theme_count < MAX_THEMES) {
    if (entry->d_name[0] == '.')
      continue;
    char full_path[2048], cursors_dir[2048];
    snprintf(full_path, sizeof(full_path), "%s/%s", ICON_PATH, entry->d_name);
    snprintf(cursors_dir, sizeof(cursors_dir), "%s/cursors", full_path);
    struct stat st;
    if (stat(cursors_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
      strncpy(themes[theme_count].name, entry->d_name, 255);
      strncpy(themes[theme_count].path, full_path, 511);
      theme_count++;
    }
  }
  closedir(dir);
}

static void apply_theme_action(GtkWidget *widget, gpointer data) {
  char cmd[512], config_file[512], buffer[2048], *lines[2048];
  int line_count = 0, found = 0;
  const char *theme_name = themes[selected_idx].name;

  snprintf(cmd, sizeof(cmd), "swaymsg seat seat0 xcursor_theme %s %d",
           theme_name, cursor_size);
  system(cmd);

  snprintf(cmd, sizeof(cmd),
           "gsettings set org.gnome.desktop.interface cursor-theme '%s' && "
           "gsettings set org.gnome.desktop.interface cursor-size %d",
           theme_name, cursor_size);
  system(cmd);

  snprintf(config_file, sizeof(config_file), "%s/%s", getenv("HOME"),
           CONFIG_PATH);

  FILE *fp = fopen(config_file, "r");
  if (fp) {
    while (fgets(buffer, sizeof(buffer), fp) && line_count < 2048) {
      if (strstr(buffer, "seat") && strstr(buffer, "xcursor_theme")) {
        snprintf(buffer, sizeof(buffer), "seat seat0 xcursor_theme %s %d\n",
                 theme_name, cursor_size);
        found = 1;
      }
      lines[line_count++] = strdup(buffer);
    }
    fclose(fp);

    if ((fp = fopen(config_file, "w"))) {
      for (int i = 0; i < line_count; i++) {
        fputs(lines[i], fp);
        free(lines[i]);
      }
      if (!found)
        fprintf(fp, "\nseat seat0 xcursor_theme %s %d\n", theme_name,
                cursor_size);
      fclose(fp);
    }
  }
}

static void on_size_changed(GtkWidget *btn, gpointer data) {
  cursor_size += GPOINTER_TO_INT(data);
  if (cursor_size < 8)
    cursor_size = 8;
  if (cursor_size > 128)
    cursor_size = 128;
  update_preview();
}

static void on_row_selected(GtkListBox *box, GtkListBoxRow *row,
                            gpointer data) {
  if (!row)
    return;
  selected_idx = gtk_list_box_row_get_index(row);
  update_preview();
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "SwayXcursor");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 600);

  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_margin_start(main_vbox, 15);
  gtk_widget_set_margin_end(main_vbox, 15);
  gtk_widget_set_margin_top(main_vbox, 15);
  gtk_widget_set_margin_bottom(main_vbox, 15);
  gtk_window_set_child(GTK_WINDOW(window), main_vbox);

  GtkWidget *header_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  gtk_widget_set_halign(header_hbox, GTK_ALIGN_CENTER);

  GtkWidget *preview_frame = gtk_frame_new(NULL);
  gtk_widget_set_size_request(preview_frame, 0, 0);

  preview_image = gtk_picture_new();
  gtk_widget_set_halign(preview_image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(preview_image, GTK_ALIGN_CENTER);

  gtk_picture_set_content_fit(GTK_PICTURE(preview_image),
                              GTK_CONTENT_FIT_CONTAIN);
  gtk_frame_set_child(GTK_FRAME(preview_frame), preview_image);

  GtkWidget *size_ctrl = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_valign(size_ctrl, GTK_ALIGN_CENTER);
  GtkWidget *btn_inc = gtk_button_new_with_label("+");
  GtkWidget *btn_dec = gtk_button_new_with_label("-");
  size_label = gtk_label_new("24");

  g_signal_connect(btn_dec, "clicked", G_CALLBACK(on_size_changed),
                   GINT_TO_POINTER(-2));
  g_signal_connect(btn_inc, "clicked", G_CALLBACK(on_size_changed),
                   GINT_TO_POINTER(2));

  gtk_box_append(GTK_BOX(size_ctrl), btn_inc);
  gtk_box_append(GTK_BOX(size_ctrl), size_label);
  gtk_box_append(GTK_BOX(size_ctrl), btn_dec);

  gtk_box_append(GTK_BOX(header_hbox), preview_frame);
  gtk_box_append(GTK_BOX(header_hbox), size_ctrl);
  gtk_box_append(GTK_BOX(main_vbox), header_hbox);

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_widget_set_vexpand(scroll, TRUE);
  list_box = gtk_list_box_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), list_box);
  gtk_box_append(GTK_BOX(main_vbox), scroll);

  for (int i = 0; i < theme_count; i++) {
    gtk_list_box_append(GTK_LIST_BOX(list_box), gtk_label_new(themes[i].name));
  }

  GtkWidget *apply_btn = gtk_button_new_with_label("Apply Theme");
  gtk_widget_add_css_class(apply_btn, "suggested-action");
  g_signal_connect(apply_btn, "clicked", G_CALLBACK(apply_theme_action), NULL);
  gtk_box_append(GTK_BOX(main_vbox), apply_btn);

  g_signal_connect(list_box, "row-selected", G_CALLBACK(on_row_selected), NULL);

  update_preview();
  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
  if (argc > 1 &&
      (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
    printf("swayxcursor %s\n", VERSION);
    return 0;
  }
  load_themes();
  GtkApplication *app =
      gtk_application_new("org.sway.xcursor", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
