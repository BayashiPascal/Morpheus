#include "gui.h"

// Error manager
PBErr* AppErr = &thePBErr;

// Declare the global instance of the application
GUI app;

// Display the current dataset in the text box of the dataset tab
void DisplayGDataset(void);

// Include the callbacks
#include "gui-callbacks.c"

// Include the code relative to the thread
#include "gui-threads.c"

// Function to init the windows
void GUIInitWindows(GtkBuilder* const gtkBuilder) {

#if BUILDMODE == 0

  if (gtkBuilder == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'gtkBuilder' is null");
    PBErrCatch(AppErr);

  }

#endif

  // Get the main window
  GObject* mainWindow=
    gtk_builder_get_object(
      gtkBuilder,
      "appMainWin");
  appWins.main = GTK_WIDGET(mainWindow);

  // Allow window resizing
  gtk_window_set_resizable(
    GTK_WINDOW(appWins.main),
    TRUE);

}

// Free memory used by the runtime configuration
void GUIFreeConf(void) {

  free(appConf.gladeFilePath);
  free(appConf.configFilePath);
  free(appConf.rootDir);
  JSONFree(&(appConf.config));

}

// Free memory used by the application
void GUIFree(void) {

  // Free memory
  GUIFreeConf();
  GDataSetVecFloatFreeStatic(&(app.dataset));
  if (appNeuranet != NULL) {

    NeuraNetFree(&appNeuranet);

  }

}

// Function called before the application quit
void GUIQuit(void) {

  printf("quit\n");

  // Save the configuration
  GUISaveConfig();

  // Free memory used by the GUI
  GUIFree();

  // Quit the application at GTK level
  gtk_main_quit();

  // Quit the application at G level
  g_application_quit(app.gApp);

}

// Function to init the callbacks
void GUIInitCallbacks(GtkBuilder* const gtkBuilder) {

#if BUILDMODE == 0

  if (gtkBuilder == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'gtkBuilder' is null");
    PBErrCatch(AppErr);

  }

#endif

  // Set the callback on the delete-event of the main window
  g_signal_connect(
    appWins.main,
    "delete-event",
    G_CALLBACK(CbAppWindowDeleteEvent),
    NULL);

  // Set the callback on the check-resize of the main window
  g_signal_connect(
    appWins.main,
    "check-resize",
    G_CALLBACK(CbAppWindowResizeEvent),
    NULL);

  // Set the callback on the 'clicked' event of btnDataset
  GObject* obj =
    gtk_builder_get_object(
      gtkBuilder,
      "btnDataset");
  GtkWidget* btnDataset = GTK_WIDGET(obj);
  g_signal_connect(
    btnDataset,
    "clicked",
    G_CALLBACK(CbBtnDatasetClicked),
    NULL);

  // Set the callback on the 'clicked' event of btnEvalNeuraNet
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "btnEvalNeuraNet");
  GtkWidget* btnEvalNeuraNet = GTK_WIDGET(obj);
  g_signal_connect(
    btnEvalNeuraNet,
    "clicked",
    G_CALLBACK(CbBtnEvalNeuraNetClicked),
    NULL);

  // Set the callback on the 'clicked' event of btnShuffle
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "btnShuffle");
  GtkWidget* btnShuffle = GTK_WIDGET(obj);
  g_signal_connect(
    btnShuffle,
    "clicked",
    G_CALLBACK(CbBtnShuffleClicked),
    NULL);

  // Set the callback on the 'clicked' event of btnSplit
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "btnSplit");
  GtkWidget* btnSplit = GTK_WIDGET(obj);
  g_signal_connect(
    btnSplit,
    "clicked",
    G_CALLBACK(CbBtnSplitClicked),
    NULL);

  // Set the callback on the 'changed' event of inpDataset
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpDataset");
  GtkWidget* inpDataset = GTK_WIDGET(obj);
  g_signal_connect(
    inpDataset,
    "changed",
    G_CALLBACK(CbInpDatasetChanged),
    NULL);

  // Set the callback on the 'changed' event of inpEvalNeuraNet
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpEvalNeuraNet");
  GtkWidget* inpEvalNeuraNet = GTK_WIDGET(obj);
  g_signal_connect(
    inpEvalNeuraNet,
    "changed",
    G_CALLBACK(CbInpEvalNeuraNetChanged),
    NULL);

  // Set the callback on the 'changed' event of inpNbIn
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpNbIn");
  GtkWidget* inpNbIn = GTK_WIDGET(obj);
  g_signal_connect(
    inpNbIn,
    "changed",
    G_CALLBACK(CbInpNbInChanged),
    NULL);

  // Set the callback on the 'changed' event of inpNbOut
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpNbOut");
  GtkWidget* inpNbOut = GTK_WIDGET(obj);
  g_signal_connect(
    inpNbOut,
    "changed",
    G_CALLBACK(CbInpNbOutChanged),
    NULL);

  // Set the callback on the 'changed' event of inpSplitTrain
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpSplitTrain");
  GtkWidget* inpSplitTrain = GTK_WIDGET(obj);
  g_signal_connect(
    inpSplitTrain,
    "changed",
    G_CALLBACK(CbInpSplitTrainChanged),
    NULL);

  // Set the callback on the 'changed' event of inpSplitValid
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpSplitValid");
  GtkWidget* inpSplitValid = GTK_WIDGET(obj);
  g_signal_connect(
    inpSplitValid,
    "changed",
    G_CALLBACK(CbInpSplitValidChanged),
    NULL);

  // Set the callback on the 'changed' event of inpSplitEval
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "inpSplitEval");
  GtkWidget* inpSplitEval = GTK_WIDGET(obj);
  g_signal_connect(
    inpSplitEval,
    "changed",
    G_CALLBACK(CbInpSplitEvalChanged),
    NULL);

  // Set the callback on the 'clicked' event of btnEval
  obj =
    gtk_builder_get_object(
      gtkBuilder,
      "btnEval");
  GtkWidget* btnEval = GTK_WIDGET(obj);
  g_signal_connect(
    btnEval,
    "clicked",
    G_CALLBACK(CbBtnEvalClicked),
    NULL);

  // Disable the other signals defined in the UI definition file
  gtk_builder_connect_signals(
    gtkBuilder,
    NULL);

}

