// Add a pod to the results of the training thread
void AddThreadTrainPod(
  GSet* set,
  void* pod) {

  g_mutex_lock(&appMutex);
  GSetAppend(
    set,
    pod);
  g_mutex_unlock(&appMutex);

}

void AddThreadTrainPodTxt(
  GtkTextView* txtView,
   const char* msg) {

  ThreadTrainResultTxt* podTxt =
    PBErrMalloc(
      AppErr,
      sizeof(ThreadTrainResultTxt));
  podTxt->txtView = txtView;
  sprintf(
    podTxt->msg,
    "%s",
    msg);
  AddThreadTrainPod(
    threadTrainTxt,
    podTxt);

}

void AddThreadTrainPodProg(
  GtkProgressBar* progBar,
     const double comp) {

  ThreadTrainResultProg* podProg =
    PBErrMalloc(
      AppErr,
      sizeof(ThreadTrainResultProg));
  podProg->prog = progBar;
  podProg->comp = comp;
  AddThreadTrainPod(
    threadTrainProg,
    podProg);

}

void AddThreadTrainPodLbl(
    GtkLabel* lbl,
  const char* msg) {

    ThreadTrainResultLbl* podLbl =
      PBErrMalloc(
        AppErr,
        sizeof(ThreadTrainResultLbl));
    podLbl->lbl = lbl;
    sprintf(
      podLbl->msg,
      "%s",
      msg);
    AddThreadTrainPod(
      threadTrainLbl,
      podLbl);

}

// Force the scroll bars down in the training tab
void ScrollDownTrainTxtbox(void) {

  GtkScrollable* scrollables[2] = {

    GTK_SCROLLABLE(appTextBoxTrainMsgDepth),
    GTK_SCROLLABLE(appTextBoxTrainMsgTotal)

  };

  for (
    int i = 2;
    i--;) {

    GtkAdjustment* adjustment =
      gtk_scrollable_get_vadjustment(scrollables[i]);
    gtk_adjustment_set_value(
      adjustment,
      gtk_adjustment_get_upper(adjustment));
    gtk_scrollable_set_vadjustment(
      scrollables[i],
      adjustment);

  }

}

// Thread worker for the evaluation
gpointer ThreadWorkerEval(gpointer data) {

  (void)data;

  // Init variables for the evaluation
  VecFloat* vecIn = VecFloatCreate(threadEvalNbInput);

  // Variable to perform the evaluation
  bool flagStep = TRUE;
  long iSample = 0;
  long nbSamples =
    GDSGetSizeCat(
      appDataset,
      threadEvalCat);

  // Loop on the samples with a copy of the internal iterator of the
  // GDataset as we can't use it in a multihtreaded context
  GSetIterForward iter = appDataset->_dataSet._iterators[threadEvalCat];
  GSetIterReset(&iter);
  do {

    // Allocate memory for the result
    ThreadEvalResult* evalResult =
      PBErrMalloc(
        AppErr,
        sizeof(ThreadEvalResult));
    evalResult->result = VecFloatCreate(threadEvalNbOutput);
    evalResult->iSample = iSample;

    // Get the sample
    evalResult->sample = GSetIterGet(&iter);

    // Init the input of the NeuraNet
    for (
      int i = threadEvalNbInput;
      i--;) {

      float val =
        VecGet(
          evalResult->sample,
          i);
      VecSet(
        vecIn,
        i,
        val);

    }

    // Eval the NeuraNet
    NNEval(
      appNeuranet,
      vecIn,
      evalResult->result);

    // Append the result to the set of results
    g_mutex_lock(&appMutex);
    GSetAppend(
      appEvalResults,
      evalResult);
    g_mutex_unlock(&appMutex);

    // Process results by the main thread
    g_idle_add(
      ProcessThreadWorkerEval,
      NULL);

    // Step to the next sample
    ++iSample;
    flagStep = GSetIterStep(&iter);

    // Update the percentage of completion
    threadEvalCompletion =
      ((float)iSample) /
      ((float)nbSamples);

  } while (flagStep);

  // Process last results by the main thread
  g_idle_add(
    ProcessThreadWorkerEval,
    NULL);

  // Send end signal to the main thread
  g_idle_add(
    EndThreadWorkerEval,
    NULL);

  // Free memory
  free(vecIn);

  return NULL;

}

