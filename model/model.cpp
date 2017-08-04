//
// Guttemberg Machado on 24/07/17.
//
//      -----------------------+------------------------------+
//      |      TRAINING:       |        CLASSIFYING           |
//      +----------------------+------------------------------+
//      | List files           | List file or folder          |
//      +----------------------+------------------------------+
//      | Iterate Files        |                              |
//      |    Extract samples   |                              |
//      +----------------------+------------------------------+
//      | Iterate samples      | Load dictionary from file    |
//      |    Extract features  |                              |
//      | Create Dictionary    |                              |
//      +----------------------+------------------------------+
//      | Iterate Samples      | Load labels from file        |
//      |    Create Labels     |                              |
//      +----------------------+------------------------------+
//      | Iterate Samples      | Load training set from file  |
//      |    Extract features  |                              |
//      | Create Training Set  |                              |
//      +----------------------+------------------------------+
//      | Train                | Predict                      |
//      +----------------------+------------------------------+


#include "model.h"


bool Model::create(string sampleFolder){

	try{
        int64 startTask = getTick();
        int64 startSubtask;

        vector<string> files;

        //Is the input folder actually an existing folder?
        if(isFolder(sampleFolder)) {

            //Load all files recursivelly
            files = listFiles(sampleFolder);

            //Is there at least one image file?
            if (files.size() > 0){

                samples.clear();

                Log(log_Debug, "model.cpp", "create", "   Checking files from folder '%s'...", sampleFolder.c_str());
                startSubtask = getTick();
                for (int i = 0; i < files.size(); i++) {

                    Sample s;

                    //Loads the sample
                    if (s.load(files[i])){

                        //Check if sample is too small
                        if (s.originalMat.cols > minDimension && s.originalMat.rows > minDimension) {

                            //Check if sample needs resizing
                            if(s.originalMat.cols < maxDimension && s.originalMat.rows < maxDimension) {

                                //Creates grayscale mat of the sample
                                if(!s.create_grayscale())
                                    break;

                                //Creates monochromatic mat of the grayscale sample
                                if(!s.create_binary(binarizationMethod))
                                    break;

                                //Creates XYCut images
                                if(!s.create_XYCut())
                                    break;

                                if(saveIntermediateFiles)
                                    s.saveIntermediate(folder, false, true, false, false, false);

                                //Uses the folder as the label
                                s.label = replace(getFolderName(s.filename), sampleFolder, "");
                                samples.push_back(s);
                                Log(log_Detail, "model.cpp", "create", "         File '%s' was loaded as sample %i...", files[i].c_str(), (samples.size() +1)) ;
                                s.dump();

                            }else{
                                Log(log_Error, "model.cpp", "create", "         File '%s' was ignored because it is larger than %i pixels!!", files[i].c_str(), maxDimension);
                            }
                        }else{
                            Log(log_Error, "model.cpp", "create", "         File '%s' was ignored because it is smaller than %i pixels!", files[i].c_str(), minDimension);
                        }
                    }
                }
                Log(log_Debug, "model.cpp", "create", "      Done. Loaded %i samples out of %i files in %s seconds.", samples.size(), files.size(), getDifString(startSubtask).c_str());

                //Did we manage to load any file at all?
                if(samples.size() > 0) {

                    if(createDictionary()){

                        if(prepareTrainingSet()){

                            startSubtask = getTick();

                            Log(log_Debug, "model.cpp", "saveModel", "   Preparing labels...");
                            vector<String>	labels;
                            for (int i = 0; i < samples.size(); i++)
                            {
                                //Verifica se já adicionamos esse Label
                                if (find(labels.begin(), labels.end(), samples[i].label) == labels.end())
                                    labels.push_back(samples[i].label.c_str());
                            }

                            for (int i = 0; i < labels.size(); i++)
                                Log(log_Debug, "model.cpp", "create", "      Label %i is '%s'.", (i+1), labels[i].c_str());

                            for (int i = 0; i < samples.size(); i++)
                            {
                                int label_index = find(labels.begin(), labels.end(), samples[i].label) - labels.begin();
                                trainingLabel.push_back((float)label_index);
                            }

                            Log(log_Debug, "model.cpp", "create", "      Done. Preparing the labels took %s seconds.", getDifString(startSubtask).c_str());

                            startSubtask = getTick();

                            Log(log_Error, "model.cpp", "create", "   Training the SVM...");
                            bool res = svm->train(trainingData, ROW_SAMPLE, trainingLabel);
                            if(res)
                                Log(log_Debug, "model.cpp", "create", "      Done. Training took %s seconds.", getDifString(startSubtask).c_str());
                            else
                                Log(log_Debug, "model.cpp", "create", "      Done. Training failed after %s seconds.", getDifString(startSubtask).c_str());

                            Log(log_Error, "model.cpp", "create", "   Done creating model after %s seconds.", getDifString(startTask).c_str());
                            return res;
                        }
                    }

                }else
                    Log(log_Debug, "model.cpp", "create", "   No samples found.");
            }else
                Log(log_Error, "model.cpp", "create", "   Failed to find sample folder '%s'", sampleFolder.c_str() );
        }else
            Log(log_Error, "model.cpp", "create",  "   Sample folder contains no files.");
    }catch(const std::exception& e){
        Log(log_Error, "model.cpp", "create",  "   Error creating model: %s", e.what()) ;
    }

    return false;

}

