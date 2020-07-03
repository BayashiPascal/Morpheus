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

  // Loop on the samples
  GDSReset(
    appDataset,
    threadEvalCat);
  bool flagStep = TRUE;
  long iSample = 0;
  long nbSamples =
    GDSGetSizeCat(
      appDataset,
      threadEvalCat);
  do {

    // Allocate memory for the result
    ThreadEvalResult* evalResult =
      PBErrMalloc(
        AppErr,
        sizeof(ThreadEvalResult));
    evalResult->result = VecFloatCreate(threadEvalNbOutput);
    evalResult->iSample = iSample;

    // Get the sample
    evalResult->sample =
      GDSGetSample(
        appDataset,
        threadEvalCat);

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
    flagStep =
      GDSStepSample(
        appDataset,
        threadEvalCat);

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

    // Save the result vector for later analyis
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

  // Create the NeuraNet to train the new topology
  long nbMaxHidden = depth * threadTrainNbOut;
  long nbMaxBases = 1 + iLink;
  long nbMaxLinks = nbMaxBases;
  NeuraNet* nn =
    NeuraNetCreate(
      threadTrainNbIn,
      threadTrainNbOut,
      nbMaxHidden,
      nbMaxBases,
      nbMaxLinks);

  // If there is a current best topology
  if (threadTrainBestTopo.links != NULL) {

    // Copy the best topo
    /*for (
      long i = VecGetDim(threadTrainBestTopo.links);
      i--;) {

      long val =
        VecGet(
          threadTrainBestTopo.links,
          i);
      VecSet(
        nn->_links,
        i,
        val);

    }

    for (
      long i = VecGetDim(threadTrainBestTopo.bases);
      i--;) {

      long val =
        VecGet(
          threadTrainBestTopo.bases,
          i);
      VecSet(
        nn->_bases,
        i,
        val);

    } */

    // TODO Add the new link
    /*VecSet(
      nn->_links,
      VecGetDim(threadTrainBestTopo.links),
      iLink);
    VecSet(
      nn->_links,
      VecGetDim(threadTrainBestTopo.links) + 1,
      iIn);
    VecSet(
      nn->_links,
      VecGetDim(threadTrainBestTopo.links) + 2,
      iOut); */

  // Else, there is no current best topo
  } else {

    // Add the new link
    /*VecSet(
      nn->_links,
      0,
      iLink);
    VecSet(
      nn->_links,
      1,
      iIn);
    VecSet(
      nn->_links,
      2,
      iOut); */

  }

  // TODO set the mutable link

  // Return the NeuraNet
  return nn;

}

// Thread worker for the training of one NeuraNet
// data's type is NeuraNet*
gpointer ThreadWorkerGenAlg(gpointer data) {

  // Convert the argument
  NeuraNetPod* pod = data;
  NeuraNet* nn = pod->nn;
  float* nnVal = &(pod->val);

  // Train the NeuraNet
  sleep(1);
  (void)nn;
  *nnVal = rnd() * -100.0;

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
    iLink < VecGetDim(topo->bases);
    iLink += 3) {

    long in =
      VecGet(
        topo->bases,
        iLink + 1);
    long out =
      VecGet(
        topo->bases,
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
      "Create the topologies for depth #%d.\n",
      iDepth);
    AddThreadTrainPodTxt(
      appTextBoxTrainMsgTotal,
      msg);

    // TODO Loop on the id of new link at the current depth
    for (
      int iLink = 0;
      iLink < threadTrainNbIn * threadTrainNbOut;
      ++iLink) {

      // TODO Loop on the source of the new link
      for (
        int iIn = 0;
        iIn < threadTrainNbIn + iDepth * threadTrainNbOut;
        ++iIn) {

        // Loop on the destination of the new link
        for (
          int iOut = 0;
          iOut < threadTrainNbOut;
          ++iOut) {

          // TODO Create the NeuraNet to train the new topology
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

    }

    // Send the signal to process the result of training
    gdk_threads_add_idle(
      ProcessThreadWorkerTrain,
      NULL);

    // Memorize the total number of topologies to train
    long nbTopoToTrain = GSetNbElem(threadTrainNNToBeTrained);

    // Send a message to the user
    sprintf(
      msg,
      "Max number of topologies to train: %ld.\n",
      nbTopoToTrain);
    AddThreadTrainPodTxt(
      appTextBoxTrainMsgTotal,
      msg);

    // While there are topologies to train
    while (GSetNbElem(threadTrainNNToBeTrained) > 0) {

      // Flush the trained topologies
      while (GSetNbElem(threadTrainNNTrained) > 0) {

        g_mutex_lock(&appMutex);
        NeuraNetPod* pod = GSetPop(threadTrainNNTrained);
        g_mutex_unlock(&appMutex);

        // Update the best topology if it's worst than this trained topo
        if (pod->val > threadTrainCurBestVal) {

          UpdateBestTopo(pod);

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

        // Send a message to the user
        sprintf(
          msg,
          "Interrupted by user.\n");
        AddThreadTrainPodTxt(
          appTextBoxTrainMsgTotal,
          msg);

      }

      if (flagSkip == true) {

        // Flush the remaining topologies
        g_mutex_lock(&appMutex);
        while (GSetNbElem(threadTrainNNToBeTrained) > 0) {

          NeuraNetPod* pod = GSetPop(threadTrainNNToBeTrained);
          NeuraNetFree(&(pod->nn));
          free(pod);
          --nbTopoToTrain;

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
      threadTrainCompletionDepth = 1.0 -
        ((float)GSetNbElem(threadTrainNNToBeTrained) +
        (float)GSetNbElem(threadTrainNNUnderTraining)) /
        ((float)nbTopoToTrain);
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

      // Update the progress bar for the 'depth' section
      threadTrainCompletionDepth = 1.0 -
        ((float)GSetNbElem(threadTrainNNToBeTrained) +
        (float)GSetNbElem(threadTrainNNUnderTraining)) /
        ((float)nbTopoToTrain);
      AddThreadTrainPodProg(
        appProgTrainDepth,
        threadTrainCompletionDepth);

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
