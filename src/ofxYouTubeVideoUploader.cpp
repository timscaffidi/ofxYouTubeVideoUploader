//
//  ofxYouTubeVideoUploader.cpp
//  videoUploader
//
//  Created by Timothy Scaffidi on 11/27/12. for Helios Interactive
//
//
#include "ofxYouTubeVideoUploader.h"

ofxYouTubeVideoUploader::ofxYouTubeVideoUploader(){
    curl.setup();
    authInfo.bIsAuthorized = false;
    bIsUploading = false;
    lastPollTime = 0;
}
ofxYouTubeVideoUploader::~ofxYouTubeVideoUploader(){
    authInfo.getJSONElement().save(authInfo.jsonFile,true);
}


void ofxYouTubeVideoUploader::setup(string settingsFileName){
    ofxJSONElement json;
    json.openLocal(settingsFileName);
    authInfo.setFromJSON(json);
    authInfo.jsonFile = settingsFileName;
    setup();
}

void ofxYouTubeVideoUploader::setup(ofxYouTubeOAuthInfo info){
    authInfo = info;
    setup();
}

void ofxYouTubeVideoUploader::setup(){
    //check that required fields are populated
    bool requiredFields = true;
    requiredFields &= !authInfo.dev_key.empty();
    requiredFields &= !authInfo.client_id.empty();
    requiredFields &= !authInfo.client_secret.empty();
    requiredFields &= !authInfo.scope.empty();
    requiredFields &= !authInfo.grant_type.empty();
    
    if(!requiredFields) {
        ofLogError() << "ofxYouTubeVideoUploader::setup(): error: missing required oAuth fields\n"
        << "dev_key: " << authInfo.dev_key << endl
        << "client_id: " << authInfo.client_id << endl
        << "client_secret: " << authInfo.client_secret << endl
        << "scope: " << authInfo.scope << endl
        << "grant_type: " << authInfo.grant_type << endl;
        return;
    }
    
    //try to refresh the existing login if possible
        if(!authInfo.refresh_token.empty() && refreshAccess()){
            ofLogNotice() << "ofxYouTubeVideoUploader::setup() was able to refresh access successfully.\n";
            startThread(false,false);
        }
        else {
            ofLogNotice() << "ofxYouTubeVideoUploader::setup() was not able to refresh access. Requesting a new authorization.\n";
            requestAccess();
            startThread(false,false);
        }
}

void ofxYouTubeVideoUploader::threadedFunction() {
    if(isThreadRunning()){
        while(authInfo.bIsAuthorized){
            // loop for refreshing access periodically
            if(authInfo.access_time + authInfo.expires_in*1000 < ofGetElapsedTimeMillis() + 10000){
                //access_code about to expire, refresh it.
                if(refreshAccess()){
                    ofLogNotice() << "ofxYouTubeVideoUploader::threadedFunction() was able to refresh access successfully.\n";
                }
                else {
                    ofLogWarning() << "ofxYouTubeVideoUploader::threadedFunction() was unable to refresh access.\n";
                    break;
                }
            }
            
            //if we need to upload
            if(bIsUploading){
                curl.perform();
                
                //TODO: check response header for "201 Created"
                
                curl.cleanup();
                bIsUploading=false;
            }
            
            ofSleepMillis(500);
        }
        while (authInfo.bIsPolling){
            // loop for polling auth server to see if user has granted access yet
            if(authInfo.access_time + authInfo.expires_in*1000 < ofGetElapsedTimeMillis()){
                //user never granted access, user_code expired
                authInfo.bIsPolling = false;
                //setup(); //restart the process? make the user (application) do it?
                break;
            }
            
            pollAccessServer();
            
            ofSleepMillis(authInfo.interval);
        }
        
    }
}

string ofxYouTubeVideoUploader::requestAccess(){
    curl.setURL("https://accounts.google.com/o/oauth2/device/code");
    curl.setOpt(CURLOPT_POST, 1);
    curl.addFormField("client_id", authInfo.client_id);
    curl.addFormField("scope", authInfo.scope);
    
    curl.perform();
    ofxJSONElement json;
    json.parse(curl.getResponseBody());
    curl.cleanup();
    
    authInfo.device_code = json.get("device_code", "").asString();
    authInfo.user_code = json.get("user_code", "").asString();
    authInfo.verification_url = json.get("verification_url", "").asString();
    authInfo.expires_in = json.get("expires_in", "0").asInt();
    authInfo.interval = json.get("interval", "0").asInt()*1000;
    authInfo.access_time = ofGetElapsedTimeMillis();
    
    if(!authInfo.verification_url.empty()){
        ofLaunchBrowser(authInfo.verification_url);
        authInfo.bIsPolling = true;
    }
    
    return authInfo.user_code;
}