bool Model::load(){

    try{

        if (isFile(filename)){
            Log(log_Debug, "model.cpp", "load", "   Loading model file...");

            //Reads the model
            //svm.load(filename.c_str());
            string trainingSetFile = filename + "_trainingSetFile.xml";

            if (isFile(trainingSetFile)){

                FileStorage fs(trainingSetFile, FileStorage::READ);
                fs["dictionary"] >> dictionary;
                //fs["labels"] >> labels;
                fs.release();

                Log(log_Debug, "model.cpp", "loadDictionary", "      Loading existing dictionary...");


                //FileStorage fs(trainingSetFile, FileStorage::READ);
                //f//s["dictionary"] >> dictionary;
                //fs["trainingData"] >> trainingData;
                //fs["labels"] >> labels;
                //fs["trainingLabel"] >> trainingLabel;
                //fs.release();

                //Lê o modelo
                //svm.load(filename.c_str());


                Log(log_Debug, "model.cpp", "load", "      TODO: (NOT IMPLEMENTED): Model was loaded.");
                return true;

            }else{
                Log(log_Error, "model.cpp", "loadModel", "      Dictionary file was not found.");
            }
        }
        else{
            Log(log_Error, "model.cpp", "load", "      Model file was not found.");
        }

    }catch(const std::exception& e){
        Log(log_Error, "model.cpp", "load", "      Failed to load model: %s", e.what() ) ;
    }

    return false;
}

bool Model::save(){

    int64 startTask = getTick();

	Log(log_Debug, "model.cpp", "save", "   Saving model file...");

	try{

        Log(log_Debug, "model.cpp", "save", "      Saving dictionary on file '" + filename + "'...");
        startTask = getTick();

        //Saves
        FileStorage fs(filename, FileStorage::WRITE);
        fs << "dictionary" << dictionary;
        fs << "trainingData" << trainingData;

        fs.release();

        //svm.save(filename.c_str());

        Log(log_Debug, "model.cpp", "save", "         Done saving dictionary file in %s seconds.",  getDifString(startTask).c_str());

		return true;

	}catch(const std::exception& e){
		Log(log_Error, "model.cpp", "save", "      Failed to save model: %s", e.what() ) ;
	}

	return false;
}

