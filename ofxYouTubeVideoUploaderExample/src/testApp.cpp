#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    ytUploader.setup("oAuthSettings.json");
    ofSetBackgroundColor(0, 0, 0);
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){
    
    if (ytUploader.isWaitingForAuthorization()) {
        stringstream ss;
        ss<< "Waiting for user to authorize YouTube Access.\n"
        << "Enter the code: " << ytUploader.getUserCode() << endl
        << "at the following web address:\n" << ytUploader.getVerificationUrl();
        ofDrawBitmapString(ss.str(), 20,20);
    }
    else if(ytUploader.isAuthorized()){
        stringstream ss;
        ofxYouTubeOAuthInfo info = ytUploader.getAuthInfo();
        ss<< "Authorized!\n"
        << "expires in: " << ((info.access_time + info.expires_in*1000) - ofGetElapsedTimeMillis())/1000.0 << " seconds\n"
        << (ytUploader.isUploading()?"uploading...":"press any key to upload a video.");
        ofDrawBitmapString(ss.str(), 20,20);
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    ofFileDialogResult fileDialogResult = ofSystemLoadDialog("select a video to upload", false, ofFilePath::getAbsolutePath(""));
    if(fileDialogResult.bSuccess){
        ofLog() << "raw path:\n" << fileDialogResult.getPath() << fileDialogResult.getName() << endl
        << "abs path:\n" << ofFilePath::getAbsolutePath(fileDialogResult.getPath() + fileDialogResult.getName()) << endl;
        
        ytUploader.uploadVideoFile(fileDialogResult.getPath() , fileDialogResult.getName());
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}