// Function to process the data from the thread worker for evaluation
gboolean ProcessThreadWorkerEval(gpointer data) {

  // Unused data
  (void)data;

  // Lock the mutex
  g_mutex_lock(&appMutex);

  // Get the text buffer to display results
  GtkTextBuffer* txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));

  // Process the data from the thread worker
  while (GSetNbElem(appEvalResults) > 0) {

    // Pop out the result for one sample
    ThreadEvalResult* evalResult = GSetPop(appEvalResults);

    // Display the result
    char buffer[100];
    sprintf(
      buffer,
      "#%05ld: ",
      evalResult->iSample);
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      buffer,
      strlen(buffer));

    for (
      int iCol = 0;
      iCol < VecGetDim(evalResult->result);
      ++iCol) {

      float val =
        VecGet(
          evalResult->result,
          iCol);
      sprintf(
        buffer,
        " %+08.3f ",
        val);
      gtk_text_buffer_insert_at_cursor(
        txtBuffer,
        buffer,
        strlen(buffer));

      // Update the result with the distance to the correct value
      float valOut =
        VecGet(
          evalResult->sample,
          threadEvalNbInput + iCol);
      float diff = fabs(val - valOut);
      VecSet(
        evalResult->result,
        iCol,
        diff);

    }

    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      "|",
      1);

    for (
      int iCol = 0;
      iCol < VecGetDim(evalResult->result);
      ++iCol) {

      float val =
        VecGet(
          evalResult->result,
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

    // Memorize the result vector for later analyis
    GDSAddSample(
      threadEvalDataset,
      evalResult->result);

    // Free the memory used by the result
    free(evalResult);

  }

  // Update the progress bar
  gtk_progress_bar_set_fraction(
    appProgEval,
    threadEvalCompletion);

  // Unlock the mutex
  g_mutex_unlock(&appMutex);

  return FALSE;

}

// Function to process the end of the thread worker for evaluation
gboolean EndThreadWorkerEval(gpointer data) {

  // Unused data
  (void)data;

  // Lock the mutex
  g_mutex_lock(&appMutex);

  // Variables for the analysis
  GtkTextBuffer* txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));
  char buffer[100];
  VecShort2D indices = VecShortCreateStatic2D();
  char* msg = "\nX: abs(truth-pred)\n";
  gtk_text_buffer_insert_at_cursor(
    txtBuffer,
    msg,
    strlen(msg));

  // Analysis of the result
  VecFloat* means = GDSGetMean(threadEvalDataset);
  VecFloat* maxs = GDSGetMax(threadEvalDataset);
  for (
    int iOut = 0;
    iOut < VecGetDim(maxs);
    ++iOut) {

    float mean =
      VecGet(
        means,
        iOut);
    float max =
      VecGet(
        maxs,
        iOut);
    VecSet(
      &indices,
      0,
      iOut);
    VecSet(
      &indices,
      1,
      iOut);
    float variance =
      GDSGetCovariance(
        threadEvalDataset,
        &indices);

    // Display the result
    sprintf(
      buffer,
      "Output#%d | Max(X): %+08.3f | " \
      "Avg(X): %+08.3f | Var(X): %+08.3f \n",
      iOut,
      max,
      mean,
      variance);
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      buffer,
      strlen(buffer));

  }

  // Free memory
  VecFree(&means);
  VecFree(&maxs);

  // Unlock the buttons to run another evaluation
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnEvalNeuraNet),
    TRUE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnEval),
    TRUE);
  if (appIsTraining == FALSE) {

    gtk_widget_set_sensitive(
      GTK_WIDGET(appBtnDataset),
      TRUE);
    gtk_widget_set_sensitive(
      GTK_WIDGET(appBtnShuffle),
      TRUE);
    gtk_widget_set_sensitive(
      GTK_WIDGET(appBtnSplit),
      TRUE);

  }

  // Pull down the flag
  appIsEvaluating = FALSE;

  // Unlock the mutex
  g_mutex_unlock(&appMutex);

  return FALSE;

}

