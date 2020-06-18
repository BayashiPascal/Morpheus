#include "pberr.h"
#include "pbjson.h"
#include "pbmath.h"
#include "gdataset.h"
#include "neuranet.h"
#include <gtk/gtk.h>

#define appWins (app.windows)
#define appMainWin (app.windows.main)
#define appConf (app.config)
#define appInp (app.inputs)
#define appInpDataset (app.inputs.inpDataset)
#define appInpEvalNeuraNet (app.inputs.inpEvalNeuraNet)
#define appInpNbIn (app.inputs.inpNbIn)
#define appInpNbOut (app.inputs.inpNbOut)
#define appInpSplitTrain (app.inputs.inpSplitTrain)
#define appInpSplitEval (app.inputs.inpSplitEval)
#define appInpSplitValid (app.inputs.inpSplitValid)
#define appRadEvalTrain (app.inputs.radEvalTrain)
#define appRadEvalValid (app.inputs.radEvalValid)
#define appRadEvalEval (app.inputs.radEvalEval)
#define appTextBoxes (app.textboxes)
#define appTextBoxDataset (app.textboxes.txtDataset)
#define appTextBoxEval (app.textboxes.txtEval)
#define appDataset (&(app.dataset))
#define appNeuranet (app.neuranet)

typedef struct GUIWindows {

  // Main window of the application
  GtkWidget* main;

} GUIWindows;

typedef struct GUIConfig {

  // Path to the gui definition file
  char* gladeFilePath;

  // Path to the folder conatining the application
  char* rootDir;

  // Content of the configuration file
  JSONNode* config;

  // Path to the configuration file
  char* configFilePath;

  // Timer interval
  unsigned int timerInterval;

} GUIConfig;

typedef struct GUIInputs {

  // Inputs of the dataset tab
  GtkEntry* inpDataset;
  GtkEntry* inpEvalNeuraNet;
  GtkEntry* inpNbIn;
  GtkEntry* inpNbOut;
  GtkEntry* inpSplitTrain;
  GtkEntry* inpSplitEval;
  GtkEntry* inpSplitValid;
  GtkRadioButton* radEvalTrain;
  GtkRadioButton* radEvalValid;
  GtkRadioButton* radEvalEval;

} GUIInputs;

typedef struct GUITextboxes {

  // Text boxes of the dataset tab
  GtkTextView* txtDataset;

  // Text boxes of the eval tab
  GtkTextView* txtEval;

} GUITextBoxes;

typedef struct GUI {

  // Runtime configuration
  GUIConfig config;

  // Input widgets
  GUIInputs inputs;

  // Text box widgets
  GUITextBoxes textboxes;

  // Windows
  GUIWindows windows;

  // Timer
  unsigned int timerId;

  // GTK application and its G version
  GtkApplication* gtkApp;
  GApplication* gApp;

  // The GDataset
  GDataSetVecFloat dataset;

  // The NeuraNet
  NeuraNet* neuranet;

} GUI;

// Declare the global instance of the application
extern GUI app;

// Callback for the timer
gboolean CbTimer(gpointer data);

// Function to init the windows
void GUIInitWindows(GtkBuilder* const gtkBuilder);

// Callback function for the 'clicked' event on btnEval
gboolean CbBtnEvalClicked(
  GtkButton* btn,
    gpointer user_data);

// Callback function for the 'clicked' event on btnSelectDataset
gboolean CbBtnSelectDatasetClicked(
  GtkButton* btn,
    gpointer user_data);

// Callback function for the 'clicked' event on btnShuffle
gboolean CbBtnShuffleClicked(
  GtkButton* btn,
    gpointer user_data);

// Callback function for the 'clicked' event on btnSplit
gboolean CbBtnSplitClicked(
  GtkButton* btn,
    gpointer user_data);

// Free memory used by the runtime configuration
void GUIFreeConf(void);

// Free memory used by the application
void GUIFree(void);

// Function called before the application quit
void GUIQuit(void);

// Callback function for the 'delete-event' event on the GTK application
// window
gboolean CbAppWindowDeleteEvent(
          GtkWidget* widget,
  GdkEventConfigure* event,
            gpointer user_data);

// Callback function for the 'check-resize' event on the GTK application
// window
gboolean CbAppWindowResizeEvent(
          GtkWidget* widget,
  GdkEventConfigure* event,
            gpointer user_data);

// Function to init the callbacks
void GUIInitCallbacks(GtkBuilder* const gtkBuilder);

// Callback function for the 'activate' event on the GTK application
void CbGtkAppActivate(
  GtkApplication* gtkApp,
         gpointer user_data);

// Parse the command line arguments
// Return true if the arguments were valid, false else
bool GUIParseArg(
     int argc,
  char** argv);

// Function to load the parameters of the application from the
// config file
void GUILoadConfig(void);

// Save the current parameters in the config file
void GUISaveConfig(void);

// Function to init the configuration
void GUIInitConf(
     int argc,
  char** argv);

// Init the inputs
void GUIInitInputs(GtkBuilder* const gtkBuilder);

// Init the text boxes
void GUIInitTextBoxes(GtkBuilder* const gtkBuilder);

// Free memory used by the drawables
void GUIFreeDrawables(void);

// Function to refresh the content of the control widget
void GUIRefreshWidgetControl(void);

// Function to refresh the content of all graphical widgets
//(cameras and control)
void GUIRefreshWidgets(void);

// Create an instance of the application
GUI GUICreate(
     int argc,
  char** argv);

// Main function of the application
int GUIMain(void);

// Load a GDataset into the application from the file at 'path'
void LoadGDataset(const char* path);

// Load a NeuraNet into the application from the file at 'path'
void LoadNeuraNet(const char* path);