// Parse the command line arguments
// Return true if the arguments were valid, false else
bool GUIParseArg(
     int argc,
  char** argv) {

#if BUILDMODE == 0

  if (argv == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'argv' is null");
    PBErrCatch(AppErr);

  }

  if (argc < 0) {

    AppErr->_type = PBErrTypeInvalidArg;
    sprintf(
      AppErr->_msg,
      "'argc' is invalid (%d>=0)",
      argc);
    PBErrCatch(AppErr);

  }

#endif

  // Declare the return flag
  bool flag = TRUE;

  // Loop on the arguments, abort if we found invalid argument
  for (
    int iArg = 0;
    iArg < argc && flag;
    ++iArg) {

    // Parse the argument
    int retCmp =
      strcmp(
        argv[iArg],
        "-help");
    if (retCmp == 0) {

      // Print the help and quit
      printf("[-help] display the help\n");
      printf("[-root] path to the folder of the executable\n");
      printf("-conf <path_to_conf> config file\n");
      exit(1);

    }

    retCmp =
      strcmp(
        argv[iArg],
        "-root");
    if (
      iArg < argc - 1 &&
      retCmp == 0) {

      // If the path to the folder is not null free the memory
      if (appConf.rootDir != NULL)
        free(appConf.rootDir);

      // Memorize the path to the folder of the executable
      appConf.rootDir = strdup(argv[iArg + 1]);

    }

    retCmp =
      strcmp(
        argv[iArg],
        "-conf");
    if (retCmp == 0) {

      // If it's not the last argument and the argument is valid
      FILE* configFile = NULL;
      configFile =
        fopen(
          argv[iArg + 1],
          "r");
      if (
        iArg < argc - 1 &&
        configFile != NULL) {

        // Load the config file
        bool ret =
          JSONLoad(
            appConf.config,
            configFile);
        if (ret == FALSE) {

          // The config file is invalid
          sprintf(
            AppErr->_msg,
            "Couldn't load the config file");
          flag = FALSE;

        }

        // If the path to the config file is not null free the memory
        if (appConf.configFilePath != NULL)
          free(appConf.configFilePath);

        // Memorize the path to the config file to update it
        // when the user close the window
        ++iArg;
        appConf.configFilePath = strdup(argv[iArg]);

        // Close the config file
        fclose(configFile);

      // Else, it is the last argument
      } else {

        // The path to the config file is missing
        sprintf(
          AppErr->_msg,
          "Missing path to config file");
        flag = FALSE;

      }

    }

  }

  // Check for mandatory arguments
  if (appConf.config == NULL) {

    sprintf(
      AppErr->_msg,
      "Couldn't load the configuration file");
    flag = FALSE;

  }

  if (appConf.rootDir == NULL) {

    sprintf(
      AppErr->_msg,
      "No root dir");
    flag = FALSE;

  }

  // Return the flag
  return flag;

}

