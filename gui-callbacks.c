// Callback for the timer
gboolean CbTimer(gpointer data) {

  // Unused parameter
  (void)data;

  printf("Tick\n");

  // Refresh all the widgets
  GUIRefreshWidgets();

  // Return TRUE to keep the timer alive
  // It will be killed by GUIFree
  return TRUE;

}

// Callback function for the 'clicked' event on btnDataset
gboolean CbBtnDatasetClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  GtkWidget* dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  dialog = \
    gtk_file_chooser_dialog_new(
      "Select the GDataSet file",
      GTK_WINDOW(app.windows.main),
      action,
      "_Cancel",
      GTK_RESPONSE_CANCEL,
      "_Open",
      GTK_RESPONSE_ACCEPT,
      NULL);

  res = gtk_dialog_run(GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT) {
    char* filename = NULL;
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
    filename = gtk_file_chooser_get_filename(chooser);

    gtk_entry_set_text(
      appInpDataset,
      filename);

    // Load the dataset
    LoadGDataset(filename);

    g_free(filename);
  }

  gtk_widget_destroy (dialog);

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpDataset
gboolean CbInpDatasetChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpDataset");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpNbIn
gboolean CbInpNbInChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpNbIn");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpNbOut
gboolean CbInpNbOutChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpNbOut");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpSplitTrain
gboolean CbInpSplitTrainChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpSplitTrain");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpSplitValid
gboolean CbInpSplitValidChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpSplitValid");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpSplitEval
gboolean CbInpSplitEvalChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpSplitEval");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'clicked' event on btnShuffle
gboolean CbBtnShuffleClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  printf("btnShuffle clicked\n");

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'clicked' event on btnEval
gboolean CbBtnEvalClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  printf("btnEval clicked\n");

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'clicked' event on btnSelectDataset
gboolean CbBtnSelectDatasetClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  printf("btnSelectDataset clicked\n");

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'delete-event' event on the GTK application
// window
gboolean CbAppWindowDeleteEvent(
          GtkWidget* widget,
  GdkEventConfigure* event,
            gpointer user_data) {

  // Unused arguments
  (void)widget;
  (void)event;
  (void)user_data;

  // Quit the application
  GUIQuit();

  // Return false to continue the callback chain
  return FALSE;

}

// Callback function for the 'check-resize' event on the GTK application
// window
gboolean CbAppWindowResizeEvent(
          GtkWidget* widget,
  GdkEventConfigure* event,
            gpointer user_data) {

  // Unused arguments
  (void)widget;
  (void)event;
  (void)user_data;

  printf("resize\n");

  // Return false to continue the callback chain
  return FALSE;

}

// Callback function for the 'activate' event on the GTK application
void CbGtkAppActivate(
  GtkApplication* gtkApp,
         gpointer user_data) {

  // Unused arguments
  (void)gtkApp;
  (void)user_data;

  // Create a GTK builder with the GUI definition file
  GtkBuilder* gtkBuilder =
    gtk_builder_new_from_file("main.glade");

  // Set the GTK application in the GTK builder
  gtk_builder_set_application(
    gtkBuilder,
    app.gtkApp);

  // Init the inputs
  GUIInitInputs(gtkBuilder);

  // Init the windows
  GUIInitWindows(gtkBuilder);

  // Init the callbacks
  GUIInitCallbacks(gtkBuilder);

  // Start the timer
  unsigned int timerIntervalMs = 1000;
  app.timerId = g_timeout_add (
    timerIntervalMs,
    CbTimer,
    NULL);

  // Free memory used by the GTK builder
  g_object_unref(G_OBJECT(gtkBuilder));

  // Display the main window and all its components
  gtk_widget_show_all(appWins.main);

  // Run the application at the GTK level
  gtk_main();

}