bool ofxYouTubeVideoUploader::pollAccessServer(){
    bool retval = false;
    if((lastPollTime + authInfo.interval) < ofGetElapsedTimeMillis()){
        curl.setup();
        curl.setURL("https://accounts.google.com/o/oauth2/token");
        curl.setOpt(CURLOPT_POST, 1);
        curl.addFormField("client_id", authInfo.client_id);
        curl.addFormField("client_secret", authInfo.client_secret);
        curl.addFormField("code", authInfo.device_code);
        curl.addFormField("grant_type", authInfo.grant_type);
        curl.perform();
        
        ofxJSONElement json(curl.getResponseBody());
        string error = json.get("error","").asString();
        if(error.empty()){
            //no errors, should contain access_token
            authInfo.access_token = json.get("access_token","").asString();
            if(authInfo.access_token.empty()){
                ofLogError() << "ofxYouTubeVideoUploader::pollAccessServer(): no errors returned, but no access key found." << endl
                << "ORIGINAL RESPONSE:\n\n"
                << curl.getResponseBody() << endl;
            }
            else { // access_token contains a value
                authInfo.expires_in = json.get("expires_in","0").asInt();
                authInfo.access_time = ofGetElapsedTimeMillis();
                authInfo.refresh_token = json.get("refresh_token","0").asString();
                authInfo.token_type = json.get("token_type", "").asString();
                authInfo.bIsAuthorized = true;
                authInfo.bIsPolling = false;
                ofLogNotice() << "ofxYouTubeVideoUploader::pollAccessServer(): access granted!\n";
                authInfo.getJSONElement().save(authInfo.jsonFile,true);
                retval = true;
            }
        }
        else { // we have an error returned
            if(error.compare("authorization_pending") == 0){
                ofLogNotice() << "ofxYouTubeVideoUploader::pollAccessServer(): server responed with error: authorization_pending. Please enter user code "
                << authInfo.user_code << " at " << authInfo.verification_url << " to grant access.\n";
            }
            if(error.compare("slow_down") == 0){
                authInfo.interval += 250;
                ofLogNotice() << "ofxYouTubeVideoUploader::pollAccessServer(): server responed with error: slow_down, adding time to poll interval. interval: "
                << authInfo.interval << endl;
            }
        }
        
        curl.cleanup();
        lastPollTime = ofGetElapsedTimeMillis();
    }
    
    return retval;
}

bool ofxYouTubeVideoUploader::refreshAccess(){
    curl.setup();
    curl.setURL("https://accounts.google.com/o/oauth2/token");
    curl.addFormField("client_id", authInfo.client_id);
    curl.addFormField("client_secret", authInfo.client_secret);
    curl.addFormField("refresh_token", authInfo.refresh_token);
    curl.addFormField("grant_type", "refresh_token");
    
    curl.perform();
    
    ofxJSONElement json(curl.getResponseBody());
    
    authInfo.access_token = json.get("access_token","").asString();
    if(authInfo.access_token.empty()){
        ofLogNotice() << "ofxYouTubeVideoUploader::refreshAccess(): could not refresh access. server returned:\n"
        << curl.getResponseHeader() << endl
        << curl.getResponseBody() << endl;
        authInfo.bIsAuthorized = false;
    }
    else {
        authInfo.access_time = ofGetElapsedTimeMillis();
        authInfo.expires_in = 60;//json.get("expires_in","0").asInt();
        authInfo.token_type = json.get("token_type", "").asString();
        authInfo.bIsAuthorized = true;
        authInfo.getJSONElement().save(authInfo.jsonFile,true);
    }
    return authInfo.bIsAuthorized;
}

void ofxYouTubeVideoUploader::uploadVideoFile(string path, string fileName){
    
    curl.setup();
    curl.setURL("http://uploads.gdata.youtube.com/resumable/feeds/api/users/default/uploads");
    curl.setOpt(CURLOPT_POST, 1);
    curl.addHeader("Authorization: " + authInfo.token_type + " " + authInfo.access_token);
    curl.addHeader("GData-Version: 2");
    curl.addHeader("X-GData-Key: key=" + authInfo.dev_key);
    curl.addHeader("Content-Length: 0");
    curl.addHeader("Content-Type:");
    curl.addHeader("Slug: " + fileName);
    
    curl.perform();
    
    ofLogVerbose() << "-----CURL RESPONSE HEADER-----\n" << curl.getResponseHeader() << endl
    << "-----CURL RESPONSE BODY-----\n" << curl.getResponseBody() << endl;
    
    //find upload url in response header
    string header = curl.getResponseHeader();
    curl.cleanup();
    size_t loc_start = header.find("http://");
    size_t loc_end = header.find("Date: ");
    if(loc_start!=string::npos && loc_end!=string::npos) {
        //location found
        string uploadUrl = header.substr(loc_start, loc_end-loc_start-1);
        ofLogNotice() << "ofxYouTubeVideoUploader::uploadVideoFile(): response url found:\n"
        << uploadUrl << endl << "starting upload...\n";
        curl.setup();
        curl.setURL(uploadUrl);
        curl.setUploadFile(path);
        //moved to threaded function
//        curl.perform();
//        curl.cleanup();
        bIsUploading = true;
        if(isThreadRunning()){
            
            startThread(false,false);
        }
    }
    else {
        ofLogError() << "ofxYouTubeVideoUploader::uploadVideoFile(): could not extract upload URL from server response\n";
    }
    
}