// Function to load the parameters of the application from the
// config file
void GUILoadConfig(void) {

  // Check the content of the configuration file
  // If there are missing parameters, create them
  char* paramNames[7] = {

    "inpDataset",
    "inpEvalNeuraNet",
    "inpNbIn",
    "inpNbOut",
    "inpSplitTrain",
    "inpSplitValid",
    "inpSplitEval"

  };

  for (
    int iParam = 0;
    iParam < 7;
    ++iParam) {

    JSONNode* node =
      JSONProperty(
        appConf.config,
        paramNames[iParam]);
    if (node == NULL) {

      JSONAddProp(
        appConf.config,
        paramNames[iParam],
        "");

    }

  }

}

// Save the current parameters in the config file
void GUISaveConfig(void) {

  // Save the configuration file
  FILE* configFile =
    fopen(
      appConf.configFilePath,
      "w");
  if (configFile != NULL) {

    bool ret =
      JSONSave(
        appConf.config,
        configFile,
        FALSE);
    if (ret == FALSE) {

      printf("Couldn't update the config file (JSONSave failed) !");

    }

    // Close the config file
    fclose(configFile);

  } else {

    printf("Couldn't update the config file (fopen failed) !");

  }

}

// Function to init the configuration
void GUIInitConf(
     int argc,
  char** argv) {

#if BUILDMODE == 0

  if (argv == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'argv' is null");
    PBErrCatch(AppErr);

  }

  if (argc < 0) {

    AppErr->_type = PBErrTypeInvalidArg;
    sprintf(
      AppErr->_msg,
      "'argc' is invalid (%d>=0)",
      argc);
    PBErrCatch(AppErr);

  }

#endif

  // Init the path to the config file
  appConf.configFilePath = NULL;

  // Init the path to the folder of the executable
  appConf.rootDir = NULL;

  // Init the default path for the Glade file
  appConf.gladeFilePath = strdup("./main.glade");

  // Init the content of the configuration file
  appConf.config = JSONCreate();

  // Parse the arguments
  bool ret =
    GUIParseArg(
      argc,
      argv);
  if (ret == FALSE) {

    // Terminate the application if the arguments are invalid
    printf("Invalid arguments");
    exit(0);

  }

  // Reload the parameters from the config file
  GUILoadConfig();

}