// Function to process the data from the thread worker for training
gboolean ProcessThreadWorkerTrain(gpointer data) {

  // Unused data
  (void)data;

  // Lock the mutex
  g_mutex_lock(&appMutex);

  // Update the text boxes
  while (GSetNbElem(threadTrainTxt) > 0) {

    ThreadTrainResultTxt* pod = GSetPop(threadTrainTxt);
    GtkTextBuffer* txtBuffer =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(pod->txtView));
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      pod->msg,
      strlen(pod->msg));
    free(pod);

  }

  // Update the labels
  while (GSetNbElem(threadTrainLbl) > 0) {

    ThreadTrainResultLbl* pod = GSetPop(threadTrainLbl);
    gtk_label_set_text(
      pod->lbl,
      pod->msg);
    free(pod);

  }

  // Update the progress bars
  while (GSetNbElem(threadTrainProg) > 0) {

    ThreadTrainResultProg* pod = GSetPop(threadTrainProg);
    gtk_progress_bar_set_fraction(
      pod->prog,
      pod->comp);
    free(pod);

  }

  // Wait a little to be able to scroll down to the right place
  sleep(1);

  // Ensure the scrollbars are at the bottom
  ScrollDownTrainTxtbox();

  // Unlock the mutex
  g_mutex_unlock(&appMutex);

  return FALSE;

}

// Function to process the end of the thread worker for training
gboolean EndThreadWorkerTrain(gpointer data) {

  // Unused data
  (void)data;

  // Lock the mutex
  g_mutex_lock(&appMutex);

  // Unlock the button to enable running another training
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnTrainNeuraNet),
    TRUE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnTrainStart),
    TRUE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnTrainStop),
    FALSE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnDataset),
    TRUE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnShuffle),
    TRUE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnSplit),
    TRUE);

  // Unlock the mutex
  g_mutex_unlock(&appMutex);

  return FALSE;

}

// Create the NeuraNet to train the new topology for a given depth,
// link index, input and ouput ID
NeuraNet* CreateNewTopo(
  const int depth,
  const int iLink,
  const int iIn,
  const int iOut) {

  (void)iLink;
  (void)depth;

  // Declare the returned NeuraNet
  NeuraNet* nn = NULL;

  // If there is no current best topology
  if (threadTrainBestTopo.links == NULL) {

    // Create a topology with only one link (iIn->iOut)
    // NeuraNet needs at least one hidden value, even if we don't
    // actually use it here
    long nbHidden = 1;
    long nbBases = 1;
    long nbLinks = nbBases;
    nn =
      NeuraNetCreate(
        threadTrainNbIn,
        threadTrainNbOut,
        nbHidden,
        nbBases,
        nbLinks);

    // Set the new link base
    VecSet(
      nn->_links,
      0,
      0);

    // Set the new link input
    VecSet(
      nn->_links,
      1,
      iIn);

    // Set the new link output
    VecSet(
      nn->_links,
      2,
      threadTrainNbIn + nbHidden + iOut);

  // Else, there is a current best topology
  } else {

    // TODO Create a new topology made of the topology of the current best
    // plus the new link (iIn->iOut)
    long nbHidden = threadTrainBestTopo.nbHidden;
    long nbBasesBest =
      VecGetDim(threadTrainBestTopo.bases) / NN_NBPARAMBASE;
    long nbBases = nbBasesBest + 1;
    long nbLinks = nbBases;
    nn =
      NeuraNetCreate(
        threadTrainNbIn,
        threadTrainNbOut,
        nbHidden,
        nbBases,
        nbLinks);

    // Copy the best topology
    for (
      long iGene = 0;
      iGene < nbBasesBest;
      ++iGene) {

      for (
        int shift = NN_NBPARAMLINK;
        shift--;) {

        long val =
          VecGet(
            threadTrainBestTopo.links,
            iGene * NN_NBPARAMLINK + shift);
        VecSet(
          nn->_links,
          iGene * NN_NBPARAMLINK + shift,
          val);

      }

      for (
        int shift = NN_NBPARAMBASE;
        shift--;) {

        float val =
          VecGet(
            threadTrainBestTopo.bases,
            iGene * NN_NBPARAMBASE + shift);
        VecSet(
          nn->_bases,
          iGene * NN_NBPARAMBASE + shift,
          val);

      }

    }

    // Append the new link base
    VecSet(
      nn->_links,
      nbBasesBest * NN_NBPARAMBASE,
      nbBasesBest);

    // Append the new link input
    VecSet(
      nn->_links,
      nbBasesBest * NN_NBPARAMBASE + 1,
      iIn);

    // Append the new link output
    VecSet(
      nn->_links,
      nbBasesBest * NN_NBPARAMBASE + 2,
      threadTrainNbIn + nbHidden + iOut);

  }

  // Return the NeuraNet
  return nn;

}