bool Model::initialize(){

    try{

        //TODO: Check what is the optimized size of a dictionary (based on samples, number of labels, etc)
        dictionarySize = 1500;

        //TODO: ALL THE 'initialize functions' from OPENCV changed from version 2.x to 3.x.  CHECK THIS CAREFULLY


        Log(log_Error, "model.cpp", "initialize", "      Initializing detector module: '" + getFeatureName() + "'...");
        switch (featureType)
        {
            case feature_SIFT: {
                detector =  SiftFeatureDetector::create();      //or makePtr<SiftFeatureDetector>();       //it was (on opencv 2.x): = new SiftFeatureDetector();
                extractor = SiftDescriptorExtractor::create();  //or makePtr<SiftDescriptorExtractor>()    //it was (on opencv 2.x): = new SiftDescriptorExtractor();
                Log(log_Error, "model.cpp", "initialize", "         Done.");

                break;
            }
            case feature_SURF:
            case feature_ORB:
            case feature_LBP:
            case feature_FAST:
            case feature_BRIEF:
            case feature_START:
            case feature_MSER:
            case feature_GFTT:
            case feature_HARRIS:
            case feature_DENSE:
            case feature_BLOB:
                Log(log_Error, "model.cpp", "initialize", "         FEATURE '" + getFeatureName() + "' NOT IMPLEMENTED.");
                return false;
        }

        Log(log_Error, "model.cpp", "initialize", "      Initializing matcher module: '" + getMatcherName() + "'...");
        switch (matcherModel)
        {
            case matcher_FLANN: {
                //TODO: Isso aqui mudou! Alterar
                matcher = new FlannBasedMatcher();
                Log(log_Error, "model.cpp", "initialize", "         Done.");
                break;
            }
            case matcher_K_MEANS_CLUSTERING:
            case matcher_BRUTE_FORCE:
                //matcher = new BFMatcher::create("BruteForce");
                Log(log_Error, "model.cpp", "initialize", "         MATCHER '" + getMatcherName() + "' NOT IMPLEMENTED.");
                return false;
        }

        Log(log_Error, "model.cpp", "initialize", "      Initializing trainer module: '" + getModelName() + "'...");
        switch (classificationModel)
        {
            case model_BAG_OF_FEATURES:{
                Log(log_Error, "model.cpp", "initialize", "         Creating trainner...");
                trainer = new BOWKMeansTrainer(dictionarySize,  TermCriteria(CV_TERMCRIT_ITER, 10, 0.001), 1, KMEANS_PP_CENTERS);
                break;
            }
            case model_PROJECTION:{
                Log(log_Error, "model.cpp", "initialize", "         MODEL '" + getModelName() + "' NOT IMPLEMENTED.");
                return false;
            }

        }

        Log(log_Error, "model.cpp", "create", "   Creating SVM (Suport Vector Machine)...");
        svm = SVM::create();

        Log(log_Error, "model.cpp", "create", "   Configuring SVM params...");
        svm->setKernel(SVM::RBF);
        svm->setType(SVM::C_SVC);
        svm->setGamma(0.50625000000000009);
        svm->setC(312.50000000000000);
        svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER, 100, 0.000001));

        return true;

    }catch(const std::exception& e){
        Log(log_Error, "model.cpp", "create",  "   Error creating model: %s", e.what()) ;
    }

}

