#include "gui.h"

// Error manager
PBErr* AppErr = &thePBErr;

// Declare the global instance of the application
GUI app;

// Include the callbacks
#include "gui-callbacks.c"

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

// Function to refresh the content of all graphical widgets
//(cameras and control)
void GUIRefreshWidgets(void) {

  // Give back the hand to the main loop to process pending events
  while(gtk_events_pending()) {

    gtk_main_iteration();

  }

}

// Free memory used by the application
void GUIFree(void) {

  // Stop the timer
  bool ret = g_source_remove(app.timerId);
  (void)ret;

  // Free memory
  GUIFreeConf();

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

  // Set the callback on the 'clicked' event of btnEval
  GObject* obj =
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
  char* paramNames[6] = {

    "inpDataset",
    "inpNbIn",
    "inpNbOut",
    "inpSplitTrain",
    "inpSplitValid",
    "inpSplitEval"

  };

  for (
    int iParam = 0;
    iParam < 6;
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

  // Init the values in the GUI with the values in the config
  // TODO

  // Reload the dataset if possible
  // TODO

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

  // Init the widgets with the value in the config file
  JSONNode* inp =
    JSONProperty(
      appConf.config,
      "inpDataset");
  gtk_entry_set_text(
    appInpDataset,
    JSONLblVal(inp));
  inp =
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