// Evalutation function for the NeuraNet 'that' on the GDataSet 'dataset'
// Return the value of the NeuraNet, value <= 0.0, the bigger the better
float EvaluateNeuraNet(
  const NeuraNet* const that,
              const int cat,
                  float thresholdVal) {

  // Declare 3 vectors to memorize the input, output and correct answer
  // values
  VecFloat* input = VecFloatCreate(NNGetNbInput(that));
  VecFloat* output = VecFloatCreate(NNGetNbOutput(that));
  VecFloat* answer = VecFloatCreate(NNGetNbOutput(that));

  // Declare some temporary variables to calculate the value of
  // the NeuraNet
  float val = 0.0;
  bool flagStep = TRUE;
  long iSample = 0;
  long nbSamples =
    GDSGetSizeCat(
      appDataset,
      cat);

  // Loop on the samples with a copy of the internal iterator of the
  // GDataset as we can't use it in a multihtreaded context
  GSetIterForward iter = appDataset->_dataSet._iterators[0];
  GSetIterReset(&iter);
  do {

    // Get the sample
    VecFloat* sample = GSetIterGet(&iter);

    // Initialize the input vector
    for (
      int iInp = NNGetNbInput(that);
      iInp--;) {

      float v =
        VecGet(
          sample,
          iInp);
      VecSet(
        input,
        iInp,
        v);

    }

    // Initialize the answer vector
    for (
      int iOut = NNGetNbOutput(that);
      iOut--;) {

      float v =
        VecGet(
          sample,
          NNGetNbInput(that) + iOut);
      VecSet(
        answer,
        iOut,
        v);

    }

    // Calculate the output predicted by the NeuraNet
    NNEval(
      that,
      input,
      output);

    // Get the difference between the prediction and the correct answer
    float v =
      VecDist(
        output,
        answer);

    // Update the total of difference
    val -= v;

    // Calculate the best possible final value
    float bestPossibleVal = val / (float)(nbSamples);

    // If the best possible final value is less than the current worst
    if (bestPossibleVal < thresholdVal) {

      // Skip the remaining samples of the dataset
      val = val / (float)iSample * (float)(nbSamples);
      iSample = 0;

    }

    // Step to the next sample
    flagStep = GSetIterStep(&iter);

  } while (flagStep);

  // Calculate the average of differences
  val /= (float)(nbSamples);

  // Free memory
  VecFree(&input);
  VecFree(&output);
  VecFree(&answer);

  // Return the result of the evaluation
  return val;

}