bool Model::createDictionary() {

    int64 startTask = getTick();
    int64 startSubtask;

    vector<KeyPoint> keypoints;
    Mat descriptor;

    startSubtask= getTick();

    Log(log_Debug, "model.cpp", "createDictionary", "      Creating a new dictionary based on the %i samples found...", samples.size());
    for (int i = 0; i < samples.size(); i++) {

        Log(log_Debug, "model.cpp", "createDictionary", "         Processing sample %05d...", (i+1));

        Mat m = samples[i].grayMat;

        //Check if the original mat exists
        if (isMatValid(m)) {

            Log(log_Detail, "model.cpp", "createDictionary", "            Extracting features...");
            detector->detect(m, keypoints);

            //Get the descriptors for each keypoint
            Log(log_Detail, "model.cpp", "createDictionary", "            Computing descriptors from features...");
            extractor->compute(m, keypoints, descriptor);

            //Adds the descriptor to the trainer
            Log(log_Detail, "model.cpp", "createDictionary", "            Saving descriptors...");
            if (!descriptor.empty())
                trainer->add(descriptor);

        }
    }
    Log(log_Debug, "model.cpp", "createDictionary", "         Done. Processing all %i samples took %s seconds.", samples.size(), getDifString(startSubtask).c_str());

    //Did processing the samples find anything usefull?
    if (trainer->descriptorsCount() > 0){

        Log(log_Debug, "model.cpp", "createDictionary", "      Clustering (choosing centroids as words) features out of all %i descriptors found...", trainer->descriptorsCount());
        startSubtask = getTick();

        //Cluster (use the centroid of each cluster as the words of the dictionary)
        dictionary = trainer->cluster();

        Log(log_Debug, "model.cpp", "createDictionary", "         Done. Clustering took %s seconds.", getDifString(startSubtask).c_str());

        if(dictionary.dims > 0){
            Log(log_Debug, "model.cpp", "createDictionary", "      Done. Creating dictionary took %s seconds.", getDifString(startTask).c_str());
            return true;
        }else
            Log(log_Debug, "model.cpp", "createDictionary", "      ERROR: Cluster failed to create a dictionary!");
    }else
        Log(log_Error, "model.cpp", "createDictionary", "         ERROR: Failed to cluster because no descriptors were found!");

    return false;

}

bool Model::prepareTrainingSet() {

    int64 startTask = getTick();

    Log(log_Debug, "model.cpp", "prepareTrainingSet", "      Preparing training data from all %i samples...", samples.size());

    Log(log_Error, "model.cpp", "prepareTrainingSet", "         Creating bag of words extractor...");
    BOWImgDescriptorExtractor bow(extractor, matcher);

    Log(log_Error, "model.cpp", "prepareTrainingSet", "         Setting vocabulary...");
    bow.setVocabulary(dictionary);

    vector<KeyPoint> keypoints;
    Mat descriptor;

    for (int i = 0; i < samples.size(); i++) {

        Log(log_Debug, "model.cpp", "prepareTrainingSet", "         Preparing sample %05d...", (i+1));

        //Get the keypoints
        Log(log_Detail, "model.cpp", "prepareTrainingSet", "            Extracting features (key points)...");
        detector->detect(samples[i].binaryMat, keypoints);

        Log(log_Detail, "model.cpp", "prepareTrainingSet", "            Computing descriptors from features (key points)...");
        bow.compute(samples[i].binaryMat, keypoints, descriptor);

        //Adds the descriptor to the trainning array
        Log(log_Detail, "model.cpp", "prepareTrainingSet", "            Adding descriptors to training data...");
        trainingData.push_back(descriptor);

        Log(log_Detail, "model.cpp", "prepareTrainingSet", "            Adding label '%s' to training data...", samples[i].label);
        //trainingLabel.push_back(samples[i].label);

    }

    Log(log_Debug, "model.cpp", "prepareTrainingSet", "         Done. Preparing training data took %s seconds.",  getDifString(startTask).c_str());
    return isMatValid(trainingData);

}

