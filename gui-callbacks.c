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

// Callback function for the 'clicked' event on btnEvalNeuraNet
gboolean CbBtnEvalNeuraNetClicked(
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
      "Select the NeuraNet file",
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
      appInpEvalNeuraNet,
      filename);

    // Load the NeuraNet
    LoadNeuraNet(filename);

    g_free(filename);

  }

  gtk_widget_destroy (dialog);

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'clicked' event on btnTrainNeuraNet
gboolean CbBtnTrainNeuraNetClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  GtkWidget* dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;

  dialog = \
    gtk_file_chooser_dialog_new(
      "Select the NeuraNet file",
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

    // Try to open the file to check if it's correct
    FILE* fp =
      fopen(
        filename,
        "w");
    if (fp != NULL) {

      gtk_entry_set_text(
        appInpTrainNeuraNet,
        filename);
      fclose(fp);

    }

    g_free(filename);

  }

  gtk_widget_destroy (dialog);

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'clicked' event on btnTrainStart
gboolean CbBtnTrainStartClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  // Empty the text buffers
  GtkTextBuffer* txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainMsgDepth));
  gtk_text_buffer_set_text(
    txtBuffer,
    "\0",
    -1);
  txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainNeuraNetDepth));
  gtk_text_buffer_set_text(
    txtBuffer,
    "\0",
    -1);
  txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainMsgTotal));
  gtk_text_buffer_set_text(
    txtBuffer,
    "\0",
    -1);
  txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainNeuraNetTotal));
  gtk_text_buffer_set_text(
    txtBuffer,
    "\0",
    -1);

  // Reset the progress bars
  threadTrainCompletionTotal = 0.0;
  gtk_progress_bar_set_fraction(
    appProgTrainTotal,
    threadTrainCompletionTotal);
  threadTrainCompletionDepth = 0.0;
  gtk_progress_bar_set_fraction(
    appProgTrainDepth,
    threadTrainCompletionDepth);

  // If the dataset has at least 2 categories
  if (GDSGetNbCat(appDataset) > 1) {

    // If there are samples in the train and valid category of the
    // current dataset
    long sizeCatTrain =
      GDSGetSizeCat(
        appDataset,
        0);
    long sizeCatValid =
      GDSGetSizeCat(
        appDataset,
        1);
    if (
      sizeCatTrain > 0 &&
      sizeCatValid > 0) {

      // If the parameters of the traning are valid
      JSONNode* node =
        JSONProperty(
          appConf.config,
          "inpTrainNbEpoch");
      threadTrainNbEpoch = atoi(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpTrainDepth");
      threadTrainDepth = atoi(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpTrainNbElite");
      threadTrainNbElite = atoi(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpTrainSizePool");
      threadTrainSizePool = atoi(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpTrainBestVal");
      threadTrainBestVal = atof(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpTrainNbThread");
      threadTrainNbThread = atoi(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpNbIn");
      threadTrainNbIn = atoi(JSONLblVal(node));
      node =
        JSONProperty(
          appConf.config,
          "inpNbOut");
      threadTrainNbOut = atoi(JSONLblVal(node));
      int sampleDim =
        VecGet(
          GDSSampleDim(appDataset),
          0);
      if (
        threadEvalNbInput > 0 &&
        threadEvalNbOutput > 0 &&
        threadEvalNbInput + threadEvalNbOutput == sampleDim &&
        threadTrainNbEpoch > 0 &&
        threadTrainDepth > 0 &&
        threadTrainNbElite > 0 &&
        threadTrainSizePool > threadTrainNbElite &&
        threadTrainBestVal <= 0.0 &&
        threadTrainNbThread > 0) {

        // Raise the flag
        appIsTraining = TRUE;

        // Init the current best val and topo
        threadTrainCurBestVal = threadTrainBestVal - 1000.0;
        threadTrainBestTopo.bases = NULL;
        threadTrainBestTopo.links = NULL;
        *threadTrainTopos = GSetCreateStatic();

        // Lock the button to avoid running another training
        // before this one ended
        gtk_widget_set_sensitive(
          GTK_WIDGET(appBtnTrainNeuraNet),
          FALSE);
        gtk_widget_set_sensitive(
          GTK_WIDGET(appBtnTrainStart),
          FALSE);
        gtk_widget_set_sensitive(
          GTK_WIDGET(appBtnDataset),
          FALSE);
        gtk_widget_set_sensitive(
          GTK_WIDGET(appBtnShuffle),
          FALSE);
        gtk_widget_set_sensitive(
          GTK_WIDGET(appBtnSplit),
          FALSE);

        // Start a thread
        GThread* thread =
          g_thread_new(
            "threadTrain",
            ThreadWorkerTrain,
            NULL);
        g_thread_unref(thread);

      } else {

        GtkTextBuffer* txtBuffer =
          gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(appTextBoxTrainMsgTotal));
        char* msg = "The parameters for training are invalid.\n";
        gtk_text_buffer_insert_at_cursor(
          txtBuffer,
          msg,
          strlen(msg));

      }

    } else {

      GtkTextBuffer* txtBuffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainMsgTotal));
      char* msg = "The train and/or valid category is empty.\n";
      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        msg,
        strlen(msg));

    }

  } else {

    GtkTextBuffer* txtBuffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainMsgTotal));
    char* msg = "The dataset has not been splitted.\n";
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      msg,
      strlen(msg));

  }

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