// Thread worker for the training of one NeuraNet
// data's type is NeuraNet*
gpointer ThreadWorkerGenAlg(gpointer data) {

  // Convert the argument
  NeuraNetPod* pod = data;
  NeuraNet* nn = pod->nn;
  float* nnVal = &(pod->val);

  // Create the GenAlg
  GenAlg* ga =
    GenAlgCreate(
      threadTrainSizePool,
      threadTrainNbElite,
      NNGetGAAdnFloatLength(nn),
      NNGetGAAdnIntLength(nn));
  NNSetGABoundsBases(
    nn,
    ga);
  NNSetGABoundsLinks(
    nn,
    ga);

  // TODO Switch the GenAlg to Morpheus mode with the indices of the
  // currently trained links
  long iBases[2] = {0, 0};
  iBases[0] = NNGetNbMaxBases(nn) - 1;
  int nbBase = 1;
  GASetTypeMorpheus(
    ga,
    nbBase,
    iBases,
    NNBases(nn),
    NNLinks(nn));

  // Init the GenAlg
  GAInit(ga);
  GASetDiversityThreshold(
    ga,
    0.001);

  // Declare a variable to memorize the best/worst value in the
  // current epoch
  float bestVal = threadTrainBestVal - 1000.0;
  float curBest = 0.0;
  float curWorst = 0.0;
  float curWorstElite = 0.0;

  // Learning loop
  while (
    bestVal < threadTrainBestVal &&
    GAGetCurEpoch(ga) < threadTrainNbEpoch) {

    curWorst = curBest;
    curBest = threadTrainBestVal - 1000.0;
    curWorstElite = threadTrainBestVal - 1000.0;
    int curBestI = 0;

    // For each adn in the GenAlg
    for (
      int iEnt = 0;
      iEnt < GAGetNbAdns(ga);
      ++iEnt) {

      // Get the adn
      GenAlgAdn* adn =
        GAAdn(ga,
        iEnt);

      // If this adn is new
      if (GAAdnIsNew(adn) == true) {

        // Set the links and base functions of the NeuraNet according
        // to this adn
        if (GABestAdnF(ga) != NULL) {

          NNSetBases(
            nn,
            GAAdnAdnF(adn));

        }

        // Evaluate the NeuraNet
        float value =
          EvaluateNeuraNet(
            nn,
            0,
            curWorstElite);

        // Depreciate entites identical to the current best
        if (fabs(value - curBest) < PBMATH_EPSILON) {

          value -= 1000.0;

        }

        // Update the value of this adn
        GASetAdnValue(
          ga,
          adn,
          value);

        // Update the best value in the current epoch
        if (value > curBest) {

          curBest = value;
          curBestI = iEnt;

        }

        if (value < curWorst) {

          curWorst = value;

        }

      }

    }

    // Memorize the current value of the worst elite
    GenAlgAdn* worstEliteAdn =
      GAAdn(
        ga,
        GAGetNbElites(ga) - 1);
    curWorstElite = GAAdnGetVal(worstEliteAdn);

    // If there has been improvement during this epoch
    if (curBest > bestVal) {

      bestVal = curBest;
      GenAlgAdn* bestAdn =
        GAAdn(
          ga,
          curBestI);

      // Set the links and base functions of the NeuraNet according
      // to the best adn
      if (GAAdnAdnF(bestAdn) != NULL) {

        NNSetBases(
          nn,
          GAAdnAdnF(bestAdn));

      }

    }

    // Step the GenAlg
    GAStep(ga);

  }

  // Memorize the result of training
  *nnVal = bestVal;
  NNSetBases(
    nn,
    GABestAdnF(ga));

  // Free memory
  GenAlgFree(&ga);

  // Lock the mutex
  g_mutex_lock(&appMutex);

  // Move the trained NeuraNet to the set of trained NeuraNet
  GSetRemoveFirst(
    threadTrainNNUnderTraining,
    pod);
  GSetAppend(
    threadTrainNNTrained,
    pod);

  // Unlock the mutex
  g_mutex_unlock(&appMutex);

  return NULL;

}

// Display a topology in a text view
void DisplayTrainedTopology(
            GtkTextView* textbox,
    ThreadTrainTopology* topo) {

  ThreadTrainResultTxt* podTxt;
  for (
    long iLink = 0;
    iLink < VecGetDim(topo->links);
    iLink += NN_NBPARAMBASE) {

    long in =
      VecGet(
        topo->links,
        iLink + 1);
    long out =
      VecGet(
        topo->links,
        iLink + 2);
    podTxt =
      PBErrMalloc(
        AppErr,
        sizeof(ThreadTrainResultTxt));
    podTxt->txtView = textbox;
    sprintf(
      podTxt->msg,
      "%ld->%ld, ",
      in,
      out);
    AddThreadTrainPod(
      threadTrainTxt,
      podTxt);

  }

  podTxt =
    PBErrMalloc(
      AppErr,
      sizeof(ThreadTrainResultTxt));
  podTxt->txtView = textbox;
  sprintf(
    podTxt->msg,
    "\n");
  AddThreadTrainPod(
    threadTrainTxt,
    podTxt);

}