bool Model::classify(string path){

    int64 startTask = getTick();
    int64 startSubtask;

    vector<KeyPoint> keypoints;
    Mat m;
    Mat	descriptor;
    string sRet;

    Log(log_Debug, "model.cpp", "classify", "   Classifing path '%s'...", path.c_str());

    startSubtask = getTick();
    Log(log_Debug, "model.cpp", "classify", "      Extracting keypoints...");
    detector->detect(m, keypoints);
    Log(log_Debug, "model.cpp", "classify", "         Done in %s seconds.",  getDifString(startSubtask).c_str());

    startSubtask = getTick();
    Log(log_Debug, "model.cpp", "classify", "      Creating BOW exctractor...");
    BOWImgDescriptorExtractor bow(extractor, matcher);
    Log(log_Debug, "model.cpp", "classify", "         Done in %s seconds.",  getDifString(startSubtask).c_str());

    startSubtask = getTick();
    Log(log_Debug, "model.cpp", "classify", "      Setting vocabulary...");
    bow.setVocabulary(dictionary);
    Log(log_Debug, "model.cpp", "classify", "         Done in %s seconds.",  getDifString(startSubtask).c_str());

    startSubtask = getTick();
    Log(log_Debug, "model.cpp", "classify", "      Extracting descriptors from keypoints...");
    bow.compute(m, keypoints, descriptor);
    Log(log_Debug, "model.cpp", "classify", "         Done in %s seconds.",  getDifString(startSubtask).c_str());

    startSubtask = getTick();
    Log(log_Debug, "model.cpp", "classify", "      Predicting from file's descriptors...");
    float response = svm->predict(descriptor);
    Log(log_Debug, "model.cpp", "classify", "         Done in %s seconds.",  getDifString(startSubtask).c_str());

    if (response >= 0){
       Log(log_Debug, "model.cpp", "classify", "      Predict returned " + labels[response] + " (" + to_string(response) + ")!");
       sRet = labels[response];
    }
    else{
       Log(log_Debug, "model.cpp", "classify", "      Predict returned UNKNOWN!");
       sRet = "UNKNOWN";
    }

    Log(log_Debug, "model.cpp", "classify", "         Done in %s seconds.",  getDifString(startTask).c_str());

}

string Model::getModelName(){
    switch (classificationModel)
    {
        case model_PROJECTION:	    return "PROJECTION";
        case model_BAG_OF_FEATURES:	return "BAG OF FEATURES";
        default:				    return "UNKNOWN";
    }
};

string Model::getFeatureName(){
    switch (featureType)
    {
        case feature_SIFT:	return "SIFT (Scale Invariant Feature Transform)";
        case feature_SURF:	return "SURF (Speeded Up Robust Features)";
        case feature_LBP:	return "LBP (Local Binary Patters)";
        case feature_FAST:	return "FAST (Feature From Accelerated Segment Tests)";
        case feature_BRIEF:	return "BRIEF (Binary Robust Independent Elementary Features)";
        case feature_ORB:	return "ORB (Oriented Fast & Rotated BRIEF)";
        default:			return "UNKNOWN";
    }
}

string Model::getMatcherName(){
    switch (matcherModel)
    {
        case matcher_BRUTE_FORCE:		    return "BRUTE FORCE";
        case matcher_FLANN:                 return "FLANN (Fast Library for Approximating Nearest Neighbors)";
        case matcher_K_MEANS_CLUSTERING:	return "K MEANS CLUSTERING";
        default:				            return "UNKNOWN";
    }
}

string Model::getBinarizationName(){
    switch (binarizationMethod){
        case binarization_TRESHOLD:	  return "TRESHOLD (Byte values bigger than thresold are white, lowe is black. Threshold is 127)";
        case binarization_MEAN:	      return "MEAN (treshold is the mean of neighbourhood area)";
        case binarization_GAUSSIAN:	  return "GAUSSIAN (threshold is weighted sum of neighbourhood values where weights are a gaussian window";
        case binarization_NIBLACK:	  return "NIBLACK (Wayne Niblack)";
        case binarization_SAUVOLA:    return "SAUVOLA (Jaakko J. Sauvola)";
        case binarization_WOLFJOLION: return "WOLFJOLION (Christian Wolf, Jean-Michel Jolion and Francoise Chassaing)";
        case binarization_BRADLEY:	  return "BRADLEY (Derek Bradley)";
        case binarization_CLAHE:	  return "CLAHE (Contrast Limited Adaptive Histogram Equalization)";
        default:				      return "UNKNOWN";
    }
}
