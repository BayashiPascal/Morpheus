#include "pberr.h"
#include "pbjson.h"
#include "pbmath.h"
#include "gdataset.h"
#include "neuranet.h"
#include "respublish.h"
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
#define appInpTrainNeuraNet (app.inputs.inpTrainNeuraNet)
#define appInpTrainNbEpoch (app.inputs.inpTrainNbEpoch)
#define appInpTrainDepth (app.inputs.inpTrainDepth)
#define appInpTrainNbElite (app.inputs.inpTrainNbElite)
#define appInpTrainSizePool (app.inputs.inpTrainSizePool)
#define appInpTrainBestVal (app.inputs.inpTrainBestVal)
#define appInpTrainNbThread (app.inputs.inpTrainNbThread)
#define appLblETCDepth (app.textboxes.lblETCDepth)
#define appLblETCTotal (app.textboxes.lblETCTotal)
#define appRadEvalTrain (app.inputs.radEvalTrain)
#define appRadEvalValid (app.inputs.radEvalValid)
#define appRadEvalEval (app.inputs.radEvalEval)
#define appBtnEval (app.inputs.btnEval)
#define appBtnEvalNeuraNet (app.inputs.btnEvalNeuraNet)
#define appBtnDataset (app.inputs.btnDataset)
#define appBtnShuffle (app.inputs.btnShuffle)
#define appBtnTrainNeuraNet (app.inputs.btnTrainNeuraNet)
#define appBtnTrainStart (app.inputs.btnTrainStart)
#define appBtnTrainStop (app.inputs.btnTrainStop)
#define appProgEval (app.inputs.progEval)
#define appProgTrainDepth (app.inputs.progTrainDepth)
#define appProgTrainTotal (app.inputs.progTrainTotal)
#define appBtnSplit (app.inputs.btnSplit)
#define appTextBoxes (app.textboxes)
#define appTextBoxDataset (app.textboxes.txtDataset)
#define appTextBoxEval (app.textboxes.txtEval)
#define appTextBoxTrainMsgTotal (app.textboxes.txtTrainMsgTotal)
#define appTextBoxTrainMsgDepth (app.textboxes.txtTrainMsgDepth)
#define appDataset (&(app.dataset))
#define appNeuranet (app.neuranet)
#define appMutex (app.mutexThread)
#define appEvalResults (&(app.evalResults))
#define appIsEvaluating (app.isEvaluating)
#define appIsTraining (app.isTraining)
#define appTrainETCTotal (&(app.trainETCTotal))
#define appTrainETCDepth (&(app.trainETCDepth))
#define threadEvalNbInput (app.threadEvalData.nbInput)
#define threadEvalNbOutput (app.threadEvalData.nbOutput)
#define threadEvalCat (app.threadEvalData.cat)
#define threadEvalCompletion (app.threadEvalData.completion)
#define threadEvalDataset (&(app.threadEvalData.dataset))
#define threadTrainCompletionTotal (app.threadTrainData.completionTotal)
#define threadTrainCompletionDepth (app.threadTrainData.completionDepth)
#define threadTrainNbEpoch (app.threadTrainData.nbEpoch)
#define threadTrainDepth (app.threadTrainData.depth)
#define threadTrainNbElite (app.threadTrainData.nbElite)
#define threadTrainSizePool (app.threadTrainData.sizePool)
#define threadTrainBestVal (app.threadTrainData.bestVal)
#define threadTrainNbThread (app.threadTrainData.nbThread)
#define threadTrainNbIn (app.threadTrainData.nbInput)
#define threadTrainNbOut (app.threadTrainData.nbOutput)
#define threadTrainCurBestVal (app.threadTrainData.curBestVal)
#define threadTrainBestTopo (app.threadTrainData.bestTopo)
#define threadTrainNNToBeTrained (&(app.threadTrainData.NNToBeTrained))
#define threadTrainNNUnderTraining (&(app.threadTrainData.NNUnderTraining))
#define threadTrainNNTrained (&(app.threadTrainData.NNTrained))
#define threadTrainFlagInterrupt (app.threadTrainData.flagInterrupt)
#define threadTrainTxt (&(app.threadTrainResult.setTxt))
#define threadTrainLbl (&(app.threadTrainResult.setLbl))
#define threadTrainProg (&(app.threadTrainResult.setProg))

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
  GtkEntry* inpTrainNeuraNet;
  GtkEntry* inpTrainNbEpoch;
  GtkEntry* inpTrainDepth;
  GtkEntry* inpTrainNbElite;
  GtkEntry* inpTrainSizePool;
  GtkEntry* inpTrainBestVal;
  GtkEntry* inpTrainNbThread;
  GtkRadioButton* radEvalTrain;
  GtkRadioButton* radEvalValid;
  GtkRadioButton* radEvalEval;
  GtkButton* btnEval;
  GtkButton* btnEvalNeuraNet;
  GtkButton* btnDataset;
  GtkButton* btnShuffle;
  GtkButton* btnSplit;
  GtkButton* btnTrainNeuraNet;
  GtkButton* btnTrainStart;
  GtkButton* btnTrainStop;
  GtkProgressBar* progEval;
  GtkProgressBar* progTrainTotal;
  GtkProgressBar* progTrainDepth;

} GUIInputs;