// Update the best topology with the result of the training of one NeuraNet
void UpdateBestTopo(const NeuraNetPod* const pod) {

  // Memorize the best value
  threadTrainCurBestVal = pod->val;

  // Memorize the best topology
  if (threadTrainBestTopo.links != NULL) {

    VecFree(&(threadTrainBestTopo.links));

  }

  threadTrainBestTopo.links = VecClone(pod->nn->_links);

  if (threadTrainBestTopo.bases != NULL) {

    VecFree(&(threadTrainBestTopo.bases));

  }

  threadTrainBestTopo.bases = VecClone(pod->nn->_bases);

  threadTrainBestTopo.nbHidden = NNGetNbMaxHidden(pod->nn);

  // Save the NeuraNet
  g_mutex_lock(&appMutex);
  FILE* fp =
    fopen(
      gtk_entry_get_text(appInpTrainNeuraNet),
      "w");
  NNSave(
    pod->nn,
    fp,
    true);
  fclose(fp);
  g_mutex_unlock(&appMutex);

  // Send a message to the user
  ThreadTrainResultTxt* podTxt =
    PBErrMalloc(
      AppErr,
      sizeof(ThreadTrainResultTxt));
  podTxt->txtView = appTextBoxTrainMsgDepth;
  sprintf(
    podTxt->msg,
    "Improved topology with value: %f.\n",
    pod->val);
  AddThreadTrainPod(
    threadTrainTxt,
    podTxt);

  // Print the topology
  DisplayTrainedTopology(
    appTextBoxTrainMsgDepth,
    &threadTrainBestTopo);

}