// Init the inputs
void GUIInitInputs(GtkBuilder* const gtkBuilder) {

#if BUILDMODE == 0

  if (gtkBuilder == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'gtkBuilder' is null");
    PBErrCatch(AppErr);

  }

#endif

  // Get the GtkWidget for the inputs
  appInpDataset = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpDataset"));
  appInpEvalNeuraNet = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpEvalNeuraNet"));
  appInpNbIn = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpNbIn"));
  appInpNbOut = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpNbOut"));
  appInpSplitTrain = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpSplitTrain"));
  appInpSplitValid = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpSplitValid"));
  appInpSplitEval = GTK_ENTRY(
    gtk_builder_get_object(
      gtkBuilder,
      "inpSplitEval"));
  appRadEvalTrain = GTK_RADIO_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "radEvalTrain"));
  appRadEvalValid = GTK_RADIO_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "radEvalValid"));
  appRadEvalEval = GTK_RADIO_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "radEvalEval"));
  appBtnEvalNeuraNet = GTK_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "btnEvalNeuraNet"));
  appBtnEval = GTK_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "btnEval"));
  appBtnDataset = GTK_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "btnDataset"));
  appBtnShuffle = GTK_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "btnShuffle"));
  appBtnSplit = GTK_BUTTON(
    gtk_builder_get_object(
      gtkBuilder,
      "btnSplit"));
  appProgEval = GTK_PROGRESS_BAR(
    gtk_builder_get_object(
      gtkBuilder,
      "progEval"));

  // Init the widgets with the value in the config file
  JSONNode* inp =
    JSONProperty(
      appConf.config,
      "inpNbIn");
  gtk_entry_set_text(
    appInpNbIn,
    JSONLblVal(inp));
  inp =
    JSONProperty(
      appConf.config,
      "inpNbOut");
  gtk_entry_set_text(
    appInpNbOut,
    JSONLblVal(inp));
  inp =
    JSONProperty(
      appConf.config,
      "inpSplitTrain");
  gtk_entry_set_text(
    appInpSplitTrain,
    JSONLblVal(inp));
  inp =
    JSONProperty(
      appConf.config,
      "inpSplitValid");
  gtk_entry_set_text(
    appInpSplitValid,
    JSONLblVal(inp));
  inp =
    JSONProperty(
      appConf.config,
      "inpSplitEval");
  gtk_entry_set_text(
    appInpSplitEval,
    JSONLblVal(inp));
  inp =
    JSONProperty(
      appConf.config,
      "inpDataset");
  gtk_entry_set_text(
    appInpDataset,
    JSONLblVal(inp));
  inp =
    JSONProperty(
      appConf.config,
      "inpEvalNeuraNet");
  gtk_entry_set_text(
    appInpEvalNeuraNet,
    JSONLblVal(inp));

}

// Init the text boxes
void GUIInitTextBoxes(GtkBuilder* const gtkBuilder) {

#if BUILDMODE == 0

  if (gtkBuilder == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'gtkBuilder' is null");
    PBErrCatch(AppErr);

  }

#endif

  // Get the GtkWidget for the text boxes
  appTextBoxDataset = GTK_TEXT_VIEW(
    gtk_builder_get_object(
      gtkBuilder,
      "txtDataset"));
  appTextBoxEval = GTK_TEXT_VIEW(
    gtk_builder_get_object(
      gtkBuilder,
      "txtEval"));

}

// Create an instance of the application
GUI GUICreate(
     int argc,
  char** argv) {

#if BUILDMODE == 0

  if (argv == NULL) {

    AppErr->_type = PBErrTypeNullPointer;
    sprintf(
      AppErr->_msg,
      "'argv' is null");
    PBErrCatch(AppErr);

  }

  if (argc < 0) {

    AppErr->_type = PBErrTypeInvalidArg;
    sprintf(
      AppErr->_msg,
      "'argc' is invalid (%d>=0)",
      argc);
    PBErrCatch(AppErr);

  }

#endif

  // Initialise the dataset
  *appDataset = GDataSetVecFloatCreateStatic();

  // Initialise the neuranet
  appNeuranet = NULL;

  // Initialise the set of ThreadEvalResult
  *appEvalResults = GSetCreateStatic();

  // Init the runtime configuration
  GUIInitConf(
    argc,
    argv);

  // Create a GTK application
  app.gtkApp = gtk_application_new(
    NULL,
    G_APPLICATION_FLAGS_NONE);
  app.gApp = G_APPLICATION(app.gtkApp);

  // Connect the callback function on the 'activate' event of the GTK
  // application
  g_signal_connect(
    app.gtkApp,
    "activate",
    G_CALLBACK(CbGtkAppActivate),
    NULL);

  // Return the instance of the application
  return app;

}