// Callback function for the 'changed' event on inpEvalNeuraNet
gboolean CbInpEvalNeuraNetChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpEvalNeuraNet");
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

  // Shuffle all the categories of the dataset
  GDSShuffleAll(appDataset);

  // Display the dataset
  DisplayGDataset();

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'clicked' event on btnSplit
gboolean CbBtnSplitClicked(
  GtkButton* btn,
    gpointer user_data) {

  // Unused argument
  (void)btn;
  (void)user_data;

  // Get the splitting
  int nbTrain = atoi(gtk_entry_get_text(appInpSplitTrain));
  int nbValid = atoi(gtk_entry_get_text(appInpSplitValid));
  int nbEval = atoi(gtk_entry_get_text(appInpSplitEval));

  // If the splitting is correct
  if (
    nbTrain > 0 &&
    nbValid > 0 &&
    nbEval > 0 &&
    nbTrain + nbValid + nbEval <= GDSGetSize(appDataset)) {

    // Split the dataset
    VecShort3D split = VecShortCreateStatic3D();
    VecSet(
      &split,
      0,
      nbTrain);
    VecSet(
      &split,
      1,
      nbValid);
    VecSet(
      &split,
      2,
      nbEval);
    GDSSplit(
      appDataset,
      (VecShort*)&split);

  }

  // Display the dataset
  DisplayGDataset();

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

  // Empty the text buffer
  GtkTextBuffer* txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));
  gtk_text_buffer_set_text(
    txtBuffer,
    "\0",
    -1);

  // Reset the progress bar
  threadEvalCompletion = 0.0;
  gtk_progress_bar_set_fraction(
    appProgEval,
    threadEvalCompletion);

  // Reset the result dataset
  GDSRemoveAllSample(threadEvalDataset);

  // If there is a NeuraNet loaded
  if (appNeuranet != NULL) {

    // Get the index of the category
    threadEvalCat = 0;
    if (gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(appRadEvalTrain))) {

      threadEvalCat = 0;

    } else if (gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(appRadEvalValid))) {

      threadEvalCat = 1;

    } else if (gtk_toggle_button_get_active(
      GTK_TOGGLE_BUTTON(appRadEvalEval))) {

      threadEvalCat = 2;

    }

    // If the category is valid
    if (threadEvalCat < GDSGetNbCat(appDataset)) {

      // If there are samples in the selected category of the
      // current dataset
      long sizeCat =
        GDSGetSizeCat(
          appDataset,
          threadEvalCat);
      if (sizeCat > 0) {

        // If the number of input and output are valid
        JSONNode* node =
          JSONProperty(
            appConf.config,
            "inpNbIn");
        threadEvalNbInput = atoi(JSONLblVal(node));
        node =
          JSONProperty(
            appConf.config,
            "inpNbOut");
        threadEvalNbOutput = atoi(JSONLblVal(node));
        int sampleDim =
          VecGet(
            GDSSampleDim(appDataset),
            0);
        if (
          threadEvalNbInput > 0 &&
          threadEvalNbOutput > 0 &&
          threadEvalNbInput + threadEvalNbOutput == sampleDim &&
          threadEvalNbInput == NNGetNbInput(appNeuranet) &&
          threadEvalNbOutput == NNGetNbOutput(appNeuranet)) {

          // Raise the flag
          appIsEvaluating = TRUE;

          // Lock the button to avoid running another evaluation
          // before this one ended
          gtk_widget_set_sensitive(
            GTK_WIDGET(appBtnEvalNeuraNet),
            FALSE);
          gtk_widget_set_sensitive(
            GTK_WIDGET(appBtnEval),
            FALSE);
          gtk_widget_set_sensitive(
            GTK_WIDGET(appBtnDataset),
            FALSE);
          gtk_widget_set_sensitive(
            GTK_WIDGET(appBtnShuffle),
            FALSE);
          gtk_widget_set_sensitive(
            GTK_WIDGET(appBtnSplit),
            FALSE);

          // Start a thread
          GThread* thread =
            g_thread_new(
              "threadEval",
              ThreadWorkerEval,
              NULL);
          g_thread_unref(thread);

        } else {

          GtkTextBuffer* txtBuffer =
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));
          char* msg = "The number of input/output is invalid.\n";
          gtk_text_buffer_insert_at_cursor(
            txtBuffer,
            msg,
            strlen(msg));

        }

      } else {

        GtkTextBuffer* txtBuffer =
          gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));
        char* msg = "The selected category is empty.\n";
        gtk_text_buffer_insert_at_cursor(
          txtBuffer,
          msg,
          strlen(msg));

      }

    } else {

      GtkTextBuffer* txtBuffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));
      char* msg = "The dataset has not been splitted.\n";
      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        msg,
        strlen(msg));

    }

  } else {

    GtkTextBuffer* txtBuffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));
    char* msg = "Please load a NeuraNet.\n";
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      msg,
      strlen(msg));

  }

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

  // Init the text boxes
  GUIInitTextBoxes(gtkBuilder);

  // Init the windows
  GUIInitWindows(gtkBuilder);

  // Init the callbacks
  GUIInitCallbacks(gtkBuilder);

  // Try to reload the last used dataset
  JSONNode* inp =
    JSONProperty(
      appConf.config,
      "inpDataset");
  LoadGDataset(JSONLblVal(inp));

  // Try to reload the last used neuranet
  inp =
    JSONProperty(
      appConf.config,
      "inpEvalNeuraNet");
  LoadNeuraNet(JSONLblVal(inp));

  // Free memory used by the GTK builder
  g_object_unref(G_OBJECT(gtkBuilder));

  // Display the main window and all its components
  gtk_widget_show_all(appWins.main);

  // Run the application at the GTK level
  gtk_main();

}

// Callback function for the 'changed' event on inpTrainNeuraNet
gboolean CbInpTrainNeuraNetChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainNeuraNet");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpTrainNbEpoch
gboolean CbInpTrainNbEpochChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainNbEpoch");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpTrainDepth
gboolean CbInpTrainDepthChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainDepth");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpTrainNbElite
gboolean CbInpTrainNbEliteChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainNbElite");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpTrainSizePool
gboolean CbInpTrainSizePoolChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainSizePool");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpTrainBestVal
gboolean CbInpTrainBestValChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainBestVal");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

// Callback function for the 'changed' event on inpTrainNbThread
gboolean CbInpTrainNbThreadChanged(
  GtkEntry* inp,
   gpointer user_data) {

  // Unused argument
  (void)user_data;

  JSONNode* node =
    JSONProperty(
      appConf.config,
      "inpTrainNbThread");
  JSONSetVal(
    node,
    gtk_entry_get_text(inp));

  // Return true to stop the callback chain
  return TRUE;

}