// Thread worker for the training
gpointer ThreadWorkerTrain(gpointer data) {

  // Unused argument
  (void)data;

  // Variable to send signals to the main thread
  char msg[100];

  // Reset the total ETC
  ETCReset(appTrainETCTotal);

  // Loop on the depth
  for (
    int iDepth = 0;
    iDepth < threadTrainDepth &&
    threadTrainCurBestVal < threadTrainBestVal &&
    threadTrainFlagInterrupt == false;
    ++iDepth) {

    // Reset the depth ETC
    ETCReset(appTrainETCDepth);

    // Send a message to the user
    sprintf(
      msg,
      "Train the topologies for depth #%d.\n",
      iDepth);
    AddThreadTrainPodTxt(
      appTextBoxTrainMsgTotal,
      msg);

    // Loop on the number of new link
    for (
      int iLink = 0;
      iLink < threadTrainNbIn * threadTrainNbOut &&
      threadTrainFlagInterrupt == false;
      ++iLink) {

      // Loop on the source of the new link
      for (
        int iIn = 0;
        iIn < threadTrainNbIn;
        ++iIn) {

        // Loop on the destination of the new link
        for (
          int iOut = 0;
          iOut < threadTrainNbOut;
          ++iOut) {

          // Create the NeuraNet to train the new topology
          NeuraNet* nn =
            CreateNewTopo(
              iDepth,
              iLink,
              iIn,
              iOut);

          // Add the NeuraNet to the set of topologies to train
          NeuraNetPod* pod =
            PBErrMalloc(
              AppErr,
              sizeof(NeuraNetPod));
          pod->nn = nn;
          GSetAppend(
            threadTrainNNToBeTrained,
            pod);

        }

      }

      // Memorize the number of topologies to train
      long nbTopoToTrain = GSetNbElem(threadTrainNNToBeTrained);

      // Send the signal to process the result of training
      gdk_threads_add_idle(
        ProcessThreadWorkerTrain,
        NULL);

      // While there are topologies to train
      while (GSetNbElem(threadTrainNNToBeTrained) > 0) {

        // Flush the trained topologies
        while (GSetNbElem(threadTrainNNTrained) > 0) {

          g_mutex_lock(&appMutex);
          NeuraNetPod* pod = GSetPop(threadTrainNNTrained);
          g_mutex_unlock(&appMutex);

          // Update the best topology if it's worst than this trained topo
          // and this trained topo's value on the validation is also
          // better than the current best topology
          if (pod->val > threadTrainCurBestVal) {

            float valid =
              EvaluateNeuraNet(
                pod->nn,
                1,
                threadTrainCurBestVal);

            if (valid > threadTrainCurBestVal) {

              UpdateBestTopo(pod);

            }

          }

          // Free memory used by the trained topo
          NeuraNetFree(&(pod->nn));
          free(pod);

        }

        // If the current best is better than the requested best
        bool flagSkip = false;
        if (threadTrainCurBestVal > threadTrainBestVal) {

          flagSkip = true;

          // Send a message to the user
          sprintf(
            msg,
            "Best value reached. Skip the last %ld topologies.\n",
            GSetNbElem(threadTrainNNToBeTrained));
          AddThreadTrainPodTxt(
            appTextBoxTrainMsgTotal,
            msg);

        }

        if (threadTrainFlagInterrupt == true) {

          flagSkip = true;

        }

        if (flagSkip == true) {

          // Send a message to the user
          sprintf(
            msg,
            "Wait for running threads...\n");
          AddThreadTrainPodTxt(
            appTextBoxTrainMsgTotal,
            msg);

          // Flush the remaining topologies
          g_mutex_lock(&appMutex);
          while (GSetNbElem(threadTrainNNToBeTrained) > 0) {

            NeuraNetPod* pod = GSetPop(threadTrainNNToBeTrained);
            NeuraNetFree(&(pod->nn));
            free(pod);

          }

          g_mutex_unlock(&appMutex);

        }

        // While there is a thread available to train a topology
        // and there are topologies to train
        while (
          GSetNbElem(threadTrainNNUnderTraining) < threadTrainNbThread &&
          GSetNbElem(threadTrainNNToBeTrained) > 0) {

          // Create the thread to train the topology
          g_mutex_lock(&appMutex);
          NeuraNetPod* pod = GSetPop(threadTrainNNToBeTrained);
          GSetAppend(
            threadTrainNNUnderTraining,
            pod);
          g_mutex_unlock(&appMutex);
          GThread* thread =
            g_thread_new(
              "threadGenAlg",
              ThreadWorkerGenAlg,
              (gpointer)pod);
          g_thread_unref(thread);

        }

        // Update the progress bar for the 'depth' section
        threadTrainCompletionDepth =
          (float)iLink /
          (float)(threadTrainNbIn * threadTrainNbOut);
        threadTrainCompletionDepth +=
          (1.0 - ((float)GSetNbElem(threadTrainNNToBeTrained) /
          (float)nbTopoToTrain)) /
          (float)(threadTrainNbIn * threadTrainNbOut);
        AddThreadTrainPodProg(
          appProgTrainDepth,
          threadTrainCompletionDepth);

        // Update the ETC
        const gchar* etc =
          ETCGet(
            appTrainETCDepth,
            threadTrainCompletionDepth);
        AddThreadTrainPodLbl(
          appLblETCDepth,
          etc);

        // Slow down the main training thread
        sleep(1);

        // Send the signal to process the result of training
        gdk_threads_add_idle(
          ProcessThreadWorkerTrain,
          NULL);

      }

      // While there are threads training topologies
      while (GSetNbElem(threadTrainNNUnderTraining) > 0) {

        threadTrainCompletionDepth =
          (float)iLink / (float)(threadTrainNbIn * threadTrainNbOut);
        threadTrainCompletionDepth +=
          (1.0 - ((float)GSetNbElem(threadTrainNNToBeTrained) /
          (float)nbTopoToTrain)) /
          (float)(threadTrainNbIn * threadTrainNbOut);
        AddThreadTrainPodProg(
          appProgTrainDepth,
          threadTrainCompletionDepth);

        // Update the ETC
        const gchar* etc =
          ETCGet(
            appTrainETCDepth,
            threadTrainCompletionDepth);
        AddThreadTrainPodLbl(
          appLblETCDepth,
          etc);

        // Wait for the child training threads to end
        sleep(1);

        // Send the signal to process the result of training
        gdk_threads_add_idle(
          ProcessThreadWorkerTrain,
          NULL);

      }

      // Flush the trained topologies
      while (GSetNbElem(threadTrainNNTrained) > 0) {

        g_mutex_lock(&appMutex);
        NeuraNetPod* pod = GSetPop(threadTrainNNTrained);
        g_mutex_unlock(&appMutex);

        // Update the best topology if necessary
        if (pod->val > threadTrainCurBestVal) {

          UpdateBestTopo(pod);

        }

        NeuraNetFree(&(pod->nn));
        free(pod);

      }

      // Update the progress bar for the 'depth' section
      threadTrainCompletionDepth =
        (float)iLink / (float)(threadTrainNbIn * threadTrainNbOut);
      threadTrainCompletionDepth +=
        (1.0 - ((float)GSetNbElem(threadTrainNNToBeTrained) /
        (float)nbTopoToTrain)) /
        (float)(threadTrainNbIn * threadTrainNbOut);
      AddThreadTrainPodProg(
        appProgTrainDepth,
        threadTrainCompletionDepth);

      // Update the ETC
      const gchar* etc =
        ETCGet(
          appTrainETCDepth,
          threadTrainCompletionDepth);
      AddThreadTrainPodLbl(
        appLblETCDepth,
        etc);

    }

    if (threadTrainFlagInterrupt == true) {

      // Send a message to the user
      sprintf(
        msg,
        "Interrupted by user.\n");
      AddThreadTrainPodTxt(
        appTextBoxTrainMsgTotal,
        msg);

    }

    // Update the progress bar
    threadTrainCompletionTotal =
      ((float)iDepth + 1.0) / ((float)threadTrainDepth);
    AddThreadTrainPodProg(
      appProgTrainTotal,
      threadTrainCompletionTotal);

    threadTrainCompletionDepth = 1.0;
    AddThreadTrainPodProg(
      appProgTrainDepth,
      threadTrainCompletionDepth);

    // Update the ETC
    const char* etc =
      ETCGet(
        appTrainETCTotal,
        threadTrainCompletionTotal);
    AddThreadTrainPodLbl(
      appLblETCTotal,
      etc);

    // Update the message for the 'total' section
    sprintf(
      msg,
      "Current best value: %f.\n",
      threadTrainCurBestVal);
    AddThreadTrainPodTxt(
      appTextBoxTrainMsgTotal,
      msg);

    DisplayTrainedTopology(
      appTextBoxTrainMsgTotal,
      &threadTrainBestTopo);

  }

  // Make sure the progress bar are filled as we may have skipped some
  // depth
  threadTrainCompletionTotal = 1.0;
  AddThreadTrainPodProg(
    appProgTrainTotal,
    threadTrainCompletionTotal);
  AddThreadTrainPodProg(
    appProgTrainDepth,
    threadTrainCompletionTotal);
  const char* etc =
    ETCGet(
      appTrainETCTotal,
      1.0);
  AddThreadTrainPodLbl(
    appLblETCTotal,
    etc);
  AddThreadTrainPodLbl(
    appLblETCDepth,
    etc);

  // Display a last message
  const gchar* elapsed = ETCGetElapsed(appTrainETCTotal);
  sprintf(
    msg,
    "Training has ended in %s.\n",
    elapsed);
  AddThreadTrainPodTxt(
    appTextBoxTrainMsgTotal,
    msg);

  // Send a last time the signal to process the remaining results of
  // training
  gdk_threads_add_idle(
    ProcessThreadWorkerTrain,
    NULL);

  // Send the signal to indicate the termination of training
  gdk_threads_add_idle(
    EndThreadWorkerTrain,
    NULL);

  return NULL;

}