// Main function of the application
int GUIMain(void) {

  // Run the application at the G level
  int status =
    g_application_run(
      app.gApp,
      0,
      NULL);

  // If the application failed
  if (status != 0) {

    printf(
      "g_application_run failed (%d)",
      status);

  }

  // Unreference the GTK application
  g_object_unref(app.gtkApp);

  // Return the status code
  return status;

}

// Load a GDataset into the application from the file at 'path'
void LoadGDataset(const char* path) {

  if (path != NULL) {

    FILE* fp =
      fopen(
        path,
        "r");

    if (fp != NULL) {

      bool ret =
        GDSLoad(
          appDataset,
          fp);

      if (ret == FALSE) {

        *appDataset = GDataSetVecFloatCreateStatic();

      }

      fclose(fp);

    } else {

      *appDataset = GDataSetVecFloatCreateStatic();

    }

  } else {

    *appDataset = GDataSetVecFloatCreateStatic();

  }

  // Display the dataset
  DisplayGDataset();

}

// Load a NeuraNet into the application from the file at 'path'
void LoadNeuraNet(const char* path) {

  if (path != NULL) {

    FILE* fp =
      fopen(
        path,
        "r");

    if (fp != NULL) {

      bool ret =
        NNLoad(
          &appNeuranet,
          fp);
      (void)ret;

      fclose(fp);

    }

  }

}

// Display the current dataset in the text box of the dataset tab
void DisplayGDataset(void) {

  // Get the text buffer to display info about the dataset
  GtkTextBuffer* txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxDataset));

  // Empty the text buffer
  gtk_text_buffer_set_text(
    txtBuffer,
    "\0",
    -1);

  // Display the dataset info

  const char* name = GDSName(appDataset);
  gtk_text_buffer_insert_at_cursor(
    txtBuffer,
    name,
    strlen(name));
  gtk_text_buffer_insert_at_cursor(
    txtBuffer,
    "\n",
    1);

  const char* desc = GDSDesc(appDataset);
  gtk_text_buffer_insert_at_cursor(
    txtBuffer,
    desc,
    strlen(desc));
  gtk_text_buffer_insert_at_cursor(
    txtBuffer,
    "\n",
    1);

  char buffer[100];
  sprintf(
    buffer,
    "%ld samples\n",
    GDSGetSize(appDataset));
  gtk_text_buffer_insert_at_cursor(
    txtBuffer,
    buffer,
    strlen(buffer));

  if (GDSGetNbCat(appDataset) > 1) {

    char* strSplit = "Split: ";
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      strSplit,
      strlen(strSplit));
    for (
      int iCat = 0;
      iCat < GDSGetNbCat(appDataset);
      ++iCat) {

      if (iCat > 0) {

        gtk_text_buffer_insert_at_cursor(
          txtBuffer,
          "/",
          1);

      }

      long sizeCat =
        GDSGetSizeCat(
          appDataset,
          iCat);
      sprintf(
        buffer,
        "%ld",
        sizeCat);
      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        buffer,
        strlen(buffer));

    }

    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      "\n",
      1);

  } else {

    char* strSplit = "No split\n";
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      strSplit,
      strlen(strSplit));

  }

  // Display the samples
  GDSResetAll(appDataset);
  for (
    int iCat = 0;
    iCat < GDSGetNbCat(appDataset);
    ++iCat) {

    sprintf(
      buffer,
      " - Category %d -\n",
      iCat);
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      buffer,
      strlen(buffer));

    long iSample = 0;
    do {

      sprintf(
        buffer,
        "#%05ld: ",
        iSample);
      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        buffer,
        strlen(buffer));

      VecFloat* sample =
        GDSGetSample(
          appDataset,
          iCat);

      for (
        int iCol = 0;
        iCol < VecGetDim(sample);
        ++iCol) {

      float val =
        VecGet(
          sample,
          iCol);
      sprintf(
        buffer,
        " %+08.3f ",
        val);
      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        buffer,
        strlen(buffer));

      }

      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        "\n",
        1);

      ++iSample;

    } while (GDSStepSample(appDataset, iCat));

  }

}