typedef struct GUITextboxes {

  // Text boxes of the dataset tab
  GtkTextView* txtDataset;

  // Text boxes of the eval tab
  GtkTextView* txtEval;

  // Text boxes and labels of the train tab
  GtkTextView* txtTrainMsgTotal;
  GtkTextView* txtTrainMsgDepth;
  GtkLabel* lblETCTotal;
  GtkLabel* lblETCDepth;

} GUITextBoxes;

typedef struct ThreadEvalData {

  // Number of input and output in the sample
  int nbInput;
  int nbOutput;

  // Used category in the GDataset
  int cat;

  // Percentage of completion (in 0.0, 1.0)
  float completion;

  // Dataset to analyse the result of evaluation
  GDataSetVecFloat dataset;

} ThreadEvalData;

typedef struct ThreadTrainTopology {

  VecFloat* bases;
  VecLong* links;

} ThreadTrainTopology;

typedef struct NeuraNetPod {

  NeuraNet* nn;
  float val;

} NeuraNetPod;

typedef struct ThreadTrainData {

  // Percentage of completion (in 0.0, 1.0)
  float completionTotal;
  float completionDepth;

  // Parameters for the training
  unsigned long nbEpoch;
  int depth;
  int trainDepth;
  int nbElite;
  int sizePool;
  float bestVal;
  int nbThread;
  int nbInput;
  int nbOutput;

  // Current best value
  float curBestVal;

  // Current best topology
  ThreadTrainTopology bestTopo;

  // GSet of NeuraNet to be trained
  GSet NNToBeTrained;

  // GSet of NeuraNet under training
  GSet NNUnderTraining;

  // GSet of trained NeuraNet
  GSet NNTrained;

  // Flag to interrupt the training
  bool flagInterrupt;

} ThreadTrainData;

typedef struct ThreadEvalResult {

  // Sample
  const VecFloat* sample;

  // Output of NeuraNetEval
  VecFloat* result;

  // Index of the sample evaluated
  long iSample;

} ThreadEvalResult;

typedef struct ThreadTrainResultTxt {

  GtkTextView* txtView;
  char msg[100];

} ThreadTrainResultTxt;

typedef struct ThreadTrainResultLbl {

  GtkLabel* lbl;
  char msg[100];

} ThreadTrainResultLbl;

typedef struct ThreadTrainResultProg {

  GtkProgressBar* prog;
  float comp;

} ThreadTrainResultProg;

typedef struct ThreadTrainResult {

  // GSet of ThreadTrainResultTxt
  GSet setTxt;

  // GSet of ThreadTrainResultLbl
  GSet setLbl;

  // GSet of ThreadTrainResultProg
  GSet setProg;

} ThreadTrainResult;

typedef struct GUI {

  // Runtime configuration
  GUIConfig config;

  // Input widgets
  GUIInputs inputs;

  // Text box widgets
  GUITextBoxes textboxes;

  // Windows
  GUIWindows windows;

  // GTK application and its G version
  GtkApplication* gtkApp;
  GApplication* gApp;

  // The GDataset
  GDataSetVecFloat dataset;

  // The NeuraNet
  NeuraNet* neuranet;

  // Mutex for the thread workers
  GMutex mutexThread;

  // Data for the thread eval
  ThreadEvalData threadEvalData;

  // Data for the thread train
  ThreadTrainData threadTrainData;

  // GSet of ThreadEvalResult
  GSet evalResults;

  // Results from the thread train
  ThreadTrainResult threadTrainResult;

  // Flags to manage the lock of buttons furing eval and training
  bool isEvaluating;
  bool isTraining;

  // Estimated time to completion for the training
  EstimTimeToComp trainETCTotal;
  EstimTimeToComp trainETCDepth;

} GUI;

// Declare the global instance of the application
extern GUI app;

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

// Thread worker for the evaluation
gpointer ThreadWorkerEval(gpointer data);

// Function to process the data from the thread worker for evaluation
gboolean ProcessThreadWorkerEval(gpointer data);

// Function to process the end of the thread worker for evaluation
gboolean EndThreadWorkerEval(gpointer data);

// Thread worker for the training
gpointer ThreadWorkerTrain(gpointer data);
