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

  // Process the data from the thread worker
  GtkTextBuffer* txtBuffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(appTextBoxEval));

  while (GSetNbElem(appEvalResults) > 0) {

    // Pop out the result for one sample
    ThreadEvalResult* evalResult = GSetPop(appEvalResults);

    // Variable to calculate the difference with the correct output
    float diff = 0.0;

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

      float valOut =
        VecGet(
          evalResult->sample,
          threadEvalNbInput + iCol);
      diff +=
        pow(
          val - valOut,
          2.0);

    }

    diff = sqrt(diff);
    sprintf(
      buffer,
      "| %+08.3f ",
      diff);
    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      buffer,
      strlen(buffer));

    gtk_text_buffer_insert_at_cursor(
      txtBuffer,
      "\n",
      1);

    // Free the memory used by the result
    free(evalResult->result);
    free(evalResult);

  }

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
