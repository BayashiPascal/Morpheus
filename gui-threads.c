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

  // Unlock the button to run another evaluation
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnEvalNeuraNet),
    TRUE);
  gtk_widget_set_sensitive(
    GTK_WIDGET(appBtnEval),
    TRUE);
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
