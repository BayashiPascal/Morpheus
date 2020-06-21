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
      processThreadWorkerEval,
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
    processThreadWorkerEval,
    NULL);

  // Send end signal to the main thread
  g_idle_add(
    endThreadWorkerEval,
    NULL);

  // Free memory
  free(vecIn);

  return NULL;

}

// Function to process the data from the thread worker for evaluation
gboolean processThreadWorkerEval(gpointer data) {

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
gboolean endThreadWorkerEval(gpointer data) {

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
  /*if (threadTrainBestTopo.links != NULL) {

    // Copy the best topo
    for (
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

    }

    // Add the new link
    VecSet(
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
      iOut);

  // Else, there is no current best topo
  } else {

    // Add the new link
    VecSet(
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
      iOut);

  }*/

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
  *nnVal = rand() * 100.0;

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

// Update the best topology with the result of the training of one NeuraNet
void UpdateBestTopo(const NeuraNetPod* const pod) {

  // Lock the mutex
  g_mutex_lock(&appMutex);

  // Update the best value
  threadTrainCurBestVal = pod->val;

  // Update the best topology
  if (threadTrainBestTopo.links != NULL) {

    VecFree(&(threadTrainBestTopo.links));

  }
  threadTrainBestTopo.links = VecClone(pod->nn->_links);
  if (threadTrainBestTopo.bases != NULL) {

    VecFree(&(threadTrainBestTopo.bases));

  }
  threadTrainBestTopo.bases = VecClone(pod->nn->_bases);

  // Send a message to the user
  GtkTextBuffer* txtBufferDepth =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainMsgDepth));
  char msg[100];
  sprintf(
    msg,
    "Improved topology with value: %f.\n",
    pod->val);
  gtk_text_buffer_insert_at_cursor(
    txtBufferDepth,
    msg,
    strlen(msg));
  // TODO print the topology

  // Unlock the mutex
  g_mutex_unlock(&appMutex);

}

// Thread worker for the training
gpointer ThreadWorkerTrain(gpointer data) {

  (void)data;

  // Get the text buffers
  GtkTextBuffer* txtBufferTotal =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxTrainMsgTotal));
  char msg[100];

  // Loop on the depth
  for (
    int iDepth = 0;
    iDepth < threadTrainDepth &&
    threadTrainCurBestVal < threadTrainBestVal;
    ++iDepth) {

    // Send a message to the user
    sprintf(
      msg,
      "Create the topologies for depth #%d.\n",
      iDepth);
    gtk_text_buffer_insert_at_cursor(
      txtBufferTotal,
      msg,
      strlen(msg));

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

    // Memorize the total number of topologies to train
    long nbTopoToTrain = GSetNbElem(threadTrainNNToBeTrained);

    // Send a message to the user
    sprintf(
      msg,
      "Max number of topologies to train: %ld.\n",
      nbTopoToTrain);
    gtk_text_buffer_insert_at_cursor(
      txtBufferTotal,
      msg,
      strlen(msg));

    // While there are topologies to train
    while (GSetNbElem(threadTrainNNToBeTrained) > 0) {
printf("train\n");
      // Flush the trained topologies
      while (GSetNbElem(threadTrainNNTrained) > 0) {
printf("flush trained\n");
        NeuraNetPod* pod = GSetPop(threadTrainNNTrained);

        // Update the best topology if necessary
        if (pod->val > threadTrainCurBestVal) {

          UpdateBestTopo(pod);

        }

        NeuraNetFree(&(pod->nn));
        free(pod);

      }

      // If the current best is better than the requested best
      if (threadTrainCurBestVal > threadTrainBestVal) {

        // Send a message to the user
        sprintf(
          msg,
          "Best value reached. Skip the last %ld topologies.\n",
          GSetNbElem(threadTrainNNToBeTrained));
        gtk_text_buffer_insert_at_cursor(
          txtBufferTotal,
          msg,
          strlen(msg));

        // Flush the remaining topologies
        while (GSetNbElem(threadTrainNNToBeTrained) > 0) {
printf("flush remaining\n");
          NeuraNetPod* pod = GSetPop(threadTrainNNToBeTrained);
          NeuraNetFree(&(pod->nn));
          free(pod);
          --nbTopoToTrain;

        }

      }

      // While there is a thread available to train a topology
      // and there are topologies to train
      while (
        GSetNbElem(threadTrainNNUnderTraining) < threadTrainNbThread &&
        GSetNbElem(threadTrainNNToBeTrained) > 0) {
printf("add thread\n");
        // Lock the mutex
        g_mutex_lock(&appMutex);

        // TODO Create the thread to train the topology
        NeuraNetPod* pod = GSetPop(threadTrainNNToBeTrained);
        GSetAppend(
          threadTrainNNUnderTraining,
          pod);
        GThread* thread =
          g_thread_new(
            "threadGenAlg",
            ThreadWorkerGenAlg,
            (gpointer)pod);
        g_thread_unref(thread);

        // Unlock the mutex
        g_mutex_unlock(&appMutex);

      }

      // Update the progress bar for the 'depth' section
      threadTrainCompletionDepth = 1.0 -
        ((float)GSetNbElem(threadTrainNNToBeTrained) +
        (float)GSetNbElem(threadTrainNNUnderTraining)) /
        ((float)nbTopoToTrain);
      gtk_progress_bar_set_fraction(
        appProgTrainDepth,
        threadTrainCompletionDepth);

      // Slow down the main training thread
      sleep(1);

    }

    // While there are threads training topologies
    while (GSetNbElem(threadTrainNNUnderTraining) > 0) {
printf("wait last thread\n");
      // Update the progress bar for the 'depth' section
      threadTrainCompletionDepth = 1.0 -
        ((float)GSetNbElem(threadTrainNNToBeTrained) +
        (float)GSetNbElem(threadTrainNNUnderTraining)) /
        ((float)nbTopoToTrain);
      gtk_progress_bar_set_fraction(
        appProgTrainDepth,
        threadTrainCompletionDepth);

      // Wait for the child training threads to end
      sleep(1);

    }

    // Flush the trained topologies
    while (GSetNbElem(threadTrainNNTrained) > 0) {
printf("flush last trained\n");
      NeuraNetPod* pod = GSetPop(threadTrainNNTrained);

      // Update the best topology if necessary
      if (pod->val > threadTrainCurBestVal) {

        UpdateBestTopo(pod);

      }

      NeuraNetFree(&(pod->nn));
      free(pod);

    }

    // TODO Update the progress bar and message for the 'total' section
    threadTrainCompletionTotal =
      ((float)iDepth) / ((float)threadTrainDepth);
    gtk_progress_bar_set_fraction(
      appProgTrainTotal,
      threadTrainCompletionTotal);
    threadTrainCompletionDepth = 1.0;
    gtk_progress_bar_set_fraction(
      appProgTrainDepth,
      threadTrainCompletionDepth);

  }

  return NULL;

}
